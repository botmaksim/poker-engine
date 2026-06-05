#include "../../include/AI/BaselineStrategies.hpp"
#include <algorithm>
#include <cmath>

namespace PokerEngine {
namespace AI {

namespace {
    ActionProbs normalize(double f, double c, double r) {
        double sum = f + c + r;
        if (sum == 0.0) return {1.0, 0.0, 0.0};
        return {f / sum, c / sum, r / sum};
    }
}

// =============== Action Strategies ===============

ActionProbs BalancedAction::getProbs(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    bool bluff = dist(rng) < 0.1;
    if (equity >= 0.7) {
        return normalize(0.05, 0.25, 0.70);
    } else if (equity >= 0.4) {
        return normalize(0.20, 0.60, 0.20);
    } else {
        if (bluff) return normalize(0.20, 0.10, 0.70);
        return normalize(0.85, 0.10, 0.05);
    }
}

ActionProbs PolarizedAction::getProbs(double equity, std::mt19937& rng) const {
    if (equity >= 0.8) {
        return normalize(0.01, 0.10, 0.89);
    } else if (equity <= 0.2) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < 0.25) return normalize(0.10, 0.0, 0.90);
        return normalize(0.90, 0.10, 0.0);
    } else {
        return normalize(0.60, 0.40, 0.0);
    }
}

ActionProbs LAGAction::getProbs(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    bool bluff = dist(rng) < 0.3;
    if (equity >= 0.6) {
        return normalize(0.0, 0.20, 0.80);
    } else if (equity >= 0.35) {
        if (bluff) return normalize(0.10, 0.30, 0.60);
        return normalize(0.20, 0.60, 0.20);
    } else {
        if (bluff) return normalize(0.20, 0.10, 0.70);
        return normalize(0.70, 0.20, 0.10);
    }
}

ActionProbs NitAction::getProbs(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    bool bluff = dist(rng) < 0.02;
    if (equity >= 0.85) {
        return normalize(0.0, 0.30, 0.70);
    } else if (equity >= 0.70) {
        return normalize(0.10, 0.80, 0.10);
    } else {
        if (bluff) return normalize(0.70, 0.10, 0.20);
        return normalize(0.95, 0.05, 0.0);
    }
}

ActionProbs ManiacAction::getProbs(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < 0.333) {
        return normalize(0.33, 0.33, 0.34);
    }
    return normalize(0.10, 0.20, 0.70);
}

ActionProbs StationAction::getProbs(double equity, std::mt19937& rng) const {
    if (equity >= 0.95) {
        return normalize(0.0, 0.70, 0.30);
    } else if (equity >= 0.20) {
        return normalize(0.05, 0.90, 0.05);
    }
    return normalize(0.40, 0.55, 0.05);
}

ActionProbs ABCAction::getProbs(double equity, std::mt19937& rng) const {
    if (equity < 0.4) {
        return normalize(1.0, 0.0, 0.0);
    } else if (equity <= 0.7) {
        return normalize(0.0, 1.0, 0.0);
    } else {
        return normalize(0.0, 0.0, 1.0);
    }
}

// =============== Sizing Strategies ===============

double GaussianSizing::getSizing(double equity, std::mt19937& rng) const {
    std::normal_distribution<double> dist(0.75, 0.2);
    return std::max(0.1, dist(rng));
}

double UniformSizing::getSizing(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.1, 2.0);
    return dist(rng);
}

double BimodalSizing::getSizing(double equity, std::mt19937& rng) const {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < 0.5) return 0.2;
    return 1.5;
}

double ExponentialSizing::getSizing(double equity, std::mt19937& rng) const {
    std::exponential_distribution<double> dist(1.5);
    return std::clamp(dist(rng), 0.1, 5.0);
}

double FixedSizing::getSizing(double equity, std::mt19937& rng) const {
    return 1.0;
}

double GradientSizing::getSizing(double equity, std::mt19937& rng) const {
    double size = 0.5 + equity * 1.0;
    return std::max(0.1, size);
}

double ReverseSizing::getSizing(double equity, std::mt19937& rng) const {
    double size = 1.5 - equity * 1.0; 
    return std::max(0.1, size);
}

} // namespace AI
} // namespace PokerEngine
