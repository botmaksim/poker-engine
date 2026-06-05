#include "../../include/Games/ShortDeck.hpp"
#include "../../include/Core/Bitboard.hpp"
#include "../../include/Engine/EvaluatorCore.hpp"
#include <algorithm>

namespace PokerEngine::Games {
std::string ShortDeck::getName() const { return "Short Deck (6+ Hold'em)"; }
int ShortDeck::getMaxBettingRounds() const { return 4; }
bool ShortDeck::hasCommunityCards() const { return true; }

int ShortDeck::getStartingHandSize() const { return 2; }

void ShortDeck::postMandatoryBets(std::vector<Player>& players, int& pot, std::vector<PlayerAction>& history, const TableConfig& config) const {
    if (players.size() < 2) return;
    int sbAmount = std::min(players[0].chipCount, config.smallBlind);
    players[0].chipCount -= sbAmount;
    players[0].currentBet += sbAmount;
    pot += sbAmount;
    if (players[0].chipCount == 0) players[0].isAllIn = true;
    history.push_back({players[0].id, ActionType::PostBlind, players[0].currentBet, sbAmount});

    int bbAmount = std::min(players[1].chipCount, config.bigBlind);
    players[1].chipCount -= bbAmount;
    players[1].currentBet += bbAmount;
    pot += bbAmount;
    if (players[1].chipCount == 0) players[1].isAllIn = true;
    history.push_back({players[1].id, ActionType::PostBlind, players[1].currentBet, bbAmount});
}

void ShortDeck::dealStreet(std::vector<Player>& players, Hand& boardCards, Hand& deck, int round) const {
    if (round == 0) {
        auto it = std::remove_if(deck.begin(), deck.end(), [](CardMask c){ 
            int rank = Bitboard::getRank(c); return rank >= 0 && rank <= 3; // Remove 2, 3, 4, 5
        });
        deck.erase(it, deck.end());
        for (int i = 0; i < 2; ++i) {
            for (auto& p : players) {
                if (!p.hasFolded && !deck.empty()) { p.faceDownCards.push_back(deck.back()); deck.pop_back(); }
            }
        }
    } else if (round == 1) { // Flop
        for (int i = 0; i < 3; ++i) { if (!deck.empty()) { boardCards.push_back(deck.back()); deck.pop_back(); } }
    } else if (round == 2 || round == 3) { // Turn, River
        if (!deck.empty()) { boardCards.push_back(deck.back()); deck.pop_back(); }
    }
}

int ShortDeck::evaluateHand(const Hand& holeCards, const Hand& boardCards) const {
    Hand allCards = holeCards;
    allCards.insert(allCards.end(), boardCards.begin(), boardCards.end());
    return Engine::UniversalEvaluator::evaluateShortDeck(allCards);
}
}
