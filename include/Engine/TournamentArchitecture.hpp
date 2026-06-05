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
// Data structures for logging (optimized for cache lines and serialization)
// -----------------------------------------------------------------------------

#pragma pack(push, 1)

enum class GameVariant {
    TexasHoldem,
    Omaha,
    ShortDeck,
    SevenCardStud,
    Razz,
    FiveCardDraw,
    DeuceToSevenTripleDraw
};

/**
 * @brief Structure for logging actions, optimized for binary storage.
 */
struct ActionLogRecord {
    uint64_t sessionID;      ///< Unique session ID
    uint32_t handID;         ///< Hand ID within the session
    uint32_t playerID;       ///< Player ID
    uint16_t botProfileID;   ///< Bot profile ID
    uint8_t street;          ///< Street (0: PreFlop, 1: Flop, 2: Turn, 3: River)
    uint8_t position;        ///< Player position at the table (0: BTN, 1: SB, 2: BB, ...)
    uint32_t stackBefore;    ///< Player stack before taking the action
    uint8_t actionType;      ///< Action type (Fold, Check, Call, Bet, Raise)
    uint32_t actionAmount;   ///< Action amount (for Bet/Raise/Call)
    float spr;               ///< Stack-to-Pot Ratio at the time of decision
};

/**
 * @brief Structure for logging player metrics after a hand.
 */
struct PlayerMetricRecord {
    uint64_t sessionID;         ///< Unique session ID
    uint32_t handID;            ///< Hand ID
    uint32_t playerID;          ///< Player ID
    uint64_t holeCardsMask;     ///< Hole cards mask (bitmask)
    float botEstimatedEquity;   ///< Win probability estimated by the bot
    float objectiveEquity;      ///< True (mathematical) equity with known cards
    float evLoss;               ///< Mistake Analysis: EV loss relative to perfect action
    float exploitability;       ///< Exploitability Metric
    float vpip;                 ///< Voluntarily Put In Pot per session
    float pfr;                  ///< Preflop Raise per session
    float aggressionFactor;     ///< Weighted Aggression Factor
    float tiltIndex;            ///< Reaction to variance (aggression deviation after loss)
};

#pragma pack(pop)

// -----------------------------------------------------------------------------
// Session Statistics
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
    float getTiltIndex() const { return consecutiveLosses >= 3 ? getAF() * 1.5f : getAF(); } // Approximate tilt index
};

// -----------------------------------------------------------------------------
// High-speed Asynchronous Logger
// (Encapsulates batching and columnar format writing)
// -----------------------------------------------------------------------------

/**
 * @brief Class for high-speed background tracking, does not block the main simulation thread.
 */
class AsyncTournamentLogger {
public:
    /**
     * @brief Constructor. Starts the background thread.
     * @param outputPath Prefix/directory for the log files.
     */
    AsyncTournamentLogger(const std::string& outputPath);
    
    /**
     * @brief Destructor. Flushes data and cleanly terminates the thread.
     */
    ~AsyncTournamentLogger();

    /**
     * @brief Pushes an action record to the asynchronous buffer.
     * @param record Populated ActionLogRecord structure
     */
    void logAction(const ActionLogRecord& record);

    /**
     * @brief Pushes a metric record to the asynchronous buffer.
     * @param record Populated PlayerMetricRecord structure
     */
    void logMetric(const PlayerMetricRecord& record);

    /**
     * @brief Forces a flush of all buffered data to disk.
     */
    void flush();

private:
    void ioWorkerLoop();

    std::string outputPath;

    // Fast lock-free queue (in a real project, consider moodycamel::ConcurrentQueue)
    // Simplified double-buffering architecture for demonstration.
    std::vector<ActionLogRecord> actionBufferA, actionBufferB;
    std::vector<PlayerMetricRecord> metricBufferA, metricBufferB;
    
    std::atomic<bool> usingBufferA{true};
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> isRunning{true};
    std::thread backgroundThread;
};

// -----------------------------------------------------------------------------
// Tournament Classes (Phase 1 and 2)
// -----------------------------------------------------------------------------

/**
 * @brief Represents an individual game session between bots.
 */
class TournamentSession {
public:
    /**
     * @brief Session constructor.
     * @param id Unique session identifier.
     * @param logger Reference to shared logger for tracking events.
     */
    TournamentSession(uint64_t id, AsyncTournamentLogger& logger);
    
    /**
     * @brief Starts a Heads-Up match
     * @param bot1ProfileID Bot 1 profile ID
     * @param bot2ProfileID Bot 2 profile ID
     * @param numHands Number of hands
     * @param variant Poker variant
     */
    void runHeadsUpMatch(int bot1ProfileID, int bot2ProfileID, int numHands, GameVariant variant = GameVariant::TexasHoldem);
    
    /**
     * @brief Starts a Mass Simulation (e.g., 5-max tables)
     * @param maxHands Number of hands to simulate at table
     * @param botPool Available bot profiles
     * @param variant Poker variant
     */
    void runMassSimulation(int maxHands, const std::vector<int>& botPool, GameVariant variant = GameVariant::TexasHoldem);

private:
    void executeHand(Engine::GameState& state, std::vector<int>& activeBotProfiles, uint32_t handID);
    void computeMetricsAndLog(Engine::GameState& state, const std::vector<int>& activeBotProfiles, uint32_t handID);
    
    uint64_t sessionID;
    AsyncTournamentLogger& logger;
    std::map<int, PlayerSessionStats> sessionStats;
};

/**
 * @brief Overall tournament coordinator, controls threads and simulation phases.
 */
class TournamentCoordinator {
public:
    /**
     * @brief Instantiates the coordinator.
     * @param outputDir Directory to save logs (Parquet/CSV)
     * @param numThreads Number of worker threads (defaults to hardware cores)
     */
    TournamentCoordinator(const std::string& outputDir, int numThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief Executes Phase 1 (Round-Robin matches across bot combinations).
     * @param botPool List of bot profile IDs entering the tournament
     * @param variant Poker variant
     */
    void executePhase1(const std::vector<int>& botPool, GameVariant variant = GameVariant::TexasHoldem);
    
    /**
     * @brief Executes Phase 2 (Mass sessions tracking bot pools at random).
     * @param botPool Discovered/available pool of tournament bots
     * @param totalMassSessions Number of independent mass sessions to execute
     * @param variant Poker variant
     */
    void executePhase2(const std::vector<int>& botPool, uint64_t totalMassSessions = 100000, GameVariant variant = GameVariant::TexasHoldem);

private:
    std::unique_ptr<AsyncTournamentLogger> globalLogger;
    int workerThreadCount;
};

} // namespace Tournament
} // namespace PokerEngine
