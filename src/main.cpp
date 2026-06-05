#include <iostream>
#include "../include/Engine/GameState.hpp"
#include "../include/Games/TexasHoldem.hpp"

int main() {
    std::cout << "Starting Poker Engine (UPIS) Core..." << std::endl;
    
    // Demonstrate architecture integration without AI solver
    auto game = std::make_unique<PokerEngine::Games::TexasHoldem>();
    PokerEngine::TableConfig config{PokerEngine::LimitType::NoLimit, 5, 10, 0, 0, 0};
    PokerEngine::Engine::GameState state(config, std::move(game));
    
    state.addPlayer(1, 1000);
    state.addPlayer(2, 1000);
    
    state.startHand();
    std::cout << "Hand started. Current Pot: " << state.getPot() << std::endl;
    
    return 0;
}
