#pragma once

#include <random>

namespace PokerEngine {
namespace AI {

struct ActionProbs {
    double pFold;
    double pCall;
    double pRaise;
};

class IActionStrategy {
public:
    virtual ~IActionStrategy() = default;
    [[nodiscard]] virtual ActionProbs getProbs(double equity, std::mt19937& rng) const = 0;
};

class ISizingStrategy {
public:
    virtual ~ISizingStrategy() = default;
    [[nodiscard]] virtual double getSizing(double equity, std::mt19937& rng) const = 0;
};

} // namespace AI
} // namespace PokerEngine
