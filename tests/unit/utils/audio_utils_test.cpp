#include "utils/audio_utils.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "utils/constants.hpp"

TEST(AudioUtilsTest, IsValidMP3File) {
    // Create a temporary directory for test files
    auto tempDir = std::filesystem::temp_directory_path() / "autovibez_audio_test";
    std::filesystem::create_directories(tempDir);

    // Create a mock MP3 file with proper ID3 header and MP3 frames
    auto mockMp3Path = tempDir / "test.mp3";
    std::ofstream mockMp3(mockMp3Path, std::ios::binary);

    // Write ID3v2 header (10 bytes)
    char id3Header[10] = {'I', 'D', '3', 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    mockMp3.write(id3Header, 10);

    // Write ID3v2 tag size (4 bytes) - small tag
    char tagSize[4] = {0x00, 0x00, 0x00, 0x10};  // 16 bytes tag
    mockMp3.write(tagSize, 4);

    // Write some tag data (16 bytes)
    std::string tagData(16, 'T');
    mockMp3.write(tagData.c_str(), tagData.length());

    // Write MP3 frame header (4 bytes) - MPEG-1 Layer 3, 128kbps, 44.1kHz
    unsigned char mp3FrameHeader[4] = {0xFF, 0xFB, 0x90, 0x44};  // Valid MP3 frame header
    mockMp3.write(reinterpret_cast<const char*>(mp3FrameHeader), 4);

    // Write some MP3 frame data (enough to meet minimum size)
    std::string frameData(Constants::MIN_MP3_FILE_SIZE - 34, 'M');  // 34 = 10 + 4 + 16 + 4
    mockMp3.write(frameData.c_str(), frameData.length());
    mockMp3.close();

    // Test that it recognizes the MP3 file
    EXPECT_TRUE(AutoVibez::Utils::AudioUtils::isValidMP3File(mockMp3Path.string()));

    // Test with non-existent file
    EXPECT_FALSE(AutoVibez::Utils::AudioUtils::isValidMP3File("/nonexistent/file.mp3"));

    // Test with non-MP3 file
    auto textFilePath = tempDir / "test.txt";
    std::ofstream textFile(textFilePath);
    textFile << "text content";
    textFile.close();
    EXPECT_FALSE(AutoVibez::Utils::AudioUtils::isValidMP3File(textFilePath.string()));

    // Test with file that has ID3 header but no MP3 frames
    auto invalidMp3Path = tempDir / "invalid.mp3";
    std::ofstream invalidMp3(invalidMp3Path, std::ios::binary);
    invalidMp3.write(id3Header, 10);
    std::string padding(Constants::MIN_MP3_FILE_SIZE - 10, 'A');
    invalidMp3.write(padding.c_str(), padding.length());
    invalidMp3.close();
    EXPECT_FALSE(AutoVibez::Utils::AudioUtils::isValidMP3File(invalidMp3Path.string()));

    // Cleanup
    std::filesystem::remove_all(tempDir);
}
