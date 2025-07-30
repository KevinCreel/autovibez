#ifndef logger_hpp
#define logger_hpp

#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <atomic>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Comprehensive error logging system with structured logging, file output, and performance monitoring
 */
class Logger {
public:
    enum class Level {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5
    };

    enum class OutputTarget {
        CONSOLE = 1,
        FILE = 2,
        BOTH = 3
    };

    struct LogContext {
        std::string component;
        std::string function;
        std::string file;
        int line;
        std::string thread_id;
        
        LogContext() : line(0) {}
        LogContext(const std::string& comp, const std::string& func) 
            : component(comp), function(func), line(0) {}
        LogContext(const std::string& comp, const std::string& func, 
                  const std::string& f, int l) 
            : component(comp), function(func), file(f), line(l) {}
    };

    struct ErrorInfo {
        std::string error_id;
        std::string error_type;
        std::string message;
        std::string stack_trace;
        LogContext context;
        std::chrono::system_clock::time_point timestamp;
        std::unordered_map<std::string, std::string> metadata;
        
        ErrorInfo() {}
        ErrorInfo(const std::string& id, const std::string& type, 
                 const std::string& msg, const LogContext& ctx)
            : error_id(id), error_type(type), message(msg), context(ctx),
              timestamp(std::chrono::system_clock::now()) {}
    };

    static Logger& getInstance();
    
    // Configuration
    void setLogLevel(Level level);
    Level getLogLevel() const;
    void setOutputTarget(OutputTarget target);
    OutputTarget getOutputTarget() const;
    void setLogFilePath(const std::string& path);
    std::string getLogFilePath() const;
    void setMaxFileSize(size_t size_bytes);
    size_t getMaxFileSize() const;
    void setMaxFiles(size_t count);
    size_t getMaxFiles() const;
    
    // Colored output
    void setColoredOutput(bool enabled);
    bool isColoredOutputEnabled() const;
    
    // Structured logging
    void log(Level level, const std::string& message, const LogContext& context = LogContext());
    void log(Level level, const char* format, const LogContext& context = LogContext(), ...);
    
    // Convenience methods with context
    void trace(const std::string& message, const LogContext& context = LogContext());
    void debug(const std::string& message, const LogContext& context = LogContext());
    void info(const std::string& message, const LogContext& context = LogContext());
    void warning(const std::string& message, const LogContext& context = LogContext());
    void error(const std::string& message, const LogContext& context = LogContext());
    void fatal(const std::string& message, const LogContext& context = LogContext());
    
    // Error tracking
    std::string logError(const std::string& error_type, const std::string& message, 
                        const LogContext& context = LogContext());
    std::string logErrorWithMetadata(const std::string& error_type, const std::string& message,
                                   const std::unordered_map<std::string, std::string>& metadata,
                                   const LogContext& context = LogContext());
    ErrorInfo getErrorInfo(const std::string& error_id) const;
    std::vector<ErrorInfo> getRecentErrors(size_t count = 10) const;
    void clearErrorHistory();
    
    // Performance monitoring
    void startTimer(const std::string& timer_name);
    double endTimer(const std::string& timer_name);
    void logPerformance(const std::string& operation, double duration_ms, 
                       const LogContext& context = LogContext());
    
    // Statistics
    size_t getLogCount(Level level) const;
    size_t getErrorCount() const;
    double getAverageLogTime() const;
    
    // Testing support
    void reset(); // Reset all counters and statistics
    
    // File management
    void rotateLogFile();
    void flush();
    void close();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Internal methods
    std::string generateErrorId() const;
    std::string formatLogMessage(Level level, const std::string& message, 
                                const LogContext& context) const;
    std::string formatTimestamp() const;
    std::string levelToString(Level level) const;
    std::string getColorCode(Level level) const;
    std::string getResetColorCode() const;
    void writeToFile(const std::string& message);
    void checkFileRotation();
    std::string getThreadId() const;
    
    // Member variables
    Level current_level_;
    OutputTarget output_target_;
    bool colored_output_;
    std::string log_file_path_;
    std::ofstream log_file_;
    size_t max_file_size_;
    size_t max_files_;
    size_t current_file_size_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Error tracking
    std::unordered_map<std::string, ErrorInfo> error_history_;
    std::atomic<size_t> error_counter_;
    
    // Performance tracking
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers_;
    std::unordered_map<Level, std::atomic<size_t>> log_counts_;
    std::atomic<uint64_t> total_log_time_us_;
    std::atomic<size_t> log_count_;
    
    // Statistics
    std::chrono::high_resolution_clock::time_point start_time_;
};

// Global logging macros for convenience
#define LOG_TRACE(...) Logger::getInstance().trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().error(__VA_ARGS__)
#define LOG_FATAL(...) Logger::getInstance().fatal(__VA_ARGS__)

// Context-aware logging macros
#define LOG_CONTEXT(component, function) \
    LogContext(component, function, __FILE__, __LINE__)

#define LOG_TRACE_CONTEXT(component, function, ...) \
    Logger::getInstance().trace(__VA_ARGS__, LOG_CONTEXT(component, function))

#define LOG_DEBUG_CONTEXT(component, function, ...) \
    Logger::getInstance().debug(__VA_ARGS__, LOG_CONTEXT(component, function))

#define LOG_INFO_CONTEXT(component, function, ...) \
    Logger::getInstance().info(__VA_ARGS__, LOG_CONTEXT(component, function))

#define LOG_WARNING_CONTEXT(component, function, ...) \
    Logger::getInstance().warning(__VA_ARGS__, LOG_CONTEXT(component, function))

#define LOG_ERROR_CONTEXT(component, function, ...) \
    Logger::getInstance().error(__VA_ARGS__, LOG_CONTEXT(component, function))

#define LOG_FATAL_CONTEXT(component, function, ...) \
    Logger::getInstance().fatal(__VA_ARGS__, LOG_CONTEXT(component, function))

// Performance monitoring macros
#define LOG_PERFORMANCE_START(timer_name) \
    Logger::getInstance().startTimer(timer_name)

#define LOG_PERFORMANCE_END(timer_name) \
    Logger::getInstance().endTimer(timer_name)

#define LOG_PERFORMANCE(operation, duration_ms) \
    Logger::getInstance().logPerformance(operation, duration_ms)

} // namespace Utils
} // namespace AutoVibez

#endif /* logger_hpp */ 