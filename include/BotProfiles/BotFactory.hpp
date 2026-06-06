#pragma once

#include <memory>

#include "BaselineStrategies.hpp"
#include "CompositeBot.hpp"

namespace PokerEngine {
    namespace AI {

        /**
         * @enum ActionProfile
         * @brief Categorization of structural action topologies.
         */
        enum class ActionProfile {
            Balanced,
            Polarized,
            LAG,
            Nit,
            Maniac,
            Station,
            ABC
        };

        /**
         * @enum SizingProfile
         * @brief Categorization of mathematical sizing multipliers topologies.
         */
        enum class SizingProfile {
            Gaussian,
            Uniform,
            Bimodal,
            Exponential,
            Fixed,
            Gradient,
            Reverse
        };

        /**
         * @class BotFactory
         * @brief Factory architectural pattern to create all 49 combinations of
         * baseline bots safely.
         *
         * Maps distinct enum-driven configurations strictly to dynamic standard
         * polymorphic instantiations of the specific IActionStrategy and
         * ISizingStrategy configurations.
         */
        class BotFactory {
        public:
            /**
             * @brief Creates a CompositeBot composed of designated strategies.
             * @param aType Enum value representing the ActionStrategy target
             * mapping.
             * @param sType Enum value representing the SizingStrategy target
             * mapping.
             * @param seed Structural random seed ensuring repeatable
             * distributions per instance.
             * @return Unique heap-allocated pointer instance of CompositeBot.
             * @note Time Complexity: O(1) allocation operations.
             */
            [[nodiscard]] static std::unique_ptr<CompositeBot> createBot(
                ActionProfile aType, SizingProfile sType, int seed);
        };

    }  // namespace AI
}  // namespace PokerEngine
