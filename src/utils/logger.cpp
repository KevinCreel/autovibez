#include "logger.hpp"
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>
#include <thread>
#include <algorithm>
#include <random>

using namespace AutoVibez::Utils;

Logger::Logger() 
    : current_level_(Level::INFO)
    , output_target_(OutputTarget::CONSOLE)
    , colored_output_(true)
    , max_file_size_(10 * 1024 * 1024) // 10MB
    , max_files_(5)
    , current_file_size_(0)
    , error_counter_(0)
    , total_log_time_us_(0)
    , log_count_(0)
    , start_time_(std::chrono::high_resolution_clock::now()) {
    
    // Initialize log counts
    for (int i = 0; i <= static_cast<int>(Level::FATAL); ++i) {
        log_counts_[static_cast<Level>(i)] = 0;
    }
}

Logger::~Logger() {
    close();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Configuration methods
void Logger::setLogLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

Logger::Level Logger::getLogLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_level_;
}

void Logger::setOutputTarget(OutputTarget target) {
    std::lock_guard<std::mutex> lock(mutex_);
    output_target_ = target;
}

Logger::OutputTarget Logger::getOutputTarget() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return output_target_;
}

void Logger::setLogFilePath(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_file_path_ = path;
    
    // Close existing file if open
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    // Open new file
    if (!path.empty()) {
        std::filesystem::path log_path(path);
        std::filesystem::create_directories(log_path.parent_path());
        log_file_.open(path, std::ios::out | std::ios::app);
        
        current_file_size_ = std::filesystem::exists(path) ? std::filesystem::file_size(path) : 0;
    }
}

std::string Logger::getLogFilePath() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_file_path_;
}

void Logger::setMaxFileSize(size_t size_bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_file_size_ = size_bytes;
}

size_t Logger::getMaxFileSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_file_size_;
}

void Logger::setMaxFiles(size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_files_ = count;
}

size_t Logger::getMaxFiles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_files_;
}

void Logger::setColoredOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    colored_output_ = enabled;
}

bool Logger::isColoredOutputEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return colored_output_;
}

// Structured logging
void Logger::log(Level level, const std::string& message, const LogContext& context) {
    if (level < current_level_) {
        return;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Update statistics
    log_counts_[level]++;
    log_count_++;
    
    std::string formatted_message = formatLogMessage(level, message, context);
    
    // Output to console
    if (output_target_ == OutputTarget::CONSOLE || output_target_ == OutputTarget::BOTH) {
        std::string color_code = colored_output_ ? getColorCode(level) : "";
        std::string reset_code = colored_output_ ? getResetColorCode() : "";
        std::cout << color_code << formatted_message << reset_code << std::endl;
    }
    
    // Output to file
    if (output_target_ == OutputTarget::FILE || output_target_ == OutputTarget::BOTH) {
        std::cerr << "DEBUG: About to write to file: " << formatted_message.substr(0, 50) << "..." << std::endl;
        writeToFile(formatted_message);
    }
    
    // Update performance statistics
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    total_log_time_us_.fetch_add(duration.count()); // Convert to milliseconds
}

void Logger::log(Level level, const char* format, const LogContext& context, ...) {
    if (level < current_level_) {
        return;
    }
    
    // Format the message
    va_list args;
    va_start(args, context);
    
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
        log(level, message, context);
    }
    
    va_end(args);
}

// Convenience methods
void Logger::trace(const std::string& message, const LogContext& context) {
    log(Level::TRACE, message, context);
}

void Logger::debug(const std::string& message, const LogContext& context) {
    log(Level::DEBUG, message, context);
}

void Logger::info(const std::string& message, const LogContext& context) {
    log(Level::INFO, message, context);
}

void Logger::warning(const std::string& message, const LogContext& context) {
    log(Level::WARNING, message, context);
}

void Logger::error(const std::string& message, const LogContext& context) {
    log(Level::ERROR, message, context);
}

void Logger::fatal(const std::string& message, const LogContext& context) {
    log(Level::FATAL, message, context);
}

// Error tracking
std::string Logger::logError(const std::string& error_type, const std::string& message, 
                           const LogContext& context) {
    std::string error_id = generateErrorId();
    ErrorInfo error_info(error_id, error_type, message, context);
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        error_history_[error_id] = error_info;
        error_counter_++;
    }
    
    // Log the error
    log(Level::ERROR, "[" + error_id + "] " + error_type + ": " + message, context);
    
    return error_id;
}

std::string Logger::logErrorWithMetadata(const std::string& error_type, const std::string& message,
                                       const std::unordered_map<std::string, std::string>& metadata,
                                       const LogContext& context) {
    std::string error_id = generateErrorId();
    ErrorInfo error_info(error_id, error_type, message, context);
    error_info.metadata = metadata;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        error_history_[error_id] = error_info;
        error_counter_++;
    }
    
    // Format metadata for logging
    std::string metadata_str;
    for (const auto& [key, value] : metadata) {
        if (!metadata_str.empty()) metadata_str += ", ";
        metadata_str += key + "=" + value;
    }
    
    std::string full_message = "[" + error_id + "] " + error_type + ": " + message;
    if (!metadata_str.empty()) {
        full_message += " {" + metadata_str + "}";
    }
    
    log(Level::ERROR, full_message, context);
    
    return error_id;
}

Logger::ErrorInfo Logger::getErrorInfo(const std::string& error_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = error_history_.find(error_id);
    if (it != error_history_.end()) {
        return it->second;
    }
    return ErrorInfo();
}

std::vector<Logger::ErrorInfo> Logger::getRecentErrors(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ErrorInfo> recent_errors;
    
    // Convert to vector and sort by timestamp
    std::vector<std::pair<std::string, ErrorInfo>> sorted_errors;
    for (const auto& [id, error] : error_history_) {
        sorted_errors.emplace_back(id, error);
    }
    
    std::sort(sorted_errors.begin(), sorted_errors.end(),
              [](const auto& a, const auto& b) {
                  return a.second.timestamp > b.second.timestamp;
              });
    
    // Take the most recent ones
    for (size_t i = 0; i < std::min(count, sorted_errors.size()); ++i) {
        recent_errors.push_back(sorted_errors[i].second);
    }
    
    return recent_errors;
}

void Logger::clearErrorHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    error_history_.clear();
    error_counter_ = 0;
}

// Performance monitoring
void Logger::startTimer(const std::string& timer_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    timers_[timer_name] = std::chrono::high_resolution_clock::now();
}

double Logger::endTimer(const std::string& timer_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = timers_.find(timer_name);
    if (it == timers_.end()) {
        return -1.0; // Timer not found
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - it->second);
    double duration_ms = duration.count() / 1000.0;
    
    timers_.erase(it);
    return duration_ms;
}

void Logger::logPerformance(const std::string& operation, double duration_ms, 
                          const LogContext& context) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << duration_ms;
    std::string message = "Performance: " + operation + " took " + 
                         oss.str() + "ms";
    log(Level::INFO, message, context);
}

// Statistics
size_t Logger::getLogCount(Level level) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = log_counts_.find(level);
    return it != log_counts_.end() ? it->second.load() : 0;
}

size_t Logger::getErrorCount() const {
    return error_counter_.load();
}

double Logger::getAverageLogTime() const {
    size_t count = log_count_.load();
    if (count == 0) return 0.0;
    return static_cast<double>(total_log_time_us_.load()) / (count * 1000.0); // Convert to milliseconds
}

void Logger::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Reset all log counts
    for (auto& pair : log_counts_) {
        pair.second.store(0);
    }
    
    // Reset performance statistics
    total_log_time_us_.store(0);
    log_count_.store(0);
    
    // Reset error tracking
    error_counter_.store(0);
    error_history_.clear();
    
    // Reset timers
    timers_.clear();
    
    // Reset start time
    start_time_ = std::chrono::high_resolution_clock::now();
}

// File management
void Logger::rotateLogFile() {
    if (log_file_path_.empty()) return;
    
    // Close current file
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    // Rotate existing files
    std::filesystem::path log_path(log_file_path_);
    for (int i = max_files_ - 1; i > 0; --i) {
        std::filesystem::path old_file = log_path.parent_path() / 
            (log_path.stem().string() + "." + std::to_string(i) + log_path.extension().string());
        std::filesystem::path new_file = log_path.parent_path() / 
            (log_path.stem().string() + "." + std::to_string(i + 1) + log_path.extension().string());
        
        if (std::filesystem::exists(old_file)) {
            if (i == max_files_ - 1) {
                std::filesystem::remove(old_file);
            } else {
                std::filesystem::rename(old_file, new_file);
            }
        }
    }
    
    // Rename current log file
    if (std::filesystem::exists(log_file_path_)) {
        std::filesystem::path backup_file = log_path.parent_path() / 
            (log_path.stem().string() + ".1" + log_path.extension().string());
        std::filesystem::rename(log_file_path_, backup_file);
    }
    
    // Open new log file
    log_file_.open(log_file_path_, std::ios::app);
    current_file_size_ = 0;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.flush();
    }
}

void Logger::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

// Private methods
std::string Logger::generateErrorId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";
    
    std::string id = "ERR_";
    for (int i = 0; i < 8; ++i) {
        id += hex_chars[dis(gen)];
    }
    return id;
}

std::string Logger::formatLogMessage(Level level, const std::string& message, 
                                   const LogContext& context) const {
    std::ostringstream oss;
    
    // Timestamp
    oss << "[" << formatTimestamp() << "] ";
    
    // Level
    oss << "[" << levelToString(level) << "] ";
    
    // Thread ID
    oss << "[" << getThreadId() << "] ";
    
    // Context (if available)
    if (!context.component.empty() || !context.function.empty()) {
        oss << "[" << context.component << "::" << context.function;
        if (!context.file.empty()) {
            oss << ":" << context.file << ":" << context.line;
        }
        oss << "] ";
    }
    
    // Message
    oss << message;
    
    return oss.str();
}

std::string Logger::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Add milliseconds
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration - seconds);
    oss << "." << std::setfill('0') << std::setw(3) << milliseconds.count();
    
    return oss.str();
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::TRACE:   return "TRACE";
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARN";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

std::string Logger::getColorCode(Level level) const {
    switch (level) {
        case Level::TRACE:   return "\033[90m"; // Gray
        case Level::DEBUG:   return "\033[36m"; // Cyan
        case Level::INFO:    return "\033[32m"; // Green
        case Level::WARNING: return "\033[33m"; // Yellow
        case Level::ERROR:   return "\033[31m"; // Red
        case Level::FATAL:   return "\033[35m"; // Magenta
        default:             return "\033[0m";  // Reset
    }
}

std::string Logger::getResetColorCode() const {
    return "\033[0m";
}

void Logger::writeToFile(const std::string& message) {
    if (!log_file_.is_open()) {
        std::cerr << "DEBUG: File not open in writeToFile" << std::endl;
        return;
    }
    
    if (!log_file_.good()) {
        std::cerr << "DEBUG: File stream not good in writeToFile" << std::endl;
        return;
    }
    
    log_file_ << message << std::endl;
    log_file_.flush(); // Ensure the data is written to disk
    
    if (!log_file_.good()) {
        std::cerr << "DEBUG: File stream not good after write" << std::endl;
    }
    
    current_file_size_ += message.length() + 1; // +1 for newline
    
    checkFileRotation();
}

void Logger::checkFileRotation() {
    if (current_file_size_ >= max_file_size_) {
        rotateLogFile();
    }
}

std::string Logger::getThreadId() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
} 