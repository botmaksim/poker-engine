#pragma once

#include <map>
#include <vector>

#include "../BayesianInference/Range.hpp"
#include "../CoreEngine/GameState.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @brief Utility for deriving structural equity metrics across
         * differing game variants.
         */
        class Analyzer {
        public:
            /**
             * @brief Performs Monte Carlo simulation to calculate hot/cold
             * equities.
             * @param state The current table state including board cards.
             * @param opponentRanges Maps player IDs to their respective ranges.
             * Missing IDs will use exact known hole cards if available.
             * @param iterations Number of simulated rollouts.
             * @return std::vector<double> Ordered equities corresponding
             * sequentially to active players.
             */
            [[nodiscard]] static std::vector<double> calculateEquity(
                const GameState& state,
                const std::map<int, AI::Range>& opponentRanges,
                int iterations = 10000);
        };

    }  // namespace Engine
}  // namespace PokerEngine
