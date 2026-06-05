#pragma once

#include "StrategyInterfaces.hpp"
#include <memory>
#include <random>

namespace PokerEngine {
namespace AI {

/**
 * @brief Standalone AI agent composed of independent action and sizing strategies.
 */
class CompositeBot {
private:
    std::unique_ptr<IActionStrategy> actionStrategy;
    std::unique_ptr<ISizingStrategy> sizingStrategy;
    std::mt19937 rng;

public:
    /**
     * @brief Constructs the bot with given strategies and a random seed.
     * @param actionStr The action strategy.
     * @param sizingStr The sizing strategy.
     * @param seed Random seed.
     */
    CompositeBot(std::unique_ptr<IActionStrategy> actionStr,
                 std::unique_ptr<ISizingStrategy> sizingStr,
                 int seed);

    /**
     * @brief Evaluates action probabilities.
     * @param equity Win probability (0.0 to 1.0).
     * @return Probabilities for Fold, Call, and Raise.
     */
    [[nodiscard]] ActionProbs getActionProbs(double equity);

    /**
     * @brief Evaluates bet sizing multiplier.
     * @param equity Win probability (0.0 to 1.0).
     * @return Bet sizing multiplier.
     */
    [[nodiscard]] double getSizing(double equity);
};

} // namespace AI
} // namespace PokerEngine
