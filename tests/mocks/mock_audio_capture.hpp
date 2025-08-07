#pragma once

#include <gmock/gmock.h>

#include <memory>
#include <vector>

/**
 * @brief Mock audio capture class for testing
 */
class MockAudioCapture {
public:
    MOCK_METHOD(bool, initAudioInput, (int device_index), ());
    MOCK_METHOD(void, closeAudioInput, (), ());
    MOCK_METHOD(int, getAudioDeviceCount, (), (const));
    MOCK_METHOD(std::string, getAudioDeviceName, (int device_index), (const));
    MOCK_METHOD(void, cycleAudioDevice, (), ());
    MOCK_METHOD(int, getCurrentAudioDeviceIndex, (), (const));
    MOCK_METHOD(bool, isAudioDeviceOpen, (), (const));
    MOCK_METHOD(std::vector<float>, getAudioData, (), (const));
    MOCK_METHOD(int, getSampleRate, (), (const));
    MOCK_METHOD(int, getChannels, (), (const));
    MOCK_METHOD(bool, isCapturing, (), (const));
    MOCK_METHOD(void, startCapture, (), ());
    MOCK_METHOD(void, stopCapture, (), ());

    // Helper methods for testing
    void setMockAudioData(const std::vector<float>& data) {
        mock_audio_data = data;
    }

    void setMockSampleRate(int rate) {
        mock_sample_rate = rate;
    }

    void setMockChannels(int ch) {
        mock_channels = ch;
    }

    void setMockDeviceCount(int count) {
        mock_device_count = count;
    }

    void setMockDeviceNames(const std::vector<std::string>& names) {
        mock_device_names = names;
    }

private:
    std::vector<float> mock_audio_data;
    int mock_sample_rate = 44100;
    int mock_channels = 2;
    int mock_device_count = 1;
    std::vector<std::string> mock_device_names = {"Mock Audio Device"};
};