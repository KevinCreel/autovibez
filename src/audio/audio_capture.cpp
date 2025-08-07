#include "audio_capture.hpp"
#include "autovibez_app.hpp"
using AutoVibez::Core::AutoVibezApp;


namespace AutoVibez {
namespace Audio {

void audioInputCallbackF32(void* userData, const float* buffer, int len) {
    AutoVibezApp *app = static_cast<AutoVibezApp*>(userData);
    
    // stream contains float data in native byte order, len is in bytes
    // Convert to float pointer safely
    const float* floatStream = static_cast<const float*>(static_cast<const void*>(buffer));
    int numSamples = len / sizeof(float) / app->getAudioChannelsCount();  // Use getter method
    
    if (app->getAudioChannelsCount() == 1)
        projectm_pcm_add_float(app->getProjectM(), const_cast<float*>(floatStream), numSamples, PROJECTM_MONO);
    else if (app->getAudioChannelsCount() == 2)
        projectm_pcm_add_float(app->getProjectM(), const_cast<float*>(floatStream), numSamples, PROJECTM_STEREO);
    else {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Multichannel audio not supported");
        SDL_Quit();
    }
}

} // namespace Audio
} // namespace AutoVibez

int AutoVibezApp::initializeAudioInput() {
    SDL_AudioSpec desired, obtained;

    // Initialize SDL audio
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL audio init failed: %s", SDL_GetError());
        return 0;
    }

    // Set hint to include monitor devices like the original
#ifdef SDL_HINT_AUDIO_INCLUDE_MONITORS
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");
#endif

    // Get number of audio devices
    _numAudioDevices = SDL_GetNumAudioDevices(SDL_TRUE);
    
    // Set up audio format - use AUDIO_F32 like the original
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = AUDIO_F32;  // Changed from AUDIO_F32SYS to AUDIO_F32
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = [](void* userdata, unsigned char* stream, int len) {
        // Convert to float and call our callback
        const float* floatStream = static_cast<const float*>(static_cast<const void*>(stream));
        AutoVibez::Audio::audioInputCallbackF32(userdata, floatStream, len);
    };
    desired.userdata = this;

    // Open audio device - handle -1 for default device like the original
    const char* deviceName = nullptr;
    if (_selectedAudioDeviceIndex >= 0 && _selectedAudioDeviceIndex < _numAudioDevices) {
        deviceName = SDL_GetAudioDeviceName(_selectedAudioDeviceIndex, SDL_TRUE);
        if (!deviceName) {
            // Device name is null, fallback to default device
            SDL_Log("Device name is null for index %d, falling back to default device", _selectedAudioDeviceIndex);
            deviceName = nullptr;
            _selectedAudioDeviceIndex = -1;
        }
    }
    
    _audioDeviceId = SDL_OpenAudioDevice(deviceName, SDL_TRUE, &desired, &obtained, SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (_audioDeviceId == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        
        // Try fallback to default device if we weren't already trying the default
        if (deviceName != nullptr) {
            SDL_Log("Trying fallback to default audio device");
            _selectedAudioDeviceIndex = -1;
            _audioDeviceId = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &desired, &obtained, SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
            if (_audioDeviceId == 0) {
                SDL_Log("Failed to open default audio device: %s", SDL_GetError());
                return 0;
            }
        } else {
            return 0;
        }
    }

    _audioChannelsCount = obtained.channels;

    return 1;
}

int AutoVibezApp::toggleAudioInput()
{
    if (this->fakeAudio)
    {
        this->fakeAudio = false;
        this->endAudioCapture();
        this->beginAudioCapture();
        return 1;
    }
    else
    {
        this->fakeAudio = true;
        this->endAudioCapture();
        if (this->initializeAudioInput())
        {
            this->beginAudioCapture();
            return 1;
        }
        else
        {
            this->fakeAudio = false;
            return 0;
        }
    }
}

void AutoVibezApp::beginAudioCapture() {
    // allocate a buffer to store PCM data for feeding in
    SDL_PauseAudioDevice(_audioDeviceId, false);
}

void AutoVibezApp::endAudioCapture() {
    if (_audioDeviceId != 0) {
        SDL_PauseAudioDevice(_audioDeviceId, true);
        SDL_CloseAudioDevice(_audioDeviceId);
        _audioDeviceId = 0;  // Reset device ID after closing
    }
}

