#include "../../include/AI/BayesianTracker.hpp"

namespace PokerEngine {
namespace AI {

void BayesianTracker::initializePlayer(int playerId) {
    opponentProfiles[playerId] = std::vector<double>(49, 1.0 / 49.0);
}

void BayesianTracker::removePlayer(int playerId) {
    opponentProfiles.erase(playerId);
}

void BayesianTracker::updateFromAction(int playerId, ActionType type, int sizingPctOfPot) {
    auto& profile = opponentProfiles[playerId];
    
    // Core idea: 7x7 matrix (49 elements) representing Looseness vs Aggressiveness
    // type/sizing informs which quadrant is more likely.
    
    std::vector<double> likelihood(49, 1.0);
    
    for (int i = 0; i < 49; ++i) {
        int looseness = i % 7;       // 0 (Tight) to 6 (Loose)
        int aggro = i / 7;           // 0 (Passive) to 6 (Aggressive)
        
        switch(type) {
            case ActionType::Fold:
                // Highly tight players fold more. 
                likelihood[i] *= (7.0 - looseness) / 7.0; 
                break;
            case ActionType::Call:
            case ActionType::Check:
                // Passive behavior. Aggressive players do this less.
                likelihood[i] *= (7.0 - aggro) / 7.0;
                break;
            case ActionType::Bet:
            case ActionType::Raise:
                // Aggressive behavior. Scales with sizing
                double sizingFactor = std::min(1.0, sizingPctOfPot / 100.0);
                likelihood[i] *= (aggro / 7.0) * (0.5 + sizingFactor);
                break;
        }
    }
    
    // Normalize Bayesian update
    double sum = 0.0;
    for (int i = 0; i < 49; ++i) {
        profile[i] *= likelihood[i];
        sum += profile[i];
    }
    
    if (sum > 0) {
        for (double& p : profile) {
            p /= sum;
        }
    } else {
        // Fallback to uniform if extreme case happened
        for (double& p : profile) {
            p = 1.0 / 49.0;
        }
    }
}

const std::vector<double>& BayesianTracker::getProfile(int playerId) const {
    return opponentProfiles.at(playerId);
}

} // namespace AI
} // namespace PokerEngine
