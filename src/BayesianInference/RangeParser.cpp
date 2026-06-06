#include "../../include/BayesianInference/RangeParser.hpp"

#include <algorithm>
#include <sstream>

#include "../../include/CoreEngine/Bitboard.hpp"

namespace PokerEngine {
    namespace AI {

        static int charToRank(char c) {
            switch (c) {
                case '2':
                    return 0;
                case '3':
                    return 1;
                case '4':
                    return 2;
                case '5':
                    return 3;
                case '6':
                    return 4;
                case '7':
                    return 5;
                case '8':
                    return 6;
                case '9':
                    return 7;
                case 'T':
                    return 8;
                case 't':
                    return 8;
                case 'J':
                    return 9;
                case 'j':
                    return 9;
                case 'Q':
                    return 10;
                case 'q':
                    return 10;
                case 'K':
                    return 11;
                case 'k':
                    return 11;
                case 'A':
                    return 12;
                case 'a':
                    return 12;
                default:
                    return -1;
            }
        }

        static void addPairs(AI::Range& range, int rank1, int rank2) {
            for (int r = rank1; r <= rank2; ++r) {
                for (int s1 = 0; s1 < 4; ++s1) {
                    for (int s2 = s1 + 1; s2 < 4; ++s2) {
                        Hand h = {Bitboard::makeCard(r, s1),
                                  Bitboard::makeCard(r, s2)};
                        range.addCombo(h, 1.0);
                    }
                }
            }
        }

        static void addSuited(AI::Range& range, int high, int low_min,
                              int low_max) {
            for (int low = low_min; low <= low_max; ++low) {
                if (high == low) continue;
                for (int s = 0; s < 4; ++s) {
                    Hand h = {Bitboard::makeCard(high, s),
                              Bitboard::makeCard(low, s)};
                    range.addCombo(h, 1.0);
                }
            }
        }

        static void addOffsuit(AI::Range& range, int high, int low_min,
                               int low_max) {
            for (int low = low_min; low <= low_max; ++low) {
                if (high == low) continue;
                for (int sh = 0; sh < 4; ++sh) {
                    for (int sl = 0; sl < 4; ++sl) {
                        if (sh != sl) {
                            Hand h = {Bitboard::makeCard(high, sh),
                                      Bitboard::makeCard(low, sl)};
                            range.addCombo(h, 1.0);
                        }
                    }
                }
            }
        }

        void RangeParser::parseHoldemRange(AI::Range& range,
                                           const std::string& rangeString) {
            std::stringstream ss(rangeString);
            std::string tokenStr;

            while (std::getline(ss, tokenStr, ',')) {
                // remove spaces
                tokenStr.erase(
                    std::remove_if(tokenStr.begin(), tokenStr.end(), ::isspace),
                    tokenStr.end());
                if (tokenStr.empty()) continue;

                bool isPlus = (tokenStr.back() == '+');
                if (isPlus) tokenStr.pop_back();

                if (tokenStr.size() == 2) {
                    int r1 = charToRank(tokenStr[0]);
                    int r2 = charToRank(tokenStr[1]);
                    if (r1 == -1 || r2 == -1) continue;

                    if (r1 == r2) {
                        if (isPlus) {
                            addPairs(range, r1, 12);
                        } else {
                            addPairs(range, r1, r1);
                        }
                    } else {
                        int high = std::max(r1, r2);
                        int low_min = std::min(r1, r2);
                        int low_max = isPlus ? high - 1 : low_min;
                        addSuited(range, high, low_min, low_max);
                        addOffsuit(range, high, low_min, low_max);
                    }
                } else if (tokenStr.size() == 3) {
                    int r1 = charToRank(tokenStr[0]);
                    int r2 = charToRank(tokenStr[1]);
                    char type = tokenStr[2];
                    if (r1 == -1 || r2 == -1) continue;

                    int high = std::max(r1, r2);
                    int low_min = std::min(r1, r2);
                    int low_max = isPlus ? high - 1 : low_min;

                    if (type == 's' || type == 'S') {
                        addSuited(range, high, low_min, low_max);
                    } else if (type == 'o' || type == 'O') {
                        addOffsuit(range, high, low_min, low_max);
                    }
                }
            }

            range.compile();
        }

    }  // namespace AI
}  // namespace PokerEngine
