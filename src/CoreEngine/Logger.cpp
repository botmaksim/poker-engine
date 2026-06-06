#include "../../include/CoreEngine/Logger.hpp"

namespace PokerEngine {
    namespace CoreEngine {

        LogLevel Logger::currentLevel = LogLevel::INFO;

        void Logger::log(LogLevel level, const std::string& msg) {
            if (!isEnabled(level)) return;

            std::string prefix;
            switch (level) {
                case LogLevel::TRACE:
                    prefix = "[TRACE] ";
                    break;
                case LogLevel::DEBUG:
                    prefix = "[DEBUG] ";
                    break;
                case LogLevel::INFO:
                    prefix = "[INFO]  ";
                    break;
                case LogLevel::WARN:
                    prefix = "[WARN]  ";
                    break;
                case LogLevel::ERROR:
                    prefix = "[ERROR] ";
                    break;
                default:
                    return;
            }

            // Outputs safely to standard out or standard error depending on
            // severity
            if (level == LogLevel::ERROR) {
                std::cerr << prefix << msg << std::endl;
            } else {
                std::cout << prefix << msg << std::endl;
            }
        }

    }  // namespace CoreEngine
}  // namespace PokerEngine
