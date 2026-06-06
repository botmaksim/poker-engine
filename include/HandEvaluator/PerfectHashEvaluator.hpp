#pragma once

#include <cstdint>

#include "../CoreEngine/Types.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @class PerfectHashEvaluator
         * @brief Represents O(1) Perfect Hash Table lookups combined with SIMD
         * vectorizations.
         *
         * Bypasses sequential rank accumulation leveraging bitwise parallel
         * computations (SWAR).
         */
        class PerfectHashEvaluator {
        public:
            /**
             * @brief Computes a unique associative integer hash for sub-arrays
             * of cards.
             * @param cards Input buffer array of cards.
             * @param count Size of the input array.
             * @return Deterministic 32-bit unique key payload.
             * @note Time Complexity: O(count) operations.
             */
            static uint32_t calculateHash(const CardMask* cards, int count);

            /**
             * @brief Evaluates best standard 5-card combination exploiting SWAR
             * (SIMD within a register) pathways.
             * @param cards Input buffer array (typically 7 size).
             * @param count Element size.
             * @return Internal absolute integer representation of structural
             * hand rank.
             * @note Time Complexity: O(count) for SWAR accumulator, then O(1)
             * or subsequent O(N) evaluation fallback.
             */
            static int evaluateHighSIMD(const CardMask* cards, int count);
        };

    }  // namespace Engine
}  // namespace PokerEngine
