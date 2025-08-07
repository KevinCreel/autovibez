#include <gtest/gtest.h>
#include "audio_utils.hpp"
#include <fstream>
#include <filesystem>

using AutoVibez::Utils::AudioUtils;

class AudioUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        test_dir = std::filesystem::temp_directory_path() / "autovibez_test";
        std::filesystem::create_directories(test_dir);
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }
    
    std::filesystem::path test_dir;
    
    void createTestFile(const std::string& filename, const std::string& content) {
        std::filesystem::path file_path = test_dir / filename;
        std::ofstream file(file_path);
        file << content;
        file.close();
    }
    
    void createTestMP3File(const std::string& filename) {
        std::filesystem::path file_path = test_dir / filename;
        std::ofstream file(file_path, std::ios::binary);
        
        // Write a minimal MP3 header (ID3v2)
        char header[] = {'I', 'D', '3', 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        file.write(header, sizeof(header));
        
        // Add some dummy data to make it look like a real MP3
        for (int i = 0; i < 1024; i++) {
            file.put(0);
        }
        
        file.close();
    }
};



TEST_F(AudioUtilsTest, FileExists_ExistingFile) {
    createTestFile("test.txt", "test content");
    std::string file_path = (test_dir / "test.txt").string();
    EXPECT_TRUE(AudioUtils::isValidMP3File(file_path) == false); // Should fail validation, not existence
}

TEST_F(AudioUtilsTest, IsValidMP3File_ValidMP3) {
    createTestMP3File("test.mp3");
    std::string file_path = (test_dir / "test.mp3").string();
    EXPECT_TRUE(AudioUtils::isValidMP3File(file_path));
}

TEST_F(AudioUtilsTest, IsValidMP3File_InvalidExtension) {
    createTestMP3File("test.wav");
    std::string file_path = (test_dir / "test.wav").string();
    EXPECT_FALSE(AudioUtils::isValidMP3File(file_path));
}

TEST_F(AudioUtilsTest, IsValidMP3File_NonExistentFile) {
    std::string file_path = (test_dir / "nonexistent.mp3").string();
    EXPECT_FALSE(AudioUtils::isValidMP3File(file_path));
}

TEST_F(AudioUtilsTest, IsValidMP3File_EmptyFile) {
    createTestFile("empty.mp3", "");
    std::string file_path = (test_dir / "empty.mp3").string();
    EXPECT_FALSE(AudioUtils::isValidMP3File(file_path));
}

TEST_F(AudioUtilsTest, IsValidMP3File_SmallFile) {
    createTestFile("small.mp3", "small");
    std::string file_path = (test_dir / "small.mp3").string();
    EXPECT_FALSE(AudioUtils::isValidMP3File(file_path));
}
