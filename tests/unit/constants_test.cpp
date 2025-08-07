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

TEST_F(ConstantsTest, BeatSensitivityStepValidation) {
    // Test that beat sensitivity step is reasonable
    EXPECT_EQ(BEAT_SENSITIVITY_STEP, 0.1f);
    
    // Test that step is positive and reasonable
    EXPECT_GT(BEAT_SENSITIVITY_STEP, 0.0f);
    EXPECT_LE(BEAT_SENSITIVITY_STEP, 1.0f); // Not too coarse
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
} 