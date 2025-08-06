#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <SDL2/SDL.h>
#include "audio_manager.hpp"
#include "autovibez_app.hpp"

// Mock AutoVibezApp for testing
class MockAutoVibezApp {
public:
    MockAutoVibezApp() = default;
    ~MockAutoVibezApp() = default;
    
    // Mock methods that AudioManager might call
    void initAudioInput() {}
    void beginAudioCapture() {}
    void endAudioCapture() {}
    int getSelectedAudioDeviceIndex() const { return 0; }
    void setSelectedAudioDeviceIndex(int index) {}
};

class AudioManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for testing
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            GTEST_SKIP() << "SDL audio initialization failed";
        }
        
        mockApp = std::make_unique<MockAutoVibezApp>();
    }
    
    void TearDown() override {
        mockApp.reset();
        SDL_Quit();
    }
    
    std::unique_ptr<MockAutoVibezApp> mockApp;
};

TEST_F(AudioManagerTest, Constructor_WithValidApp_ShouldSucceed) {
    // Arrange & Act
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    
    // Assert
    // Constructor should not throw
    EXPECT_NO_THROW();
}

TEST_F(AudioManagerTest, Initialize_WithValidSDL_ShouldSucceed) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    
    // Act
    bool result = manager.initialize();
    
    // Assert
    EXPECT_TRUE(result);
}

TEST_F(AudioManagerTest, GetDeviceCount_AfterInitialization_ShouldReturnValidCount) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    int deviceCount = manager.getDeviceCount();
    
    // Assert
    EXPECT_GE(deviceCount, 0);
    // Should have at least one device (system default)
    EXPECT_GT(deviceCount, 0);
}

TEST_F(AudioManagerTest, GetCurrentDevice_AfterInitialization_ShouldReturnValidIndex) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    int currentDevice = manager.getCurrentDevice();
    
    // Assert
    EXPECT_GE(currentDevice, 0);
    EXPECT_LT(currentDevice, manager.getDeviceCount());
}

TEST_F(AudioManagerTest, GetCurrentDeviceName_AfterInitialization_ShouldReturnValidName) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    std::string deviceName = manager.getCurrentDeviceName();
    
    // Assert
    EXPECT_FALSE(deviceName.empty());
    EXPECT_GT(deviceName.length(), 0);
}

TEST_F(AudioManagerTest, GetDeviceNames_AfterInitialization_ShouldReturnValidNames) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    std::vector<std::string> deviceNames = manager.getDeviceNames();
    
    // Assert
    EXPECT_EQ(deviceNames.size(), manager.getDeviceCount());
    for (const auto& name : deviceNames) {
        EXPECT_FALSE(name.empty());
    }
}

TEST_F(AudioManagerTest, CycleDevice_WithMultipleDevices_ShouldChangeDevice) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    int initialDevice = manager.getCurrentDevice();
    int deviceCount = manager.getDeviceCount();
    
    // Skip test if only one device available
    if (deviceCount <= 1) {
        GTEST_SKIP() << "Only one audio device available, cannot test cycling";
    }
    
    // Act
    manager.cycleDevice();
    int newDevice = manager.getCurrentDevice();
    
    // Assert
    EXPECT_NE(initialDevice, newDevice);
    EXPECT_GE(newDevice, 0);
    EXPECT_LT(newDevice, deviceCount);
}

TEST_F(AudioManagerTest, SetDevice_WithValidIndex_ShouldChangeDevice) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    int deviceCount = manager.getDeviceCount();
    int targetDevice = (manager.getCurrentDevice() + 1) % deviceCount;
    
    // Act
    manager.setDevice(targetDevice);
    
    // Assert
    EXPECT_EQ(manager.getCurrentDevice(), targetDevice);
}

TEST_F(AudioManagerTest, SetDevice_WithInvalidIndex_ShouldHandleGracefully) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    int initialDevice = manager.getCurrentDevice();
    int invalidDevice = -1;
    
    // Act
    manager.setDevice(invalidDevice);
    
    // Assert
    // Should either keep current device or handle gracefully
    EXPECT_GE(manager.getCurrentDevice(), 0);
    EXPECT_LT(manager.getCurrentDevice(), manager.getDeviceCount());
}

TEST_F(AudioManagerTest, IsCapturing_Initially_ShouldBeFalse) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act & Assert
    EXPECT_FALSE(manager.isCapturing());
}

TEST_F(AudioManagerTest, StartCapture_ShouldBeginCapturing) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    manager.startCapture();
    
    // Assert
    EXPECT_TRUE(manager.isCapturing());
}

TEST_F(AudioManagerTest, StopCapture_AfterStart_ShouldStopCapturing) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    manager.startCapture();
    ASSERT_TRUE(manager.isCapturing());
    
    // Act
    manager.stopCapture();
    
    // Assert
    EXPECT_FALSE(manager.isCapturing());
}

TEST_F(AudioManagerTest, ToggleInput_WhenNotCapturing_ShouldStartCapturing) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    ASSERT_FALSE(manager.isCapturing());
    
    // Act
    manager.toggleInput();
    
    // Assert
    EXPECT_TRUE(manager.isCapturing());
}

TEST_F(AudioManagerTest, ToggleInput_WhenCapturing_ShouldStopCapturing) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    manager.startCapture();
    ASSERT_TRUE(manager.isCapturing());
    
    // Act
    manager.toggleInput();
    
    // Assert
    EXPECT_FALSE(manager.isCapturing());
}

TEST_F(AudioManagerTest, GetDeviceId_AfterInitialization_ShouldReturnValidId) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    SDL_AudioDeviceID deviceId = manager.getDeviceId();
    
    // Assert
    EXPECT_GT(deviceId, 0);
}

TEST_F(AudioManagerTest, GetChannelsCount_AfterInitialization_ShouldReturnValidCount) {
    // Arrange
    AutoVibez::Audio::AudioManager manager(reinterpret_cast<AutoVibez::Core::AutoVibezApp*>(mockApp.get()));
    ASSERT_TRUE(manager.initialize());
    
    // Act
    unsigned short channelsCount = manager.getChannelsCount();
    
    // Assert
    EXPECT_GT(channelsCount, 0);
    EXPECT_LE(channelsCount, 8); // Reasonable upper limit for audio channels
} 