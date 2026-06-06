#include <iostream>
#include <string>
#include <vector>

#include "../include/CoreEngine/Logger.hpp"
#include "../include/CoreEngine/TournamentArchitecture.hpp"

using namespace PokerEngine::Tournament;

std::string variantToString(GameVariant variant) {
    switch (variant) {
        case GameVariant::TexasHoldem:
            return "TexasHoldem";
        case GameVariant::Omaha:
            return "Omaha";
        case GameVariant::ShortDeck:
            return "ShortDeck";
        case GameVariant::SevenCardStud:
            return "SevenCardStud";
        case GameVariant::Razz:
            return "Razz";
        case GameVariant::FiveCardDraw:
            return "FiveCardDraw";
        case GameVariant::DeuceToSevenTripleDraw:
            return "DeuceToSevenTripleDraw";
        default:
            return "Unknown";
    }
}

int main() {
    PokerEngine::CoreEngine::Logger::setLevel(
        PokerEngine::CoreEngine::LogLevel::INFO);

    LOG_INFO("=======================================");
    LOG_INFO("PokerEngine Scale Tournament Test (All Variants)");
    LOG_INFO("=======================================");

    std::vector<int> botPool;
    for (int i = 0; i < 50; ++i) {
        botPool.push_back(i % 15);
    }

    std::vector<GameVariant> variants = {GameVariant::TexasHoldem,
                                         GameVariant::Omaha,
                                         GameVariant::ShortDeck,
                                         GameVariant::SevenCardStud,
                                         GameVariant::Razz,
                                         GameVariant::FiveCardDraw,
                                         GameVariant::DeuceToSevenTripleDraw};

    uint64_t targetPhase2Sessions =
        1000;  // Reduced to allow all variants to complete in a reasonable time

    for (auto variant : variants) {
        std::string variantName = variantToString(variant);
        LOG_INFO("");
        LOG_INFO("---------------------------------------");
        LOG_INFO("Starting Tournament for Variant: " + variantName);
        LOG_INFO("---------------------------------------");

        // Create tournament coordinator for specific poker variant
        TournamentCoordinator coordinator("tournament_data_" + variantName, 4);

        LOG_INFO("Initiating Phase 1 (Heads-Up Round-Robin)...");
        coordinator.executePhase1(botPool, variant);

        LOG_INFO("Initiating Phase 2 (Mass Simulation) with " +
                 std::to_string(targetPhase2Sessions) + " sessions...");
        coordinator.executePhase2(botPool, targetPhase2Sessions, variant);

        LOG_INFO("Completed Variant: " + variantName);
    }

    LOG_INFO("");
    LOG_INFO("All Tournaments fully completed.");

    return 0;
}
