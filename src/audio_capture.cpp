#include "audio_capture.hpp"
#include "autovibez_app.hpp"


int AutoVibezApp::initAudioInput() {
    SDL_AudioSpec desired, obtained;

    // Initialize SDL audio
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL audio init failed: %s", SDL_GetError());
        return 0;
    }

    // Get number of audio devices
    _numAudioDevices = SDL_GetNumAudioDevices(SDL_TRUE);
    
    // Set up audio format
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = audioInputCallbackF32;
    desired.userdata = this;

    // Open audio device
    _audioDeviceId = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &desired, &obtained, 0);
    if (_audioDeviceId == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return 0;
    }

    _audioChannelsCount = obtained.channels;

    return 1;
}

void AutoVibezApp::audioInputCallbackF32(void *userdata, unsigned char *stream, int len) {
    AutoVibezApp *app = static_cast<AutoVibezApp*>(userdata);
//    printf("\nLEN: %i\n", len);
//    for (int i = 0; i < 64; i++)
//        printf("%X ", stream[i]);
    // stream is (i think) samples*channels floats (native byte order) of len BYTES
    // Note: reinterpret_cast is necessary here for audio callback interfacing
    if (app->_audioChannelsCount == 1)
        projectm_pcm_add_float(app->_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_MONO);
    else if (app->_audioChannelsCount == 2)
        projectm_pcm_add_float(app->_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_STEREO);
    else {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Multichannel audio not supported");
        SDL_Quit();
    }
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
        if (this->initAudioInput())
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
    SDL_PauseAudioDevice(_audioDeviceId, true);
    SDL_CloseAudioDevice(_audioDeviceId);
}

