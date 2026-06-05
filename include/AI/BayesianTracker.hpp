#pragma once

#include <vector>
#include <map>
#include "../Core/Types.hpp"

namespace PokerEngine {
namespace AI {

/**
 * @brief Tracks Bayesian probabilities of opponent bot profiles.
 */
class BayesianTracker {
private:
    std::map<int, std::vector<double>> opponentProfiles;

public:
    /**
     * @brief Initializes standard uniform probability distribution for a new player.
     * @param playerId The player being tracked.
     */
    void initializePlayer(int playerId);

    /**
     * @brief Removes a player from the shared tracking memory.
     * @param playerId The player being removed.
     */
    void removePlayer(int playerId);

    /**
     * @brief Updates the probability distribution based on observed action and sizing.
     * @param playerId The observed player.
     * @param type The action taken.
     * @param sizingPctOfPot Bet sizing expressed as a percentage of the pot.
     */
    void updateFromAction(int playerId, ActionType type, int sizingPctOfPot);

    /**
     * @brief Retrieves the 49-element probability vector defining the opponent.
     * @param playerId The player profile to query.
     * @return 49-element vector mapping to the 7x7 bot types.
     */
    [[nodiscard]] const std::vector<double>& getProfile(int playerId) const;
};

} // namespace AI
} // namespace PokerEngine
