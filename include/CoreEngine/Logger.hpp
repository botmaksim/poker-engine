#pragma once

#include <iostream>
#include <string>

namespace PokerEngine {
    namespace CoreEngine {

        /**
         * @enum LogLevel
         * @brief Represents the severity level of a log message.
         */
        enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, NONE };

        /**
         * @class Logger
         * @brief High-performance leveled logging system.
         *
         * Provides near-zero overhead logging when a log level is disabled.
         * Evaluates lazily using macro wrappers to guarantee that formatting
         * or object copies do not occur when the target log level is disabled.
         * Time Complexity (Disabled): O(1) mostly branching overhead.
         * Time Complexity (Enabled): O(N) where N is the message length.
         */
        class Logger {
        public:
            static LogLevel currentLevel;

            /**
             * @brief Sets the global runtime log level.
             * @param level The minimum level to output.
             */
            static void setLevel(LogLevel level) { currentLevel = level; }

            /**
             * @brief Checks if a log level is currently enabled.
             * @param level The level to check.
             * @return True if the level is enabled, false otherwise.
             */
            inline static bool isEnabled(LogLevel level) {
                return level >= currentLevel;
            }

            /**
             * @brief Outputs a log message with the specified severity.
             * @param level The severity level.
             * @param msg The message to log.
             */
            static void log(LogLevel level, const std::string& msg);
        };

    }  // namespace CoreEngine
}  // namespace PokerEngine

#define POKER_LOG(level, msg)                                     \
    do {                                                          \
        if (::PokerEngine::CoreEngine::Logger::isEnabled(         \
                ::PokerEngine::CoreEngine::LogLevel::level)) {    \
            ::PokerEngine::CoreEngine::Logger::log(               \
                ::PokerEngine::CoreEngine::LogLevel::level, msg); \
        }                                                         \
    } while (0)

#define LOG_TRACE(msg) POKER_LOG(TRACE, msg)
#define LOG_DEBUG(msg) POKER_LOG(DEBUG, msg)
#define LOG_INFO(msg) POKER_LOG(INFO, msg)
#define LOG_WARN(msg) POKER_LOG(WARN, msg)
#define LOG_ERROR(msg) POKER_LOG(ERROR, msg)
