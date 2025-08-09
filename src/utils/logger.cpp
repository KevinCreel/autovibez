#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "platform/path_manager.hpp"

namespace AutoVibez::Utils {

Logger::Logger() : fileLoggingEnabled_(true) {
    initializeLogFile();
}

Logger::Logger(bool enableFileLogging) : fileLoggingEnabled_(enableFileLogging) {
    if (fileLoggingEnabled_) {
        initializeLogFile();
    }
}

Logger::~Logger() {
    if (logFile_ && logFile_->is_open()) {
        logFile_->close();
    }
}

void Logger::logInfo(const std::string& message) {
    writeToLogFile(LogLevel::INFO, message);
}

void Logger::logDebug(const std::string& message) {
    writeToLogFile(LogLevel::DEBUG, message);
}

void Logger::logWarning(const std::string& message) {
    writeToLogFile(LogLevel::WARNING, message);
}

void Logger::logError(const std::string& message) {
    // Set error state like ErrorHandler would
    ErrorHandler::setError(message);
    // Also log to file
    writeToLogFile(LogLevel::ERROR, message);
}

void Logger::setError(const std::string& message) {
    // Call parent ErrorHandler to maintain state
    ErrorHandler::setError(message);
    // Add logging functionality
    writeToLogFile(LogLevel::ERROR, message);
}

void Logger::writeToLogFile(LogLevel level, const std::string& message) {
    // Early return if file logging is disabled or log level is below minimum
    if (!fileLoggingEnabled_ || level < minLogLevel_) {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex_);

    if (!logFile_ || !logFile_->is_open()) {
        return;
    }

    const std::string logEntry = formatLogEntry(level, message);

    *logFile_ << logEntry << std::endl;
    // Only flush for ERROR level for immediate visibility of critical issues
    if (level >= LogLevel::ERROR) {
        logFile_->flush();
    }
}

void Logger::initializeLogFile() {
    try {
        // Use PathManager for cross-platform directory structure
        const std::string stateDir = PathManager::getStateDirectory();
        PathManager::ensureDirectoryExists(stateDir);

        logFilePath_ = stateDir + "/autovibez.log";

        logFile_ = std::make_unique<std::ofstream>(logFilePath_, std::ios::app);

        if (logFile_->is_open()) {
            // Log startup message
            const std::string startupEntry = formatLogEntry(LogLevel::INFO, "Logger initialized");
            *logFile_ << startupEntry << std::endl;
            logFile_->flush();
        }
    } catch (const std::exception& e) {
        // Fallback: log to stderr if file logging fails
        std::cerr << "Failed to initialize log file: " << e.what() << std::endl;
    }
}

std::string Logger::formatLogEntry(LogLevel level, const std::string& message) const {
    std::stringstream ss;
    ss << "[" << getCurrentTimestamp() << "] ";
    ss << levelToString(level) << ": ";
    ss << message;
    return ss.str();
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void Logger::setMinLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex_);
    minLogLevel_ = level;
}

Logger::LogLevel Logger::getMinLogLevel() const {
    std::lock_guard<std::mutex> lock(logMutex_);
    return minLogLevel_;
}

}  // namespace AutoVibez::Utils
