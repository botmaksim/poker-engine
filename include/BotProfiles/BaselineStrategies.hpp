#pragma once

#include "StrategyInterfaces.hpp"

namespace PokerEngine {
    namespace AI {

        /**
         * @defgroup BaselineStrategy AI Evaluation Baseline Strategies
         * @brief Pre-defined baseline behaviors providing GTO or exploitative
         * topologies.
         * @{
         */

        // =============== Action Strategies ===============

        /**
         * @brief Standard balanced distribution mixing solid value-betting with
         * moderate bluffing.
         */
        class BalancedAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Polarized action preferring 3-bets or folds minimizing static
         * calling frequencies.
         */
        class PolarizedAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Loose-Aggressive (LAG) tendency heavily exploiting fold-equity
         * via over-betting.
         */
        class LAGAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Tight-Passive (Nit) tendency exclusively playing premium
         * equity mathematically.
         */
        class NitAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Pure randomization agent independent of underlying
         * mathematics.
         */
        class ManiacAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Loose-Passive (Calling Station) tendency mathematically bound
         * to check/call.
         */
        class StationAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        /**
         * @brief Vanilla ABC Poker evaluating rigid predefined check/fold
         * limits.
         */
        class ABCAction : public IActionStrategy {
        public:
            [[nodiscard]] ActionProbs getProbs(
                double equity, std::mt19937& rng) const override;
        };

        // =============== Sizing Strategies ===============

        /**
         * @brief Approximates Gaussian cluster distribution strictly around 75%
         * pot multiplier.
         */
        class GaussianSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief High variance flat-scale uniform distribution up to 2x pot
         * multipliers.
         */
        class UniformSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief Extreme minimum vs maximum topological spikes mimicking
         * polarized pot builds.
         */
        class BimodalSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief Long-tail distribution permitting highly aggressive outlier
         * multipliers overbets.
         */
        class ExponentialSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief Deterministic 100% full-pot mathematical scaler lock.
         */
        class FixedSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief Positive-scale line equation directly indexing bet multiplier
         * strictly to Win probability.
         */
        class GradientSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /**
         * @brief Inverse-scale trick topology creating high multipliers
         * deliberately on low theoretical equity.
         */
        class ReverseSizing : public ISizingStrategy {
        public:
            [[nodiscard]] double getSizing(double equity,
                                           std::mt19937& rng) const override;
        };

        /** @} */

    }  // namespace AI
}  // namespace PokerEngine
