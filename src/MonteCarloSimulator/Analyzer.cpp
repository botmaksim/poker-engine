#include "../../include/MonteCarloSimulator/Analyzer.hpp"
#include "../../include/CoreEngine/Bitboard.hpp"
#include <algorithm>
#include <random>
#include <thread>
#include <future>
#include <utility>

#include "../../include/HandEvaluator/PreflopHeuristics.hpp"
#include "../../include/HandEvaluator/PerfectHashEvaluator.hpp"

namespace PokerEngine {
namespace Engine {

static inline uint64_t toBitFlag(CardMask c) {
    return 1ULL << (((c >> 4) * 4) + (c & 0xF));
}

struct PlayerData {
    int id;
    bool hasFolded;
    CardMask initialHoles[7];
    int numInitialHoles;
    bool hasRange;
    AI::Range range;
    int targetIndex; // index in equities array
};

std::vector<double> Analyzer::calculateEquity(const GameState& state, const std::map<int, AI::Range>& opponentRanges, int iterations) {
    const auto& activePlayers = state.getPlayers();
    std::vector<double> equities(activePlayers.size(), 0.0);
    if (activePlayers.empty() || iterations <= 0) return equities;

    const auto* rules = state.getRules();
    bool usesCommunity = rules->hasCommunityCards();
    int startingSize = rules->getStartingHandSize();
    
    int targetHoleCards = usesCommunity ? startingSize : (startingSize == 3 ? 7 : 5);
    int targetBoardCards = usesCommunity ? 5 : 0;
    bool isShortDeck = (rules->getName() == "Short Deck (6+ Hold'em)");
    
    uint64_t initialDeadMask = 0;
    Hand initialBoardVector = state.getBoardCards();
    CardMask initialBoard[7];
    int numInitialBoard = initialBoardVector.size();
    for (int i=0; i<numInitialBoard; ++i) {
        initialBoard[i] = initialBoardVector[i];
        initialDeadMask |= toBitFlag(initialBoard[i]);
    }

    std::vector<PlayerData> pdata;
    for (size_t i = 0; i < activePlayers.size(); ++i) {
        const auto& p = activePlayers[i];
        PlayerData d;
        d.id = p.id;
        d.hasFolded = p.hasFolded;
        d.targetIndex = i;
        d.hasRange = false;
        d.numInitialHoles = p.faceDownCards.size();
        for(int j=0; j<d.numInitialHoles; ++j) {
            d.initialHoles[j] = p.faceDownCards[j];
        }
        
        if (!p.hasFolded) {
            auto it = opponentRanges.find(p.id);
            if (it != opponentRanges.end()) {
                d.hasRange = true;
                d.range = it->second;
            } else {
                for (int j=0; j<d.numInitialHoles; ++j) {
                    initialDeadMask |= toBitFlag(d.initialHoles[j]);
                }
            }
        }
        pdata.push_back(d);
    }
    
    int numPlayers = pdata.size();
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads <= 0) numThreads = 4;
    int itersPerThread = iterations / numThreads;
    
    auto worker = [&](int threadIters, int threadId) {
        std::vector<double> localEq(numPlayers, 0.0);
        int successfulIters = 0;
        int maxAttempts = threadIters * 50; 
        int totalAttempts = 0;

        std::random_device rd;
        std::mt19937 rng(rd() + threadId);
        
        CardMask sampledHoles[10][7];
        int numSampledHoles[10];

        CardMask masterDeck[52];
        CardMask boardCards[7];

        while (successfulIters < threadIters && totalAttempts < maxAttempts) {
            totalAttempts++;
            uint64_t iterDeadMask = initialDeadMask;
            bool validSample = true;

            for (int p = 0; p < numPlayers; ++p) {
                const auto& pd = pdata[p];
                if (pd.hasFolded) continue;
                
                if (pd.hasRange) {
                    const auto* combo = pd.range.sampleCombo(rng, iterDeadMask);
                    if (!combo) {
                        validSample = false;
                        break;
                    }
                    iterDeadMask |= combo->conflictMask;
                    numSampledHoles[p] = combo->cards.size();
                    for(int c=0; c<numSampledHoles[p]; ++c) {
                        sampledHoles[p][c] = combo->cards[c];
                    }
                } else {
                    numSampledHoles[p] = pd.numInitialHoles;
                    for(int c=0; c<pd.numInitialHoles; ++c) {
                        sampledHoles[p][c] = pd.initialHoles[c];
                    }
                }
            }

            if (!validSample) continue;

            int deckSize = 0;
            for (int r = 0; r < 13; ++r) {
                if (isShortDeck && r < 4) continue;
                for (int s = 0; s < 4; ++s) {
                    CardMask c = Bitboard::makeCard(r, s);
                    if ((iterDeadMask & toBitFlag(c)) == 0) {
                        masterDeck[deckSize++] = c;
                    }
                }
            }

            for(int i = 0; i < deckSize; i++) {
                int swapIdx = i + (rng() % (deckSize - i));
                std::swap(masterDeck[i], masterDeck[swapIdx]);
            }
            
            int drawIndex = 0;
            
            int numBoard = numInitialBoard;
            for(int i=0; i<numBoard; ++i) boardCards[i] = initialBoard[i];
            
            if (usesCommunity) {
                while (numBoard < targetBoardCards && drawIndex < deckSize) {
                    boardCards[numBoard++] = masterDeck[drawIndex++];
                }
            }
            
            for (int p = 0; p < numPlayers; ++p) {
                const auto& pd = pdata[p];
                if (pd.hasFolded) continue;
                while (numSampledHoles[p] < targetHoleCards && drawIndex < deckSize) {
                    sampledHoles[p][numSampledHoles[p]++] = masterDeck[drawIndex++];
                }
            }
            
            int bestScore = -2000000000;
            int winners[10];
            int numWinners = 0;
            
            for (int p = 0; p < numPlayers; ++p) {
                if (pdata[p].hasFolded) continue;
                
                int score = rules->evaluateHandFast(sampledHoles[p], numSampledHoles[p], boardCards, numBoard);
                if (score > bestScore) {
                    bestScore = score;
                    winners[0] = p;
                    numWinners = 1;
                } else if (score == bestScore) {
                    winners[numWinners++] = p;
                }
            }
            
            if (numWinners > 0) {
                double award = 1.0 / numWinners;
                for (int w = 0; w < numWinners; ++w) {
                    localEq[winners[w]] += award;
                }
            }
            successfulIters++;
        }
        return std::make_pair(localEq, successfulIters);
    };

    std::vector<std::future<std::pair<std::vector<double>, int>>> futures;
    for (int t = 0; t < numThreads; ++t) {
        int iters = itersPerThread + (t == 0 ? iterations % numThreads : 0);
        futures.push_back(std::async(std::launch::async, worker, iters, t));
    }
    
    int totalSuccess = 0;
    for (auto& f : futures) {
        auto res = f.get();
        for (int p = 0; p < numPlayers; ++p) {
            equities[pdata[p].targetIndex] += res.first[p];
        }
        totalSuccess += res.second;
    }

    if (totalSuccess > 0) {
        for (double& eq : equities) eq /= totalSuccess;
    }
    
    return equities;
}

} // namespace Engine
} // namespace PokerEngine
