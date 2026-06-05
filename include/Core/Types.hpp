#pragma once

#include <cstdint>
#include <vector>

namespace PokerEngine {

using CardMask = uint64_t;
using Hand = std::vector<CardMask>;

enum class ActionType {
    Fold,
    Check,
    Call,
    Bet,
    Raise,
    PostBlind,
    Ante,
    BringIn
};

enum class LimitType {
    NoLimit,
    PotLimit,
    FixedLimit
};

struct TableConfig {
    LimitType limitType;
    int smallBlind;
    int bigBlind;
    int ante;
    int bringIn;
    int maxRaisesPerRound;
};

struct PlayerAction {
    int playerId;
    ActionType type;
    int absoluteAmount;
    int relativeCost;
};

struct Player {
    int id;
    int chipCount;
    Hand faceDownCards;
    Hand faceUpCards;
    bool hasFolded;
    bool isAllIn;
    int currentBet;
};

} // namespace PokerEngine
