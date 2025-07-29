#include "console_output.hpp"
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <mutex>

// Static member initialization
std::mutex ConsoleOutput::mutex_;

void ConsoleOutput::output(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << message << std::endl;
}

void ConsoleOutput::output(const char* format, ...) {
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
        
        // Output exactly what was formatted
        std::cout << message << std::endl;
    }
    
    va_end(args);
} 