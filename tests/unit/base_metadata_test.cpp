#include "base_metadata.hpp"

#include <gtest/gtest.h>

#include "mix_metadata.hpp"
#include "mp3_analyzer.hpp"

using AutoVibez::Data::BaseMetadata;

class BaseMetadataTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BaseMetadataTest, DefaultConstructor_InitializesFields) {
    BaseMetadata metadata;

    EXPECT_TRUE(metadata.id.empty());
    EXPECT_TRUE(metadata.title.empty());
    EXPECT_TRUE(metadata.artist.empty());
    EXPECT_TRUE(metadata.genre.empty());
    EXPECT_TRUE(metadata.tags.empty());
    EXPECT_TRUE(metadata.description.empty());
    EXPECT_TRUE(metadata.local_path.empty());
    EXPECT_EQ(metadata.duration_seconds, 0);
    EXPECT_TRUE(metadata.date_added.empty());
    EXPECT_TRUE(metadata.last_played.empty());
    EXPECT_EQ(metadata.play_count, 0);
    EXPECT_FALSE(metadata.is_favorite);
}

TEST_F(BaseMetadataTest, HasValidId_EmptyId) {
    BaseMetadata metadata;
    EXPECT_FALSE(metadata.hasValidId());
}

TEST_F(BaseMetadataTest, HasValidId_ValidId) {
    BaseMetadata metadata;
    metadata.id = "test-id";
    EXPECT_TRUE(metadata.hasValidId());
}

TEST_F(BaseMetadataTest, HasValidTitle_EmptyTitle) {
    BaseMetadata metadata;
    EXPECT_FALSE(metadata.hasValidTitle());
}

TEST_F(BaseMetadataTest, HasValidTitle_ValidTitle) {
    BaseMetadata metadata;
    metadata.title = "Test Track";
    EXPECT_TRUE(metadata.hasValidTitle());
}

TEST_F(BaseMetadataTest, HasValidLocalPath_EmptyPath) {
    BaseMetadata metadata;
    EXPECT_FALSE(metadata.hasValidLocalPath());
}

TEST_F(BaseMetadataTest, HasValidLocalPath_ValidPath) {
    BaseMetadata metadata;
    metadata.local_path = "/path/to/file.mp3";
    EXPECT_TRUE(metadata.hasValidLocalPath());
}

TEST_F(BaseMetadataTest, HasBeenPlayed_ZeroPlayCount) {
    BaseMetadata metadata;
    metadata.play_count = 0;
    EXPECT_FALSE(metadata.hasBeenPlayed());
}

TEST_F(BaseMetadataTest, HasBeenPlayed_PositivePlayCount) {
    BaseMetadata metadata;
    metadata.play_count = 5;
    EXPECT_TRUE(metadata.hasBeenPlayed());
}

TEST_F(BaseMetadataTest, HasValidDuration_ZeroDuration) {
    BaseMetadata metadata;
    metadata.duration_seconds = 0;
    EXPECT_FALSE(metadata.hasValidDuration());
}

TEST_F(BaseMetadataTest, HasValidDuration_PositiveDuration) {
    BaseMetadata metadata;
    metadata.duration_seconds = 180;
    EXPECT_TRUE(metadata.hasValidDuration());
}

TEST_F(BaseMetadataTest, GetDisplayName_EmptyTitle) {
    BaseMetadata metadata;
    EXPECT_EQ(metadata.getDisplayName(), "Unknown Track");
}

TEST_F(BaseMetadataTest, GetDisplayName_ValidTitle) {
    BaseMetadata metadata;
    metadata.title = "Test Track";
    EXPECT_EQ(metadata.getDisplayName(), "Test Track");
}

TEST_F(BaseMetadataTest, GetDisplayArtist_EmptyArtist) {
    BaseMetadata metadata;
    EXPECT_EQ(metadata.getDisplayArtist(), "Unknown Artist");
}

TEST_F(BaseMetadataTest, GetDisplayArtist_ValidArtist) {
    BaseMetadata metadata;
    metadata.artist = "Test Artist";
    EXPECT_EQ(metadata.getDisplayArtist(), "Test Artist");
}

TEST_F(BaseMetadataTest, IsComplete_EmptyFields) {
    BaseMetadata metadata;
    EXPECT_FALSE(metadata.isComplete());
}

TEST_F(BaseMetadataTest, IsComplete_MissingId) {
    BaseMetadata metadata;
    metadata.title = "Test Track";
    metadata.local_path = "/path/to/file.mp3";
    EXPECT_FALSE(metadata.isComplete());
}

TEST_F(BaseMetadataTest, IsComplete_MissingTitle) {
    BaseMetadata metadata;
    metadata.id = "test-id";
    metadata.local_path = "/path/to/file.mp3";
    EXPECT_FALSE(metadata.isComplete());
}

TEST_F(BaseMetadataTest, IsComplete_MissingLocalPath) {
    BaseMetadata metadata;
    metadata.id = "test-id";
    metadata.title = "Test Track";
    EXPECT_FALSE(metadata.isComplete());
}

TEST_F(BaseMetadataTest, IsComplete_AllFieldsPresent) {
    BaseMetadata metadata;
    metadata.id = "test-id";
    metadata.title = "Test Track";
    metadata.local_path = "/path/to/file.mp3";
    EXPECT_TRUE(metadata.isComplete());
}

TEST_F(BaseMetadataTest, Inheritance_MP3Metadata) {
    // Test that MP3Metadata inherits from BaseMetadata
    AutoVibez::Audio::MP3Metadata mp3_metadata;

    // Should have all base fields
    EXPECT_TRUE(mp3_metadata.id.empty());
    EXPECT_TRUE(mp3_metadata.title.empty());
    EXPECT_TRUE(mp3_metadata.artist.empty());
    EXPECT_EQ(mp3_metadata.duration_seconds, 0);
    EXPECT_EQ(mp3_metadata.play_count, 0);
    EXPECT_FALSE(mp3_metadata.is_favorite);

    // Should have MP3-specific fields
    EXPECT_EQ(mp3_metadata.bitrate, 0);
    EXPECT_EQ(mp3_metadata.sample_rate, 0);
    EXPECT_EQ(mp3_metadata.channels, 0);
    EXPECT_EQ(mp3_metadata.file_size, 0);
    EXPECT_TRUE(mp3_metadata.format.empty());
    EXPECT_TRUE(mp3_metadata.url.empty());

    // Should have base methods
    EXPECT_FALSE(mp3_metadata.hasValidId());
    EXPECT_FALSE(mp3_metadata.hasValidTitle());
    EXPECT_FALSE(mp3_metadata.hasValidLocalPath());
    EXPECT_FALSE(mp3_metadata.hasBeenPlayed());
    EXPECT_FALSE(mp3_metadata.hasValidDuration());
    EXPECT_EQ(mp3_metadata.getDisplayName(), "Unknown Track");
    EXPECT_EQ(mp3_metadata.getDisplayArtist(), "Unknown Artist");
    EXPECT_FALSE(mp3_metadata.isComplete());
}

TEST_F(BaseMetadataTest, Inheritance_Mix) {
    // Test that Mix inherits from BaseMetadata
    AutoVibez::Data::Mix mix;

    // Should have all base fields
    EXPECT_TRUE(mix.id.empty());
    EXPECT_TRUE(mix.title.empty());
    EXPECT_TRUE(mix.artist.empty());
    EXPECT_EQ(mix.duration_seconds, 0);
    EXPECT_EQ(mix.play_count, 0);
    EXPECT_FALSE(mix.is_favorite);

    // Should have Mix-specific fields
    EXPECT_TRUE(mix.url.empty());
    EXPECT_TRUE(mix.original_filename.empty());

    // Should have base methods
    EXPECT_FALSE(mix.hasValidId());
    EXPECT_FALSE(mix.hasValidTitle());
    EXPECT_FALSE(mix.hasValidLocalPath());
    EXPECT_FALSE(mix.hasBeenPlayed());
    EXPECT_FALSE(mix.hasValidDuration());
    EXPECT_EQ(mix.getDisplayName(), "Unknown Track");
    EXPECT_EQ(mix.getDisplayArtist(), "Unknown Artist");
    EXPECT_FALSE(mix.isComplete());
}
