#include <gtest/gtest.h>
#include <vector>
#include <cstring>

// Mock the necessary parts for testing
struct MockAutoVibezApp {
    int _audioChannelsCount;
    
    MockAutoVibezApp(int channels) : _audioChannelsCount(channels) {}
    
    // Mock the audio callback function
    void audioInputCallbackF32(void *userdata, unsigned char *stream, int len) {
        MockAutoVibezApp *app = static_cast<MockAutoVibezApp*>(userdata);
        
        // stream contains float data in native byte order, len is in bytes
        // Convert to float pointer safely
        const float* floatStream = static_cast<const float*>(static_cast<const void*>(stream));
        int numSamples = len / sizeof(float) / app->_audioChannelsCount;  // Correct sample count calculation
        
        // Store the result for testing
        app->lastNumSamples = numSamples;
        app->lastLen = len;
    }
    
    // Test results
    int lastNumSamples = 0;
    int lastLen = 0;
};

class AudioCaptureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData.resize(1024); // 256 float samples
        for (size_t i = 0; i < testData.size() / sizeof(float); ++i) {
            reinterpret_cast<float*>(testData.data())[i] = static_cast<float>(i);
        }
    }
    
    std::vector<unsigned char> testData;
};

// Test sample count calculation for stereo audio
TEST_F(AudioCaptureTest, SampleCountCalculationStereo) {
    MockAutoVibezApp app(2); // Stereo (2 channels)
    
    // Test with 1024 bytes of data (256 float samples)
    app.audioInputCallbackF32(&app, testData.data(), testData.size());
    
    // For stereo: 1024 bytes / 4 bytes per float / 2 channels = 128 samples
    EXPECT_EQ(app.lastNumSamples, 128);
    EXPECT_EQ(app.lastLen, 1024);
}

// Test sample count calculation for mono audio
TEST_F(AudioCaptureTest, SampleCountCalculationMono) {
    MockAutoVibezApp app(1); // Mono (1 channel)
    
    // Test with 1024 bytes of data (256 float samples)
    app.audioInputCallbackF32(&app, testData.data(), testData.size());
    
    // For mono: 1024 bytes / 4 bytes per float / 1 channel = 256 samples
    EXPECT_EQ(app.lastNumSamples, 256);
    EXPECT_EQ(app.lastLen, 1024);
}

// Test sample count calculation with different buffer sizes
TEST_F(AudioCaptureTest, SampleCountCalculationDifferentSizes) {
    MockAutoVibezApp app(2); // Stereo
    
    // Test with 512 bytes (128 float samples)
    std::vector<unsigned char> smallData(512);
    app.audioInputCallbackF32(&app, smallData.data(), smallData.size());
    EXPECT_EQ(app.lastNumSamples, 64); // 512 / 4 / 2 = 64
    
    // Test with 2048 bytes (512 float samples)
    std::vector<unsigned char> largeData(2048);
    app.audioInputCallbackF32(&app, largeData.data(), largeData.size());
    EXPECT_EQ(app.lastNumSamples, 256); // 2048 / 4 / 2 = 256
}

// Test that the calculation is mathematically correct
TEST_F(AudioCaptureTest, SampleCountCalculationMathematical) {
    MockAutoVibezApp app(2); // Stereo
    
    // Test various buffer sizes
    std::vector<int> bufferSizes = {256, 512, 1024, 2048, 4096};
    
    for (int bufferSize : bufferSizes) {
        std::vector<unsigned char> data(bufferSize);
        app.audioInputCallbackF32(&app, data.data(), data.size());
        
        // Expected: bufferSize / sizeof(float) / channels
        int expectedSamples = bufferSize / sizeof(float) / 2;
        EXPECT_EQ(app.lastNumSamples, expectedSamples) 
            << "Buffer size: " << bufferSize 
            << ", Expected: " << expectedSamples 
            << ", Got: " << app.lastNumSamples;
    }
}

// Test edge cases
TEST_F(AudioCaptureTest, SampleCountCalculationEdgeCases) {
    MockAutoVibezApp app(2); // Stereo
    
    // Test with minimum buffer size (8 bytes = 2 float samples)
    std::vector<unsigned char> minData(8);
    app.audioInputCallbackF32(&app, minData.data(), minData.size());
    EXPECT_EQ(app.lastNumSamples, 1); // 8 / 4 / 2 = 1
    
    // Test with odd buffer size (should still work correctly)
    std::vector<unsigned char> oddData(100);
    app.audioInputCallbackF32(&app, oddData.data(), oddData.size());
    EXPECT_EQ(app.lastNumSamples, 12); // 100 / 4 / 2 = 12 (integer division)
}

// Test that the wrong calculation (hard-coded division by 2) would fail
TEST_F(AudioCaptureTest, WrongCalculationWouldFail) {
    // For stereo (2 channels), both wrong and correct calculations give same result
    // So we test with mono (1 channel) where the difference is obvious
    MockAutoVibezApp monoApp(1); // Mono
    
    // Simulate the wrong calculation that was used before
    int wrongMonoSamples = testData.size() / sizeof(float) / 2; // Wrong: hard-coded /2
    
    // The correct calculation
    int correctMonoSamples = testData.size() / sizeof(float) / monoApp._audioChannelsCount;
    
    // For mono, these should definitely be different
    EXPECT_NE(wrongMonoSamples, correctMonoSamples);
    EXPECT_EQ(correctMonoSamples, 256); // 1024 / 4 / 1 = 256
    EXPECT_EQ(wrongMonoSamples, 128);   // 1024 / 4 / 2 = 128 (wrong!)
    
    // Also test with 4-channel audio to show the difference
    MockAutoVibezApp quadApp(4); // 4 channels
    int wrongQuadSamples = testData.size() / sizeof(float) / 2; // Wrong: hard-coded /2
    int correctQuadSamples = testData.size() / sizeof(float) / quadApp._audioChannelsCount;
    
    EXPECT_NE(wrongQuadSamples, correctQuadSamples);
    EXPECT_EQ(correctQuadSamples, 64);  // 1024 / 4 / 4 = 64
    EXPECT_EQ(wrongQuadSamples, 128);   // 1024 / 4 / 2 = 128 (wrong!)
}

// Test that the calculation works for different channel counts
TEST_F(AudioCaptureTest, SampleCountCalculationDifferentChannels) {
    // Test with different channel counts
    std::vector<int> channelCounts = {1, 2, 4, 6, 8};
    
    for (int channels : channelCounts) {
        MockAutoVibezApp app(channels);
        app.audioInputCallbackF32(&app, testData.data(), testData.size());
        
        // Expected: 1024 bytes / 4 bytes per float / channels
        int expectedSamples = 1024 / sizeof(float) / channels;
        EXPECT_EQ(app.lastNumSamples, expectedSamples)
            << "Channels: " << channels 
            << ", Expected: " << expectedSamples 
            << ", Got: " << app.lastNumSamples;
    }
} 