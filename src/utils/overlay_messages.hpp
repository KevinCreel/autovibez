#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace AutoVibez::Utils {

struct NamedMessageConfig {
    std::function<std::string()> formatter;
    std::chrono::milliseconds duration;
    bool colorTransition;
};

// Type alias for message factory function
using MessageFactory = std::function<NamedMessageConfig(const std::vector<std::string>&)>;

class OverlayMessages {
private:
    static std::unordered_map<std::string, MessageFactory> messageRegistry;
    static bool initialized;

public:
    /**
     * @brief Initialize all message definitions
     */
    static void initializeMessages();

    /**
     * @brief Create a named message with arbitrary arguments
     */
    template <typename... Args>
    static NamedMessageConfig createMessage(const std::string& name, Args... args);

private:
    /**
     * @brief Convert variadic arguments to string vector
     */
    template <typename... Args>
    static std::vector<std::string> argsToStrings(Args... args);
};

// Template implementations must be in header
template <typename... Args>
NamedMessageConfig OverlayMessages::createMessage(const std::string& name, Args... args) {
    if (!initialized) {
        initializeMessages();
    }

    auto it = messageRegistry.find(name);
    if (it != messageRegistry.end()) {
        std::vector<std::string> stringArgs = argsToStrings(args...);
        return it->second(stringArgs);
    }

    // Default fallback
    return NamedMessageConfig{[]() { return "Unknown message"; }, std::chrono::milliseconds(3000), false};
}

// Helper function to convert any type to string
template <typename T>
std::string convertToString(T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, const char*> ||
                  std::is_convertible_v<T, std::string>) {
        return std::string(value);
    } else {
        return std::to_string(value);
    }
}

template <typename... Args>
std::vector<std::string> OverlayMessages::argsToStrings(Args... args) {
    std::vector<std::string> result;
    ((result.push_back(convertToString(args))), ...);  // C++17 fold expression
    return result;
}

}  // namespace AutoVibez::Utils
