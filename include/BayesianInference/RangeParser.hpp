#pragma once

#include <string>

#include "Range.hpp"

namespace PokerEngine {
    namespace AI {

        /**
         * @brief Utility for parsing common string representations of poker
         * ranges.
         */
        class RangeParser {
        public:
            /**
             * @brief Parses a Hold'em range string and populates the given
             * Range object.
             * @param range The AI::Range object to populate.
             * @param rangeString String representation of the range (e.g., "AA,
             * KK, A2s+, 22+").
             */
            static void parseHoldemRange(AI::Range& range,
                                         const std::string& rangeString);
        };

    }  // namespace AI
}  // namespace PokerEngine
