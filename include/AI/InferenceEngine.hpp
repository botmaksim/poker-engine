#pragma once

#include <map>
#include "../Engine/GameState.hpp"
#include "CompositeBot.hpp"
#include "Range.hpp"

namespace PokerEngine {
namespace AI {

/**
 * @brief Evaluates logic bridging game state and AI profiles.
 */
class InferenceEngine {
public:
    /**
     * @brief Decides the optimal action for a given bot.
     * @param state The current table context.
     * @param botPlayerId Intended bot player.
     * @param profile The instantiated behavioral profile.
     * @param currentOpponentRanges Estimated probability mappings for opposing hands.
     * @return Legal action derived from the profile's instructions.
     */
    static PlayerAction decideAction(
        const Engine::GameState& state, 
        int botPlayerId, 
        CompositeBot& profile, 
        const std::map<int, Range>& currentOpponentRanges
    );
};

} // namespace AI
} // namespace PokerEngine
