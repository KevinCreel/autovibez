#ifndef console_output_hpp
#define console_output_hpp

#include <string>
#include <mutex>
#include <iostream>

/**
 * @brief Console output utility for user-facing messages
 * 
 * This class provides a thread-safe way to output anything to the console,
 * including ANSI colors and emojis. It outputs exactly what it's given.
 */
class ConsoleOutput {
public:
    /**
     * @brief Output anything to console
     * @param message The message to output (can contain ANSI colors, emojis, etc.)
     */
    static void output(const std::string& message);
    
    /**
     * @brief Output a formatted message to console
     * @param format Format string (printf-style)
     * @param ... Variable arguments
     */
    static void output(const char* format, ...);

private:
    static std::mutex mutex_;
};

#endif /* console_output_hpp */ 