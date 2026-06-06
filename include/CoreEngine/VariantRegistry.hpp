#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "IGameRules.hpp"

namespace PokerEngine {
    namespace CoreEngine {

        class VariantRegistry {
        public:
            using CreatorFunc = std::function<std::unique_ptr<IGameRules>()>;

            static VariantRegistry& getInstance() {
                static VariantRegistry instance;
                return instance;
            }

            void registerVariant(const std::string& name, CreatorFunc creator) {
                registry[name] = std::move(creator);
            }

            std::unique_ptr<IGameRules> createVariant(
                const std::string& name) const {
                auto it = registry.find(name);
                if (it != registry.end()) {
                    return it->second();
                }
                return nullptr;
            }

        private:
            VariantRegistry() = default;
            std::map<std::string, CreatorFunc> registry;
        };

    }  // namespace CoreEngine
}  // namespace PokerEngine
