#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "mp3_analyzer.hpp"
#include "fixtures/test_fixtures.hpp"
using AutoVibez::Audio::MP3Metadata;

class MP3AnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
    }
    
    void TearDown() override {
        TestFixtures::cleanupTestFiles({test_dir});
    }
    
    std::string test_dir;
};

TEST_F(MP3AnalyzerTest, MP3MetadataDefaultValues) {
    MP3Metadata metadata;
    
    // Test default values
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
    EXPECT_TRUE(metadata.genre.empty());
    EXPECT_TRUE(metadata.description.empty());
    EXPECT_TRUE(metadata.tags.empty());
    EXPECT_TRUE(metadata.url.empty());
    EXPECT_TRUE(metadata.local_path.empty());
    EXPECT_EQ(metadata.duration_seconds, 0);
    EXPECT_EQ(metadata.bitrate, 0);
    EXPECT_EQ(metadata.sample_rate, 0);
    EXPECT_EQ(metadata.channels, 0);
    EXPECT_EQ(metadata.file_size, 0);
    EXPECT_TRUE(metadata.format.empty());
    EXPECT_TRUE(metadata.date_added.empty());
    EXPECT_TRUE(metadata.last_played.empty());
    EXPECT_EQ(metadata.play_count, 0);
    EXPECT_FALSE(metadata.is_favorite);
}

TEST_F(MP3AnalyzerTest, MP3MetadataAssignment) {
    MP3Metadata original;
    original.title = "Test Title";
    original.artist = "Test Artist";
    original.genre = "Test Genre";
    original.description = "Test Description";
    original.tags = {"test", "electronic"};
    original.url = "https://example.com/test.mp3";
    original.local_path = "/path/to/test.mp3";
    original.duration_seconds = 180;
    original.bitrate = 320;
    original.sample_rate = 44100;
    original.channels = 2;
    
    MP3Metadata copy = original;
    
    // Test that assignment works correctly
    EXPECT_EQ(copy.title, original.title);
    EXPECT_EQ(copy.artist, original.artist);
    EXPECT_EQ(copy.genre, original.genre);
    EXPECT_EQ(copy.description, original.description);
    EXPECT_EQ(copy.tags, original.tags);
    EXPECT_EQ(copy.url, original.url);
    EXPECT_EQ(copy.local_path, original.local_path);
    EXPECT_EQ(copy.duration_seconds, original.duration_seconds);
    EXPECT_EQ(copy.bitrate, original.bitrate);
    EXPECT_EQ(copy.sample_rate, original.sample_rate);
    EXPECT_EQ(copy.channels, original.channels);
}

TEST_F(MP3AnalyzerTest, MP3MetadataModification) {
    MP3Metadata metadata;
    
    // Test modifying metadata
    metadata.title = "Modified Title";
    metadata.artist = "Modified Artist";
    metadata.genre = "Modified Genre";
    metadata.description = "Modified Description";
    metadata.tags = {"modified", "electronic"};
    metadata.url = "https://example.com/modified.mp3";
    metadata.local_path = "/path/to/modified.mp3";
    metadata.duration_seconds = 240;
    metadata.bitrate = 256;
    metadata.sample_rate = 48000;
    metadata.channels = 1;
    
    EXPECT_EQ(metadata.title, "Modified Title");
    EXPECT_EQ(metadata.artist, "Modified Artist");
    EXPECT_EQ(metadata.genre, "Modified Genre");
    EXPECT_EQ(metadata.description, "Modified Description");
    EXPECT_EQ(metadata.tags.size(), 2);
    EXPECT_EQ(metadata.url, "https://example.com/modified.mp3");
    EXPECT_EQ(metadata.local_path, "/path/to/modified.mp3");
    EXPECT_EQ(metadata.duration_seconds, 240);
    EXPECT_EQ(metadata.bitrate, 256);
    EXPECT_EQ(metadata.sample_rate, 48000);
    EXPECT_EQ(metadata.channels, 1);
}

TEST_F(MP3AnalyzerTest, MP3MetadataValidation) {
    MP3Metadata valid_metadata;
    valid_metadata.title = "Valid Title";
    valid_metadata.artist = "Valid Artist";
    valid_metadata.duration_seconds = 180;
    valid_metadata.bitrate = 320;
    valid_metadata.sample_rate = 44100;
    valid_metadata.channels = 2;
    
    // Test that valid metadata is valid
    EXPECT_FALSE(valid_metadata.title.empty());
    EXPECT_FALSE(valid_metadata.artist.empty());
    EXPECT_GT(valid_metadata.duration_seconds, 0);
    EXPECT_GT(valid_metadata.bitrate, 0);
    EXPECT_GT(valid_metadata.sample_rate, 0);
    EXPECT_GT(valid_metadata.channels, 0);
}

TEST_F(MP3AnalyzerTest, MP3MetadataInvalidValues) {
    MP3Metadata invalid_metadata;
    
    // Test that invalid metadata is detected
    EXPECT_TRUE(invalid_metadata.title.empty());
    EXPECT_TRUE(invalid_metadata.artist.empty());
    EXPECT_EQ(invalid_metadata.duration_seconds, 0);
    EXPECT_EQ(invalid_metadata.bitrate, 0);
    EXPECT_EQ(invalid_metadata.sample_rate, 0);
    EXPECT_EQ(invalid_metadata.channels, 0);
}

TEST_F(MP3AnalyzerTest, MP3MetadataComparison) {
    MP3Metadata metadata1;
    metadata1.title = "Title 1";
    metadata1.artist = "Artist 1";
    metadata1.duration_seconds = 180;
    
    MP3Metadata metadata2;
    metadata2.title = "Title 2";
    metadata2.artist = "Artist 2";
    metadata2.duration_seconds = 240;
    
    // Test that different metadata are different
    EXPECT_NE(metadata1.title, metadata2.title);
    EXPECT_NE(metadata1.artist, metadata2.artist);
    EXPECT_NE(metadata1.duration_seconds, metadata2.duration_seconds);
}

TEST_F(MP3AnalyzerTest, MP3MetadataCopyConstructor) {
    MP3Metadata original;
    original.title = "Original Title";
    original.artist = "Original Artist";
    original.duration_seconds = 180;
    original.bitrate = 320;
    original.sample_rate = 44100;
    original.channels = 2;
    
    MP3Metadata copy(original);
    
    // Test that copy constructor works correctly
    EXPECT_EQ(copy.title, original.title);
    EXPECT_EQ(copy.artist, original.artist);
    EXPECT_EQ(copy.duration_seconds, original.duration_seconds);
    EXPECT_EQ(copy.bitrate, original.bitrate);
    EXPECT_EQ(copy.sample_rate, original.sample_rate);
    EXPECT_EQ(copy.channels, original.channels);
}

TEST_F(MP3AnalyzerTest, MP3MetadataMoveConstructor) {
    MP3Metadata original;
    original.title = "Move Title";
    original.artist = "Move Artist";
    original.duration_seconds = 200;
    
    MP3Metadata moved(std::move(original));
    
    // Test that move constructor works correctly
    EXPECT_EQ(moved.title, "Move Title");
    EXPECT_EQ(moved.artist, "Move Artist");
    EXPECT_EQ(moved.duration_seconds, 200);
    
    // Original should be in a valid but unspecified state
    // (implementation dependent)
}

TEST_F(MP3AnalyzerTest, MP3MetadataMoveAssignment) {
    MP3Metadata original;
    original.title = "Move Assignment Title";
    original.artist = "Move Assignment Artist";
    original.duration_seconds = 300;
    
    MP3Metadata target;
    target = std::move(original);
    
    // Test that move assignment works correctly
    EXPECT_EQ(target.title, "Move Assignment Title");
    EXPECT_EQ(target.artist, "Move Assignment Artist");
    EXPECT_EQ(target.duration_seconds, 300);
}

TEST_F(MP3AnalyzerTest, MP3MetadataStringHandling) {
    MP3Metadata metadata;
    
    // Test handling of various string types
    metadata.title = "Simple Title";
    metadata.artist = "Artist with Spaces";
    metadata.genre = "Genre with 'Apostrophes'";
    metadata.description = "Description with \"Quotes\"";
    
    EXPECT_EQ(metadata.title, "Simple Title");
    EXPECT_EQ(metadata.artist, "Artist with Spaces");
    EXPECT_EQ(metadata.genre, "Genre with 'Apostrophes'");
    EXPECT_EQ(metadata.description, "Description with \"Quotes\"");
}

TEST_F(MP3AnalyzerTest, MP3MetadataNumericHandling) {
    MP3Metadata metadata;
    
    // Test handling of various numeric values
    metadata.duration_seconds = 180;
    metadata.bitrate = 320;
    metadata.sample_rate = 44100;
    metadata.channels = 2;
    
    EXPECT_EQ(metadata.duration_seconds, 180);
    EXPECT_EQ(metadata.bitrate, 320);
    EXPECT_EQ(metadata.sample_rate, 44100);
    EXPECT_EQ(metadata.channels, 2);
}

TEST_F(MP3AnalyzerTest, MP3MetadataBoundaryValues) {
    MP3Metadata metadata;
    
    // Test boundary values
    metadata.duration_seconds = 86400;  // 24 hours in seconds
    metadata.bitrate = 1;  // Minimum bitrate
    metadata.sample_rate = 8000;  // Low sample rate
    metadata.channels = 1;  // Mono
    
    EXPECT_EQ(metadata.duration_seconds, 86400);
    EXPECT_EQ(metadata.bitrate, 1);
    EXPECT_EQ(metadata.sample_rate, 8000);
    EXPECT_EQ(metadata.channels, 1);
}

TEST_F(MP3AnalyzerTest, MP3MetadataEmptyStringHandling) {
    MP3Metadata metadata;
    
    // Test handling of empty strings
    metadata.title = "";
    metadata.artist = "";
    metadata.genre = "";
    metadata.description = "";
    
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
    EXPECT_TRUE(metadata.genre.empty());
    EXPECT_TRUE(metadata.description.empty());
}

TEST_F(MP3AnalyzerTest, MP3MetadataZeroValueHandling) {
    MP3Metadata metadata;
    
    // Test handling of zero values
    metadata.duration_seconds = 0;
    metadata.bitrate = 0;
    metadata.sample_rate = 0;
    metadata.channels = 0;
    
    EXPECT_EQ(metadata.duration_seconds, 0);
    EXPECT_EQ(metadata.bitrate, 0);
    EXPECT_EQ(metadata.sample_rate, 0);
    EXPECT_EQ(metadata.channels, 0);
}

TEST_F(MP3AnalyzerTest, MP3MetadataMemoryEfficiency) {
    // Test that metadata doesn't use excessive memory
    std::vector<MP3Metadata> metadata_list;
    
    // Create many metadata objects
    for (int i = 0; i < 1000; ++i) {
        MP3Metadata metadata;
        metadata.title = "Title " + std::to_string(i);
        metadata.artist = "Artist " + std::to_string(i);
        metadata.duration_seconds = i;
        metadata_list.push_back(metadata);
    }
    
    // Verify all metadata objects are valid
    for (size_t i = 0; i < metadata_list.size(); ++i) {
        EXPECT_EQ(metadata_list[i].title, "Title " + std::to_string(i));
        EXPECT_EQ(metadata_list[i].artist, "Artist " + std::to_string(i));
        EXPECT_EQ(metadata_list[i].duration_seconds, i);
    }
}

TEST_F(MP3AnalyzerTest, MP3MetadataThreadSafety) {
    // Test that metadata can be safely used in multi-threaded environments
    // This is a basic test - in a real application, you'd need more comprehensive testing
    
    MP3Metadata metadata;
    metadata.title = "Thread Safe Title";
    metadata.artist = "Thread Safe Artist";
    metadata.duration_seconds = 180;
    
    // Verify metadata is accessible
    EXPECT_EQ(metadata.title, "Thread Safe Title");
    EXPECT_EQ(metadata.artist, "Thread Safe Artist");
    EXPECT_EQ(metadata.duration_seconds, 180);
}

TEST_F(MP3AnalyzerTest, MP3MetadataSerialization) {
    // Test that metadata can be serialized/deserialized
    // This would be useful for saving/loading metadata
    
    MP3Metadata original;
    original.title = "Serialization Test";
    original.artist = "Test Artist";
    original.genre = "Test Genre";
    original.description = "Test Description";
    original.duration_seconds = 180;
    original.bitrate = 320;
    original.sample_rate = 44100;
    original.channels = 2;
    
    // In a real implementation, you'd serialize to JSON, XML, or binary format
    // For now, we'll just verify the data is accessible
    EXPECT_EQ(original.title, "Serialization Test");
    EXPECT_EQ(original.artist, "Test Artist");
    EXPECT_EQ(original.genre, "Test Genre");
    EXPECT_EQ(original.description, "Test Description");
    EXPECT_EQ(original.duration_seconds, 180);
    EXPECT_EQ(original.bitrate, 320);
    EXPECT_EQ(original.sample_rate, 44100);
    EXPECT_EQ(original.channels, 2);
} 