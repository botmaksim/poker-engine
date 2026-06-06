#pragma once

#include <map>
#include <vector>

#include "../CoreEngine/Types.hpp"

namespace PokerEngine {
    namespace AI {

        /**
         * @class BayesianTracker
         * @brief Tracks Bayesian probabilities of opponent bot profiles
         * incorporating regularized updates.
         *
         * Maps a 7x7 matrix (Looseness vs. Aggressiveness) for an opponent
         * where each cell represents a discretized persona. Utilizes Dirichlet
         * priors to prevent probability mass collapsing to absolute zero.
         */
        class BayesianTracker {
        private:
            std::map<int, std::vector<double>> opponentProfiles;

        public:
            /**
             * @brief Initializes standard uniform probability distribution for
             * a new player.
             * @param playerId The player being tracked.
             * @note Time Complexity: O(1) assuming matrix size is constant
             * (49).
             */
            void initializePlayer(int playerId);

            /**
             * @brief Removes a player from the shared tracking memory.
             * @param playerId The player being removed.
             * @note Time Complexity: O(log N) mapping erasure.
             */
            void removePlayer(int playerId);

            /**
             * @brief Updates the probability distribution based on observed
             * action and sizing.
             * @param playerId The observed player.
             * @param type The action taken.
             * @param sizingPctOfPot Bet sizing expressed as a percentage of the
             * pot.
             * @note Time Complexity: O(1) bounded array updates with
             * SIMD-capable continuous arrays.
             */
            void updateFromAction(int playerId, ActionType type,
                                  int sizingPctOfPot);

            /**
             * @brief Interprets the latent profile matrix to estimate real-time
             * fold probability.
             * @param playerId The targeted player.
             * @return Double in [0.0, 1.0] representing estimated fold
             * probability.
             * @note Time Complexity: O(1) matrix reduction.
             */
            [[nodiscard]] double estimateFoldProbability(int playerId) const;

            /**
             * @brief Calculates normalized Shannon Entropy of the Bayesian
             * probability distribution.
             * @param playerId The targeted player.
             * @return Double in [0.0, 1.0], where 1.0 is maximum uncertainty
             * (uniform) and 0.0 is perfect certainty.
             * @note Time Complexity: O(1).
             */
            [[nodiscard]] double getProfileEntropy(int playerId) const;

            /**
             * @brief Retrieves the 49-element probability vector defining the
             * opponent.
             * @param playerId The player profile to query.
             * @return Const reference to the 49-element vector mapping the 7x7
             * bot types.
             */
            [[nodiscard]] const std::vector<double>& getProfile(
                int playerId) const;
        };

    }  // namespace AI
}  // namespace PokerEngine
