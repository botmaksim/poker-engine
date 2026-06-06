#pragma once

#include "../CoreEngine/Types.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @class PreflopHeuristics
         * @brief Heuristic Binary Tree for preflop equity evaluation.
         *
         * Determines expected equity based on a hierarchical tree-like
         * traversal of starting hand characteristics avoiding deep Monte Carlo
         * computation dynamically.
         */
        class PreflopHeuristics {
        public:
            /**
             * @brief Gets pre-calculated heads-up equity for a given 2-card
             * starting hand against a random hand.
             * @param cards Array of hole cards.
             * @param count Number of cards.
             * @return Double representing the baseline equity, or -1.0 if not
             * preflop or invalid state.
             * @note Time Complexity: O(1) operations.
             */
            static double getBaselinePreflopEquity(const CardMask* cards,
                                                   int count);

            /**
             * @brief Looks up exact match in a heuristic binary tree mapping
             * rank combinations to equity.
             * @param rank1 Primary card rank.
             * @param rank2 Secondary card rank.
             * @param suited True if Both cards hold same suit.
             * @return Estimated baseline equity.
             * @note Time Complexity: O(1) branching. Space Complexity: O(1).
             */
            static double heuristicTreeLookup(int rank1, int rank2,
                                              bool suited);
        };

    }  // namespace Engine
}  // namespace PokerEngine
