#include "../../include/Engine/Analyzer.hpp"
#include "../../include/Core/Bitboard.hpp"
#include <algorithm>
#include <random>

namespace PokerEngine {
namespace Engine {

static inline uint64_t toBitFlag(CardMask c) {
    return 1ULL << (((c >> 4) * 4) + (c & 0xF));
}

std::vector<double> Analyzer::calculateEquity(const GameState& state, const std::map<int, AI::Range>& opponentRanges, int iterations) {
    const auto& activePlayers = state.getPlayers();
    std::vector<double> equities(activePlayers.size(), 0.0);
    if (activePlayers.empty() || iterations <= 0) return equities;

    std::random_device rd;
    std::mt19937 rng(rd());
    
    const auto* rules = state.getRules();
    bool usesCommunity = rules->hasCommunityCards();
    int startingSize = rules->getStartingHandSize();
    
    int targetHoleCards = usesCommunity ? startingSize : (startingSize == 3 ? 7 : 5);
    int targetBoardCards = usesCommunity ? 5 : 0;

    bool isShortDeck = (rules->getName() == "Short Deck (6+ Hold'em)");
    
    uint64_t initialDeadMask = 0;
    Hand initialBoard = state.getBoardCards();
    for (CardMask c : initialBoard) initialDeadMask |= toBitFlag(c);

    for (const auto& player : activePlayers) {
        if (!player.hasFolded) {
            if (opponentRanges.find(player.id) == opponentRanges.end()) {
                for (CardMask c : player.faceDownCards) {
                    initialDeadMask |= toBitFlag(c);
                }
            }
        }
    }

    int successfulIters = 0;
    int maxAttempts = iterations * 50; // prevent infinite loops in impossible situations
    int totalAttempts = 0;

    while (successfulIters < iterations && totalAttempts < maxAttempts) {
        totalAttempts++;
        uint64_t iterDeadMask = initialDeadMask;
        std::map<int, Hand> sampledHoles;
        bool validSample = true;

        for (const auto& player : activePlayers) {
            if (player.hasFolded) continue;
            
            auto it = opponentRanges.find(player.id);
            if (it != opponentRanges.end()) {
                const auto& range = it->second;
                Hand sampled = range.sampleHand(rng, iterDeadMask);
                if (sampled.empty()) {
                    validSample = false;
                    break;
                }
                for (CardMask c : sampled) iterDeadMask |= toBitFlag(c);
                sampledHoles[player.id] = sampled;
            } else {
                sampledHoles[player.id] = player.faceDownCards;
            }
        }

        if (!validSample) continue;

        Hand masterDeck;
        masterDeck.reserve(52);
        for (int r = 0; r < 13; ++r) {
            if (isShortDeck && r < 4) continue;
            for (int s = 0; s < 4; ++s) {
                CardMask c = Bitboard::makeCard(r, s);
                if ((iterDeadMask & toBitFlag(c)) == 0) {
                    masterDeck.push_back(c);
                }
            }
        }

        std::shuffle(masterDeck.begin(), masterDeck.end(), rng);
        
        Hand boardCards = initialBoard;
        if (usesCommunity) {
            while (boardCards.size() < targetBoardCards && !masterDeck.empty()) {
                boardCards.push_back(masterDeck.back());
                masterDeck.pop_back();
            }
        }
        
        for (const auto& player : activePlayers) {
            if (player.hasFolded) continue;
            auto& holes = sampledHoles[player.id];
            while (holes.size() < targetHoleCards && !masterDeck.empty()) {
                holes.push_back(masterDeck.back());
                masterDeck.pop_back();
            }
        }
        
        int bestScore = -2000000000;
        std::vector<int> winners;
        
        for (size_t i = 0; i < activePlayers.size(); ++i) {
            if (activePlayers[i].hasFolded) continue;
            
            int score = rules->evaluateHand(sampledHoles[activePlayers[i].id], boardCards);
            if (score > bestScore) {
                bestScore = score;
                winners.clear();
                winners.push_back(static_cast<int>(i));
            } else if (score == bestScore) {
                winners.push_back(static_cast<int>(i));
            }
        }
        
        if (!winners.empty()) {
            double award = 1.0 / winners.size();
            for (int w : winners) {
                equities[w] += award;
            }
        }
        successfulIters++;
    }
    
    if (successfulIters > 0) {
        for (double& eq : equities) {
            eq /= successfulIters;
        }
    }
    
    return equities;
}

} // namespace Engine
} // namespace PokerEngine
