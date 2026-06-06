#include "../../include/BayesianInference/Range.hpp"

#include <numeric>

namespace PokerEngine {
    namespace AI {

        void Range::addCombo(const Hand& hand, double weight) {
            if (weight > 0.0) {
                uint64_t mask = 0;
                for (CardMask c : hand) {
                    mask |= (1ULL << (((c >> 4) * 4) + (c & 0xF)));
                }
                combos.push_back({hand, mask, weight});
                isCompiled = false;
            }
        }

        void Range::compile() {
            if (combos.empty()) return;

            double sum = 0.0;
            for (const auto& combo : combos) {
                sum += combo.weight;
            }

            std::vector<double> weights;
            weights.reserve(combos.size());
            if (sum > 0.0) {
                for (auto& combo : combos) {
                    combo.weight /= sum;
                    weights.push_back(combo.weight);
                }
            } else {
                for (auto& combo : combos) {
                    weights.push_back(combo.weight);
                }
            }

            dist = std::discrete_distribution<size_t>(weights.begin(),
                                                      weights.end());
            isCompiled = true;
        }

        Hand Range::sampleHand(std::mt19937& rng, uint64_t deadMask) const {
            const WeightedCombo* combo = sampleCombo(rng, deadMask);
            if (!combo) return {};
            return combo->cards;
        }

        const WeightedCombo* Range::sampleCombo(std::mt19937& rng,
                                                uint64_t deadMask) const {
            if (!isCompiled || combos.empty()) return nullptr;

            size_t idx = dist(rng);
            const auto& combo = combos[idx];

            if ((combo.conflictMask & deadMask) != 0) {
                return nullptr;
            }

            return &combo;
        }

    }  // namespace AI
}  // namespace PokerEngine
