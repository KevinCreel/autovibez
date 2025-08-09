#include "system_volume_controller.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <unistd.h>
#elif __linux__
#include <unistd.h>
#endif

namespace AutoVibez::Utils {

// Linux Implementation
LinuxVolumeController::LinuxVolumeController() : _audioSystem(AudioSystem::UNKNOWN), _available(false) {
    _audioSystem = detectAudioSystem();
    _available = (_audioSystem != AudioSystem::UNKNOWN);
}

LinuxVolumeController::AudioSystem LinuxVolumeController::detectAudioSystem() {
    if (system("which pactl > /dev/null 2>&1") == 0) {
        return AudioSystem::PULSE_AUDIO;
    }
    if (system("which amixer > /dev/null 2>&1") == 0) {
        return AudioSystem::ALSA;
    }
    return AudioSystem::UNKNOWN;
}

bool LinuxVolumeController::executeCommand(const std::string& command) {
    int result = system(command.c_str());
    if (result != 0) {
        _lastError = "Command failed with exit code: " + std::to_string(result);
        return false;
    }
    return true;
}

int LinuxVolumeController::getCurrentVolume() {
    if (!_available) {
        _lastError = "No audio system available";
        return -1;
    }

    std::string command;
    if (_audioSystem == AudioSystem::PULSE_AUDIO) {
        command = "pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null";
    } else if (_audioSystem == AudioSystem::ALSA) {
        command = "amixer sget Master 2>/dev/null";
    } else {
        _lastError = "Unknown audio system";
        return -1;
    }

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        _lastError = "Failed to execute command";
        return -1;
    }

    std::string output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);

    return parseVolumeFromOutput(output);
}

int LinuxVolumeController::parseVolumeFromOutput(const std::string& output) {
    if (_audioSystem == AudioSystem::PULSE_AUDIO) {
        std::regex pattern(R"((\d+)%)");
        std::smatch match;
        if (std::regex_search(output, match, pattern)) {
            return std::stoi(match[1].str());
        }
    } else if (_audioSystem == AudioSystem::ALSA) {
        std::regex pattern(R"(\[(\d+)%\])");
        std::smatch match;
        if (std::regex_search(output, match, pattern)) {
            return std::stoi(match[1].str());
        }
    }

    _lastError = "Failed to parse volume from output";
    return -1;
}

bool LinuxVolumeController::setVolume(int volumeLevel) {
    if (!_available) {
        _lastError = "No audio system available";
        return false;
    }

    if (volumeLevel < 0 || volumeLevel > 100) {
        _lastError = "Volume level must be between 0 and 100";
        return false;
    }

    std::string command;
    if (_audioSystem == AudioSystem::PULSE_AUDIO) {
        command = "pactl set-sink-volume @DEFAULT_SINK@ " + std::to_string(volumeLevel) + "%";
    } else if (_audioSystem == AudioSystem::ALSA) {
        command = "amixer sset Master " + std::to_string(volumeLevel) + "% > /dev/null 2>&1";
    } else {
        _lastError = "Unknown audio system";
        return false;
    }

    return executeCommand(command);
}

bool LinuxVolumeController::increaseVolume(int step) {
    if (!_available) {
        _lastError = "No audio system available";
        return false;
    }

    std::string command;
    if (_audioSystem == AudioSystem::PULSE_AUDIO) {
        command = "pactl set-sink-volume @DEFAULT_SINK@ +" + std::to_string(step) + "%";
    } else if (_audioSystem == AudioSystem::ALSA) {
        command = "amixer sset Master " + std::to_string(step) + "%+ > /dev/null 2>&1";
    } else {
        _lastError = "Unknown audio system";
        return false;
    }

    return executeCommand(command);
}

bool LinuxVolumeController::decreaseVolume(int step) {
    if (!_available) {
        _lastError = "No audio system available";
        return false;
    }

    std::string command;
    if (_audioSystem == AudioSystem::PULSE_AUDIO) {
        command = "pactl set-sink-volume @DEFAULT_SINK@ -" + std::to_string(step) + "%";
    } else if (_audioSystem == AudioSystem::ALSA) {
        command = "amixer sset Master " + std::to_string(step) + "%- > /dev/null 2>&1";
    } else {
        _lastError = "Unknown audio system";
        return false;
    }

    return executeCommand(command);
}

bool LinuxVolumeController::isAvailable() {
    return _available;
}

std::string LinuxVolumeController::getLastError() {
    return _lastError;
}

// Windows Implementation
WindowsVolumeController::WindowsVolumeController() : _available(false) {
#ifdef _WIN32
    _available = true;
#endif
}

bool WindowsVolumeController::executeCommand(const std::string& command) {
#ifdef _WIN32
    int result = system(command.c_str());
    if (result != 0) {
        _lastError = "Command failed with exit code: " + std::to_string(result);
        return false;
    }
    return true;
#else
    _lastError = "Windows volume control not available on this platform";
    return false;
#endif
}

int WindowsVolumeController::getCurrentVolume() {
#ifdef _WIN32
    std::string command = "powershell -c \"[audio]::Volume * 100\"";
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        _lastError = "Failed to execute command";
        return -1;
    }

    std::string output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    _pclose(pipe);

    return parseVolumeFromOutput(output);
#else
    _lastError = "Windows volume control not available on this platform";
    return -1;
#endif
}

int WindowsVolumeController::parseVolumeFromOutput(const std::string& output) {
    try {
        double volume = std::stod(output);
        return static_cast<int>(volume);
    } catch (const std::exception&) {
        _lastError = "Failed to parse volume from output";
        return -1;
    }
}

bool WindowsVolumeController::setVolume(int volumeLevel) {
#ifdef _WIN32
    if (volumeLevel < 0 || volumeLevel > 100) {
        _lastError = "Volume level must be between 0 and 100";
        return false;
    }

    double level = static_cast<double>(volumeLevel) / 100.0;
    std::string command = "powershell -c \"[audio]::Volume = " + std::to_string(level) + "\"";
    return executeCommand(command);
#else
    _lastError = "Windows volume control not available on this platform";
    return false;
#endif
}

bool WindowsVolumeController::increaseVolume(int step) {
    int currentVolume = getCurrentVolume();
    if (currentVolume == -1) {
        return false;
    }
    return setVolume(std::min(100, currentVolume + step));
}

bool WindowsVolumeController::decreaseVolume(int step) {
    int currentVolume = getCurrentVolume();
    if (currentVolume == -1) {
        return false;
    }
    return setVolume(std::max(0, currentVolume - step));
}

bool WindowsVolumeController::isAvailable() {
    return _available;
}

std::string WindowsVolumeController::getLastError() {
    return _lastError;
}

// macOS Implementation
MacOSVolumeController::MacOSVolumeController() : _available(false) {
#ifdef __APPLE__
    _available = (system("which osascript > /dev/null 2>&1") == 0);
#endif
}

bool MacOSVolumeController::executeAppleScript(const std::string& script) {
#ifdef __APPLE__
    std::string command = "osascript -e \"" + script + "\"";
    int result = system(command.c_str());
    if (result != 0) {
        _lastError = "AppleScript command failed with exit code: " + std::to_string(result);
        return false;
    }
    return true;
#else
    _lastError = "macOS volume control not available on this platform";
    return false;
#endif
}

int MacOSVolumeController::getCurrentVolume() {
#ifdef __APPLE__
    std::string command = "osascript -e \"output volume of (get volume settings)\"";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        _lastError = "Failed to execute command";
        return -1;
    }

    std::string output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);

    return parseVolumeFromOutput(output);
#else
    _lastError = "macOS volume control not available on this platform";
    return -1;
#endif
}

int MacOSVolumeController::parseVolumeFromOutput(const std::string& output) {
    try {
        return std::stoi(output);
    } catch (const std::exception&) {
        _lastError = "Failed to parse volume from output";
        return -1;
    }
}

bool MacOSVolumeController::setVolume(int volumeLevel) {
#ifdef __APPLE__
    if (volumeLevel < 0 || volumeLevel > 100) {
        _lastError = "Volume level must be between 0 and 100";
        return false;
    }

    std::string script = "set volume output volume " + std::to_string(volumeLevel);
    return executeAppleScript(script);
#else
    _lastError = "macOS volume control not available on this platform";
    return false;
#endif
}

bool MacOSVolumeController::increaseVolume(int step) {
    int currentVolume = getCurrentVolume();
    if (currentVolume == -1) {
        return false;
    }
    return setVolume(std::min(100, currentVolume + step));
}

bool MacOSVolumeController::decreaseVolume(int step) {
    int currentVolume = getCurrentVolume();
    if (currentVolume == -1) {
        return false;
    }
    return setVolume(std::max(0, currentVolume - step));
}

bool MacOSVolumeController::isAvailable() {
    return _available;
}

std::string MacOSVolumeController::getLastError() {
    return _lastError;
}

// Factory Implementation
std::unique_ptr<ISystemVolumeController> SystemVolumeControllerFactory::create() {
#ifdef __linux__
    auto controller = std::make_unique<LinuxVolumeController>();
    if (controller->isAvailable()) {
        return controller;
    }
#elif _WIN32
    auto controller = std::make_unique<WindowsVolumeController>();
    if (controller->isAvailable()) {
        return controller;
    }
#elif __APPLE__
    auto controller = std::make_unique<MacOSVolumeController>();
    if (controller->isAvailable()) {
        return controller;
    }
#endif
    return nullptr;
}

bool SystemVolumeControllerFactory::isSupported() {
#if defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

}  // namespace AutoVibez::Utils
