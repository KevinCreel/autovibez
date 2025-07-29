#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "constants.hpp"

using namespace Constants;

class ConstantsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }
    
    void TearDown() override {
        // No cleanup needed
    }
};

TEST_F(ConstantsTest, AudioDefaults) {
    // Test audio-related constants
    EXPECT_GT(DEFAULT_SAMPLE_RATE, 0);
    EXPECT_EQ(DEFAULT_SAMPLE_RATE, 44100);
    
    EXPECT_GT(DEFAULT_CHANNELS, 0);
    EXPECT_EQ(DEFAULT_CHANNELS, 2);
    
    EXPECT_GT(DEFAULT_SAMPLES, 0);
    EXPECT_EQ(DEFAULT_SAMPLES, 512);
}

TEST_F(ConstantsTest, BeatSensitivityDefaults) {
    // Test beat sensitivity constants
    EXPECT_GE(MIN_BEAT_SENSITIVITY, 0.0f);
    EXPECT_EQ(MIN_BEAT_SENSITIVITY, 0.0f);
    
    EXPECT_GT(MAX_BEAT_SENSITIVITY, MIN_BEAT_SENSITIVITY);
    EXPECT_EQ(MAX_BEAT_SENSITIVITY, 5.0f);
    
    EXPECT_GT(BEAT_SENSITIVITY_STEP, 0.0f);
    EXPECT_EQ(BEAT_SENSITIVITY_STEP, 0.1f);
}

TEST_F(ConstantsTest, BeatSensitivityRange) {
    // Test that beat sensitivity range is valid
    float range = MAX_BEAT_SENSITIVITY - MIN_BEAT_SENSITIVITY;
    EXPECT_GT(range, 0.0f);
    EXPECT_EQ(range, 5.0f);
    
    // Test that step size is reasonable for the range
    int steps = static_cast<int>(range / BEAT_SENSITIVITY_STEP);
    EXPECT_GT(steps, 0);
    EXPECT_EQ(steps, 50); // 5.0 / 0.1 = 50
}

TEST_F(ConstantsTest, UIDefaults) {
    // Test UI-related constants
    EXPECT_GT(DEFAULT_WINDOW_WIDTH, 0);
    EXPECT_EQ(DEFAULT_WINDOW_WIDTH, 512);
    
    EXPECT_GT(DEFAULT_WINDOW_HEIGHT, 0);
    EXPECT_EQ(DEFAULT_WINDOW_HEIGHT, 512);
    
    EXPECT_GT(MIX_STATUS_DISPLAY_TIME, 0);
    EXPECT_EQ(MIX_STATUS_DISPLAY_TIME, 300);
}

TEST_F(ConstantsTest, WindowAspectRatio) {
    // Test that window dimensions are reasonable
    EXPECT_EQ(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT); // Square window
    
    float aspectRatio = static_cast<float>(DEFAULT_WINDOW_WIDTH) / DEFAULT_WINDOW_HEIGHT;
    EXPECT_EQ(aspectRatio, 1.0f); // Square aspect ratio
}

TEST_F(ConstantsTest, PCMDefaults) {
    // Test PCM-related constants
    EXPECT_GT(PCM_BUFFER_SIZE, 0);
    EXPECT_EQ(PCM_BUFFER_SIZE, 512);
    
    EXPECT_GT(PCM_MAX_VALUE, 0);
    EXPECT_EQ(PCM_MAX_VALUE, 16384);
    
    // Test that PCM buffer size is reasonable
    EXPECT_LE(PCM_BUFFER_SIZE, DEFAULT_SAMPLES);
    EXPECT_EQ(PCM_BUFFER_SIZE, DEFAULT_SAMPLES);
}

TEST_F(ConstantsTest, PCMValueRange) {
    // Test PCM value range
    EXPECT_GT(PCM_MAX_VALUE, 0);
    EXPECT_EQ(PCM_MAX_VALUE, 16384); // 2^14
    
    // Test that max value is a power of 2
    int powerOfTwo = 1;
    for (int i = 0; i < 14; ++i) {
        powerOfTwo *= 2;
    }
    EXPECT_EQ(PCM_MAX_VALUE, powerOfTwo);
}

TEST_F(ConstantsTest, WindowDefaults) {
    // Test window-related constants
    EXPECT_GT(DEFAULT_FPS, 0);
    EXPECT_EQ(DEFAULT_FPS, 60);
    
    EXPECT_GT(FRAME_DELAY_MS, 0);
    EXPECT_EQ(FRAME_DELAY_MS, 1000 / DEFAULT_FPS);
    EXPECT_EQ(FRAME_DELAY_MS, 16); // 1000 / 60 ≈ 16.67, truncated to 16
}

TEST_F(ConstantsTest, FrameDelayCalculation) {
    // Test frame delay calculation
    int calculatedDelay = 1000 / DEFAULT_FPS;
    EXPECT_EQ(FRAME_DELAY_MS, calculatedDelay);
    
    // Test that frame delay is reasonable for 60 FPS
    EXPECT_GE(FRAME_DELAY_MS, 16);
    EXPECT_LE(FRAME_DELAY_MS, 17);
}

TEST_F(ConstantsTest, FilePathDefaults) {
    // Test file path constants
    EXPECT_NE(DEFAULT_CONFIG_FILE, nullptr);
    EXPECT_NE(DEFAULT_PRESET_PATH, nullptr);
    EXPECT_NE(DEFAULT_TEXTURE_PATH, nullptr);
    
    // Test that paths are not empty
    EXPECT_FALSE(std::string(DEFAULT_CONFIG_FILE).empty());
    EXPECT_FALSE(std::string(DEFAULT_PRESET_PATH).empty());
    EXPECT_FALSE(std::string(DEFAULT_TEXTURE_PATH).empty());
}

TEST_F(ConstantsTest, FilePathValidation) {
    // Test that file paths are valid
    std::string configFile(DEFAULT_CONFIG_FILE);
    std::string presetPath(DEFAULT_PRESET_PATH);
    std::string texturePath(DEFAULT_TEXTURE_PATH);
    
    // Test that paths contain expected directories
    EXPECT_NE(configFile.find("config"), std::string::npos);
    EXPECT_NE(presetPath.find("presets"), std::string::npos);
    EXPECT_NE(texturePath.find("textures"), std::string::npos);
}

TEST_F(ConstantsTest, ApplicationDefaults) {
    // Test application constants
    EXPECT_NE(APP_NAME, nullptr);
    EXPECT_NE(WINDOW_TITLE, nullptr);
    
    // Test that names are not empty
    EXPECT_FALSE(std::string(APP_NAME).empty());
    EXPECT_FALSE(std::string(WINDOW_TITLE).empty());
    
    // Test that app name and window title match
    EXPECT_EQ(std::string(APP_NAME), std::string(WINDOW_TITLE));
}

TEST_F(ConstantsTest, AudioSampleRateValidation) {
    // Test that sample rate is a standard value
    EXPECT_EQ(DEFAULT_SAMPLE_RATE, 44100); // Standard CD quality
    
    // Test that sample rate is reasonable
    EXPECT_GE(DEFAULT_SAMPLE_RATE, 8000);  // Minimum reasonable
    EXPECT_LE(DEFAULT_SAMPLE_RATE, 192000); // Maximum reasonable
}

TEST_F(ConstantsTest, AudioChannelsValidation) {
    // Test that channel count is reasonable
    EXPECT_EQ(DEFAULT_CHANNELS, 2); // Stereo
    
    // Test that channels is a valid value
    EXPECT_GE(DEFAULT_CHANNELS, 1);  // Mono minimum
    EXPECT_LE(DEFAULT_CHANNELS, 8);  // Reasonable maximum
}

TEST_F(ConstantsTest, AudioSamplesValidation) {
    // Test that sample count is reasonable
    EXPECT_EQ(DEFAULT_SAMPLES, 512);
    
    // Test that samples is a power of 2 (good for FFT)
    int samples = DEFAULT_SAMPLES;
    while (samples > 1 && samples % 2 == 0) {
        samples /= 2;
    }
    EXPECT_EQ(samples, 1); // Should be a power of 2
}

TEST_F(ConstantsTest, PCMBufferSizeValidation) {
    // Test that PCM buffer size is reasonable
    EXPECT_EQ(PCM_BUFFER_SIZE, 512);
    
    // Test that buffer size is a power of 2
    int bufferSize = PCM_BUFFER_SIZE;
    while (bufferSize > 1 && bufferSize % 2 == 0) {
        bufferSize /= 2;
    }
    EXPECT_EQ(bufferSize, 1); // Should be a power of 2
}

TEST_F(ConstantsTest, WindowDimensionsValidation) {
    // Test that window dimensions are reasonable
    EXPECT_EQ(DEFAULT_WINDOW_WIDTH, 512);
    EXPECT_EQ(DEFAULT_WINDOW_HEIGHT, 512);
    
    // Test that dimensions are powers of 2 (good for graphics)
    int width = DEFAULT_WINDOW_WIDTH;
    int height = DEFAULT_WINDOW_HEIGHT;
    
    while (width > 1 && width % 2 == 0) {
        width /= 2;
    }
    while (height > 1 && height % 2 == 0) {
        height /= 2;
    }
    
    EXPECT_EQ(width, 1);   // Should be a power of 2
    EXPECT_EQ(height, 1);  // Should be a power of 2
}

TEST_F(ConstantsTest, DisplayTimeValidation) {
    // Test that display time is reasonable
    EXPECT_EQ(MIX_STATUS_DISPLAY_TIME, 300); // 5 seconds
    
    // Test that display time is positive
    EXPECT_GT(MIX_STATUS_DISPLAY_TIME, 0);
    
    // Test that display time is reasonable (not too short, not too long)
    EXPECT_GE(MIX_STATUS_DISPLAY_TIME, 100);  // At least 1 second
    EXPECT_LE(MIX_STATUS_DISPLAY_TIME, 10000); // Not more than 10 seconds
}

TEST_F(ConstantsTest, FPSValidation) {
    // Test that FPS is reasonable
    EXPECT_EQ(DEFAULT_FPS, 60);
    
    // Test that FPS is in a reasonable range
    EXPECT_GE(DEFAULT_FPS, 30);  // Minimum smooth FPS
    EXPECT_LE(DEFAULT_FPS, 120); // Maximum reasonable FPS
}

TEST_F(ConstantsTest, BeatSensitivityStepValidation) {
    // Test that beat sensitivity step is reasonable
    EXPECT_EQ(BEAT_SENSITIVITY_STEP, 0.1f);
    
    // Test that step is positive and reasonable
    EXPECT_GT(BEAT_SENSITIVITY_STEP, 0.0f);
    EXPECT_LE(BEAT_SENSITIVITY_STEP, 1.0f); // Not too coarse
}

TEST_F(ConstantsTest, ConstantsConsistency) {
    // Test that related constants are consistent
    EXPECT_EQ(PCM_BUFFER_SIZE, DEFAULT_SAMPLES); // Should match
    
    // Test that frame delay calculation is consistent
    int expectedFrameDelay = 1000 / DEFAULT_FPS;
    EXPECT_EQ(FRAME_DELAY_MS, expectedFrameDelay);
    
    // Test that window dimensions are consistent
    EXPECT_EQ(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT); // Square window
}

TEST_F(ConstantsTest, ConstantsTypeValidation) {
    // Test that constants have appropriate types
    static_assert(std::is_integral_v<decltype(DEFAULT_SAMPLE_RATE)>, "Sample rate should be integral");
    static_assert(std::is_integral_v<decltype(DEFAULT_CHANNELS)>, "Channels should be integral");
    static_assert(std::is_integral_v<decltype(DEFAULT_SAMPLES)>, "Samples should be integral");
    static_assert(std::is_floating_point_v<decltype(MIN_BEAT_SENSITIVITY)>, "Beat sensitivity should be floating point");
    static_assert(std::is_floating_point_v<decltype(MAX_BEAT_SENSITIVITY)>, "Beat sensitivity should be floating point");
    static_assert(std::is_floating_point_v<decltype(BEAT_SENSITIVITY_STEP)>, "Beat sensitivity step should be floating point");
    
    EXPECT_TRUE(true); // If we get here, all static assertions passed
}

TEST_F(ConstantsTest, ConstantsRangeValidation) {
    // Test that constants are within reasonable ranges
    EXPECT_GT(DEFAULT_SAMPLE_RATE, 0);
    EXPECT_GT(DEFAULT_CHANNELS, 0);
    EXPECT_GT(DEFAULT_SAMPLES, 0);
    EXPECT_GE(MIN_BEAT_SENSITIVITY, 0.0f);
    EXPECT_GT(MAX_BEAT_SENSITIVITY, MIN_BEAT_SENSITIVITY);
    EXPECT_GT(BEAT_SENSITIVITY_STEP, 0.0f);
    EXPECT_GT(DEFAULT_WINDOW_WIDTH, 0);
    EXPECT_GT(DEFAULT_WINDOW_HEIGHT, 0);
    EXPECT_GT(MIX_STATUS_DISPLAY_TIME, 0);
    EXPECT_GT(PCM_BUFFER_SIZE, 0);
    EXPECT_GT(PCM_MAX_VALUE, 0);
    EXPECT_GT(DEFAULT_FPS, 0);
    EXPECT_GT(FRAME_DELAY_MS, 0);
} 