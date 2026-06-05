#pragma once

#include <string>
#include <vector>
#include "../Core/Types.hpp"

namespace PokerEngine {
namespace Interfaces {

/**
 * @brief Defines the core rules and actions for a poker game variant.
 */
class IGameRules {
public:
    virtual ~IGameRules() = default;

    /**
     * @return The string representation of the game name.
     */
    [[nodiscard]] virtual std::string getName() const = 0;

    /**
     * @return The maximum number of betting rounds allowed in this game format.
     */
    [[nodiscard]] virtual int getMaxBettingRounds() const = 0;

    /**
     * @return True if the game utilizes community cards on the board.
     */
    [[nodiscard]] virtual bool hasCommunityCards() const = 0;

    /**
     * @return The number of cards initially dealt to each player.
     */
    [[nodiscard]] virtual int getStartingHandSize() const = 0;
    
    /**
     * @brief Posts the mandatory initial bets (blinds/antes) according to table config.
     * @param players Vector of players at the table.
     * @param pot Reference to the current pot size to correctly increase it.
     * @param history Reference to the betting history of the current round.
     * @param config The table configuration specifying limits and blind amounts.
     */
    virtual void postMandatoryBets(std::vector<Player>& players, int& pot, std::vector<PlayerAction>& history, const TableConfig& config) const = 0;

    /**
     * @brief Deals the cards corresponding to the given round (street).
     * @param players Vector of players needing cards.
     * @param boardCards Vector representing current community deck on the board.
     * @param deck Reference to the live playing deck remaining.
     * @param round The current betting round index (0-indexed).
     */
    virtual void dealStreet(std::vector<Player>& players, Hand& boardCards, Hand& deck, int round) const = 0;
    
    /**
     * @brief Evaluates the strength of player hole cards combined with community cards.
     * @param holeCards The player's specific face-down hand.
     * @param boardCards The current community cards structure.
     * @return The evaluation result integer.
     */
    [[nodiscard]] virtual int evaluateHand(const Hand& holeCards, const Hand& boardCards) const = 0;
};

} // namespace Interfaces
} // namespace PokerEngine
