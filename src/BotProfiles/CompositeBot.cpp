#include "../../include/BotProfiles/CompositeBot.hpp"

namespace PokerEngine {
    namespace AI {

        CompositeBot::CompositeBot(std::unique_ptr<IActionStrategy> actionStr,
                                   std::unique_ptr<ISizingStrategy> sizingStr,
                                   int seed)
            : actionStrategy(std::move(actionStr)),
              sizingStrategy(std::move(sizingStr)),
              rng(seed) {}

        ActionProbs CompositeBot::getActionProbs(double equity) {
            if (!actionStrategy) return {1.0, 0.0, 0.0};
            return actionStrategy->getProbs(equity, rng);
        }

        double CompositeBot::getSizing(double equity) {
            if (!sizingStrategy) return 0.1;
            return sizingStrategy->getSizing(equity, rng);
        }

    }  // namespace AI
}  // namespace PokerEngine
