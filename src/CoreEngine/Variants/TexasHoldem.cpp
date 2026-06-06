#include "../../../include/CoreEngine/Variants/TexasHoldem.hpp"

#include <algorithm>

#include "../../../include/HandEvaluator/EvaluatorCore.hpp"
#include "../../../include/HandEvaluator/PerfectHashEvaluator.hpp"

namespace PokerEngine {
    namespace Games {

        std::string TexasHoldem::getName() const { return "Texas Hold'em"; }

        int TexasHoldem::getMaxBettingRounds() const {
            return 4;  // Pre-flop (0), Flop (1), Turn (2), River (3), Showdown
                       // (4)
        }

        bool TexasHoldem::hasCommunityCards() const { return true; }

        int TexasHoldem::getStartingHandSize() const { return 2; }

        void TexasHoldem::postMandatoryBets(std::vector<Player>& players,
                                            int& pot,
                                            std::vector<PlayerAction>& history,
                                            const TableConfig& config) const {
            if (players.size() < 2) return;

            // Post Small Blind for player 0
            int sbAmount = std::min(players[0].chipCount, config.smallBlind);
            players[0].chipCount -= sbAmount;
            players[0].currentBet += sbAmount;
            pot += sbAmount;
            if (players[0].chipCount == 0) players[0].isAllIn = true;
            history.push_back({players[0].id, ActionType::PostBlind,
                               players[0].currentBet, sbAmount});

            // Post Big Blind for player 1
            int bbAmount = std::min(players[1].chipCount, config.bigBlind);
            players[1].chipCount -= bbAmount;
            players[1].currentBet += bbAmount;
            pot += bbAmount;
            if (players[1].chipCount == 0) players[1].isAllIn = true;
            history.push_back({players[1].id, ActionType::PostBlind,
                               players[1].currentBet, bbAmount});
        }

        void TexasHoldem::dealStreet(std::vector<Player>& players,
                                     Hand& boardCards, Hand& deck,
                                     int round) const {
            if (round ==
                0) {  // Pre-flop: deal 2 hole cards to each active player
                for (int i = 0; i < 2; ++i) {
                    for (auto& p : players) {
                        if (!p.hasFolded && !deck.empty()) {
                            p.faceDownCards.push_back(deck.back());
                            deck.pop_back();
                        }
                    }
                }
            } else if (round == 1) {  // Flop: 3 community cards
                for (int i = 0; i < 3; ++i) {
                    if (!deck.empty()) {
                        boardCards.push_back(deck.back());
                        deck.pop_back();
                    }
                }
            } else if (round == 2 ||
                       round == 3) {  // Turn and River: 1 community card each
                if (!deck.empty()) {
                    boardCards.push_back(deck.back());
                    deck.pop_back();
                }
            }
        }

        int TexasHoldem::evaluateHand(const Hand& holeCards,
                                      const Hand& boardCards) const {
            Hand allCards = holeCards;
            allCards.insert(allCards.end(), boardCards.begin(),
                            boardCards.end());
            return PokerEngine::Engine::UniversalEvaluator::evaluateHigh(
                allCards);
        }

        int PokerEngine::Games::TexasHoldem::evaluateHandFast(
            const PokerEngine::CardMask* holeCards, int numHole,
            const PokerEngine::CardMask* boardCards, int numBoard) const {
            if (numHole == 0) return 0;
            PokerEngine::CardMask combined[14];
            int total = 0;
            for (int i = 0; i < numHole; ++i) combined[total++] = holeCards[i];
            for (int i = 0; i < numBoard; ++i)
                combined[total++] = boardCards[i];
            return Engine::PerfectHashEvaluator::evaluateHighSIMD(combined,
                                                                  total);
        }

    }  // namespace Games
}  // namespace PokerEngine
