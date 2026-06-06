#pragma once

#include <cstdint>
#include <vector>

namespace PokerEngine {

    using CardMask = uint64_t;
    using Hand = std::vector<CardMask>;

    /**
     * @brief Represents the types of actions a player can take down.
     */
    enum class ActionType {
        Fold,
        Check,
        Call,
        Bet,
        Raise,
        PostBlind,
        Ante,
        BringIn,
        Discard
    };

    enum class LimitType { NoLimit, PotLimit, FixedLimit };

    struct TableConfig {
        LimitType limitType;
        int smallBlind;
        int bigBlind;
        int ante;
        int bringIn;
        int maxRaisesPerRound;
    };

    /**
     * @brief Represents a specific action taken by a player along with its data
     * payload.
     */
    struct PlayerAction {
        int playerId;
        ActionType type;
        int absoluteAmount;
        int relativeCost;
        uint64_t actionPayload = 0;
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

}  // namespace PokerEngine
