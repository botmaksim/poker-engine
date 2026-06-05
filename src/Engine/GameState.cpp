#include "../../include/Engine/GameState.hpp"
#include "../../include/Core/Bitboard.hpp"
#include <algorithm>
#include <random>

namespace PokerEngine {
namespace Engine {

GameState::GameState(const TableConfig& tableConfig, std::unique_ptr<Interfaces::IGameRules> gameRules)
    : pot(0), currentBettingRound(0), currentWager(0), lastRaiseAmount(0), raisesThisRound(0),
      config(tableConfig), rules(std::move(gameRules)) {
}

void GameState::addPlayer(int id, int startingChips) {
    players.push_back({id, startingChips, {}, {}, false, false, 0});
}

void GameState::initializeDeck() {
    deck.clear();
    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 0; rank < 13; ++rank) {
            deck.push_back(Bitboard::makeCard(rank, suit));
        }
    }
}

void GameState::shuffleDeck() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
}

void GameState::startHand() {
    pot = 0;
    currentBettingRound = 0;
    currentWager = 0;
    lastRaiseAmount = config.bigBlind;
    raisesThisRound = 0;
    bettingHistory.clear();
    bettingHistory.push_back({}); // Round 0 history
    
    for (auto& p : players) {
        p.faceDownCards.clear();
        p.faceUpCards.clear();
        p.hasFolded = false;
        p.isAllIn = (p.chipCount == 0);
        p.currentBet = 0;
    }
    boardCards.clear();

    initializeDeck();
    shuffleDeck();

    rules->postMandatoryBets(players, pot, bettingHistory[0], config);
    
    // Auto-calculate currentWager based on bets posted
    for (const auto& p : players) {
        if (p.currentBet > currentWager) currentWager = p.currentBet;
    }

    rules->dealStreet(players, boardCards, deck, currentBettingRound);
}

void GameState::advanceRound() {
    currentBettingRound++;
    if (currentBettingRound <= rules->getMaxBettingRounds()) {
        bettingHistory.push_back({});
        rules->dealStreet(players, boardCards, deck, currentBettingRound);
        for (auto& p : players) {
            p.currentBet = 0; // Reset bets for the new round
        }
        currentWager = 0;
        lastRaiseAmount = config.bigBlind;
        raisesThisRound = 0;
    }
}

void GameState::recordAction(int playerId, ActionType type, int absoluteAmount) {
    auto it = std::find_if(players.begin(), players.end(), [playerId](const Player& p) { return p.id == playerId; });
    if (it != players.end()) {
        int relativeCost = absoluteAmount - it->currentBet;
        if (relativeCost > it->chipCount) {
             relativeCost = it->chipCount; // Cap at all-in
        }
        
        it->chipCount -= relativeCost;
        it->currentBet += relativeCost;
        pot += relativeCost;

        if (it->chipCount == 0 && relativeCost > 0) {
            it->isAllIn = true;
        }
        if (type == ActionType::Fold) {
            it->hasFolded = true;
        }

        if (type == ActionType::Bet || type == ActionType::Raise) {
            int raiseAmt = absoluteAmount - currentWager;
            if (raiseAmt >= lastRaiseAmount) lastRaiseAmount = raiseAmt;
            if (type == ActionType::Raise) raisesThisRound++;
            currentWager = absoluteAmount;
        } else if (type == ActionType::PostBlind || type == ActionType::BringIn || type == ActionType::Ante) {
            if (absoluteAmount > currentWager) currentWager = absoluteAmount;
        }

        bettingHistory.back().push_back({playerId, type, absoluteAmount, relativeCost});
    }
}

bool GameState::isActionLegal(int playerId, ActionType type, int absoluteAmount) const {
    auto it = std::find_if(players.begin(), players.end(), [playerId](const Player& p) { return p.id == playerId; });
    if (it == players.end() || it->hasFolded) return false;
    const Player& p = *it;

    int callAmount = currentWager - p.currentBet;
    int relativeAmount = absoluteAmount - p.currentBet;

    if (type == ActionType::Fold) return true;
    
    if (type == ActionType::Check) {
        return callAmount == 0 && absoluteAmount == p.currentBet;
    }

    if (type == ActionType::Call) {
        // Player must exactly match currentWager, OR go all-in for whatever they have left
        if (relativeAmount == callAmount && p.chipCount >= callAmount) return true;
        if (relativeAmount == p.chipCount && p.chipCount < callAmount) return true;
        return false;
    }

    if (type == ActionType::Bet || type == ActionType::Raise) {
        if (relativeAmount > p.chipCount) return false;
        
        int raiseAmount = absoluteAmount - currentWager;
        if (raiseAmount <= 0) return false;

        if (config.limitType == LimitType::FixedLimit) {
            if (config.maxRaisesPerRound > 0 && raisesThisRound >= config.maxRaisesPerRound && relativeAmount < p.chipCount) return false;
            int limitBet = (currentBettingRound < 2) ? config.smallBlind : config.bigBlind;
            if (config.smallBlind == 0) limitBet = config.bigBlind; // fallback
            return raiseAmount == limitBet || relativeAmount == p.chipCount;
        }

        int minRaise = std::max(lastRaiseAmount, config.bigBlind);
        int minAbsolute = currentWager + minRaise;
        
        if (absoluteAmount < minAbsolute && relativeAmount < p.chipCount) {
             return false;
        }

        if (config.limitType == LimitType::PotLimit) {
            int potAfterCall = pot + callAmount;
            int maxAbsolute = currentWager + potAfterCall;
            if (absoluteAmount > maxAbsolute && relativeAmount < p.chipCount) return false;
        }
        
        return true;
    }

    return true; // PostBlind, Ante, BringIn allowed directly via engine loops
}

const std::vector<Player>& GameState::getPlayers() const { return players; }
const Hand& GameState::getBoardCards() const { return boardCards; }
const Hand& GameState::getDeck() const { return deck; }
int GameState::getPot() const { return pot; }
int GameState::getCurrentBettingRound() const { return currentBettingRound; }
const std::vector<std::vector<PlayerAction>>& GameState::getBettingHistory() const { return bettingHistory; }
const Interfaces::IGameRules* GameState::getRules() const { return rules.get(); }
const TableConfig& GameState::getConfig() const { return config; }

} // namespace Engine
} // namespace PokerEngine
