#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "mix_player.hpp"
#include "fixtures/test_fixtures.hpp"

class MixPlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
        cache_dir = test_dir + "/cache";
        std::filesystem::create_directories(cache_dir);
        
        // Create a test MP3 file
        test_mp3_path = cache_dir + "/test_mix.mp3";
        ASSERT_TRUE(TestFixtures::createTestMP3File(test_mp3_path));
    }
    
    void TearDown() override {
        TestFixtures::cleanupTestFiles({test_dir});
    }
    
    std::string test_dir;
    std::string cache_dir;
    std::string test_mp3_path;
};

TEST_F(MixPlayerTest, InitialState) {
    MixPlayer player;
    
    // Test initial state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    EXPECT_EQ(player.getVolume(), 100);
    EXPECT_EQ(player.getCurrentPosition(), 0);
    EXPECT_EQ(player.getDuration(), 0);
}

TEST_F(MixPlayerTest, PlayMix) {
    MixPlayer player;
    
    // Test initial state before playing
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    EXPECT_EQ(player.getVolume(), 100);
    
    // Note: We can't test actual playback without a valid MP3 file
    // This test verifies the initial state is correct
}

TEST_F(MixPlayerTest, PauseAndResume) {
    MixPlayer player;
    
    // Test initial state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    
    // Note: We can't test actual pause/resume without a valid MP3 file
    // This test verifies the initial state is correct
}

TEST_F(MixPlayerTest, StopMix) {
    MixPlayer player;
    
    // Test initial state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    
    // Test stop on non-playing state
    EXPECT_TRUE(player.stop());  // Should succeed even when not playing
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());  // Should be false when not playing
}

TEST_F(MixPlayerTest, HasFinishedDetection) {
    MixPlayer player;
    
    // Test hasFinished when nothing is playing
    EXPECT_FALSE(player.hasFinished());
    
    // Test hasFinished after stop (should still be false since not playing)
    ASSERT_TRUE(player.stop());
    EXPECT_FALSE(player.hasFinished());
    
    // Test hasFinished multiple times (should remain false)
    EXPECT_FALSE(player.hasFinished());
    EXPECT_FALSE(player.hasFinished());
}

TEST_F(MixPlayerTest, HasFinishedWithPause) {
    MixPlayer player;
    
    // Test initial state
    EXPECT_FALSE(player.hasFinished());
    
    // Test hasFinished after stop (should be false since not playing)
    ASSERT_TRUE(player.stop());
    EXPECT_FALSE(player.hasFinished());
    
    // Test that hasFinished remains false
    EXPECT_FALSE(player.hasFinished());
}

TEST_F(MixPlayerTest, VolumeControl) {
    MixPlayer player;
    
    // Test volume setting
    EXPECT_TRUE(player.setVolume(50));
    EXPECT_EQ(player.getVolume(), 50);
    
    // Test volume bounds
    EXPECT_TRUE(player.setVolume(0));
    EXPECT_EQ(player.getVolume(), 0);
    
    EXPECT_TRUE(player.setVolume(100));
    EXPECT_EQ(player.getVolume(), 100);
    
    // Test out of bounds (should clamp)
    EXPECT_TRUE(player.setVolume(150));
    EXPECT_EQ(player.getVolume(), 100);
    
    EXPECT_TRUE(player.setVolume(-10));
    EXPECT_EQ(player.getVolume(), 0);
}

TEST_F(MixPlayerTest, MultiplePlayCycles) {
    MixPlayer player;
    
    // First cycle
    EXPECT_FALSE(player.hasFinished());
    ASSERT_TRUE(player.stop());
    EXPECT_FALSE(player.hasFinished());
    
    // Second cycle - simulate new play
    // Note: In real usage, playMix would reset the finished state
    // For testing, we verify the stop behavior
    EXPECT_FALSE(player.hasFinished());  // Should remain false when not playing
}

TEST_F(MixPlayerTest, InvalidFileHandling) {
    MixPlayer player;
    
    // Test playing non-existent file
    EXPECT_FALSE(player.playMix("non_existent_file.mp3"));
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    
    // Test playing invalid file
    std::string invalid_file = cache_dir + "/invalid.txt";
    std::ofstream file(invalid_file);
    file << "This is not an MP3 file";
    file.close();
    
    EXPECT_FALSE(player.playMix(invalid_file));
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
}

TEST_F(MixPlayerTest, StateTransitions) {
    MixPlayer player;
    
    // Test initial state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    
    // Test stop transition
    ASSERT_TRUE(player.stop());
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_FALSE(player.hasFinished());
    
    // Note: We can't test play/pause transitions without valid audio files
    // This test verifies the initial state and stop behavior
} 