#include "logger.hpp"
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector> // Added for std::vector

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

Logger::Level Logger::getLogLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_level_;
}

void Logger::setColoredOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    colored_output_ = enabled;
}

bool Logger::isColoredOutputEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return colored_output_;
}

void Logger::log(Level level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    
    std::string color_code = colored_output_ ? getColorCode(level) : "";
    std::string reset_code = colored_output_ ? getResetColorCode() : "";
    
    std::cout << color_code 
              << "[" << oss.str() << "] "
              << "[" << levelToString(level) << "] "
              << message 
              << reset_code << std::endl;
}

void Logger::log(Level level, const char* format, ...) {
    if (level < current_level_) {
        return;
    }
    
    // Format the message
    va_list args;
    va_start(args, format);
    
    // Get the required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size > 0) {
        std::vector<char> buffer(size + 1);
        vsnprintf(buffer.data(), buffer.size(), format, args);
        std::string message(buffer.data());
        
        // Use the existing log method
        log(level, message);
    }
    
    va_end(args);
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(Level::DEBUG, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(Level::INFO, format, args);
    va_end(args);
}

void Logger::warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(Level::WARNING, format, args);
    va_end(args);
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(Level::ERROR, format, args);
    va_end(args);
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARN";
        case Level::ERROR:   return "ERROR";
        default:             return "UNKNOWN";
    }
}

std::string Logger::getColorCode(Level level) const {
    switch (level) {
        case Level::DEBUG:   return "\033[36m"; // Cyan
        case Level::INFO:    return "\033[32m"; // Green
        case Level::WARNING: return "\033[33m"; // Yellow
        case Level::ERROR:   return "\033[31m"; // Red
        default:             return "\033[0m";  // Reset
    }
}

std::string Logger::getResetColorCode() const {
    return "\033[0m";
} 