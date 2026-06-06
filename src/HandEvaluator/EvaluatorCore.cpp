#include "../../include/HandEvaluator/EvaluatorCore.hpp"

#include <algorithm>
#include <bit>
#include <limits>

#include "../../include/CoreEngine/Bitboard.hpp"

namespace PokerEngine {
    namespace Engine {

        namespace {

            int evaluateCards(const CardMask* cards, int numCards, bool is27,
                              bool isShortDeck) {
                if (numCards == 0) return 0;

                int suitCounts[4] = {0};
                int flushSuit = -1;
                for (int i = 0; i < numCards; ++i) {
                    CardMask c = cards[i];
                    int s = Bitboard::getSuit(c);
                    if (++suitCounts[s] >= 5) flushSuit = s;
                }

                uint32_t ranksBySuit[4] = {0};
                uint32_t counts[5] = {0};

                for (int i = 0; i < numCards; ++i) {
                    CardMask c = cards[i];
                    int r = Bitboard::getRank(c);
                    int s = Bitboard::getSuit(c);
                    ranksBySuit[s] |= (1 << r);

                    if (counts[4] & (1 << r)) continue;
                    if (counts[3] & (1 << r)) {
                        counts[3] &= ~(1 << r);
                        counts[4] |= (1 << r);
                        continue;
                    }
                    if (counts[2] & (1 << r)) {
                        counts[2] &= ~(1 << r);
                        counts[3] |= (1 << r);
                        continue;
                    }
                    if (counts[1] & (1 << r)) {
                        counts[1] &= ~(1 << r);
                        counts[2] |= (1 << r);
                        continue;
                    }
                    counts[1] |= (1 << r);
                }

                // Identifies highest straight run. Takes rank bitmask in.
                auto getStraightHigh = [is27, isShortDeck](uint32_t rm) -> int {
                    uint32_t ls =
                        rm & (rm >> 1) & (rm >> 2) & (rm >> 3) & (rm >> 4);
                    if (ls) return 31 - std::countl_zero(ls) + 4;

                    if (!is27) {
                        if (isShortDeck) {
                            if ((rm & 0x103C) == 0x103C)
                                return 7;  // A-6-7-8-9 (Ace is 12, 6 is 4)
                        } else {
                            if ((rm & 0x100F) == 0x100F)
                                return 3;  // A-2-3-4-5 (Ace is 12, 2 is 0)
                        }
                    }
                    return -1;
                };

                auto getTopBits = [](uint32_t mask, int count) -> int {
                    int res = 0;
                    for (int i = 0; i < count && mask; i++) {
                        int top = 31 - std::countl_zero(mask);
                        res = (res << 4) | top;
                        mask &= ~(1 << top);
                    }
                    return res;
                };

                int typeStraightFlush = 8;
                int typeQuads = 7;
                int typeFullHouse = isShortDeck ? 5 : 6;
                int typeFlush = isShortDeck ? 6 : 5;
                int typeStraight = 4;
                int typeTrips = 3;
                int typeTwoPair = 2;
                int typePair = 1;
                int typeHighCard = 0;

                int bestScore = 0;

                if (flushSuit != -1) {
                    int sf = getStraightHigh(ranksBySuit[flushSuit]);
                    if (sf != -1)
                        bestScore = std::max(
                            bestScore, (typeStraightFlush << 24) | (sf << 20));
                }

                if (counts[4]) {
                    int quad = 31 - std::countl_zero(counts[4]);
                    uint32_t kickers =
                        (counts[1] | counts[2] | counts[3]) & ~(1 << quad);
                    int kicker = kickers ? 31 - std::countl_zero(kickers) : 0;
                    bestScore =
                        std::max(bestScore, (typeQuads << 24) | (quad << 20) |
                                                (kicker << 16));
                }

                if (counts[3]) {
                    int trips1 = 31 - std::countl_zero(counts[3]);
                    int trips2 =
                        (counts[3] & ~(1 << trips1))
                            ? 31 - std::countl_zero(counts[3] & ~(1 << trips1))
                            : -1;
                    int pairs =
                        counts[2] ? 31 - std::countl_zero(counts[2]) : -1;
                    int second = std::max(trips2, pairs);
                    if (second != -1) {
                        bestScore = std::max(bestScore, (typeFullHouse << 24) |
                                                            (trips1 << 20) |
                                                            (second << 16));
                    }
                }

                if (flushSuit != -1) {
                    bestScore = std::max(
                        bestScore, (typeFlush << 24) |
                                       getTopBits(ranksBySuit[flushSuit], 5));
                }

                uint32_t allRanks =
                    counts[1] | counts[2] | counts[3] | counts[4];

                int straight = getStraightHigh(allRanks);
                if (straight != -1) {
                    bestScore = std::max(
                        bestScore, (typeStraight << 24) | (straight << 20));
                }

                if (counts[3]) {
                    int trips = 31 - std::countl_zero(counts[3]);
                    bestScore = std::max(
                        bestScore, (typeTrips << 24) | (trips << 20) |
                                       getTopBits(allRanks & ~(1 << trips), 2));
                }

                if (counts[2]) {
                    int p1 = 31 - std::countl_zero(counts[2]);
                    uint32_t rem = counts[2] & ~(1 << p1);
                    if (rem) {
                        int p2 = 31 - std::countl_zero(rem);
                        bestScore = std::max(
                            bestScore,
                            (typeTwoPair << 24) | (p1 << 20) | (p2 << 16) |
                                getTopBits(allRanks & ~(1 << p1) & ~(1 << p2),
                                           1));
                    }
                    bestScore = std::max(
                        bestScore, (typePair << 24) | (p1 << 20) |
                                       getTopBits(allRanks & ~(1 << p1), 3));
                }

                bestScore = std::max(
                    bestScore, (typeHighCard << 24) | getTopBits(allRanks, 5));

                return bestScore;
            }

        }  // anonymous namespace

        int UniversalEvaluator::evaluateHigh(const Hand& cards) {
            return evaluateCards(cards.data(), cards.size(), false, false);
        }

        int UniversalEvaluator::evaluateHigh(const CardMask* cards, int count) {
            return evaluateCards(cards, count, false, false);
        }

        int UniversalEvaluator::evaluateShortDeck(const Hand& cards) {
            return evaluateCards(cards.data(), cards.size(), false, true);
        }

        int UniversalEvaluator::evaluateShortDeck(const CardMask* cards,
                                                  int count) {
            return evaluateCards(cards, count, false, true);
        }

        int UniversalEvaluator::evaluate27Low(const Hand& cards) {
            return evaluate27Low(cards.data(), cards.size());
        }

        int UniversalEvaluator::evaluate27Low(const CardMask* cards, int n) {
            if (n < 5) return 0;

            int minHigh = std::numeric_limits<int>::max();
            CardMask combo[5];

            if (n == 5) {
                for (int i = 0; i < 5; ++i) combo[i] = cards[i];
                minHigh = evaluateCards(combo, 5, true, false);
            } else if (n == 6) {
                for (int skip = 0; skip < 6; ++skip) {
                    int idx = 0;
                    for (int i = 0; i < 6; ++i) {
                        if (i != skip) combo[idx++] = cards[i];
                    }
                    int score = evaluateCards(combo, 5, true, false);
                    if (score < minHigh) minHigh = score;
                }
            } else if (n == 7) {
                for (int skip1 = 0; skip1 < 6; ++skip1) {
                    for (int skip2 = skip1 + 1; skip2 < 7; ++skip2) {
                        int idx = 0;
                        for (int i = 0; i < 7; ++i) {
                            if (i != skip1 && i != skip2)
                                combo[idx++] = cards[i];
                        }
                        int score = evaluateCards(combo, 5, true, false);
                        if (score < minHigh) minHigh = score;
                    }
                }
            } else {
                std::vector<bool> v(n, false);
                std::fill(v.end() - 5, v.end(), true);
                do {
                    int idx = 0;
                    for (int i = 0; i < n; ++i) {
                        if (v[i]) combo[idx++] = cards[i];
                    }
                    int score = evaluateCards(combo, 5, true, false);
                    if (score < minHigh) minHigh = score;
                } while (std::next_permutation(v.begin(), v.end()));
            }

            // Invert score so that mathematically worse high hands yield higher
            // low scores.
            return 2000000000 - minHigh;
        }

    }  // namespace Engine
}  // namespace PokerEngine
