#pragma once

#include "Types.hpp"

namespace PokerEngine {
    namespace Bitboard {

        /**
         * @brief Generates a CardMask from rank and suit.
         * Bit layout comments (Strict spec):
         * - Bits 0-3 for suit (0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades)
         * - Bits 4-7 for rank (0=2 to 12=Ace)
         * - Remainder can be used for advanced flags.
         */
        [[nodiscard]] constexpr CardMask makeCard(int rank, int suit) {
            return (static_cast<uint64_t>(rank) << 4) |
                   (static_cast<uint64_t>(suit) & 0xF);
        }

        /**
         * @brief Extract the rank from a packed CardMask.
         */
        [[nodiscard]] constexpr int getRank(CardMask card) {
            return static_cast<int>((card >> 4) & 0xF);
        }

        /**
         * @brief Extract the suit from a packed CardMask.
         */
        [[nodiscard]] constexpr int getSuit(CardMask card) {
            return static_cast<int>(card & 0xF);
        }

    }  // namespace Bitboard
}  // namespace PokerEngine
