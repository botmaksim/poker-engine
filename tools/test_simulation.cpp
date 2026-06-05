#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <map>
#include <iomanip>
#include <string>

#include "../include/Engine/GameState.hpp"
#include "../include/Engine/Session.hpp"
#include "../include/Engine/Analyzer.hpp"
#include "../include/Core/Bitboard.hpp"
#include "../include/AI/BotFactory.hpp"
#include "../include/AI/RangeParser.hpp"
#include "../include/AI/InferenceEngine.hpp"
#include "../include/AI/BayesianTracker.hpp"
#include "../include/Games/TexasHoldem.hpp"

using namespace PokerEngine;

std::string actionToString(ActionType type) {
    switch(type) {
        case ActionType::Fold: return "Fold";
        case ActionType::Check: return "Check";
        case ActionType::Call: return "Call";
        case ActionType::Bet: return "Bet";
        case ActionType::Raise: return "Raise";
        case ActionType::PostBlind: return "Blind";
        case ActionType::Ante: return "Ante";
        case ActionType::BringIn: return "BringIn";
        default: return "Unknown";
    }
}

std::string cardToString(CardMask c) {
    int r = Bitboard::getRank(c);
    int s = Bitboard::getSuit(c);
    const char* ranks = "23456789TJQKA";
    const char* suits = "shcd";
    return std::string(1, ranks[r]) + std::string(1, suits[s]);
}

std::string handToString(const Hand& h) {
    if (h.empty()) return "None";
    std::string res;
    for (size_t i = 0; i < h.size(); ++i) {
        res += cardToString(h[i]);
        if (i < h.size() - 1) res += " ";
    }
    return res;
}

std::string getMostLikelyProfileStr(const std::vector<double>& profile) {
    int bestIdx = 0;
    for (int i = 1; i < 49; ++i) {
        if (profile[i] > profile[bestIdx]) bestIdx = i;
    }
    int looseness = bestIdx % 7;
    int aggro = bestIdx / 7;
    return "L" + std::to_string(looseness) + "_A" + std::to_string(aggro);
}

/**
 * @brief Utility class for running bulk simulations of Poker games.
 */
class Simulator {
public:
    /**
     * @brief Runs a simulation loop for a given number of hands and stores results in CSV.
     * @param numHands The number of hands to simulate.
     * @param csvFilename Output file for tracking game states and Bayesian features.
     */
    static void runBulkSession(int numHands, const std::string& csvFilename) {
        std::ofstream csv(csvFilename);
        csv << "HandID,Round,Board,Bot1_Hole,Bot1_Equity,Bot1_Act,Bot1_Sizing,Bot2_Hole,Bot2_Equity,Bot2_Act,Bot2_Sizing,Bot2_EstProfile\n";
        
        auto game = std::make_unique<Games::TexasHoldem>();
        TableConfig config{LimitType::NoLimit, 5, 10, 0, 0, 0};
        Engine::GameState state(config, std::move(game));

        auto bot1 = AI::BotFactory::createBot(AI::ActionProfile::Balanced, AI::SizingProfile::Fixed, 42);
        auto bot2 = AI::BotFactory::createBot(AI::ActionProfile::LAG, AI::SizingProfile::Gaussian, 43);
        
        AI::BayesianTracker tracker;
        tracker.initializePlayer(1);
        tracker.initializePlayer(2);

        state.addPlayer(1, 10000);
        state.addPlayer(2, 10000);

        std::cout << "Starting Simulation for " << numHands << " games...\n";
        
        for (int i = 0; i < numHands; ++i) {
            state.startHand();
            
            AI::Range r1, r2;
            AI::RangeParser::parseHoldemRange(r1, "22+, A2s+, KTs+, QTs+, JTs, ATo+, KQo");
            AI::RangeParser::parseHoldemRange(r2, "22+, A2s+, K2s+, Q2s+, J2s+, T2s+, 92s+, 82s+, A2o+, K2o+, Q2o+, J2o+, T2o+");

            std::map<int, AI::Range> bot1View = {{2, r2}};
            std::map<int, AI::Range> bot2View = {{1, r1}};

            for (int r = 0; r < 4; ++r) {
                std::vector<double> eq1 = Engine::Analyzer::calculateEquity(state, bot1View, 1000);
                double bot1Equity = eq1[0]; // Assuming player 1 is index 0
    
                // Perform Inference for bot 1
                PlayerAction act1 = AI::InferenceEngine::decideAction(state, 1, *bot1, bot1View);
                state.recordAction(1, act1.type, act1.absoluteAmount);
                
                int potPct1 = state.getPot() > 0 ? (act1.absoluteAmount * 100) / state.getPot() : 0;
                tracker.updateFromAction(1, act1.type, potPct1);
                
                std::vector<double> eq2 = Engine::Analyzer::calculateEquity(state, bot2View, 1000);
                double bot2Equity = eq2[1]; // Assuming player 2 is index 1
    
                // Perform Inference for bot 2
                PlayerAction act2 = AI::InferenceEngine::decideAction(state, 2, *bot2, bot2View);
                state.recordAction(2, act2.type, act2.absoluteAmount);
                
                int potPct2 = state.getPot() > 0 ? (act2.absoluteAmount * 100) / state.getPot() : 0;
                tracker.updateFromAction(2, act2.type, potPct2);
    
                const auto& p2Dist = tracker.getProfile(2);
                std::string b2Prof = getMostLikelyProfileStr(p2Dist);
                
                std::string boardStr = handToString(state.getBoardCards());
                std::string p1HoleStr, p2HoleStr;
                for(const auto& p : state.getPlayers()){
                    if(p.id == 1) p1HoleStr = handToString(p.faceDownCards);
                    if(p.id == 2) p2HoleStr = handToString(p.faceDownCards);
                }
    
                csv << i << "," << r << ","
                    << boardStr << ","
                    << p1HoleStr << ","
                    << std::fixed << std::setprecision(3) << bot1Equity << "," << actionToString(act1.type) << "," << act1.absoluteAmount << "," 
                    << p2HoleStr << ","
                    << std::fixed << std::setprecision(3) << bot2Equity << "," << actionToString(act2.type) << "," << act2.absoluteAmount << "," 
                    << b2Prof << "\n";
                    
                if (act1.type == ActionType::Fold || act2.type == ActionType::Fold) {
                    break; // End hand early if fold
                }
                    
                state.advanceRound();
            }

            // Simulating hot-swapping player mid-session as requested
            if (i == numHands / 2) {
                std::cout << "[Event] Mid-session Hot Swap! Bot 2 replaced with a Nit profile.\n";
                bot2 = AI::BotFactory::createBot(AI::ActionProfile::Nit, AI::SizingProfile::Fixed, 44);
                
                // Clear and recreate Bayesian assumption for this seat
                tracker.removePlayer(2);
                tracker.initializePlayer(2);
            }
        }
        
        std::cout << "Simulation completed. Data saved to " << csvFilename << "\n";
    }
};

int main() {
    Simulator::runBulkSession(100, "session_results.csv");
    return 0;
}
