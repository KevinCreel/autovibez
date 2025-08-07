#include "constants.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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





TEST_F(ConstantsTest, AudioSampleRateValidation) {
    // Test that sample rate is a standard value
    EXPECT_EQ(DEFAULT_SAMPLE_RATE, 44100);  // Standard CD quality

    // Test that sample rate is reasonable
    EXPECT_GE(DEFAULT_SAMPLE_RATE, 8000);    // Minimum reasonable
    EXPECT_LE(DEFAULT_SAMPLE_RATE, 192000);  // Maximum reasonable
}

TEST_F(ConstantsTest, AudioChannelsValidation) {
    // Test that channel count is reasonable
    EXPECT_EQ(DEFAULT_CHANNELS, 2);  // Stereo

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
    EXPECT_EQ(samples, 1);  // Should be a power of 2
}



TEST_F(ConstantsTest, ConstantsTypeValidation) {
    // Test that constants have appropriate types
    static_assert(std::is_integral_v<decltype(DEFAULT_SAMPLE_RATE)>, "Sample rate should be integral");
    static_assert(std::is_integral_v<decltype(DEFAULT_CHANNELS)>, "Channels should be integral");
    static_assert(std::is_integral_v<decltype(DEFAULT_SAMPLES)>, "Samples should be integral");

    EXPECT_TRUE(true);  // If we get here, all static assertions passed
}

TEST_F(ConstantsTest, ConstantsRangeValidation) {
    // Test that constants are within reasonable ranges
    EXPECT_GT(DEFAULT_SAMPLE_RATE, 0);
    EXPECT_GT(DEFAULT_CHANNELS, 0);
    EXPECT_GT(DEFAULT_SAMPLES, 0);
}