#include "../../include/BayesianInference/InferenceEngine.hpp"

#include <algorithm>
#include <random>

#include "../../include/CoreEngine/Logger.hpp"
#include "../../include/MonteCarloSimulator/Analyzer.hpp"

namespace PokerEngine {
    namespace AI {

        PlayerAction InferenceEngine::decideAction(
            const Engine::GameState& state, int botPlayerId,
            CompositeBot& profile,
            const std::map<int, Range>& currentOpponentRanges,
            const BayesianTracker* tracker) {
            const auto& players = state.getPlayers();
            auto botIt = std::find_if(
                players.begin(), players.end(),
                [botPlayerId](const Player& p) { return p.id == botPlayerId; });

            // Safety check
            if (botIt == players.end() || botIt->hasFolded || botIt->isAllIn) {
                return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
            }

            const Player& bot = *botIt;

            int currentWager = 0;
            for (const auto& p : players) {
                if (p.currentBet > currentWager) {
                    currentWager = p.currentBet;
                }
            }

            int toCall = currentWager - bot.currentBet;
            int pot = state.getPot();

            // 1. Get Equity via fast Monte Carlo evaluation bounds
            std::vector<double> equities = Engine::Analyzer::calculateEquity(
                state, currentOpponentRanges, 5000);
            double myEquity = 0.0;
            for (size_t i = 0; i < players.size(); ++i) {
                if (players[i].id == botPlayerId) {
                    myEquity = equities[i];
                    break;
                }
            }

            // 2. Base Model (GTO / Abstract Persona Baseline)
            ActionProbs baselineProbs = profile.getActionProbs(myEquity);

            // 3. EV Exploit Calculations & blending
            double finalFold = baselineProbs.pFold;
            double finalCall = baselineProbs.pCall;
            double finalRaise = baselineProbs.pRaise;

            if (tracker != nullptr) {
                double avgFoldProb = 0.0;
                double minEntropy = 1.0;
                int activeOpponents = 0;

                for (const auto& p : players) {
                    if (p.id != botPlayerId && !p.hasFolded) {
                        avgFoldProb += tracker->estimateFoldProbability(p.id);
                        minEntropy = std::min(minEntropy,
                                              tracker->getProfileEntropy(p.id));
                        activeOpponents++;
                    }
                }

                if (activeOpponents > 0) {
                    avgFoldProb /= activeOpponents;

                    // Expected Value (EV) Formulation
                    double evFold = (toCall == 0) ? myEquity * pot : 0.0;

                    double winGivenCall = pot + toCall;
                    double loseGivenCall = -toCall;
                    double evCall = (myEquity * winGivenCall) +
                                    ((1.0 - myEquity) * loseGivenCall);

                    double proposedRaiseAmount =
                        pot * profile.getSizing(myEquity);
                    double raiseRisk = toCall + proposedRaiseAmount;
                    double winGivenRaise =
                        pot + toCall + proposedRaiseAmount;  // Approximation
                    double evRaise = (avgFoldProb * pot) +
                                     ((1.0 - avgFoldProb) *
                                      ((myEquity * winGivenRaise) -
                                       ((1.0 - myEquity) * raiseRisk)));

                    // Extract Pure Exploit max actions
                    double exploitFold = 0.01;
                    double exploitCall = 0.01;
                    double exploitRaise = 0.01;

                    if (evFold >= evCall && evFold >= evRaise) {
                        exploitFold = 0.98;
                    } else if (evCall >= evFold && evCall >= evRaise) {
                        exploitCall = 0.98;
                    } else {
                        exploitRaise = 0.98;
                    }

                    // High entropy -> 1.0 (Uniform, unsure) -> 100% weight on
                    // baseline Low entropy -> 0.0 (Certain) -> 100% weight on
                    // purely exploitative EV
                    double wBase = minEntropy;
                    double wExploit = 1.0 - minEntropy;

                    finalFold = (baselineProbs.pFold * wBase) +
                                (exploitFold * wExploit);
                    finalCall = (baselineProbs.pCall * wBase) +
                                (exploitCall * wExploit);
                    finalRaise = (baselineProbs.pRaise * wBase) +
                                 (exploitRaise * wExploit);

                    double normSum = finalFold + finalCall + finalRaise;
                    finalFold /= normSum;
                    finalCall /= normSum;
                    finalRaise /= normSum;
                }
            }

            // 4. Roll the Dice
            static thread_local std::mt19937 rng(std::random_device{}());
            std::discrete_distribution<int> actionDist(
                {finalFold, finalCall, finalRaise});
            int actionIdx = actionDist(rng);

            // 5. Validate & Size
            ActionType intendedType = ActionType::Fold;
            if (actionIdx == 1)
                intendedType = ActionType::Call;
            else if (actionIdx == 2)
                intendedType = ActionType::Raise;

            int maxAvailable = bot.currentBet + bot.chipCount;

            if (intendedType == ActionType::Fold) {
                if (toCall == 0) {
                    if (state.isActionLegal(botPlayerId, ActionType::Check,
                                            bot.currentBet)) {
                        return PlayerAction{botPlayerId, ActionType::Check,
                                            bot.currentBet, 0};
                    }
                    return PlayerAction{botPlayerId, ActionType::Call,
                                        currentWager, 0};
                }
                return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
            }

            if (intendedType == ActionType::Raise) {
                double multiplier = profile.getSizing(myEquity);
                int raiseAmount =
                    currentWager + static_cast<int>(pot * multiplier);

                if (raiseAmount > maxAvailable) {
                    raiseAmount = maxAvailable;
                }

                ActionType rType =
                    (currentWager == 0) ? ActionType::Bet : ActionType::Raise;

                if (state.isActionLegal(botPlayerId, rType, raiseAmount)) {
                    return PlayerAction{botPlayerId, rType, raiseAmount,
                                        raiseAmount - bot.currentBet};
                }

                int fallbackRaise = currentWager + state.getConfig().bigBlind;
                if (fallbackRaise <= maxAvailable &&
                    state.isActionLegal(botPlayerId, rType, fallbackRaise)) {
                    return PlayerAction{botPlayerId, rType, fallbackRaise,
                                        fallbackRaise - bot.currentBet};
                }

                if (state.isActionLegal(botPlayerId, rType, maxAvailable)) {
                    return PlayerAction{botPlayerId, rType, maxAvailable,
                                        maxAvailable - bot.currentBet};
                }

                intendedType = ActionType::Call;
            }

            if (intendedType == ActionType::Call) {
                if (toCall == 0) {
                    if (state.isActionLegal(botPlayerId, ActionType::Check,
                                            bot.currentBet)) {
                        return PlayerAction{botPlayerId, ActionType::Check,
                                            bot.currentBet, 0};
                    }
                }

                int callAmount = std::min(currentWager, maxAvailable);
                if (state.isActionLegal(botPlayerId, ActionType::Call,
                                        callAmount)) {
                    return PlayerAction{botPlayerId, ActionType::Call,
                                        callAmount,
                                        callAmount - bot.currentBet};
                }

                return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
            }

            return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
        }

    }  // namespace AI
}  // namespace PokerEngine
