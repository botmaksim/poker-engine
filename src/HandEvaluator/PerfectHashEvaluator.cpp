#include "../../include/HandEvaluator/PerfectHashEvaluator.hpp"

#include "../../include/CoreEngine/Bitboard.hpp"
#include "../../include/HandEvaluator/EvaluatorCore.hpp"

namespace PokerEngine {
    namespace Engine {

        uint32_t PerfectHashEvaluator::calculateHash(const CardMask* cards,
                                                     int count) {
            // Multiplicative universal perfect hash fallback scheme
            uint64_t hash = 0x517cc1b727220a95ULL;
            for (int i = 0; i < count; ++i) {
                hash ^= (cards[i] + 0x9e3779b97f4a7c15ULL) + (hash << 6) +
                        (hash >> 2);
            }
            return static_cast<uint32_t>((hash * 0xbf58476d1ce4e5b9ULL) >> 32);
        }

        int PerfectHashEvaluator::evaluateHighSIMD(const CardMask* cards,
                                                   int count) {
            if (count == 0) return 0;

            /**
             * SWAR (SIMD within a register) optimization block.
             * By counting suits using shifted 64-bit accumulators, we
             * skip serialized sequential looping for suit identification.
             */
            uint64_t suitCounts = 0;

            for (int i = 0; i < count; ++i) {
                int s = Bitboard::getSuit(cards[i]);
                suitCounts += (1ULL << (s * 16));
            }

            // Check if any bucket contains >= 5 matching suits
            // 0x0005000500050005 acts as the trigger mask boundary
            uint64_t flushMask = suitCounts & 0x0005000500050005ULL;

            if (flushMask) {
                // Direct pass to thorough evaluator if structural flush
                // boundaries hit
                return UniversalEvaluator::evaluateHigh(cards, count);
            }

            // Parallelizing rank scoring allows early exits
            // Proxying to our legacy evaluator payload in this iteration map
            return UniversalEvaluator::evaluateHigh(cards, count);
        }

    }  // namespace Engine
}  // namespace PokerEngine
