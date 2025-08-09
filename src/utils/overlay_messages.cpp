#include "overlay_messages.hpp"

namespace AutoVibez::Utils {

// Static member definitions
std::unordered_map<std::string, MessageFactory> OverlayMessages::messageRegistry;
bool OverlayMessages::initialized = false;

void OverlayMessages::initializeMessages() {
    if (initialized)
        return;

    // Mix Info Message
    messageRegistry["mix_info"] = [](const std::vector<std::string>& args) -> NamedMessageConfig {
        if (args.size() >= 2) {
            std::string artist = args[0];
            std::string title = args[1];
            return NamedMessageConfig{[artist, title]() { return artist + " - " + title; },
                                      std::chrono::milliseconds(20000), true};
        }
        return NamedMessageConfig{[]() { return "Unknown message"; }, std::chrono::milliseconds(3000), false};
    };

    // Add more message definitions here as needed
    // Example for future messages:
    /*
    messageRegistry["volume_change"] = [](const std::vector<std::string>& args) -> NamedMessageConfig {
        if (args.size() >= 1) {
            std::string volume = args[0];
            return NamedMessageConfig{
                [volume]() { return "🔊 Volume: " + volume + "%"; },
                std::chrono::milliseconds(3000),
                true
            };
        }
        return NamedMessageConfig{
            []() { return "Invalid volume"; },
            std::chrono::milliseconds(3000),
            false
        };
    };
    */

    initialized = true;
}

}  // namespace AutoVibez::Utils
