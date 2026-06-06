#include "../../include/CoreEngine/Session.hpp"

#include <iostream>

namespace PokerEngine {
    namespace Engine {

        Session::Session(const TableConfig& config,
                         std::unique_ptr<Interfaces::IGameRules> gameRules)
            : state(std::make_unique<GameState>(config, std::move(gameRules))),
              tracker(std::make_shared<AI::BayesianTracker>()) {}

        void Session::seatBot(int playerId, int startingChips,
                              std::unique_ptr<AI::CompositeBot> bot) {
            state->addPlayer(playerId, startingChips);
            activeBots[playerId] = std::move(bot);
            tracker->initializePlayer(playerId);
        }

        std::unique_ptr<AI::CompositeBot> Session::replacePlayer(
            int playerId, std::unique_ptr<AI::CompositeBot> newBot) {
            std::unique_ptr<AI::CompositeBot> oldBot = nullptr;

            auto it = activeBots.find(playerId);
            if (it != activeBots.end()) {
                oldBot = std::move(it->second);
                it->second = std::move(newBot);
            } else {
                activeBots[playerId] = std::move(newBot);
            }

            tracker->removePlayer(playerId);
            tracker->initializePlayer(playerId);

            return oldBot;
        }

        void Session::playHands(int numberOfHands) {
            for (int i = 0; i < numberOfHands; ++i) {
                state->startHand();

                // Mock betting rounds for all streets
                // In a real implementation we'd check if the hand has concluded
                // yet
                for (int round = 0; round < 4; ++round) {
                    // TODO: Query activeBots to generate actions
                    // TODO: state->recordAction(...)
                    // TODO: tracker->updateFromAction(...)

                    state->advanceRound();
                }

                // TODO: Evaluate winner(s) and transfer chips
                // TODO: Reset folded/all-in status of players
            }
        }

    }  // namespace Engine
}  // namespace PokerEngine
