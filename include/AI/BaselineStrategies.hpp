#pragma once

#include "StrategyInterfaces.hpp"

namespace PokerEngine {
namespace AI {

// =============== Action Strategies ===============

class BalancedAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class PolarizedAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class LAGAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class NitAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class ManiacAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class StationAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

class ABCAction : public IActionStrategy {
public:
    [[nodiscard]] ActionProbs getProbs(double equity, std::mt19937& rng) const override;
};

// =============== Sizing Strategies ===============

class GaussianSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class UniformSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class BimodalSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class ExponentialSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class FixedSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class GradientSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

class ReverseSizing : public ISizingStrategy {
public:
    [[nodiscard]] double getSizing(double equity, std::mt19937& rng) const override;
};

} // namespace AI
} // namespace PokerEngine
