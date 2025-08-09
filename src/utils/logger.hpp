#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "error_handler.hpp"

namespace AutoVibez::Utils {

/**
 * @brief Centralized logging system with multiple log levels
 *
 * Extends ErrorHandler to provide thread-safe logging to filesystem with timestamping.
 * Uses PathManager for cross-platform directory structures.
 * All setError() calls are automatically logged to file.
 */
class Logger : public ErrorHandler {
public:
    enum class LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

    Logger();
    explicit Logger(bool enableFileLogging);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // Multiple log levels
    void logInfo(const std::string& message);
    void logDebug(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);

    // Override ErrorHandler to add logging functionality (public for easy access)
    void setError(const std::string& message);

    // Log level configuration
    void setMinLogLevel(LogLevel level);
    LogLevel getMinLogLevel() const;

private:
    void writeToLogFile(LogLevel level, const std::string& message);
    void initializeLogFile();
    std::string formatLogEntry(LogLevel level, const std::string& message) const;
    std::string getCurrentTimestamp() const;
    std::string levelToString(LogLevel level) const;

    // Thread safety
    mutable std::mutex logMutex_;

    // File handling
    std::unique_ptr<std::ofstream> logFile_;
    std::string logFilePath_;
    bool fileLoggingEnabled_;

    // Log level filtering
    LogLevel minLogLevel_ = LogLevel::INFO;  // Default: INFO and above
};

// Convenience macros for better logging experience
#define LOG_INFO(logger, msg) logger.logInfo(msg)
#define LOG_DEBUG(logger, msg) logger.logDebug(msg)
#define LOG_WARNING(logger, msg) logger.logWarning(msg)
#define LOG_ERROR(logger, msg) logger.logError(msg)

}  // namespace AutoVibez::Utils
