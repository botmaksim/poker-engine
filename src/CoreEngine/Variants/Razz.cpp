#include "../../../include/CoreEngine/Variants/Razz.hpp"

#include <algorithm>
#include <vector>

#include "../../../include/CoreEngine/Bitboard.hpp"
#include "../../../include/HandEvaluator/EvaluatorCore.hpp"

namespace PokerEngine::Games {
    std::string Razz::getName() const { return "Razz"; }
    int Razz::getMaxBettingRounds() const { return 4; }
    bool Razz::hasCommunityCards() const { return false; }

    int Razz::getStartingHandSize() const { return 3; }

    void Razz::postMandatoryBets(std::vector<Player>& players, int& pot,
                                 std::vector<PlayerAction>& history,
                                 const TableConfig& config) const {
        if (players.empty()) return;
        for (auto& p : players) {
            int anteAmount = std::min(p.chipCount, config.ante);
            if (anteAmount > 0) {
                p.chipCount -= anteAmount;
                p.currentBet += anteAmount;
                pot += anteAmount;
                if (p.chipCount == 0) p.isAllIn = true;
                history.push_back(
                    {p.id, ActionType::Ante, p.currentBet, anteAmount});
            }
        }
    }

    void Razz::dealStreet(std::vector<Player>& players, Hand& boardCards,
                          Hand& deck, int round) const {
        if (round == 0) {  // 3rd street: 2 down, 1 up
            for (int i = 0; i < 2; i++) {
                for (auto& p : players) {
                    if (!p.hasFolded && !deck.empty()) {
                        p.faceDownCards.push_back(deck.back());
                        deck.pop_back();
                    }
                }
            }
            for (auto& p : players) {
                if (!p.hasFolded && !deck.empty()) {
                    p.faceUpCards.push_back(deck.back());
                    deck.pop_back();
                }
            }
        } else if (round >= 1 && round <= 3) {
            for (auto& p : players) {
                if (!p.hasFolded && !deck.empty()) {
                    p.faceUpCards.push_back(deck.back());
                    deck.pop_back();
                }
            }
        } else if (round == 4) {
            for (auto& p : players) {
                if (!p.hasFolded && !deck.empty()) {
                    p.faceDownCards.push_back(deck.back());
                    deck.pop_back();
                }
            }
        }
    }

    int Razz::evaluateHand(const Hand& holeCards,
                           const Hand& boardCards) const {
        Hand allCards = holeCards;
        allCards.insert(allCards.end(), boardCards.begin(), boardCards.end());

        int bestRazzScore = -2000000000;
        int n = allCards.size();

        if (n >= 5) {
            auto evalCombo = [&](const Hand& combo) {
                int counts[14] = {0};
                for (int i = 0; i < 5; ++i) {
                    int r = Bitboard::getRank(combo[i]);
                    counts[(r == 12) ? 1 : (r + 2)]++;
                }

                std::vector<int> vals;
                for (int val = 1; val <= 13; ++val) {
                    for (int c = 0; c < counts[val]; ++c) vals.push_back(val);
                }
                std::sort(vals.begin(), vals.end(), [&](int a, int b) {
                    if (counts[a] != counts[b]) return counts[a] > counts[b];
                    return a > b;
                });

                int score = 0;
                for (int val : vals) {
                    score = (score << 4) | val;
                }
                int razzScore = -score;
                if (razzScore > bestRazzScore) {
                    bestRazzScore = razzScore;
                }
            };

            Hand combo(5, 0);

            if (n == 5) {
                for (int i = 0; i < 5; ++i) combo[i] = allCards[i];
                evalCombo(combo);
            } else if (n == 6) {
                for (int skip = 0; skip < 6; ++skip) {
                    int idx = 0;
                    for (int i = 0; i < 6; ++i) {
                        if (i != skip) combo[idx++] = allCards[i];
                    }
                    evalCombo(combo);
                }
            } else if (n == 7) {
                for (int skip1 = 0; skip1 < 6; ++skip1) {
                    for (int skip2 = skip1 + 1; skip2 < 7; ++skip2) {
                        int idx = 0;
                        for (int i = 0; i < 7; ++i) {
                            if (i != skip1 && i != skip2)
                                combo[idx++] = allCards[i];
                        }
                        evalCombo(combo);
                    }
                }
            } else {
                std::vector<bool> v(n, false);
                std::fill(v.end() - 5, v.end(), true);
                do {
                    int idx = 0;
                    for (int i = 0; i < n; ++i) {
                        if (v[i]) combo[idx++] = allCards[i];
                    }
                    evalCombo(combo);
                } while (std::next_permutation(v.begin(), v.end()));
            }
        }

        return bestRazzScore;
    }

    int PokerEngine::Games::Razz::evaluateHandFast(
        const PokerEngine::CardMask* holeCards, int numHole,
        const PokerEngine::CardMask* boardCards, int numBoard) const {
        if (numHole == 0) return 0;
        PokerEngine::CardMask combined[14];
        int total = 0;
        for (int i = 0; i < numHole; ++i) combined[total++] = holeCards[i];
        for (int i = 0; i < numBoard; ++i) combined[total++] = boardCards[i];
        return PokerEngine::Engine::UniversalEvaluator::evaluate27Low(combined,
                                                                      total);
    }
}  // namespace PokerEngine::Games
