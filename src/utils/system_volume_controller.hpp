#pragma once

#include <memory>
#include <string>

namespace AutoVibez::Utils {

/**
 * @brief Abstract interface for cross-platform system volume control
 */
class ISystemVolumeController {
public:
    virtual ~ISystemVolumeController() = default;

    /**
     * @brief Get the current system volume level
     * @return Volume level as percentage (0-100), or -1 if failed
     */
    virtual int getCurrentVolume() = 0;

    /**
     * @brief Set the system volume level
     * @param volumeLevel Volume level as percentage (0-100)
     * @return true if successful, false otherwise
     */
    virtual bool setVolume(int volumeLevel) = 0;

    /**
     * @brief Increase system volume by specified amount
     * @param step Amount to increase (default 5%)
     * @return true if successful, false otherwise
     */
    virtual bool increaseVolume(int step = 5) = 0;

    /**
     * @brief Decrease system volume by specified amount
     * @param step Amount to decrease (default 5%)
     * @return true if successful, false otherwise
     */
    virtual bool decreaseVolume(int step = 5) = 0;

    /**
     * @brief Check if system volume control is available on this platform
     * @return true if available, false otherwise
     */
    virtual bool isAvailable() = 0;

    /**
     * @brief Get platform-specific error message for last operation
     * @return Error message string
     */
    virtual std::string getLastError() = 0;
};

/**
 * @brief Linux implementation using subprocess calls to amixer/pactl
 */
class LinuxVolumeController : public ISystemVolumeController {
public:
    LinuxVolumeController();
    ~LinuxVolumeController() override = default;

    int getCurrentVolume() override;
    bool setVolume(int volumeLevel) override;
    bool increaseVolume(int step = 5) override;
    bool decreaseVolume(int step = 5) override;
    bool isAvailable() override;
    std::string getLastError() override;

private:
    enum class AudioSystem { ALSA, PULSE_AUDIO, UNKNOWN };

    AudioSystem detectAudioSystem();
    bool executeCommand(const std::string& command);
    int parseVolumeFromOutput(const std::string& output);

    AudioSystem _audioSystem;
    std::string _lastError;
    bool _available;
};

/**
 * @brief Windows implementation using Core Audio API subprocess calls
 */
class WindowsVolumeController : public ISystemVolumeController {
public:
    WindowsVolumeController();
    ~WindowsVolumeController() override = default;

    int getCurrentVolume() override;
    bool setVolume(int volumeLevel) override;
    bool increaseVolume(int step = 5) override;
    bool decreaseVolume(int step = 5) override;
    bool isAvailable() override;
    std::string getLastError() override;

private:
    bool executeCommand(const std::string& command);
    int parseVolumeFromOutput(const std::string& output);

    std::string _lastError;
    bool _available;
};

/**
 * @brief macOS implementation using osascript subprocess calls
 */
class MacOSVolumeController : public ISystemVolumeController {
public:
    MacOSVolumeController();
    ~MacOSVolumeController() override = default;

    int getCurrentVolume() override;
    bool setVolume(int volumeLevel) override;
    bool increaseVolume(int step = 5) override;
    bool decreaseVolume(int step = 5) override;
    bool isAvailable() override;
    std::string getLastError() override;

private:
    bool executeAppleScript(const std::string& script);
    int parseVolumeFromOutput(const std::string& output);

    std::string _lastError;
    bool _available;
};

/**
 * @brief Factory class to create platform-appropriate volume controller
 */
class SystemVolumeControllerFactory {
public:
    /**
     * @brief Create platform-specific volume controller
     * @return Unique pointer to volume controller, or nullptr if not supported
     */
    static std::unique_ptr<ISystemVolumeController> create();

    /**
     * @brief Check if system volume control is supported on current platform
     * @return true if supported, false otherwise
     */
    static bool isSupported();
};

}  // namespace AutoVibez::Utils
