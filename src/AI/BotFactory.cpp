#include "../../include/AI/BotFactory.hpp"

namespace PokerEngine {
namespace AI {

std::unique_ptr<CompositeBot> BotFactory::createBot(ActionProfile aType, SizingProfile sType, int seed) {
    std::unique_ptr<IActionStrategy> action;
    switch (aType) {
        case ActionProfile::Balanced:  action = std::make_unique<BalancedAction>(); break;
        case ActionProfile::Polarized: action = std::make_unique<PolarizedAction>(); break;
        case ActionProfile::LAG:       action = std::make_unique<LAGAction>(); break;
        case ActionProfile::Nit:       action = std::make_unique<NitAction>(); break;
        case ActionProfile::Maniac:    action = std::make_unique<ManiacAction>(); break;
        case ActionProfile::Station:   action = std::make_unique<StationAction>(); break;
        case ActionProfile::ABC:       action = std::make_unique<ABCAction>(); break;
        default:                       action = std::make_unique<BalancedAction>(); break;
    }

    std::unique_ptr<ISizingStrategy> sizing;
    switch (sType) {
        case SizingProfile::Gaussian:    sizing = std::make_unique<GaussianSizing>(); break;
        case SizingProfile::Uniform:     sizing = std::make_unique<UniformSizing>(); break;
        case SizingProfile::Bimodal:     sizing = std::make_unique<BimodalSizing>(); break;
        case SizingProfile::Exponential: sizing = std::make_unique<ExponentialSizing>(); break;
        case SizingProfile::Fixed:       sizing = std::make_unique<FixedSizing>(); break;
        case SizingProfile::Gradient:    sizing = std::make_unique<GradientSizing>(); break;
        case SizingProfile::Reverse:     sizing = std::make_unique<ReverseSizing>(); break;
        default:                         sizing = std::make_unique<GaussianSizing>(); break;
    }

    return std::make_unique<CompositeBot>(std::move(action), std::move(sizing), seed);
}

} // namespace AI
} // namespace PokerEngine
