#pragma once

#include <memory>
#include <random>

#include "StrategyInterfaces.hpp"

namespace PokerEngine {
    namespace AI {

        /**
         * @class CompositeBot
         * @brief Standalone AI agent composed of independent action and sizing
         * strategies.
         *
         * Uses Strategy Pattern to mix-and-match sizing topologies (e.g.
         * Gaussian, Bimodal) with structural Action Profiles (e.g. LAG, Nit,
         * Station).
         */
        class CompositeBot {
        private:
            std::unique_ptr<IActionStrategy> actionStrategy;
            std::unique_ptr<ISizingStrategy> sizingStrategy;
            std::mt19937 rng;

        public:
            /**
             * @brief Constructs the bot with given strategies and a distinct
             * structural random seed.
             * @param actionStr The action strategy implementation.
             * @param sizingStr The sizing strategy implementation.
             * @param seed Structural random seed ensuring repeatable
             * distributions.
             */
            CompositeBot(std::unique_ptr<IActionStrategy> actionStr,
                         std::unique_ptr<ISizingStrategy> sizingStr, int seed);

            /**
             * @brief Evaluates baseline action tendencies strictly matching GTO
             * heuristics.
             * @param equity Theoretical Win probability in [0.0, 1.0] bound
             * mapping.
             * @return Raw isolated Probabilities for Fold, Call, and Raise.
             * @note Time Complexity: O(1) mathematical branches.
             */
            [[nodiscard]] ActionProbs getActionProbs(double equity);

            /**
             * @brief Evaluates normalized bet sizing multiplier mapped to
             * theoretical pot size.
             * @param equity Theoretical Win probability in [0.0, 1.0] bound
             * mapping.
             * @return Bet sizing multiplier strictly applied against current
             * Table Pot.
             * @note Time Complexity: O(1) random draws.
             */
            [[nodiscard]] double getSizing(double equity);
        };

    }  // namespace AI
}  // namespace PokerEngine
