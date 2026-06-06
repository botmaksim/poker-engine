#include "../../include/BayesianInference/BayesianTracker.hpp"

#include <algorithm>
#include <cmath>

namespace PokerEngine {
    namespace AI {

        static double FOLD_FACTORS[49];
        static double CALL_FACTORS[49];
        static double AGGRO_FACTORS[49];
        static bool factors_initialized = false;

        static void initFactors() {
            if (factors_initialized) return;
            for (int i = 0; i < 49; ++i) {
                int looseness = i % 7;  // 0 (Tight) to 6 (Loose)
                int aggro = i / 7;      // 0 (Passive) to 6 (Aggressive)
                FOLD_FACTORS[i] = ((7.0 - looseness) / 7.0) * 0.90 +
                                  0.05;  // Tight players fold more
                CALL_FACTORS[i] = ((7.0 - aggro) / 7.0) * 0.90 +
                                  0.05;  // Passive players call more
                AGGRO_FACTORS[i] =
                    (aggro / 7.0) * 0.90 + 0.05;  // Aggressive players bet more
            }
            factors_initialized = true;
        }

        void BayesianTracker::initializePlayer(int playerId) {
            opponentProfiles[playerId] = std::vector<double>(49, 1.0 / 49.0);
            initFactors();
        }

        void BayesianTracker::removePlayer(int playerId) {
            opponentProfiles.erase(playerId);
        }

        void BayesianTracker::updateFromAction(int playerId, ActionType type,
                                               int sizingPctOfPot) {
            auto& profile = opponentProfiles[playerId];
            initFactors();

            double sum = 0.0;

            switch (type) {
                case ActionType::Fold:
                    for (int i = 0; i < 49; ++i) {
                        profile[i] *= FOLD_FACTORS[i];
                        sum += profile[i];
                    }
                    break;
                case ActionType::Call:
                case ActionType::Check:
                    for (int i = 0; i < 49; ++i) {
                        profile[i] *= CALL_FACTORS[i];
                        sum += profile[i];
                    }
                    break;
                case ActionType::Bet:
                case ActionType::Raise: {
                    double sizingFactor = std::min(1.0, sizingPctOfPot / 100.0);
                    double baseMult = 0.5 + sizingFactor;
                    for (int i = 0; i < 49; ++i) {
                        profile[i] *= AGGRO_FACTORS[i] * baseMult;
                        sum += profile[i];
                    }
                    break;
                }
                default:
                    return;
            }

            // Normalize Bayesian update with a static Dirichlet prior to
            // prevent probability collapse
            constexpr double DIRICHLET_PRIOR = 0.01;
            double newSum = 0.0;

            if (sum > 0.0) {
                double invSum = 1.0 / sum;
                for (double& p : profile) {
                    p = p * invSum + DIRICHLET_PRIOR;
                    newSum += p;
                }
            } else {
                // Extreme edge case fallback
                for (double& p : profile) {
                    p = (1.0 / 49.0) + DIRICHLET_PRIOR;
                    newSum += p;
                }
            }

            // Final normalization after smoothing
            double finalInvSum = 1.0 / newSum;
            for (double& p : profile) {
                p *= finalInvSum;
            }
        }

        double BayesianTracker::estimateFoldProbability(int playerId) const {
            auto it = opponentProfiles.find(playerId);
            if (it == opponentProfiles.end()) return 0.5;  // Unknown

            const auto& profile = it->second;
            double estimatedFoldProb = 0.0;

            for (int i = 0; i < 49; ++i) {
                // Base fold probability map based on Looseness index
                // Tightest (0) folds 80% of the time. Loosest (6) folds 10%.
                int looseness = i % 7;
                double personaFoldProb = 0.80 - (looseness / 6.0) * 0.70;
                estimatedFoldProb += profile[i] * personaFoldProb;
            }

            return std::clamp(estimatedFoldProb, 0.01, 0.99);
        }

        double BayesianTracker::getProfileEntropy(int playerId) const {
            auto it = opponentProfiles.find(playerId);
            if (it == opponentProfiles.end()) return 1.0;

            const auto& profile = it->second;
            double entropy = 0.0;

            for (int i = 0; i < 49; ++i) {
                if (profile[i] > 1e-9) {
                    entropy -= profile[i] * std::log(profile[i]);
                }
            }

            // Max entropy for 49 uniform bins is ln(49) ~= 3.8918
            return std::clamp(entropy / std::log(49.0), 0.0, 1.0);
        }

        const std::vector<double>& BayesianTracker::getProfile(
            int playerId) const {
            return opponentProfiles.at(playerId);
        }

    }  // namespace AI
}  // namespace PokerEngine
