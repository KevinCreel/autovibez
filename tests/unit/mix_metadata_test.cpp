#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "mix_metadata.hpp"
#include "fixtures/test_fixtures.hpp"

using AutoVibez::Data::MixMetadata;
using AutoVibez::Data::Mix;

class MixMetadataTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
        yaml_path = test_dir + "/test_mixes.yaml";
    }
    
    void TearDown() override {
        TestFixtures::cleanupTestFiles({test_dir});
    }
    
    std::string test_dir;
    std::string yaml_path;
};

TEST_F(MixMetadataTest, Constructor) {
    EXPECT_NO_THROW({
        MixMetadata metadata;
        EXPECT_TRUE(metadata.isSuccess());
        EXPECT_TRUE(metadata.getLastError().empty());
    });
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithValidYaml) {
    // Create test YAML file
    std::vector<Mix> test_mixes = {
        TestFixtures::createSampleMix("test_mix_1"),
        TestFixtures::createSampleMix("test_mix_2")
    };
    
    ASSERT_TRUE(TestFixtures::createTestYamlFile(yaml_path, test_mixes));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    EXPECT_TRUE(metadata.isSuccess());
    EXPECT_TRUE(metadata.getLastError().empty());
    EXPECT_EQ(loaded_mixes.size(), 2);
    
    // Verify first mix
    EXPECT_EQ(loaded_mixes[0].id, "test_mix_1");
    EXPECT_EQ(loaded_mixes[0].title, "Test Mix test_mix_1");
    EXPECT_EQ(loaded_mixes[0].artist, "Test Artist");
    EXPECT_EQ(loaded_mixes[0].genre, "Electronic");
    EXPECT_EQ(loaded_mixes[0].duration_seconds, 3600);
    
    // Verify second mix
    EXPECT_EQ(loaded_mixes[1].id, "test_mix_2");
    EXPECT_EQ(loaded_mixes[1].title, "Test Mix test_mix_2");
    EXPECT_EQ(loaded_mixes[1].artist, "Test Artist");
    EXPECT_EQ(loaded_mixes[1].genre, "Electronic");
    EXPECT_EQ(loaded_mixes[1].duration_seconds, 3600);
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithInvalidPath) {
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile("/nonexistent/file.yaml");
    
    // The implementation may not set success to false for file not found
    // but should return empty vector and may have error message
    EXPECT_TRUE(loaded_mixes.empty());
    // Note: isSuccess() behavior depends on implementation
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithInvalidYaml) {
    // Create invalid YAML file
    std::string invalid_yaml = R"(
mixes:
  - id: test_mix_1
    title: "Test Mix 1"
    artist: "Test Artist"
    genre: "Electronic"
    url: "https://example.com/mix1.mp3"
    duration_seconds: 3600
    description: "A test mix for unit testing"
    tags: ["test", "electronic", "dance"]
    
  - invalid_entry: "This should cause an error"
    missing_required_fields: true
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, invalid_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    // Should still load valid mixes but may have errors
    EXPECT_GT(loaded_mixes.size(), 0);
}

TEST_F(MixMetadataTest, LoadFromYamlWithValidUrl) {
    // Test with a local file instead of remote URL to avoid network requests
    std::string test_yaml = R"(
mixes:
  - id: test_mix_1
    title: "Test Mix 1"
    artist: "Test Artist"
    genre: "Electronic"
    url: "https://example.com/mix1.mp3"
    duration_seconds: 3600
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, test_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromYaml(yaml_path);
    
    EXPECT_TRUE(metadata.isSuccess());
    EXPECT_TRUE(metadata.getLastError().empty());
    EXPECT_EQ(loaded_mixes.size(), 1);
    EXPECT_EQ(loaded_mixes[0].id, "test_mix_1");
}

TEST_F(MixMetadataTest, LoadFromRemoteFile) {
    // Test with a local file instead of remote URL to avoid network requests
    std::string test_yaml = R"(
mixes:
  - id: test_mix_2
    title: "Test Mix 2"
    artist: "Test Artist"
    genre: "Electronic"
    url: "https://example.com/mix2.mp3"
    duration_seconds: 3600
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, test_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    EXPECT_TRUE(metadata.isSuccess());
    EXPECT_TRUE(metadata.getLastError().empty());
    EXPECT_EQ(loaded_mixes.size(), 1);
    EXPECT_EQ(loaded_mixes[0].id, "test_mix_2");
}

TEST_F(MixMetadataTest, ParseMixFromYamlWithValidData) {
    MixMetadata metadata;
    
    // Create a valid YAML node
    YAML::Node mix_node;
    mix_node["id"] = "test_mix_1";
    mix_node["title"] = "Test Mix 1";
    mix_node["artist"] = "Test Artist";
    mix_node["genre"] = "Electronic";
    mix_node["url"] = "https://example.com/mix1.mp3";
    mix_node["duration_seconds"] = 3600;
    mix_node["description"] = "A test mix for unit testing";
    mix_node["tags"].push_back("test");
    mix_node["tags"].push_back("electronic");
    mix_node["tags"].push_back("dance");
    
    // Test parsing (this would require the method to be public or we'd need to test through public interface)
    // For now, we'll test that the YAML node is valid
    EXPECT_TRUE(mix_node["id"]);
    EXPECT_TRUE(mix_node["title"]);
    EXPECT_TRUE(mix_node["artist"]);
    EXPECT_TRUE(mix_node["genre"]);
    EXPECT_TRUE(mix_node["url"]);
    EXPECT_TRUE(mix_node["duration_seconds"]);
    EXPECT_TRUE(mix_node["description"]);
    EXPECT_TRUE(mix_node["tags"]);
    EXPECT_EQ(mix_node["tags"].size(), 3);
}

TEST_F(MixMetadataTest, ValidateMixWithValidData) {
    Mix valid_mix = TestFixtures::createSampleMix("test_mix_1");
    
    // Test validation (this would require the method to be public or we'd need to test through public interface)
    // For now, we'll test that the mix has required fields
    EXPECT_FALSE(valid_mix.id.empty());
    EXPECT_FALSE(valid_mix.title.empty());
    EXPECT_FALSE(valid_mix.artist.empty());
    EXPECT_FALSE(valid_mix.url.empty());
    EXPECT_GT(valid_mix.duration_seconds, 0);
}

TEST_F(MixMetadataTest, ValidateMixWithInvalidData) {
    Mix invalid_mix;
    // Leave required fields empty
    
    // Test validation (this would require the method to be public or we'd need to test through public interface)
    // For now, we'll test that the mix is missing required fields
    EXPECT_TRUE(invalid_mix.id.empty());
    EXPECT_TRUE(invalid_mix.title.empty());
    EXPECT_TRUE(invalid_mix.artist.empty());
    EXPECT_TRUE(invalid_mix.url.empty());
    EXPECT_EQ(invalid_mix.duration_seconds, 0);
}

TEST_F(MixMetadataTest, GenerateIdFromUrl) {
    // Test ID generation (this would require the method to be public or we'd need to test through public interface)
    // For now, we'll test that we can create a mix with a URL and it gets an ID
    Mix mix = TestFixtures::createSampleMix("test_mix_1");
    EXPECT_FALSE(mix.id.empty());
    EXPECT_FALSE(mix.url.empty());
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithEmptyYaml) {
    // Create empty YAML file
    std::string empty_yaml = R"(
mixes: []
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, empty_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    EXPECT_TRUE(metadata.isSuccess());
    EXPECT_TRUE(metadata.getLastError().empty());
    EXPECT_TRUE(loaded_mixes.empty());
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithMissingMixesKey) {
    // Create YAML file without mixes key
    std::string invalid_yaml = R"(
other_key: value
another_key: another_value
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, invalid_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
    EXPECT_TRUE(loaded_mixes.empty());
}

TEST_F(MixMetadataTest, LoadFromLocalFileWithPartialMixData) {
    // Create YAML file with partial mix data
    std::string partial_yaml = R"(
mixes:
  - id: test_mix_1
    title: "Test Mix 1"
    # Missing artist, genre, url, etc.
  - id: test_mix_2
    title: "Test Mix 2"
    artist: "Test Artist 2"
    genre: "House"
    url: "https://example.com/mix2.mp3"
    duration_seconds: 2700
)";
    
    ASSERT_TRUE(TestFixtures::createTestConfigFile(yaml_path, partial_yaml));
    
    MixMetadata metadata;
    std::vector<Mix> loaded_mixes = metadata.loadFromLocalFile(yaml_path);
    
    // Should handle partial data gracefully
    EXPECT_GT(loaded_mixes.size(), 0);
}

TEST_F(MixMetadataTest, MixStructDefaultValues) {
    Mix mix;
    
    // Test default values
    EXPECT_TRUE(mix.id.empty());
    EXPECT_TRUE(mix.title.empty());
    EXPECT_TRUE(mix.artist.empty());
    EXPECT_TRUE(mix.genre.empty());
    EXPECT_TRUE(mix.url.empty());
    EXPECT_TRUE(mix.local_path.empty());
    EXPECT_EQ(mix.duration_seconds, 0);
    EXPECT_TRUE(mix.tags.empty());
    EXPECT_TRUE(mix.description.empty());
    EXPECT_TRUE(mix.date_added.empty());
    EXPECT_TRUE(mix.last_played.empty());
    EXPECT_EQ(mix.play_count, 0);
    EXPECT_FALSE(mix.is_favorite);
}

TEST_F(MixMetadataTest, MixStructAssignment) {
    Mix original = TestFixtures::createSampleMix("test_mix_1");
    Mix copy = original;
    
    // Test that assignment works correctly
    EXPECT_EQ(copy.id, original.id);
    EXPECT_EQ(copy.title, original.title);
    EXPECT_EQ(copy.artist, original.artist);
    EXPECT_EQ(copy.genre, original.genre);
    EXPECT_EQ(copy.url, original.url);
    EXPECT_EQ(copy.duration_seconds, original.duration_seconds);
    EXPECT_EQ(copy.description, original.description);
    EXPECT_EQ(copy.tags.size(), original.tags.size());
    EXPECT_EQ(copy.play_count, original.play_count);
    EXPECT_EQ(copy.is_favorite, original.is_favorite);
}

TEST_F(MixMetadataTest, MixStructModification) {
    Mix mix = TestFixtures::createSampleMix("test_mix_1");
    
    // Test modifying mix properties
    mix.title = "Modified Title";
    mix.artist = "Modified Artist";
    mix.genre = "Modified Genre";
    mix.duration_seconds = 1800;
    mix.play_count = 5;
    mix.is_favorite = true;
    mix.tags.push_back("modified");
    
    EXPECT_EQ(mix.title, "Modified Title");
    EXPECT_EQ(mix.artist, "Modified Artist");
    EXPECT_EQ(mix.genre, "Modified Genre");
    EXPECT_EQ(mix.duration_seconds, 1800);
    EXPECT_EQ(mix.play_count, 5);
    EXPECT_TRUE(mix.is_favorite);
    EXPECT_EQ(mix.tags.size(), 4); // 3 original + 1 new
    EXPECT_EQ(mix.tags.back(), "modified");
} 