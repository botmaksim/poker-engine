#include "../../include/HandEvaluator/PreflopHeuristics.hpp"

#include <algorithm>

#include "../../include/CoreEngine/Bitboard.hpp"

namespace PokerEngine {
    namespace Engine {

        double PreflopHeuristics::heuristicTreeLookup(int rank1, int rank2,
                                                      bool suited) {
            /**
             * Uses binary split logic equivalent to a heuristic tree.
             */
            int high = std::max(rank1, rank2);
            int low = std::min(rank1, rank2);

            if (high == low) {
                // Pocket Pairs mapping
                return 0.5 + (high / 12.0) * 0.35;
            }

            double base = suited ? 0.05 : 0.0;

            if (high >= 8 && low >= 8) {
                // Broadway grouping
                base += 0.55 + ((high + low) / 24.0) * 0.15;
            } else if (high == 12) {
                // High Ace
                base += 0.55 + (low / 12.0) * 0.10;
            } else {
                // Connectors and generic disjoint blocks
                int gap = high - low;
                if (gap == 1 || gap == 2) {
                    base += 0.40 + (high / 12.0) * 0.10;
                } else {
                    base += 0.30 + (high / 12.0) * 0.15;
                }
            }

            return std::min(0.85, std::max(0.30, base));
        }

        double PreflopHeuristics::getBaselinePreflopEquity(
            const CardMask* cards, int count) {
            if (count != 2) return -1.0;
            int rank1 = Bitboard::getRank(cards[0]);
            int rank2 = Bitboard::getRank(cards[1]);
            int suit1 = Bitboard::getSuit(cards[0]);
            int suit2 = Bitboard::getSuit(cards[1]);

            return heuristicTreeLookup(rank1, rank2, suit1 == suit2);
        }

    }  // namespace Engine
}  // namespace PokerEngine
