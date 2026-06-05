#include "../../include/Engine/TournamentArchitecture.hpp"
#include "../../include/Games/TexasHoldem.hpp"
#include "../../include/AI/InferenceEngine.hpp"
#include "../../include/AI/RangeParser.hpp"
#include "../../include/Engine/Analyzer.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

std::mutex consoleMutex;

namespace PokerEngine {
namespace Tournament {

// -----------------------------------------------------------------------------
// AsyncTournamentLogger
// -----------------------------------------------------------------------------

AsyncTournamentLogger::AsyncTournamentLogger(const std::string& path) : outputPath(path) {
    actionBufferA.reserve(100000);
    actionBufferB.reserve(100000);
    metricBufferA.reserve(100000);
    metricBufferB.reserve(100000);
    
    backgroundThread = std::thread(&AsyncTournamentLogger::ioWorkerLoop, this);
}

AsyncTournamentLogger::~AsyncTournamentLogger() {
    isRunning = false;
    cv.notify_one();
    if (backgroundThread.joinable()) {
        backgroundThread.join();
    }
}

void AsyncTournamentLogger::logAction(const ActionLogRecord& record) {
    std::lock_guard<std::mutex> lock(mtx);
    if (usingBufferA) {
        actionBufferA.push_back(record);
    } else {
        actionBufferB.push_back(record);
    }
    if (actionBufferA.size() > 50000 || actionBufferB.size() > 50000) {
        cv.notify_one();
    }
}

void AsyncTournamentLogger::logMetric(const PlayerMetricRecord& record) {
    std::lock_guard<std::mutex> lock(mtx);
    if (usingBufferA) {
        metricBufferA.push_back(record);
    } else {
        metricBufferB.push_back(record);
    }
}

void AsyncTournamentLogger::flush() {
    std::lock_guard<std::mutex> lock(mtx);
    cv.notify_one();
}

void AsyncTournamentLogger::ioWorkerLoop() {
    std::ofstream actionFile(outputPath + "_actions.csv", std::ios::app);
    std::ofstream metricFile(outputPath + "_metrics.csv", std::ios::app);
    
    // В реальном проекте здесь будет интеграция с Parquet / ClickHouse
    // Для демо используем CSV. Пишем заголовки, если файлы пустые
    actionFile << "SessionID,HandID,PlayerID,BotProfileID,Street,Position,StackBefore,ActionType,ActionAmount,SPR\n";
    metricFile << "SessionID,HandID,PlayerID,HoleCardsMask,BotEstEquity,ObjEquity,EVLoss,Exploitability,VPIP,PFR,AggrFactor,TiltIndex\n";

    while (isRunning) {
        std::vector<ActionLogRecord> localActions;
        std::vector<PlayerMetricRecord> localMetrics;
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::milliseconds(1000), [this] {
                return !isRunning || (!usingBufferA && !actionBufferB.empty()) || (usingBufferA && !actionBufferA.empty());
            });
            
            if (usingBufferA) {
                localActions.swap(actionBufferA);
                localMetrics.swap(metricBufferA);
                usingBufferA = false;
            } else {
                localActions.swap(actionBufferB);
                localMetrics.swap(metricBufferB);
                usingBufferA = true;
            }
        }
        
        // Массовая запись в файл
        for (const auto& a : localActions) {
            actionFile << a.sessionID << "," << a.handID << "," << a.playerID << "," << a.botProfileID << ","
                       << (int)a.street << "," << (int)a.position << "," << a.stackBefore << "," 
                       << (int)a.actionType << "," << a.actionAmount << "," << a.spr << "\n";
        }
        
        for (const auto& m : localMetrics) {
            metricFile << m.sessionID << "," << m.handID << "," << m.playerID << "," << m.holeCardsMask << ","
                       << m.botEstimatedEquity << "," << m.objectiveEquity << "," << m.evLoss << "," 
                       << m.exploitability << "," << m.vpip << "," << m.pfr << "," << m.aggressionFactor << "," 
                       << m.tiltIndex << "\n";
        }
    }
}

// -----------------------------------------------------------------------------
// TournamentSession
// -----------------------------------------------------------------------------

TournamentSession::TournamentSession(uint64_t id, AsyncTournamentLogger& logger) : sessionID(id), logger(logger) {}

void TournamentSession::runHeadsUpMatch(int bot1ProfileID, int bot2ProfileID, int numHands) {
    auto game = std::make_unique<Games::TexasHoldem>();
    TableConfig config{LimitType::NoLimit, 5, 10, 0, 0, 0};
    Engine::GameState state(config, std::move(game));
    
    state.addPlayer(1, 10000);
    state.addPlayer(2, 10000);
    
    std::vector<int> activeBotProfiles = {bot1ProfileID, bot2ProfileID};

    for (int i = 0; i < numHands; ++i) {
        state.startHand();
        executeHand(state, activeBotProfiles, i);
        computeMetricsAndLog(state, activeBotProfiles, i);
    }
}

void TournamentSession::runMassSimulation(int maxHands, const std::vector<int>& botPool) {
    auto game = std::make_unique<Games::TexasHoldem>();
    TableConfig config{LimitType::NoLimit, 5, 10, 0, 0, 0};
    Engine::GameState state(config, std::move(game));
    
    std::vector<int> activeBotProfiles;

    // Начальные 5 игроков
    for (int i = 1; i <= 5; ++i) {
        state.addPlayer(i, 10000);
        activeBotProfiles.push_back(botPool[rand() % botPool.size()]);
    }

    for (int i = 0; i < maxHands; ++i) {
        // Правило турнира: Каждые 15 раздач меняется бот
        if (i > 0 && i % 15 == 0) {
            int swapIdx = rand() % 5;
            activeBotProfiles[swapIdx] = botPool[rand() % botPool.size()];
        }
    
        state.startHand();
        executeHand(state, activeBotProfiles, i);
        computeMetricsAndLog(state, activeBotProfiles, i);
    }
}

void TournamentSession::executeHand(Engine::GameState& state, std::vector<int>& activeBotProfiles, uint32_t handID) {
    for (auto& p : state.getPlayers()) {
        sessionStats[p.id].hasVpipThisHand = false;
        sessionStats[p.id].hasPfrThisHand = false;
        sessionStats[p.id].startingStack = p.chipCount;
        sessionStats[p.id].handsPlayed++;
    }

    std::map<int, AI::Range> oppRanges;
    for (const auto& player : state.getPlayers()) {
        AI::Range r;
        AI::RangeParser::parseHoldemRange(r, "22+, A2s+, K2s+, Q2s+, J2s+, T2s+, 92s+, 82s+, A2o+, K2o+, Q2o+, J2o+, T2o+");
        oppRanges[player.id] = r;
    }

    while (state.getCurrentBettingRound() <= state.getRules()->getMaxBettingRounds()) {
        int active = 0;
        for (auto& p : state.getPlayers()) if (!p.hasFolded) active++;
        if (active <= 1) break;

        for (size_t i = 0; i < state.getPlayers().size(); ++i) {
            const auto& player = state.getPlayers()[i];
            if (player.hasFolded || player.isAllIn) continue;
            
            std::map<int, AI::Range> view = oppRanges;
            view.erase(player.id);
            
            // Получить профиль ID и создать бота
            int profileId = activeBotProfiles[i];
            
            // В рамках фабрики сейчас нужны ActionProfile / SizingProfile:
            AI::ActionProfile aProf = static_cast<AI::ActionProfile>(profileId % 5);
            AI::SizingProfile sProf = static_cast<AI::SizingProfile>((profileId / 5) % 3);
            auto bot = AI::BotFactory::createBot(aProf, sProf, profileId);
            
            PlayerAction act = AI::InferenceEngine::decideAction(state, player.id, *bot, view);
            state.recordAction(player.id, act.type, act.absoluteAmount);
            
            uint8_t street = state.getCurrentBettingRound();

            // Tracking metrics
            if (act.type == ActionType::Call || act.type == ActionType::Bet || act.type == ActionType::Raise) {
                if (!sessionStats[player.id].hasVpipThisHand) {
                    sessionStats[player.id].voluntarilyPutInPot++;
                    sessionStats[player.id].hasVpipThisHand = true;
                }
            }
            if (street == 0 && act.type == ActionType::Raise && !sessionStats[player.id].hasPfrThisHand) {
                sessionStats[player.id].preflopRaises++;
                sessionStats[player.id].hasPfrThisHand = true;
            }
            if (act.type == ActionType::Bet || act.type == ActionType::Raise) {
                sessionStats[player.id].aggressiveActions++;
            } else if (act.type == ActionType::Call) {
                sessionStats[player.id].passiveActions++;
            }

            ActionLogRecord logReq;
            logReq.sessionID = sessionID;
            logReq.handID = handID; 
            logReq.playerID = player.id;
            logReq.botProfileID = profileId;
            logReq.street = street;
            logReq.position = i;
            logReq.stackBefore = player.chipCount;
            logReq.actionType = static_cast<uint8_t>(act.type);
            logReq.actionAmount = act.absoluteAmount;
            logReq.spr = (float)player.chipCount / (state.getPot() > 0 ? state.getPot() : 1);
            
            logger.logAction(logReq);
        }
        
        state.advanceRound();
    }
}

void TournamentSession::computeMetricsAndLog(Engine::GameState& state, const std::vector<int>& activeBotProfiles, uint32_t handID) {
    // 1. Calculate actual objective equity via Analyzer (50 iterations for speed)
    std::map<int, AI::Range> emptyRanges;
    std::vector<double> objEquities = Engine::Analyzer::calculateEquity(state, emptyRanges, 50);

    for (size_t i = 0; i < state.getPlayers().size(); ++i) {
        auto& p = state.getPlayers()[i];
        int pid = p.id;

        // Tilt tracking
        if (p.chipCount < sessionStats[pid].startingStack) {
            sessionStats[pid].consecutiveLosses++;
        } else if (p.chipCount > sessionStats[pid].startingStack) {
            sessionStats[pid].consecutiveLosses = 0;
        }

        uint64_t mask = 0;
        for (auto c : p.faceDownCards) mask |= c;

        float objEq = (i < objEquities.size()) ? objEquities[i] : 0.0f;
        float evLoss = (!p.hasFolded) ? 0.0f : (objEq > 0.5f ? (objEq * state.getPot()) : 0.0f);
        
        PlayerMetricRecord metric;
        metric.sessionID = sessionID;
        metric.handID = handID;
        metric.playerID = pid;
        metric.holeCardsMask = mask;
        metric.botEstimatedEquity = objEq * 0.98f; // Proxy for bot's internal tracking
        metric.objectiveEquity = objEq;
        metric.evLoss = evLoss;
        metric.exploitability = (evLoss > 5.0f) ? evLoss * 0.1f : 0.0f;
        metric.vpip = sessionStats[pid].getVPIP();
        metric.pfr = sessionStats[pid].getPFR();
        metric.aggressionFactor = sessionStats[pid].getAF();
        metric.tiltIndex = sessionStats[pid].getTiltIndex();
        logger.logMetric(metric);
    }
}

// -----------------------------------------------------------------------------
// TournamentCoordinator
// -----------------------------------------------------------------------------

TournamentCoordinator::TournamentCoordinator(const std::string& outputDir, int numThreads) 
    : workerThreadCount(numThreads) {
    globalLogger = std::make_unique<AsyncTournamentLogger>(outputDir);
}

void TournamentCoordinator::executePhase1(const std::vector<int>& botPool) {
    std::cout << "[Phase 1] Round-Robin Started on " << workerThreadCount << " threads\n";
    
    std::vector<std::pair<int, int>> pairs;
    for (size_t i = 0; i < botPool.size(); ++i) {
        for (size_t j = i + 1; j < botPool.size(); ++j) {
            pairs.push_back({botPool[i], botPool[j]});
        }
    }
    
    std::atomic<size_t> pairIdx{0};
    std::atomic<size_t> completed{0};
    size_t totalPairs = pairs.size();

    std::vector<std::thread> workers;
    for (int t = 0; t < workerThreadCount; ++t) {
        workers.emplace_back([this, &pairs, &pairIdx, &completed, totalPairs]() {
            while (true) {
                size_t idx = pairIdx.fetch_add(1);
                if (idx >= pairs.size()) break;
                
                TournamentSession session(idx, *globalLogger);
                session.runHeadsUpMatch(pairs[idx].first, pairs[idx].second, 10); // 10 раздач по условиям
                
                size_t c = completed.fetch_add(1) + 1;
                if (c % 100 == 0 || c == totalPairs) {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "[Phase 1] Progress: " << c << " / " << totalPairs << " match-ups completed.\n";
                }
            }
        });
    }
    
    for (auto& w : workers) w.join();
    globalLogger->flush();
    std::cout << "[Phase 1] Completed.\n";
}

void TournamentCoordinator::executePhase2(const std::vector<int>& botPool, uint64_t totalMassSessions) {
    std::cout << "[Phase 2] Mass Simulation Started (" << totalMassSessions << " sessions)\n";
    std::atomic<uint64_t> sessionCounter{0};
    std::atomic<uint64_t> completedPhase2{0};
    std::vector<std::thread> workers;
    
    for (int t = 0; t < workerThreadCount; ++t) {
        workers.emplace_back([this, &botPool, &sessionCounter, &completedPhase2, totalMassSessions]() {
            while (true) {
                uint64_t idx = sessionCounter.fetch_add(1);
                if (idx >= totalMassSessions) break;
                
                TournamentSession session(1000000 + idx, *globalLogger);
                session.runMassSimulation(15, botPool); // 15 раздач до замены

                uint64_t c = completedPhase2.fetch_add(1) + 1;
                if (c % (totalMassSessions / 10 + 1) == 0 || c == totalMassSessions) {
                    float pct = (float)c / totalMassSessions * 100.0f;
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "[Phase 2] Progress: " << c << " / " << totalMassSessions 
                              << " mass sessions completed (" << std::fixed << std::setprecision(1) << pct << "%).\n";
                }
            }
        });
    }
    
    for (auto& w : workers) w.join();
    globalLogger->flush();
    std::cout << "[Phase 2] Completed.\n";
}

} // namespace Tournament
} // namespace PokerEngine
