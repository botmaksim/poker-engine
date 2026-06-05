#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <thread>
#include <atomic>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>

#include "GameState.hpp"
#include "../AI/BotFactory.hpp"

namespace PokerEngine {
namespace Tournament {

// -----------------------------------------------------------------------------
// Структуры данных для логирования (оптимизировано под кэш-линии и сериализацию)
// -----------------------------------------------------------------------------

#pragma pack(push, 1)

/**
 * @brief Структура для логирования действий, оптимизирована для сохранения в бинарный формат.
 */
struct ActionLogRecord {
    uint64_t sessionID;      ///< Уникальный ID сессии
    uint32_t handID;         ///< Номер раздачи в рамках сессии
    uint32_t playerID;       ///< ID игрока
    uint16_t botProfileID;   ///< Идентификатор профиля бота
    uint8_t street;          ///< Улица (0: PreFlop, 1: Flop, 2: Turn, 3: River)
    uint8_t position;        ///< Позиция игрока за столом (0: BTN, 1: SB, 2: BB, ...)
    uint32_t stackBefore;    ///< Стек игрока до совершения действия
    uint8_t actionType;      ///< Тип действия (Fold, Check, Call, Bet, Raise)
    uint32_t actionAmount;   ///< Размер действия (для Bet/Raise/Call)
    float spr;               ///< Stack-to-Pot Ratio на момент решения
};

/**
 * @brief Структура для логирования метрик игрока по итогам раздачи.
 */
struct PlayerMetricRecord {
    uint64_t sessionID;         ///< Уникальный ID сессии
    uint32_t handID;            ///< Номер раздачи
    uint32_t playerID;          ///< ID игрока
    uint64_t holeCardsMask;     ///< Маска карманных карт (bitmask)
    float botEstimatedEquity;   ///< Оценка вероятности на победу, сделанная ботом
    float objectiveEquity;      ///< Подлинное (математическое) эквити с открытыми картами
    float evLoss;               ///< Mistake Analysis: потеря EV относительно идеального действия
    float exploitability;       ///< Exploitability Metric
    float vpip;                 ///< Voluntarily Put In Pot за сессию
    float pfr;                  ///< Preflop Raise за сессию
    float aggressionFactor;     ///< Взвешенный Aggression Factor
    float tiltIndex;            ///< Reaction to variance (отклонение агрессии после проигрыша)
};

#pragma pack(pop)

// -----------------------------------------------------------------------------
// Статистика сессии
// -----------------------------------------------------------------------------

struct PlayerSessionStats {
    int handsPlayed = 0;
    int voluntarilyPutInPot = 0;
    int preflopRaises = 0;
    int aggressiveActions = 0;
    int passiveActions = 0;
    int consecutiveLosses = 0;
    
    bool hasVpipThisHand = false;
    bool hasPfrThisHand = false;
    uint32_t startingStack = 0;

    float getVPIP() const { return handsPlayed > 0 ? (float)voluntarilyPutInPot / handsPlayed : 0.0f; }
    float getPFR() const { return handsPlayed > 0 ? (float)preflopRaises / handsPlayed : 0.0f; }
    float getAF() const { return passiveActions > 0 ? (float)aggressiveActions / passiveActions : (float)aggressiveActions; }
    float getTiltIndex() const { return consecutiveLosses >= 3 ? getAF() * 1.5f : getAF(); } // Примерный коэффициент тильта
};

// -----------------------------------------------------------------------------
// Высокоскоростной асинхронный логгер
// (Инкапсулирует батчинг и запись в столбцовый формат)
// -----------------------------------------------------------------------------

/**
 * @brief Класс для высокоскоростного логирования в фоновом потоке, не блокирующего основной поток симуляции.
 */
class AsyncTournamentLogger {
public:
    /**
     * @brief Конструктор логгера. Запускает фоновый поток.
     * @param outputPath Префикс/директория для файлов лога.
     */
    AsyncTournamentLogger(const std::string& outputPath);
    
    /**
     * @brief Деструктор логгера. Обеспечивает сброс данных и корректное завершение потока.
     */
    ~AsyncTournamentLogger();

    /**
     * @brief Отправляет запись о действии в асинхронный буфер.
     * @param record Заполненная структура ActionLogRecord
     */
    void logAction(const ActionLogRecord& record);

    /**
     * @brief Отправляет запись о метрике в асинхронный буфер.
     * @param record Заполненная структура PlayerMetricRecord
     */
    void logMetric(const PlayerMetricRecord& record);

    /**
     * @brief Принудительно сбрасывает все буферизованные данные на диск.
     */
    void flush();

private:
    void ioWorkerLoop();

    std::string outputPath;

    // Быстрая lock-free очередь (в реальном проекте предпочтительно moodycamel::ConcurrentQueue)
    // Здесь упрощаем для примера архитектуры - используем double-buffering.
    std::vector<ActionLogRecord> actionBufferA, actionBufferB;
    std::vector<PlayerMetricRecord> metricBufferA, metricBufferB;
    
    std::atomic<bool> usingBufferA{true};
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> isRunning{true};
    std::thread backgroundThread;
};

// -----------------------------------------------------------------------------
// Классы Турнира (Фазы 1 и 2)
// -----------------------------------------------------------------------------

/**
 * @brief Представляет отдельную игровую сессию между ботами.
 */
class TournamentSession {
public:
    /**
     * @brief Конструктор сессии.
     * @param id Уникальный идентификатор сессии.
     * @param logger Ссылка на общий логгер для записи событий.
     */
    TournamentSession(uint64_t id, AsyncTournamentLogger& logger);
    
    /**
     * @brief Запускает дуэльный матч Heads-Up
     * @param bot1ProfileID Профиль бота 1
     * @param bot2ProfileID Профиль бота 2
     * @param numHands Количество раздач
     */
    void runHeadsUpMatch(int bot1ProfileID, int bot2ProfileID, int numHands);
    
    /**
     * @brief Запускает Mass Simulation (например, 5-max столы)
     * @param maxHands Количество раздач для симуляции на столе
     * @param botPool Доступный пул профилей ботов
     */
    void runMassSimulation(int maxHands, const std::vector<int>& botPool);

private:
    void executeHand(Engine::GameState& state, std::vector<int>& activeBotProfiles, uint32_t handID);
    void computeMetricsAndLog(Engine::GameState& state, const std::vector<int>& activeBotProfiles, uint32_t handID);
    
    uint64_t sessionID;
    AsyncTournamentLogger& logger;
    std::map<int, PlayerSessionStats> sessionStats;
};

/**
 * @brief Координатор всего турнира, управляет потоками и фазами.
 */
class TournamentCoordinator {
public:
    /**
     * @brief Создает координатор.
     * @param outputDir Директория для сохранения логов (Parquet/CSV)
     * @param numThreads Количество рабочих потоков (по умолчанию равно числу ядер)
     */
    TournamentCoordinator(const std::string& outputDir, int numThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief Запускает Фазу 1 (Round-Robin для всех пар ботов).
     * @param botPool Список ID профилей ботов, участвующих в турнире.
     */
    void executePhase1(const std::vector<int>& botPool);
    
    /**
     * @brief Запускает Фазу 2 (Массовые сессии).
     * @param botPool Доступный пул ботов.
     * @param totalMassSessions Количество независимых сессий для запуска (по умолчанию 100K).
     */
    void executePhase2(const std::vector<int>& botPool, uint64_t totalMassSessions = 100000);

private:
    std::unique_ptr<AsyncTournamentLogger> globalLogger;
    int workerThreadCount;
};

} // namespace Tournament
} // namespace PokerEngine
