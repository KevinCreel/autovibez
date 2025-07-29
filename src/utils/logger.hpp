#ifndef logger_hpp
#define logger_hpp

#include <string>
#include <memory>
#include <mutex>

class Logger {
public:
    enum class Level {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    static Logger& getInstance();
    
    void setLogLevel(Level level);
    Level getLogLevel() const;
    
    void log(Level level, const std::string& message);
    void log(Level level, const char* format, ...);
    
    // Convenience methods
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warning(const char* format, ...);
    void error(const char* format, ...);
    
    // Enable/disable colored output
    void setColoredOutput(bool enabled);
    bool isColoredOutputEnabled() const;

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string levelToString(Level level) const;
    std::string getColorCode(Level level) const;
    std::string getResetColorCode() const;
    
    Level current_level_ = Level::INFO;
    bool colored_output_ = true;
    mutable std::mutex mutex_;
};

// Global logging macros for convenience
#define LOG_DEBUG(...) Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().error(__VA_ARGS__)

#endif /* logger_hpp */ 