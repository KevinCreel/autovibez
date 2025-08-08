#include "audio/mix_player.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "utils/constants.hpp"

class MixPlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir = std::filesystem::temp_directory_path() / "mix_player_test";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(test_dir);
    }

    std::string createMockMP3File(const std::string& filename, const std::string& content = "mock mp3 content") {
        std::string filepath = (test_dir / filename).string();
        std::ofstream file(filepath);

        // Create a file that meets minimum MP3 size requirement
        std::string large_content = content;
        while (large_content.length() < Constants::MIN_MP3_FILE_SIZE) {
            large_content += content;
        }

        file << large_content;
        file.close();
        return filepath;
    }

    std::string createEmptyFile(const std::string& filename) {
        std::string filepath = (test_dir / filename).string();
        std::ofstream file(filepath);
        file.close();
        return filepath;
    }

    std::filesystem::path test_dir;
};

TEST_F(MixPlayerTest, ConstructorInitialization) {
    AutoVibez::Audio::MixPlayer player;

    // Should be initialized in stopped state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_EQ(player.getCurrentPosition(), 0);
    EXPECT_EQ(player.getDuration(), 0);
    EXPECT_EQ(player.getVolume(), Constants::MAX_VOLUME);
}

TEST_F(MixPlayerTest, PlayValidMixFile) {
    std::string filepath = createMockMP3File("test.mp3");

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(filepath);

    // Should fail because mock file is not a valid MP3
    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
    EXPECT_TRUE(player.getLastError().find("valid MP3") != std::string::npos);
}

TEST_F(MixPlayerTest, PlayNonExistentFile) {
    std::string non_existent_file = (test_dir / "nonexistent.mp3").string();

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(non_existent_file);

    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
    EXPECT_TRUE(player.getLastError().find("File does not exist") != std::string::npos);
}

TEST_F(MixPlayerTest, PlayEmptyFile) {
    std::string empty_file = createEmptyFile("empty.mp3");

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(empty_file);

    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
}

TEST_F(MixPlayerTest, PlayNonMP3File) {
    std::string text_file = createMockMP3File("test.txt", "This is a text file");

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(text_file);

    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
    EXPECT_TRUE(player.getLastError().find("valid MP3") != std::string::npos);
}

TEST_F(MixPlayerTest, VolumeControl) {
    AutoVibez::Audio::MixPlayer player;

    // Test normal volume setting
    EXPECT_TRUE(player.setVolume(50, false));
    EXPECT_EQ(player.getVolume(), 50);

    // Test volume clamping at minimum
    EXPECT_TRUE(player.setVolume(-10, false));
    EXPECT_EQ(player.getVolume(), Constants::MIN_VOLUME);

    // Test volume clamping at maximum
    EXPECT_TRUE(player.setVolume(150, false));
    EXPECT_EQ(player.getVolume(), Constants::MAX_VOLUME);

    // Test volume with suppressed output
    EXPECT_TRUE(player.setVolume(75, true));
    EXPECT_EQ(player.getVolume(), 75);
}

TEST_F(MixPlayerTest, PlaybackControlWhenNotPlaying) {
    AutoVibez::Audio::MixPlayer player;

    // Should handle controls gracefully when not playing
    EXPECT_FALSE(player.togglePause());
    EXPECT_FALSE(player.getLastError().empty());
    EXPECT_TRUE(player.getLastError().find("No music is currently playing") != std::string::npos);

    // Stop should succeed even when not playing
    EXPECT_TRUE(player.stop());
}

TEST_F(MixPlayerTest, StateManagement) {
    AutoVibez::Audio::MixPlayer player;

    // Initial state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_EQ(player.getCurrentPosition(), 0);
    EXPECT_EQ(player.getDuration(), 0);

    // After trying to play invalid file
    std::string invalid_file = createMockMP3File("invalid.mp3");
    player.playMix(invalid_file);

    // Should still be in stopped state
    EXPECT_FALSE(player.isPlaying());
    EXPECT_FALSE(player.isPaused());
    EXPECT_EQ(player.getCurrentPosition(), 0);
}

TEST_F(MixPlayerTest, MultiplePlayerInstances) {
    std::string filepath = createMockMP3File("test.mp3");

    // Create multiple player instances
    AutoVibez::Audio::MixPlayer player1;
    AutoVibez::Audio::MixPlayer player2;

    // Both should work independently
    bool result1 = player1.playMix(filepath);
    bool result2 = player2.playMix(filepath);

    // Both should fail with invalid files
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(player1.getLastError().empty());
    EXPECT_FALSE(player2.getLastError().empty());
}

TEST_F(MixPlayerTest, ErrorStateManagement) {
    AutoVibez::Audio::MixPlayer player;

    // Initially should have no error
    EXPECT_TRUE(player.getLastError().empty());

    // Try to play non-existent file
    std::string non_existent_file = (test_dir / "nonexistent.mp3").string();
    player.playMix(non_existent_file);

    // Should have error
    EXPECT_FALSE(player.getLastError().empty());
}

TEST_F(MixPlayerTest, SetVerboseMode) {
    AutoVibez::Audio::MixPlayer player;

    // Should not throw when setting verbose mode
    EXPECT_NO_THROW(player.setVerbose(true));
    EXPECT_NO_THROW(player.setVerbose(false));
}

TEST_F(MixPlayerTest, HasFinishedWhenNotPlaying) {
    AutoVibez::Audio::MixPlayer player;

    // Should return false when not playing
    EXPECT_FALSE(player.hasFinished());
}

TEST_F(MixPlayerTest, DurationAndPosition) {
    AutoVibez::Audio::MixPlayer player;

    // Should return 0 when not playing
    EXPECT_EQ(player.getCurrentPosition(), 0);
    EXPECT_EQ(player.getDuration(), 0);
}

TEST_F(MixPlayerTest, VolumeBoundaryValues) {
    AutoVibez::Audio::MixPlayer player;

    // Test boundary values
    EXPECT_TRUE(player.setVolume(0, false));
    EXPECT_EQ(player.getVolume(), 0);

    EXPECT_TRUE(player.setVolume(100, false));
    EXPECT_EQ(player.getVolume(), 100);

    // Test values outside bounds
    EXPECT_TRUE(player.setVolume(-1, false));
    EXPECT_EQ(player.getVolume(), Constants::MIN_VOLUME);

    EXPECT_TRUE(player.setVolume(101, false));
    EXPECT_EQ(player.getVolume(), Constants::MAX_VOLUME);
}

TEST_F(MixPlayerTest, FilePathWithSpecialCharacters) {
    std::string filepath = createMockMP3File("song with spaces & symbols.mp3");

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(filepath);

    // Should fail because mock file is not a valid MP3
    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
}

TEST_F(MixPlayerTest, UnicodeFilePath) {
    std::string filepath = createMockMP3File("sóng_ñáme.mp3");

    AutoVibez::Audio::MixPlayer player;
    bool result = player.playMix(filepath);

    // Should fail because mock file is not a valid MP3
    EXPECT_FALSE(result);
    EXPECT_FALSE(player.getLastError().empty());
}

TEST_F(MixPlayerTest, RepeatedPlayAttempts) {
    std::string filepath = createMockMP3File("test.mp3");

    AutoVibez::Audio::MixPlayer player;

    // Try to play the same invalid file multiple times
    for (int i = 0; i < 3; i++) {
        bool result = player.playMix(filepath);
        EXPECT_FALSE(result);
        EXPECT_FALSE(player.getLastError().empty());
    }
}
