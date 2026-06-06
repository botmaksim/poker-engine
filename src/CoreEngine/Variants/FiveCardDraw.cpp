#include "../../../include/CoreEngine/Variants/FiveCardDraw.hpp"

#include <algorithm>

#include "../../../include/HandEvaluator/EvaluatorCore.hpp"

namespace PokerEngine::Games {
    std::string FiveCardDraw::getName() const { return "5-Card Draw"; }
    int FiveCardDraw::getMaxBettingRounds() const { return 1; }
    bool FiveCardDraw::hasCommunityCards() const { return false; }

    int FiveCardDraw::getStartingHandSize() const { return 5; }

    void FiveCardDraw::postMandatoryBets(std::vector<Player>& players, int& pot,
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

    void FiveCardDraw::dealStreet(std::vector<Player>& players,
                                  Hand& boardCards, Hand& deck,
                                  int round) const {
        if (round == 0) {
            for (int i = 0; i < 5; i++) {
                for (auto& p : players) {
                    if (!p.hasFolded && !deck.empty()) {
                        p.faceDownCards.push_back(deck.back());
                        deck.pop_back();
                    }
                }
            }
        }
    }

    int FiveCardDraw::evaluateHand(const Hand& holeCards,
                                   const Hand& boardCards) const {
        Hand allCards = holeCards;
        allCards.insert(allCards.end(), boardCards.begin(), boardCards.end());
        return PokerEngine::Engine::UniversalEvaluator::evaluateHigh(allCards);
    }

    int PokerEngine::Games::FiveCardDraw::evaluateHandFast(
        const PokerEngine::CardMask* holeCards, int numHole,
        const PokerEngine::CardMask* boardCards, int numBoard) const {
        if (numHole == 0) return 0;
        PokerEngine::CardMask combined[14];
        int total = 0;
        for (int i = 0; i < numHole; ++i) combined[total++] = holeCards[i];
        for (int i = 0; i < numBoard; ++i) combined[total++] = boardCards[i];
        return PokerEngine::Engine::UniversalEvaluator::evaluateHigh(combined,
                                                                     total);
    }
}  // namespace PokerEngine::Games
