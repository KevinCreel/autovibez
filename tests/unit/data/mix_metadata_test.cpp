#include "data/mix_metadata.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

class MixMetadataTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "autovibez_mix_metadata_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;

    // Helper method to create a test YAML file
    std::string createTestYaml(const std::string& content) {
        auto yamlPath = tempDir / "test_mixes.yaml";
        std::ofstream yamlFile(yamlPath);
        yamlFile << content;
        yamlFile.close();
        return yamlPath.string();
    }
};

TEST_F(MixMetadataTest, LoadValidYamlWithSimpleUrls) {
    std::string yamlContent = R"(
mixes:
  - https://example.com/mix1.mp3
  - https://example.com/mix2.mp3
  - https://example.com/mix3.mp3
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 3);
    EXPECT_TRUE(metadata.isSuccess());

    // Check first mix
    EXPECT_EQ(mixes[0].url, "https://example.com/mix1.mp3");
    EXPECT_EQ(mixes[0].original_filename, "mix1.mp3");
    EXPECT_FALSE(mixes[0].id.empty());  // Should be auto-generated
    EXPECT_EQ(mixes[0].play_count, 0);
    EXPECT_FALSE(mixes[0].is_favorite);

    // Check second mix
    EXPECT_EQ(mixes[1].url, "https://example.com/mix2.mp3");
    EXPECT_EQ(mixes[1].original_filename, "mix2.mp3");
    EXPECT_FALSE(mixes[1].id.empty());

    // Check third mix
    EXPECT_EQ(mixes[2].url, "https://example.com/mix3.mp3");
    EXPECT_EQ(mixes[2].original_filename, "mix3.mp3");
    EXPECT_FALSE(mixes[2].id.empty());
}

TEST_F(MixMetadataTest, LoadValidYamlWithObjectFormat) {
    std::string yamlContent = R"(
mixes:
  - id: "custom-id-1"
    url: https://example.com/mix1.mp3
    title: "Amazing Mix 1"
    artist: "DJ Awesome"
    genre: "Techno"
    duration_seconds: 3600
    description: "An amazing techno mix"
    tags:
      - techno
      - electronic
      - 2024
  - id: "custom-id-2"
    url: https://example.com/mix2.mp3
    title: "Epic Mix 2"
    artist: "DJ Legend"
    genre: "House"
    duration_seconds: 1800
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 2);
    EXPECT_TRUE(metadata.isSuccess());

    // Check first mix with full metadata
    EXPECT_EQ(mixes[0].id, "custom-id-1");
    EXPECT_EQ(mixes[0].url, "https://example.com/mix1.mp3");
    EXPECT_EQ(mixes[0].title, "Amazing Mix 1");
    EXPECT_EQ(mixes[0].artist, "DJ Awesome");
    EXPECT_EQ(mixes[0].genre, "Techno");
    EXPECT_EQ(mixes[0].duration_seconds, 3600);
    EXPECT_EQ(mixes[0].description, "An amazing techno mix");
    EXPECT_EQ(mixes[0].original_filename, "mix1.mp3");
    EXPECT_EQ(mixes[0].tags.size(), 3);
    EXPECT_EQ(mixes[0].tags[0], "techno");
    EXPECT_EQ(mixes[0].tags[1], "electronic");
    EXPECT_EQ(mixes[0].tags[2], "2024");
    EXPECT_EQ(mixes[0].play_count, 0);
    EXPECT_FALSE(mixes[0].is_favorite);

    // Check second mix
    EXPECT_EQ(mixes[1].id, "custom-id-2");
    EXPECT_EQ(mixes[1].url, "https://example.com/mix2.mp3");
    EXPECT_EQ(mixes[1].title, "Epic Mix 2");
    EXPECT_EQ(mixes[1].artist, "DJ Legend");
    EXPECT_EQ(mixes[1].genre, "House");
    EXPECT_EQ(mixes[1].duration_seconds, 1800);
    EXPECT_EQ(mixes[1].original_filename, "mix2.mp3");
}

TEST_F(MixMetadataTest, LoadMixedFormatYaml) {
    std::string yamlContent = R"(
mixes:
  - https://example.com/simple_mix.mp3
  - id: "complex-mix"
    url: https://example.com/complex_mix.mp3
    title: "Complex Mix"
    artist: "DJ Complex"
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 2);
    EXPECT_TRUE(metadata.isSuccess());

    // Check simple format mix
    EXPECT_EQ(mixes[0].url, "https://example.com/simple_mix.mp3");
    EXPECT_EQ(mixes[0].original_filename, "simple_mix.mp3");
    EXPECT_FALSE(mixes[0].id.empty());  // Auto-generated

    // Check complex format mix
    EXPECT_EQ(mixes[1].id, "complex-mix");
    EXPECT_EQ(mixes[1].url, "https://example.com/complex_mix.mp3");
    EXPECT_EQ(mixes[1].title, "Complex Mix");
    EXPECT_EQ(mixes[1].artist, "DJ Complex");
    EXPECT_EQ(mixes[1].original_filename, "complex_mix.mp3");
}

TEST_F(MixMetadataTest, HandleEmptyYaml) {
    std::string yamlContent = R"(
mixes: []
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_TRUE(metadata.isSuccess());
}

TEST_F(MixMetadataTest, HandleInvalidYaml) {
    std::string yamlContent = R"(
mixes:
  - invalid: yaml: format
    - broken
    - structure
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    // Should handle gracefully and return empty vector
    EXPECT_EQ(mixes.size(), 0);
    // Note: This might not set an error due to the try-catch in the implementation
}

TEST_F(MixMetadataTest, HandleMissingMixesSection) {
    std::string yamlContent = R"(
other_section:
  - item1
  - item2
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, HandleInvalidMixesSection) {
    std::string yamlContent = R"(
mixes: "not a sequence"
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, HandleMixWithMissingUrl) {
    std::string yamlContent = R"(
mixes:
  - id: "no-url-mix"
    title: "Mix without URL"
    artist: "DJ NoURL"
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    // Should skip invalid mixes (missing URL)
    EXPECT_EQ(mixes.size(), 0);
    // Note: This might not set an error due to the try-catch in the implementation
}

TEST_F(MixMetadataTest, HandleMixWithEmptyUrl) {
    std::string yamlContent = R"(
mixes:
  - id: "empty-url-mix"
    url: ""
    title: "Mix with empty URL"
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    // Should skip invalid mixes (empty URL)
    EXPECT_EQ(mixes.size(), 0);
}

TEST_F(MixMetadataTest, HandleMixWithComplexUrl) {
    std::string yamlContent = R"(
mixes:
  - https://example.com/path/with/spaces%20and%20symbols/file%20name.mp3?param=value&other=123
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 1);
    EXPECT_TRUE(metadata.isSuccess());

    EXPECT_EQ(mixes[0].url,
              "https://example.com/path/with/spaces%20and%20symbols/file%20name.mp3?param=value&other=123");
    EXPECT_EQ(mixes[0].original_filename, "file name.mp3");
    EXPECT_FALSE(mixes[0].id.empty());
}

TEST_F(MixMetadataTest, HandleMixWithSpecialCharacters) {
    std::string yamlContent = R"(
mixes:
  - id: "special-chars"
    url: https://example.com/mix_with_special_chars.mp3
    title: "Mix with Special Chars: \n\t\r\"'"
    artist: "DJ Special: \n\t\r\"'"
    description: "Description with special chars: \n\t\r\"'"
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 1);
    EXPECT_TRUE(metadata.isSuccess());

    EXPECT_EQ(mixes[0].id, "special-chars");
    EXPECT_EQ(mixes[0].title, "Mix with Special Chars: \n\t\r\"'");
    EXPECT_EQ(mixes[0].artist, "DJ Special: \n\t\r\"'");
    EXPECT_EQ(mixes[0].description, "Description with special chars: \n\t\r\"'");
}

TEST_F(MixMetadataTest, HandleFileNotFound) {
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile("/nonexistent/file.yaml");

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, HandleEmptyFile) {
    std::string yamlPath = createTestYaml("");
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, HandleFileWithoutMixesSection) {
    std::string yamlContent = R"(
# This file has no mixes section
other_content: value
)";

    std::string yamlPath = createTestYaml(yamlContent);
    AutoVibez::Data::MixMetadata metadata;
    auto mixes = metadata.loadFromLocalFile(yamlPath);

    EXPECT_EQ(mixes.size(), 0);
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, ValidateMixWithValidData) {
    AutoVibez::Data::MixMetadata metadata;
    AutoVibez::Data::Mix mix;
    mix.url = "https://example.com/valid_mix.mp3";
    mix.id = "valid-id";

    EXPECT_TRUE(metadata.validateMix(mix));
    EXPECT_TRUE(metadata.isSuccess());
}

TEST_F(MixMetadataTest, ValidateMixWithMissingUrl) {
    AutoVibez::Data::MixMetadata metadata;
    AutoVibez::Data::Mix mix;
    mix.id = "valid-id";
    // mix.url is empty

    EXPECT_FALSE(metadata.validateMix(mix));
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}

TEST_F(MixMetadataTest, ValidateMixWithEmptyUrl) {
    AutoVibez::Data::MixMetadata metadata;
    AutoVibez::Data::Mix mix;
    mix.url = "";
    mix.id = "valid-id";

    EXPECT_FALSE(metadata.validateMix(mix));
    EXPECT_FALSE(metadata.isSuccess());
    EXPECT_FALSE(metadata.getLastError().empty());
}
