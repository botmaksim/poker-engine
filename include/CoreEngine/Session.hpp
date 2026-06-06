#pragma once

#include <map>
#include <memory>

#include "../BayesianInference/BayesianTracker.hpp"
#include "../BotProfiles/CompositeBot.hpp"
#include "../CoreEngine/IGameRules.hpp"
#include "GameState.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @brief Manages a continuous stream of poker hands and persists data
         * across sessions.
         */
        class Session {
        private:
            std::unique_ptr<GameState> state;
            std::shared_ptr<AI::BayesianTracker> tracker;
            std::map<int, std::unique_ptr<AI::CompositeBot>> activeBots;

        public:
            /**
             * @brief Constructs a new Session spanning multiple hands.
             * @param config The table limits.
             * @param gameRules The ruleset variant played at this table.
             */
            Session(const TableConfig& config,
                    std::unique_ptr<Interfaces::IGameRules> gameRules);

            /**
             * @brief Sets up a new bot actor at the session table.
             * @param playerId An identifier for the actor.
             * @param startingChips Beginning stack.
             * @param bot The behavioral profile controlling the player.
             */
            void seatBot(int playerId, int startingChips,
                         std::unique_ptr<AI::CompositeBot> bot);

            /**
             * @brief Hot-swaps an existing player's neural/algorithmic brain
             * without altering stack depth.
             * @param playerId The actor to be lobotomized.
             * @param newBot The new behavior implementation.
             * @return The previous CompositeBot mapping (or nullptr if none
             * existed).
             */
            std::unique_ptr<AI::CompositeBot> replacePlayer(
                int playerId, std::unique_ptr<AI::CompositeBot> newBot);

            /**
             * @brief Automatically simulates a given number of hands using
             * active bots.
             * @param numberOfHands The hands to loop through.
             */
            void playHands(int numberOfHands);
        };

    }  // namespace Engine
}  // namespace PokerEngine
