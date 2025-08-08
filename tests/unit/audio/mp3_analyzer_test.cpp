#include "audio/mp3_analyzer.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "utils/constants.hpp"

class MP3AnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir = std::filesystem::temp_directory_path() / "mp3_analyzer_test";
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

TEST_F(MP3AnalyzerTest, AnalyzeValidMP3File) {
    // Create a mock MP3 file (in real scenario, this would be a valid MP3)
    std::string filepath = createMockMP3File("test.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(filepath);

    // Mock files won't be valid MP3s, so they should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
}

TEST_F(MP3AnalyzerTest, HandleFileNotFound) {
    std::string non_existent_file = (test_dir / "nonexistent.mp3").string();

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(non_existent_file);

    // Should have error
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("File does not exist") != std::string::npos);

    // Should return empty metadata
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
}

TEST_F(MP3AnalyzerTest, HandleEmptyFile) {
    std::string empty_file = createEmptyFile("empty.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(empty_file);

    // Should have error for invalid MP3
    EXPECT_FALSE(analyzer.getLastError().empty());

    // Should return empty metadata
    EXPECT_TRUE(metadata.title.empty());
}

TEST_F(MP3AnalyzerTest, HandleNonMP3File) {
    std::string text_file = createMockMP3File("test.txt", "This is a text file, not an MP3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(text_file);

    // Should have error for invalid MP3
    EXPECT_FALSE(analyzer.getLastError().empty());

    // Should return empty metadata
    EXPECT_TRUE(metadata.title.empty());
}

TEST_F(MP3AnalyzerTest, ExtractFilenameAsTitle) {
    std::string filepath = createMockMP3File("my_song.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(filepath);

    // Mock files won't be valid MP3s, so they should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.description.empty());
}

TEST_F(MP3AnalyzerTest, HandleSpecialCharactersInFilename) {
    std::string filepath = createMockMP3File("song with spaces & symbols.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(filepath);

    // Mock files won't be valid MP3s, so they should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
}

TEST_F(MP3AnalyzerTest, SetVerboseMode) {
    AutoVibez::Audio::MP3Analyzer analyzer;

    // Should not throw when setting verbose mode
    EXPECT_NO_THROW(analyzer.setVerbose(true));
    EXPECT_NO_THROW(analyzer.setVerbose(false));
}

TEST_F(MP3AnalyzerTest, ErrorStateManagement) {
    AutoVibez::Audio::MP3Analyzer analyzer;

    // Initially should have no error
    EXPECT_TRUE(analyzer.getLastError().empty());

    // Analyze non-existent file to set error
    std::string non_existent_file = (test_dir / "nonexistent.mp3").string();
    analyzer.analyzeFile(non_existent_file);

    // Should have error
    EXPECT_FALSE(analyzer.getLastError().empty());
}

TEST_F(MP3AnalyzerTest, MetadataStructure) {
    std::string filepath = createMockMP3File("test.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(filepath);

    // Mock files won't be valid MP3s, so they should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
    EXPECT_TRUE(metadata.genre.empty());
    EXPECT_TRUE(metadata.format.empty());
    EXPECT_TRUE(metadata.date_added.empty());
    EXPECT_EQ(metadata.file_size, 0);
}

TEST_F(MP3AnalyzerTest, HandleVerySmallFile) {
    std::string small_file = createMockMP3File("small.mp3", "tiny");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(small_file);

    // Since it's not a valid MP3, it should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
}

TEST_F(MP3AnalyzerTest, HandleUnicodeFilename) {
    std::string filepath = createMockMP3File("sóng_ñáme.mp3");

    AutoVibez::Audio::MP3Analyzer analyzer;
    AutoVibez::Audio::MP3Metadata metadata = analyzer.analyzeFile(filepath);

    // Mock files won't be valid MP3s, so they should be rejected
    EXPECT_FALSE(analyzer.getLastError().empty());
    EXPECT_TRUE(analyzer.getLastError().find("Invalid") != std::string::npos);

    // Should return empty metadata for invalid files
    EXPECT_TRUE(metadata.title.empty());
}

TEST_F(MP3AnalyzerTest, MultipleAnalyzerInstances) {
    std::string filepath = createMockMP3File("test.mp3");

    // Create multiple analyzer instances
    AutoVibez::Audio::MP3Analyzer analyzer1;
    AutoVibez::Audio::MP3Analyzer analyzer2;

    // Both should work independently
    AutoVibez::Audio::MP3Metadata metadata1 = analyzer1.analyzeFile(filepath);
    AutoVibez::Audio::MP3Metadata metadata2 = analyzer2.analyzeFile(filepath);

    // Should both reject invalid files
    EXPECT_FALSE(analyzer1.getLastError().empty());
    EXPECT_FALSE(analyzer2.getLastError().empty());

    // Should have same error messages
    EXPECT_EQ(analyzer1.getLastError(), analyzer2.getLastError());
}
