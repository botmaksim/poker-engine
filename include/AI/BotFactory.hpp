#pragma once

#include "CompositeBot.hpp"
#include "BaselineStrategies.hpp"
#include <memory>

namespace PokerEngine {
namespace AI {

enum class ActionProfile { Balanced, Polarized, LAG, Nit, Maniac, Station, ABC };
enum class SizingProfile { Gaussian, Uniform, Bimodal, Exponential, Fixed, Gradient, Reverse };

/**
 * @brief Factory class to create all 49 combinations of baseline bots safely.
 */
class BotFactory {
public:
    /**
     * @brief Creates a CompositeBot composed of designated strategies.
     * @param aType Enum value representing the ActionStrategy
     * @param sType Enum value representing the SizingStrategy
     * @param seed Random seed for the bot instantiation.
     * @return Unique instance of CompositeBot.
     */
    [[nodiscard]] static std::unique_ptr<CompositeBot> createBot(ActionProfile aType, SizingProfile sType, int seed);
};

} // namespace AI
} // namespace PokerEngine
