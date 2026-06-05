#include <iostream>
#include <vector>

#include "../include/Engine/TournamentArchitecture.hpp"

using namespace PokerEngine::Tournament;

int main() {
    std::cout << "=======================================\n";
    std::cout << "PokerEngine Scale Tournament Test\n";
    std::cout << "=======================================\n";
    
    // Создаем координатор турнира (логирование в текущую директорию)
    TournamentCoordinator coordinator("tournament_data", 4); // 4 потока для примера

    std::vector<int> botPool;
    // Генерируем 50 случайных профилей ботов (как запрашивалось)
    for (int i = 0; i < 50; ++i) {
        botPool.push_back(i % 15); // Профили ограничены 15 (5 Action профилей * 3 Sizing профилей)
    }

    // Фаза 1: Round-Robin (каждый с каждым)
    std::cout << "Initiating Phase 1 (Heads-Up Round-Robin)...\n";
    coordinator.executePhase1(botPool);
    
    // Фаза 2: Mass Simulation (Уменьшено до 100 000 для тестового прогона)
    uint64_t targetPhase2Sessions = 100000;
    std::cout << "Initiating Phase 2 (Mass Simulation) with " << targetPhase2Sessions << " sessions...\n";
    // Оцениваем скорость: 100k игр в 4 потока (по 25k на поток) по 15 раздач = 1.5 млн симуляций рук.
    // При производительности ~10-20k раздач/сек это может занять пару минут.
    coordinator.executePhase2(botPool, targetPhase2Sessions);
    
    std::cout << "Tournament fully completed.\n";

    return 0;
}
