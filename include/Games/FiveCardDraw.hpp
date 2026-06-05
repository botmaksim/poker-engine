#pragma once
#include "../Interfaces/IGameRules.hpp"

namespace PokerEngine {
namespace Games {

class FiveCardDraw : public Interfaces::IGameRules {
public:
    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] int getMaxBettingRounds() const override;
    [[nodiscard]] bool hasCommunityCards() const override;
    
    [[nodiscard]] int getStartingHandSize() const override;
    void postMandatoryBets(std::vector<Player>& players, int& pot, std::vector<PlayerAction>& history, const TableConfig& config) const override;
    void dealStreet(std::vector<Player>& players, Hand& boardCards, Hand& deck, int round) const override;
    [[nodiscard]] int evaluateHand(const Hand& holeCards, const Hand& boardCards) const override;
};

}
}
