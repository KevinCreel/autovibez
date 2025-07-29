#ifndef console_output_hpp
#define console_output_hpp

#include <string>
#include <mutex>
#include <iostream>

/**
 * @brief Console output utility for user-facing messages
 * 
 * This class provides a thread-safe way to output user-facing messages
 * to the console, separate from debug logging. It handles formatting,
 * colors, and ensures messages are displayed properly to the user.
 */
class ConsoleOutput {
public:
    /**
     * @brief Output types for different message categories
     */
    enum class Type {
        INFO,       // General information
        SUCCESS,    // Success messages
        WARNING,    // Warning messages
        ERROR,      // Error messages
        PLAYBACK,   // Audio playback related
        UI,         // User interface related
        SYSTEM      // System status messages
    };

    /**
     * @brief Output a message to console
     * @param type The type of message
     * @param message The message to output
     */
    static void output(Type type, const std::string& message);
    
    /**
     * @brief Output a formatted message to console
     * @param type The type of message
     * @param format Format string (printf-style)
     * @param ... Variable arguments
     */
    static void output(Type type, const char* format, ...);
    
    /**
     * @brief Output a simple message without formatting
     * @param message The message to output
     */
    static void print(const std::string& message);
    
    /**
     * @brief Output a formatted message without type formatting
     * @param format Format string (printf-style)
     * @param ... Variable arguments
     */
    static void print(const char* format, ...);
    
    /**
     * @brief Enable or disable colored output
     * @param enabled Whether to enable colored output
     */
    static void setColoredOutput(bool enabled);
    
    /**
     * @brief Check if colored output is enabled
     * @return true if colored output is enabled
     */
    static bool isColoredOutputEnabled();
    
    // Convenience methods for common outputs
    static void printNextPreset(const std::string& presetName);
    static void printPreviousPreset(const std::string& presetName);
    static void printRandomPreset(const std::string& presetName);
    static void printPause();
    static void printResume();

private:
    static std::mutex mutex_;
    static bool colored_output_;
    
    static std::string getTypePrefix(Type type);
    static std::string getColorCode(Type type);
    static std::string getResetColorCode();
};

#endif /* console_output_hpp */ 