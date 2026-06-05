#include <iostream>
#include <vector>
#include <string>

#include "../include/Engine/TournamentArchitecture.hpp"

using namespace PokerEngine::Tournament;

std::string variantToString(GameVariant variant) {
    switch (variant) {
        case GameVariant::TexasHoldem: return "TexasHoldem";
        case GameVariant::Omaha: return "Omaha";
        case GameVariant::ShortDeck: return "ShortDeck";
        case GameVariant::SevenCardStud: return "SevenCardStud";
        case GameVariant::Razz: return "Razz";
        case GameVariant::FiveCardDraw: return "FiveCardDraw";
        case GameVariant::DeuceToSevenTripleDraw: return "DeuceToSevenTripleDraw";
        default: return "Unknown";
    }
}

int main() {
    std::cout << "=======================================\n";
    std::cout << "PokerEngine Scale Tournament Test (All Variants)\n";
    std::cout << "=======================================\n";
    

    std::vector<int> botPool;
    for (int i = 0; i < 50; ++i) {
        botPool.push_back(i % 15);
    }

    std::vector<GameVariant> variants = {
        GameVariant::TexasHoldem,
        GameVariant::Omaha,
        GameVariant::ShortDeck,
        GameVariant::SevenCardStud,
        GameVariant::Razz,
        GameVariant::FiveCardDraw,
        GameVariant::DeuceToSevenTripleDraw
    };

    uint64_t targetPhase2Sessions = 1000; // Reduced to allow all variants to complete in a reasonable time

    for (auto variant : variants) {
        std::string variantName = variantToString(variant);
        std::cout << "\n---------------------------------------\n";
        std::cout << "Starting Tournament for Variant: " << variantName << "\n";
        std::cout << "---------------------------------------\n";

        // Create tournament coordinator for specific poker variant
        TournamentCoordinator coordinator("tournament_data_" + variantName, 4);

        std::cout << "Initiating Phase 1 (Heads-Up Round-Robin)...\n";
        coordinator.executePhase1(botPool, variant);
        
        std::cout << "Initiating Phase 2 (Mass Simulation) with " << targetPhase2Sessions << " sessions...\n";
        coordinator.executePhase2(botPool, targetPhase2Sessions, variant);
        
        std::cout << "Completed Variant: " << variantName << "\n";
    }
    
    std::cout << "\nAll Tournaments fully completed.\n";

    return 0;
}
