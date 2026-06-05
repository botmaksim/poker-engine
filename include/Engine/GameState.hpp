#pragma once

#include <vector>
#include <memory>
#include "../Core/Types.hpp"
#include "../Interfaces/IGameRules.hpp"

namespace PokerEngine {
namespace Engine {

/**
 * @brief Tracks all table context during a given poker hand. 
 */
class GameState {
private:
    std::vector<Player> players;
    Hand boardCards;
    Hand deck;
    int pot;
    int currentBettingRound;
    
    int currentWager;
    int lastRaiseAmount;
    int raisesThisRound;

    std::vector<std::vector<PlayerAction>> bettingHistory;
    TableConfig config;
    std::unique_ptr<Interfaces::IGameRules> rules;

    void initializeDeck();
    void shuffleDeck();

public:
    /**
     * @brief Constructs the GameState tying table settings to its defining rulebook.
     * @param tableConfig Limit configuration detailing table bounds.
     * @param gameRules Variant bounds implementation.
     */
    explicit GameState(const TableConfig& tableConfig, std::unique_ptr<Interfaces::IGameRules> gameRules);

    /**
     * @brief Seats a new player at the table.
     */
    void addPlayer(int id, int startingChips);

    /**
     * @brief Initiates a new hand from round 0.
     */
    void startHand();

    /**
     * @brief Fast-forwards to the next betting phase (Flop -> Turn, etc).
     */
    void advanceRound();

    /**
     * @brief Interprets a requested action from a player and processes it.
     */
    void recordAction(int playerId, ActionType type, int absoluteAmount);

    /**
     * @brief Validates if the action attempting to be performed is currently legal.
     */
    [[nodiscard]] bool isActionLegal(int playerId, ActionType type, int absoluteAmount) const;

    [[nodiscard]] const std::vector<Player>& getPlayers() const;
    [[nodiscard]] const Hand& getBoardCards() const;
    [[nodiscard]] const Hand& getDeck() const;
    [[nodiscard]] int getPot() const;
    [[nodiscard]] int getCurrentBettingRound() const;
    [[nodiscard]] const std::vector<std::vector<PlayerAction>>& getBettingHistory() const;
    [[nodiscard]] const Interfaces::IGameRules* getRules() const;
    [[nodiscard]] const TableConfig& getConfig() const;
};

} // namespace Engine
} // namespace PokerEngine
