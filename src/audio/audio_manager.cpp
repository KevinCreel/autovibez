#include "audio_manager.hpp"
#include "autovibez_app.hpp"
using AutoVibez::Core::AutoVibezApp;
using AutoVibez::Audio::AudioManager;
#include "constants.hpp"
#include <iostream>
#include "console_output.hpp"

namespace AutoVibez {
namespace Audio {

AudioManager::AudioManager(Core::AutoVibezApp* app)
    : _app(app) {
}

bool AudioManager::initialize() {
    // Initialize SDL audio
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL audio init failed: %s", SDL_GetError());
        return false;
    }
    
    // Get number of audio devices
    _deviceCount = SDL_GetNumAudioDevices(SDL_TRUE);
    
    // Open default device
    return openDevice(nullptr);
}

void AudioManager::cycleDevice() {
    closeDevice();
    
    _currentDevice = (_currentDevice + 1) % _deviceCount;
    
    const char* deviceName = SDL_GetAudioDeviceName(_currentDevice, SDL_TRUE);
    if (openDevice(deviceName)) {
        // Audio device notification removed - help overlay shows current device
    }
}

void AudioManager::setDevice(int deviceIndex) {
    if (deviceIndex >= 0 && deviceIndex < _deviceCount) {
        closeDevice();
        _currentDevice = deviceIndex;
        
        const char* deviceName = SDL_GetAudioDeviceName(_currentDevice, SDL_TRUE);
        openDevice(deviceName);
    }
}

int AudioManager::getCurrentDevice() const {
    return _currentDevice;
}

int AudioManager::getDeviceCount() const {
    return _deviceCount;
}

std::string AudioManager::getCurrentDeviceName() const {
    const char* deviceName = SDL_GetAudioDeviceName(_currentDevice, SDL_TRUE);
    return deviceName ? std::string(deviceName) : "Unknown";
}

std::vector<std::string> AudioManager::getDeviceNames() const {
    std::vector<std::string> names;
    for (int i = 0; i < _deviceCount; ++i) {
        const char* deviceName = SDL_GetAudioDeviceName(i, SDL_TRUE);
        if (deviceName) {
            names.push_back(std::string(deviceName));
        }
    }
    return names;
}

void AudioManager::startCapture() {
    if (_deviceId && !_isCapturing) {
        SDL_PauseAudioDevice(_deviceId, 0);
        _isCapturing = true;
    }
}

void AudioManager::stopCapture() {
    if (_deviceId && _isCapturing) {
        SDL_PauseAudioDevice(_deviceId, 1);
        _isCapturing = false;
    }
}

bool AudioManager::isCapturing() const {
    return _isCapturing;
}

void AudioManager::toggleInput() {
    if (_isCapturing) {
        stopCapture();
        // Audio stop notification removed - help overlay shows current state
    } else {
        startCapture();
        // Audio start notification removed - help overlay shows current state
    }
}

SDL_AudioDeviceID AudioManager::getDeviceId() const {
    return _deviceId;
}

unsigned short AudioManager::getChannelsCount() const {
    return _channelsCount;
}

bool AudioManager::openDevice(const char* deviceName) {
    SDL_AudioSpec desired, obtained;
    
    // Set up audio format
    SDL_zero(desired);
    desired.freq = Constants::DEFAULT_SAMPLE_RATE;
    desired.format = AUDIO_F32SYS;
    desired.channels = Constants::DEFAULT_CHANNELS;
    desired.samples = Constants::DEFAULT_SAMPLES;
    desired.callback = [](void* userdata, unsigned char* stream, int len) {
        // Convert to float and call our callback
        const float* floatStream = static_cast<const float*>(static_cast<const void*>(stream));
        audioInputCallbackF32(userdata, floatStream, len);
    };
    desired.userdata = _app;
    
    // Open audio device
    _deviceId = SDL_OpenAudioDevice(deviceName, SDL_TRUE, &desired, &obtained, 0);
    if (_deviceId == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return false;
    }
    
    _channelsCount = obtained.channels;
    return true;
}

void AudioManager::closeDevice() {
    if (_deviceId) {
        SDL_CloseAudioDevice(_deviceId);
        _deviceId = 0;
        _isCapturing = false;
    }
}

} // namespace Audio
} // namespace AutoVibez 