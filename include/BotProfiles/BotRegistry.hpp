#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "StrategyInterfaces.hpp"

namespace PokerEngine {
    namespace BotProfiles {

        class BotRegistry {
        public:
            using ActionCreator =
                std::function<std::unique_ptr<IActionStrategy>()>;
            using SizingCreator =
                std::function<std::unique_ptr<ISizingStrategy>()>;

            static BotRegistry& getInstance() {
                static BotRegistry instance;
                return instance;
            }

            void registerActionProfile(const std::string& name,
                                       ActionCreator creator) {
                actionRegistry[name] = std::move(creator);
            }

            void registerSizingProfile(const std::string& name,
                                       SizingCreator creator) {
                sizingRegistry[name] = std::move(creator);
            }

            std::unique_ptr<IActionStrategy> createActionProfile(
                const std::string& name) const {
                auto it = actionRegistry.find(name);
                if (it != actionRegistry.end()) return it->second();
                return nullptr;  // Caller handles fallback
            }

            std::unique_ptr<ISizingStrategy> createSizingProfile(
                const std::string& name) const {
                auto it = sizingRegistry.find(name);
                if (it != sizingRegistry.end()) return it->second();
                return nullptr;  // Caller handles fallback
            }

        private:
            BotRegistry() = default;
            std::map<std::string, ActionCreator> actionRegistry;
            std::map<std::string, SizingCreator> sizingRegistry;
        };

    }  // namespace BotProfiles
}  // namespace PokerEngine
