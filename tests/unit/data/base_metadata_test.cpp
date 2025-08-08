#include "data/base_metadata.hpp"

#include <gtest/gtest.h>

TEST(BaseMetadataTest, DefaultConstructor) {
    AutoVibez::Data::BaseMetadata metadata;

    // Should have default values
    EXPECT_TRUE(metadata.id.empty());
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
    EXPECT_EQ(metadata.duration_seconds, 0);
    EXPECT_EQ(metadata.play_count, 0);
    EXPECT_FALSE(metadata.is_favorite);
}

TEST(BaseMetadataTest, ValidationMethods) {
    AutoVibez::Data::BaseMetadata metadata;

    // Initially should not be valid
    EXPECT_FALSE(metadata.hasValidId());
    EXPECT_FALSE(metadata.hasValidTitle());
    EXPECT_FALSE(metadata.hasValidLocalPath());
    EXPECT_FALSE(metadata.hasBeenPlayed());
    EXPECT_FALSE(metadata.hasValidDuration());

    // Set valid data
    metadata.id = "test-id";
    metadata.title = "Test Track";
    metadata.local_path = "/path/to/file.mp3";
    metadata.duration_seconds = 180;
    metadata.play_count = 1;

    // Should now be valid
    EXPECT_TRUE(metadata.hasValidId());
    EXPECT_TRUE(metadata.hasValidTitle());
    EXPECT_TRUE(metadata.hasValidLocalPath());
    EXPECT_TRUE(metadata.hasBeenPlayed());
    EXPECT_TRUE(metadata.hasValidDuration());
}

TEST(BaseMetadataTest, DisplayMethods) {
    AutoVibez::Data::BaseMetadata metadata;

    // Test display methods with empty data
    EXPECT_EQ(metadata.getDisplayName(), "Unknown Track");
    EXPECT_EQ(metadata.getDisplayArtist(), "Unknown Artist");

    // Test display methods with valid data
    metadata.title = "My Test Track";
    metadata.artist = "Test Artist";

    EXPECT_EQ(metadata.getDisplayName(), "My Test Track");
    EXPECT_EQ(metadata.getDisplayArtist(), "Test Artist");
}

TEST(BaseMetadataTest, EdgeCases) {
    AutoVibez::Data::BaseMetadata metadata;

    // Test with very long strings
    metadata.title = std::string(1000, 'A');
    metadata.artist = std::string(1000, 'B');
    EXPECT_TRUE(metadata.hasValidTitle());
    EXPECT_EQ(metadata.getDisplayName(), std::string(1000, 'A'));
    EXPECT_EQ(metadata.getDisplayArtist(), std::string(1000, 'B'));

    // Test with special characters
    metadata.title = "Track with special chars: \n\t\r\"'";
    metadata.artist = "Artist with special chars: \n\t\r\"'";
    EXPECT_TRUE(metadata.hasValidTitle());
    EXPECT_EQ(metadata.getDisplayName(), "Track with special chars: \n\t\r\"'");
    EXPECT_EQ(metadata.getDisplayArtist(), "Artist with special chars: \n\t\r\"'");

    // Test with zero duration
    metadata.duration_seconds = 0;
    EXPECT_FALSE(metadata.hasValidDuration());

    // Test with negative values
    metadata.duration_seconds = -1;
    metadata.play_count = -5;
    EXPECT_FALSE(metadata.hasValidDuration());
    EXPECT_FALSE(metadata.hasBeenPlayed());

    // Test completeness check
    metadata.id = "test-id";
    metadata.title = "Test Track";
    metadata.local_path = "/path/to/file.mp3";
    EXPECT_TRUE(metadata.isComplete());

    // Test incomplete metadata
    metadata.local_path = "";
    EXPECT_FALSE(metadata.isComplete());
}
