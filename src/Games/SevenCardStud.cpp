#include "../../include/Games/SevenCardStud.hpp"
#include "../../include/Engine/EvaluatorCore.hpp"
#include <algorithm>

namespace PokerEngine::Games {
std::string SevenCardStud::getName() const { return "7-Card Stud"; }
int SevenCardStud::getMaxBettingRounds() const { return 4; } // 3rd, 4th, 5th, 6th, 7th street
bool SevenCardStud::hasCommunityCards() const { return false; }

int SevenCardStud::getStartingHandSize() const { return 3; }

void SevenCardStud::postMandatoryBets(std::vector<Player>& players, int& pot, std::vector<PlayerAction>& history, const TableConfig& config) const {
    if (players.empty()) return;
    for (auto& p : players) {
        int anteAmount = std::min(p.chipCount, config.ante);
        if (anteAmount > 0) {
            p.chipCount -= anteAmount;
            p.currentBet += anteAmount;
            pot += anteAmount;
            if (p.chipCount == 0) p.isAllIn = true;
            history.push_back({p.id, ActionType::Ante, p.currentBet, anteAmount});
        }
    }
}

void SevenCardStud::dealStreet(std::vector<Player>& players, Hand& boardCards, Hand& deck, int round) const {
    if (round == 0) { // 3rd street: 2 down, 1 up
        for(int i = 0; i < 2; i++) {
            for (auto& p : players) { if(!p.hasFolded && !deck.empty()) { p.faceDownCards.push_back(deck.back()); deck.pop_back(); }}
        }
        for (auto& p : players) { if(!p.hasFolded && !deck.empty()) { p.faceUpCards.push_back(deck.back()); deck.pop_back(); }}
    } else if (round >= 1 && round <= 3) { // 4th, 5th, 6th street: 1 up
        for (auto& p : players) { if(!p.hasFolded && !deck.empty()) { p.faceUpCards.push_back(deck.back()); deck.pop_back(); }}
    } else if (round == 4) { // 7th street: 1 down
        for (auto& p : players) { if(!p.hasFolded && !deck.empty()) { p.faceDownCards.push_back(deck.back()); deck.pop_back(); }}
    }
}

int SevenCardStud::evaluateHand(const Hand& holeCards, const Hand& boardCards) const {
    Hand allCards = holeCards;
    allCards.insert(allCards.end(), boardCards.begin(), boardCards.end());
    return Engine::UniversalEvaluator::evaluateHigh(allCards);
}
}
