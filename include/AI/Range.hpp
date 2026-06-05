#pragma once

#include <vector>
#include <random>
#include "../Core/Types.hpp"

namespace PokerEngine {
namespace AI {

/**
 * @brief Represents a specific combination of hand cards with its assigned probability weight.
 */
struct WeightedCombo {
    Hand cards;
    uint64_t conflictMask;
    double weight;
};

/**
 * @brief Represents a probability-weighted set of possible hands for a given scenario.
 */
class Range {
private:
    std::vector<WeightedCombo> combos;
    mutable std::discrete_distribution<size_t> dist;
    bool isCompiled = false;

public:
    Range() = default;

    /**
     * @brief Adds a combination to the range with an assigned weight.
     * @param hand The group of cards.
     * @param weight Relative probability weight.
     */
    void addCombo(const Hand& hand, double weight);

    /**
     * @brief Normalizes and internally compiles the distribution for O(1) sampling.
     */
    void compile();

    /**
     * @brief Returns a single hand sampled from the range's distribution if it avoids the deadMask.
     * @param rng External random number generator state.
     * @param deadMask Mask of dead cards (cannot be present in the sampled hand).
     * @return Hand sampled (empty if there's a conflict).
     */
    [[nodiscard]] Hand sampleHand(std::mt19937& rng, uint64_t deadMask) const;

    [[nodiscard]] bool empty() const { return combos.empty(); }
};

} // namespace AI
} // namespace PokerEngine
