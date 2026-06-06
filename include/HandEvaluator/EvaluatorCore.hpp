#pragma once
#include "../CoreEngine/Types.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @brief High-performance bitwise evaluator for calculating structural
         * poker hand scores.
         */
        class UniversalEvaluator {
        public:
            /**
             * @brief Evaluates best standard 5-card combination from the
             * provided cards.
             * @param cards Vector of 5 to 7 CardMasks.
             * @return 32-bit score integer. Higher is better.
             */
            static int evaluateHigh(const Hand& cards);
            static int evaluateHigh(const CardMask* cards, int count);

            /**
             * @brief Evaluates a Deuce-to-Seven low hand. Straights/Flushes
             * hurt, Ace is always High.
             * @return Score integer where higher is better.
             */
            static int evaluate27Low(const Hand& cards);
            static int evaluate27Low(const CardMask* cards, int count);

            /**
             * @brief Evaluates a Short Deck hand (Flush beats Full House,
             * A-6-7-8-9 straight).
             * @return 32-bit score integer. Higher is better.
             */
            static int evaluateShortDeck(const Hand& cards);
            static int evaluateShortDeck(const CardMask* cards, int count);
        };

    }  // namespace Engine
}  // namespace PokerEngine
