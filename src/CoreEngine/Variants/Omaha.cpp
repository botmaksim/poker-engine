#include "../../../include/CoreEngine/Variants/Omaha.hpp"

#include <algorithm>

#include "../../../include/HandEvaluator/EvaluatorCore.hpp"

namespace PokerEngine::Games {
    std::string Omaha::getName() const { return "Omaha"; }
    int Omaha::getMaxBettingRounds() const { return 4; }
    bool Omaha::hasCommunityCards() const { return true; }

    int Omaha::getStartingHandSize() const { return 4; }

    void Omaha::postMandatoryBets(std::vector<Player>& players, int& pot,
                                  std::vector<PlayerAction>& history,
                                  const TableConfig& config) const {
        if (players.size() < 2) return;
        int sbAmount = std::min(players[0].chipCount, config.smallBlind);
        players[0].chipCount -= sbAmount;
        players[0].currentBet += sbAmount;
        pot += sbAmount;
        if (players[0].chipCount == 0) players[0].isAllIn = true;
        history.push_back({players[0].id, ActionType::PostBlind,
                           players[0].currentBet, sbAmount});

        int bbAmount = std::min(players[1].chipCount, config.bigBlind);
        players[1].chipCount -= bbAmount;
        players[1].currentBet += bbAmount;
        pot += bbAmount;
        if (players[1].chipCount == 0) players[1].isAllIn = true;
        history.push_back({players[1].id, ActionType::PostBlind,
                           players[1].currentBet, bbAmount});
    }

    void Omaha::dealStreet(std::vector<Player>& players, Hand& boardCards,
                           Hand& deck, int round) const {
        if (round == 0) {
            for (int i = 0; i < 4; ++i) {
                for (auto& p : players) {
                    if (!p.hasFolded && !deck.empty()) {
                        p.faceDownCards.push_back(deck.back());
                        deck.pop_back();
                    }
                }
            }
        } else if (round == 1) {  // Flop
            for (int i = 0; i < 3; ++i) {
                if (!deck.empty()) {
                    boardCards.push_back(deck.back());
                    deck.pop_back();
                }
            }
        } else if (round == 2 || round == 3) {  // Turn, River
            if (!deck.empty()) {
                boardCards.push_back(deck.back());
                deck.pop_back();
            }
        }
    }

    int Omaha::evaluateHand(const Hand& holeCards,
                            const Hand& boardCards) const {
        int bestHigh = 0;

        // In Omaha, you MUST use exactly 2 cards from hole and exactly 3 from
        // board.
        if (holeCards.size() >= 2 && boardCards.size() >= 3) {
            for (size_t i = 0; i < holeCards.size() - 1; ++i) {
                for (size_t j = i + 1; j < holeCards.size(); ++j) {
                    for (size_t x = 0; x < boardCards.size() - 2; ++x) {
                        for (size_t y = x + 1; y < boardCards.size() - 1; ++y) {
                            for (size_t z = y + 1; z < boardCards.size(); ++z) {
                                Hand combo = {holeCards[i], holeCards[j],
                                              boardCards[x], boardCards[y],
                                              boardCards[z]};
                                int score = PokerEngine::Engine::
                                    UniversalEvaluator::evaluateHigh(combo);
                                if (score > bestHigh) {
                                    bestHigh = score;
                                }
                            }
                        }
                    }
                }
            }
        }

        return bestHigh;
    }

    int PokerEngine::Games::Omaha::evaluateHandFast(
        const PokerEngine::CardMask* holeCards, int numHole,
        const PokerEngine::CardMask* boardCards, int numBoard) const {
        if (numHole == 0) return 0;
        PokerEngine::CardMask combined[14];
        int total = 0;
        for (int i = 0; i < numHole; ++i) combined[total++] = holeCards[i];
        for (int i = 0; i < numBoard; ++i) combined[total++] = boardCards[i];

        int bestScore = 0;
        PokerEngine::CardMask combo[5];
        for (int i = 0; i < numHole - 1; ++i) {
            for (int j = i + 1; j < numHole; ++j) {
                combo[0] = holeCards[i];
                combo[1] = holeCards[j];
                for (int a = 0; a < numBoard - 2; ++a) {
                    for (int b = a + 1; b < numBoard - 1; ++b) {
                        for (int c = b + 1; c < numBoard; ++c) {
                            combo[2] = boardCards[a];
                            combo[3] = boardCards[b];
                            combo[4] = boardCards[c];
                            int score = PokerEngine::Engine::
                                UniversalEvaluator::evaluateHigh(combo, 5);
                            if (score > bestScore) bestScore = score;
                        }
                    }
                }
            }
        }
        return bestScore;
    }
}  // namespace PokerEngine::Games
