#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace AutoVibez { namespace Core { class AutoVibezApp; } }

namespace AutoVibez {
namespace Audio {

class AudioManager {
public:
    explicit AudioManager(AutoVibez::Core::AutoVibezApp* app);
    
    /**
     * Initialize audio system
     */
    bool initialize();
    
    /**
     * Cycle to next audio device
     */
    void cycleDevice();
    
    /**
     * Set specific audio device
     */
    void setDevice(int deviceIndex);
    
    /**
     * Get current device index
     */
    int getCurrentDevice() const;
    
    /**
     * Get total number of devices
     */
    int getDeviceCount() const;
    
    /**
     * Get current device name
     */
    std::string getCurrentDeviceName() const;
    
    /**
     * Get all available device names
     */
    std::vector<std::string> getDeviceNames() const;
    
    /**
     * Start audio capture
     */
    void startCapture();
    
    /**
     * Stop audio capture
     */
    void stopCapture();
    
    /**
     * Check if audio is currently capturing
     */
    bool isCapturing() const;
    
    /**
     * Toggle audio input
     */
    void toggleInput();
    
    /**
     * Get audio device ID
     */
    SDL_AudioDeviceID getDeviceId() const;
    
    /**
     * Get number of audio channels
     */
    unsigned short getChannelsCount() const;
    
private:
    AutoVibez::Core::AutoVibezApp* _app;
    int _currentDevice{0};
    int _deviceCount{0};
    SDL_AudioDeviceID _deviceId{0};
    unsigned short _channelsCount{0};
    bool _isCapturing{false};
    
    /**
     * Open audio device
     */
    bool openDevice(const char* deviceName);
    
    /**
     * Close current audio device
     */
    void closeDevice();
};

} // namespace Audio
} // namespace AutoVibez 