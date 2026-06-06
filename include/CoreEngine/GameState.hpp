#pragma once

#include <memory>
#include <vector>

#include "../CoreEngine/IGameRules.hpp"
#include "../CoreEngine/Types.hpp"

namespace PokerEngine {
    namespace Engine {

        /**
         * @brief Tracks all table context during a given poker hand.
         */
        class GameState {
        private:
            std::vector<Player> players;
            Hand boardCards;
            Hand deck;
            int pot;
            int currentBettingRound;

            int currentWager;
            int lastRaiseAmount;
            int raisesThisRound;

            std::vector<std::vector<PlayerAction>> bettingHistory;
            TableConfig config;
            std::unique_ptr<Interfaces::IGameRules> rules;

            void initializeDeck();
            void shuffleDeck();

        public:
            /**
             * @brief Constructs the GameState tying table settings to its
             * defining rulebook.
             * @param tableConfig Limit configuration detailing table bounds.
             * @param gameRules Variant bounds implementation.
             */
            explicit GameState(
                const TableConfig& tableConfig,
                std::unique_ptr<Interfaces::IGameRules> gameRules);

            /**
             * @brief Seats a new player at the table.
             */
            void addPlayer(int id, int startingChips);

            /**
             * @brief Initiates a new hand from round 0.
             */
            void startHand();

            /**
             * @brief Fast-forwards to the next betting phase (Flop -> Turn,
             * etc).
             */
            void advanceRound();

            /**
             * @brief Interprets a requested action from a player and processes
             * it.
             * @param playerId The ID of the player taking the action.
             * @param type The type of action being performed.
             * @param absoluteAmount The absolute amount of chips the player is
             * betting/raising to (if applicable) or amount of cards discarded.
             * @param actionPayload An optional bitmask payload; e.g. used for
             * discarding specific cards.
             */
            void recordAction(int playerId, ActionType type, int absoluteAmount,
                              uint64_t actionPayload = 0);

            /**
             * @brief Validates if the action attempting to be performed is
             * currently legal.
             * @param playerId The ID of the player taking the action.
             * @param type The type of action being performed.
             * @param absoluteAmount The absolute amount of chips the player is
             * betting/raising to (if applicable) or amount of cards discarded.
             * @param actionPayload An optional bitmask payload; e.g. used for
             * discarding specific cards.
             */
            [[nodiscard]] bool isActionLegal(int playerId, ActionType type,
                                             int absoluteAmount,
                                             uint64_t actionPayload = 0) const;

            [[nodiscard]] const std::vector<Player>& getPlayers() const;
            [[nodiscard]] const Hand& getBoardCards() const;
            [[nodiscard]] const Hand& getDeck() const;
            [[nodiscard]] int getPot() const;
            [[nodiscard]] int getCurrentBettingRound() const;
            [[nodiscard]] const std::vector<std::vector<PlayerAction>>&
            getBettingHistory() const;
            [[nodiscard]] const Interfaces::IGameRules* getRules() const;
            [[nodiscard]] const TableConfig& getConfig() const;
        };

    }  // namespace Engine
}  // namespace PokerEngine
