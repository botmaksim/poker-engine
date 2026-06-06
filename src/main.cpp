#include <iostream>

#include "../include/CoreEngine/GameState.hpp"
#include "../include/CoreEngine/Logger.hpp"
#include "../include/CoreEngine/Variants/TexasHoldem.hpp"

int main() {
    PokerEngine::CoreEngine::Logger::setLevel(
        PokerEngine::CoreEngine::LogLevel::INFO);
    LOG_INFO("Starting Poker Engine (UPIS) Core...");

    // Demonstrate architecture integration without AI solver
    auto game = std::make_unique<PokerEngine::Games::TexasHoldem>();
    PokerEngine::TableConfig config{
        PokerEngine::LimitType::NoLimit, 5, 10, 0, 0, 0};
    PokerEngine::Engine::GameState state(config, std::move(game));

    state.addPlayer(1, 1000);
    state.addPlayer(2, 1000);

    state.startHand();
    LOG_INFO("Hand started. Current Pot: " + std::to_string(state.getPot()));

    return 0;
}
