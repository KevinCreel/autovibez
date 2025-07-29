#include "console_output.hpp"
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>

// Static member initialization
std::mutex ConsoleOutput::mutex_;
bool ConsoleOutput::colored_output_ = true;

void ConsoleOutput::output(Type type, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Just print the message exactly as provided
    std::string color_code = colored_output_ ? getColorCode(type) : "";
    std::string reset_code = colored_output_ ? getResetColorCode() : "";
    
    std::cout << color_code << message << reset_code << std::endl;
}

void ConsoleOutput::output(Type type, const char* format, ...) {
    if (!format) return;
    
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
        
        // Output the formatted message
        output(type, message);
    }
    
    va_end(args);
}

void ConsoleOutput::print(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << message << std::endl;
}

void ConsoleOutput::print(const char* format, ...) {
    if (!format) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
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
        
        // Output the formatted message
        std::cout << message << std::endl;
    }
    
    va_end(args);
}

void ConsoleOutput::setColoredOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    colored_output_ = enabled;
}

bool ConsoleOutput::isColoredOutputEnabled() {
    std::lock_guard<std::mutex> lock(mutex_);
    return colored_output_;
}

std::string ConsoleOutput::getTypePrefix(Type type) {
    switch (type) {
        case Type::INFO:     return "[INFO]";
        case Type::SUCCESS:  return "[SUCCESS]";
        case Type::WARNING:  return "[WARNING]";
        case Type::ERROR:    return "[ERROR]";
        case Type::PLAYBACK: return "[PLAYBACK]";
        case Type::UI:       return "[UI]";
        case Type::SYSTEM:   return "[SYSTEM]";
        default:             return "[INFO]";
    }
}

std::string ConsoleOutput::getColorCode(Type type) {
    switch (type) {
        case Type::INFO:     return "\033[36m"; // Cyan
        case Type::SUCCESS:  return "\033[32m"; // Green
        case Type::WARNING:  return "\033[33m"; // Yellow
        case Type::ERROR:    return "\033[31m"; // Red
        case Type::PLAYBACK: return "\033[35m"; // Magenta
        case Type::UI:       return "\033[34m"; // Blue
        case Type::SYSTEM:   return "\033[37m"; // White
        default:             return "\033[0m";  // Reset
    }
}

std::string ConsoleOutput::getResetColorCode() {
    return "\033[0m";
}

// Convenience methods for common outputs
void ConsoleOutput::printNextPreset(const std::string& presetName) {
    output(Type::UI, "⏭️  Next preset: %s", presetName.c_str());
}

void ConsoleOutput::printPreviousPreset(const std::string& presetName) {
    output(Type::UI, "⏮️  Previous preset: %s", presetName.c_str());
}

void ConsoleOutput::printRandomPreset(const std::string& presetName) {
    output(Type::UI, "🎨 Loaded random preset: %s", presetName.c_str());
}

void ConsoleOutput::printPause() {
    output(Type::UI, "⏸️  Preset paused");
}

void ConsoleOutput::printResume() {
    output(Type::UI, "▶️  Preset resumed");
} 