#include "../../include/AI/InferenceEngine.hpp"
#include "../../include/Engine/Analyzer.hpp"
#include <algorithm>
#include <random>

namespace PokerEngine {
namespace AI {

PlayerAction InferenceEngine::decideAction(const Engine::GameState& state, int botPlayerId, CompositeBot& profile, const std::map<int, Range>& currentOpponentRanges) {
    const auto& players = state.getPlayers();
    auto botIt = std::find_if(players.begin(), players.end(), [botPlayerId](const Player& p){ return p.id == botPlayerId; });
    
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

    // 1. Get Equity
    std::vector<double> equities = Engine::Analyzer::calculateEquity(state, currentOpponentRanges, 5000);
    double myEquity = 0.0;
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].id == botPlayerId) {
            myEquity = equities[i];
            break;
        }
    }

    // 2. Get Action Probabilities
    ActionProbs probs = profile.getActionProbs(myEquity);

    // 3. Roll the Dice
    std::random_device rd;
    std::mt19937 rng(rd());
    std::discrete_distribution<int> actionDist({probs.pFold, probs.pCall, probs.pRaise});
    int actionIdx = actionDist(rng);

    // 4. Validate & Size
    ActionType intendedType = ActionType::Fold;
    if (actionIdx == 1) intendedType = ActionType::Call;
    else if (actionIdx == 2) intendedType = ActionType::Raise;

    int maxAvailable = bot.currentBet + bot.chipCount;

    if (intendedType == ActionType::Fold) {
        // If checking is possible, check instead of folding
        if (toCall == 0) {
            if (state.isActionLegal(botPlayerId, ActionType::Check, bot.currentBet)) {
                return PlayerAction{botPlayerId, ActionType::Check, bot.currentBet, 0};
            }
            return PlayerAction{botPlayerId, ActionType::Call, currentWager, 0};
        }
        return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
    }

    if (intendedType == ActionType::Raise) {
        double multiplier = profile.getSizing(myEquity);
        int raiseAmount = currentWager + static_cast<int>(pot * multiplier);

        if (raiseAmount > maxAvailable) {
            raiseAmount = maxAvailable;
        }

        ActionType rType = (currentWager == 0) ? ActionType::Bet : ActionType::Raise;
        
        if (state.isActionLegal(botPlayerId, rType, raiseAmount)) {
            return PlayerAction{botPlayerId, rType, raiseAmount, raiseAmount - bot.currentBet};
        }

        // Try clamping to a minimum raise guess. Since getMinRaise is not exposed, 
        // we use a heuristic based on the big blind.
        int fallbackRaise = currentWager + state.getConfig().bigBlind;
        if (fallbackRaise <= maxAvailable && state.isActionLegal(botPlayerId, rType, fallbackRaise)) {
            return PlayerAction{botPlayerId, rType, fallbackRaise, fallbackRaise - bot.currentBet};
        }

        // Try All-In fallback
        if (state.isActionLegal(botPlayerId, rType, maxAvailable)) {
            return PlayerAction{botPlayerId, rType, maxAvailable, maxAvailable - bot.currentBet};
        }

        // Fold/Call fallback if raise is illegal
        intendedType = ActionType::Call;
    }

    if (intendedType == ActionType::Call) {
        if (toCall == 0) {
            if (state.isActionLegal(botPlayerId, ActionType::Check, bot.currentBet)) {
                return PlayerAction{botPlayerId, ActionType::Check, bot.currentBet, 0};
            }
        }
        
        int callAmount = std::min(currentWager, maxAvailable);
        if (state.isActionLegal(botPlayerId, ActionType::Call, callAmount)) {
            return PlayerAction{botPlayerId, ActionType::Call, callAmount, callAmount - bot.currentBet};
        }
        
        return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
    }

    return PlayerAction{botPlayerId, ActionType::Fold, 0, 0};
}

} // namespace AI
} // namespace PokerEngine
