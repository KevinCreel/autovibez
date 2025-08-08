#include "data/config_manager.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "autovibez_config_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;

    // Helper method to create a test config file
    std::string createTestConfig(const std::string& content) {
        auto configPath = tempDir / "test_config.inp";
        std::ofstream configFile(configPath);
        configFile << content;
        configFile.close();
        return configPath.string();
    }
};

TEST_F(ConfigManagerTest, LoadValidConfigFile) {
    std::string configContent = R"(
# Test configuration file
preset_path = /path/to/presets
texture_path = /path/to/textures
audio_device = 2
show_fps = true
yaml_url = https://example.com/mixes.yaml
mixes_url = https://example.com/mixes/
auto_download = true
seek_increment = 30
volume_step = 5
crossfade_enabled = true
crossfade_duration_ms = 5000
refresh_interval = 300
preferred_genre = Techno
font_path = /path/to/font.ttf
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test basic key reading
    EXPECT_EQ(config.read<std::string>("preset_path"), "/path/to/presets");
    EXPECT_EQ(config.read<std::string>("texture_path"), "/path/to/textures");
    EXPECT_EQ(config.read<int>("audio_device"), 2);
    EXPECT_EQ(config.read<bool>("show_fps"), true);

    // Test AutoVibez-specific getter methods
    EXPECT_EQ(config.getPresetPath(), "/path/to/presets");
    EXPECT_EQ(config.getTexturePath(), "/path/to/textures");
    EXPECT_EQ(config.getAudioDeviceIndex(), 2);
    EXPECT_EQ(config.getShowFps(), true);
    EXPECT_EQ(config.getYamlUrl(), "https://example.com/mixes.yaml");
    EXPECT_EQ(config.getMixesUrl(), "https://example.com/mixes/");
    EXPECT_EQ(config.getAutoDownload(), true);
    EXPECT_EQ(config.getSeekIncrement(), 30);
    EXPECT_EQ(config.getVolumeStep(), 5);
    EXPECT_EQ(config.getCrossfadeEnabled(), true);
    EXPECT_EQ(config.getCrossfadeDuration(), 5000);
    EXPECT_EQ(config.getPreferredGenre(), "Techno");
    EXPECT_EQ(config.getFontPath(), "/path/to/font.ttf");
}

TEST_F(ConfigManagerTest, DefaultValues) {
    std::string configContent = R"(
# Minimal config with some values
preset_path = /custom/presets
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test default values for missing keys
    EXPECT_EQ(config.getTexturePath(), "");
    EXPECT_EQ(config.getAudioDeviceIndex(), 0);
    EXPECT_EQ(config.getShowFps(), false);
    EXPECT_EQ(config.getYamlUrl(), "");
    EXPECT_EQ(config.getMixesUrl(), "");
    EXPECT_EQ(config.getAutoDownload(), true);
    EXPECT_EQ(config.getSeekIncrement(), 60);
    EXPECT_EQ(config.getVolumeStep(), 10);
    EXPECT_EQ(config.getCrossfadeEnabled(), true);
    EXPECT_EQ(config.getCrossfadeDuration(), 3000);  // DEFAULT_CROSSFADE_DURATION_MS
    EXPECT_EQ(config.getPreferredGenre(), "");
    EXPECT_EQ(config.getFontPath(), "");
}

TEST_F(ConfigManagerTest, BooleanValues) {
    std::string configContent = R"(
show_fps = true
auto_download = false
crossfade_enabled = no
test_1 = TRUE
test_2 = FALSE
test_3 = Yes
test_4 = No
test_5 = 1
test_6 = 0
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test various boolean formats
    EXPECT_EQ(config.read<bool>("show_fps"), true);
    EXPECT_EQ(config.read<bool>("auto_download"), false);
    EXPECT_EQ(config.read<bool>("crossfade_enabled"), false);
    EXPECT_EQ(config.read<bool>("test_1"), true);
    EXPECT_EQ(config.read<bool>("test_2"), false);
    EXPECT_EQ(config.read<bool>("test_3"), true);
    EXPECT_EQ(config.read<bool>("test_4"), false);
    EXPECT_EQ(config.read<bool>("test_5"), true);
    EXPECT_EQ(config.read<bool>("test_6"), false);
}

TEST_F(ConfigManagerTest, KeyExistence) {
    std::string configContent = R"(
preset_path = /path/to/presets
texture_path = /path/to/textures
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test key existence
    EXPECT_TRUE(config.keyExists("preset_path"));
    EXPECT_TRUE(config.keyExists("texture_path"));
    EXPECT_FALSE(config.keyExists("nonexistent_key"));
}

TEST_F(ConfigManagerTest, ReadIntoMethod) {
    std::string configContent = R"(
preset_path = /path/to/presets
audio_device = 3
show_fps = true
seek_increment = 45
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test readInto with existing keys
    std::string presetPath;
    int audioDevice;
    bool showFps;
    int seekIncrement;

    EXPECT_TRUE(config.readInto(presetPath, "preset_path"));
    EXPECT_EQ(presetPath, "/path/to/presets");

    EXPECT_TRUE(config.readInto(audioDevice, "audio_device"));
    EXPECT_EQ(audioDevice, 3);

    EXPECT_TRUE(config.readInto(showFps, "show_fps"));
    EXPECT_EQ(showFps, true);

    EXPECT_TRUE(config.readInto(seekIncrement, "seek_increment"));
    EXPECT_EQ(seekIncrement, 45);

    // Test readInto with default values
    std::string missingString;
    int missingInt = 0;
    bool missingBool = false;

    EXPECT_FALSE(config.readInto(missingString, "missing_key"));
    EXPECT_FALSE(config.readInto(missingInt, "missing_key"));
    EXPECT_FALSE(config.readInto(missingBool, "missing_key"));

    // Test readInto with default values
    std::string defaultString = "default";
    int defaultInt = 42;
    bool defaultBool = true;

    EXPECT_FALSE(config.readInto(defaultString, "missing_key", std::string("fallback")));
    EXPECT_EQ(defaultString, "fallback");

    EXPECT_FALSE(config.readInto(defaultInt, "missing_key", 99));
    EXPECT_EQ(defaultInt, 99);

    EXPECT_FALSE(config.readInto(defaultBool, "missing_key", false));
    EXPECT_EQ(defaultBool, false);
}

TEST_F(ConfigManagerTest, CommentsAndWhitespace) {
    std::string configContent = R"(
# This is a comment
preset_path = /path/to/presets  # Inline comment
texture_path = /path/to/textures

# Another comment
audio_device = 2
show_fps = true
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test that comments are ignored and whitespace is handled
    EXPECT_EQ(config.read<std::string>("preset_path"), "/path/to/presets");
    EXPECT_EQ(config.read<std::string>("texture_path"), "/path/to/textures");
    EXPECT_EQ(config.read<int>("audio_device"), 2);
    EXPECT_EQ(config.read<bool>("show_fps"), true);
}

TEST_F(ConfigManagerTest, MultiLineValues) {
    std::string configContent = R"(
description = This is a multi-line
description that spans multiple
lines in the config file

yaml_url = https://example.com/
mixes.yaml
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test multi-line values
    EXPECT_EQ(config.read<std::string>("description"),
              "This is a multi-line\ndescription that spans multiple\nlines in the config file");
    EXPECT_EQ(config.read<std::string>("yaml_url"), "https://example.com/\nmixes.yaml");
}

TEST_F(ConfigManagerTest, FileNotFound) {
    // Test that file_not_found exception is thrown
    EXPECT_THROW(
        { AutoVibez::Data::ConfigFile config("/nonexistent/file.inp"); }, AutoVibez::Data::ConfigFile::file_not_found);
}

TEST_F(ConfigManagerTest, KeyNotFound) {
    std::string configContent = R"(
preset_path = /path/to/presets
)";

    std::string configPath = createTestConfig(configContent);
    AutoVibez::Data::ConfigFile config(configPath);

    // Test that key_not_found exception is thrown for missing keys
    EXPECT_THROW({ config.read<std::string>("nonexistent_key"); }, AutoVibez::Data::ConfigFile::key_not_found);

    EXPECT_THROW({ config.read<int>("nonexistent_key"); }, AutoVibez::Data::ConfigFile::key_not_found);
}

TEST_F(ConfigManagerTest, EmptyConfig) {
    // Test empty config file
    std::string configPath = createTestConfig("");
    AutoVibez::Data::ConfigFile config(configPath);

    // Should use default values
    EXPECT_EQ(config.getPresetPath(), "");
    EXPECT_EQ(config.getAudioDeviceIndex(), 0);
    EXPECT_EQ(config.getShowFps(), false);
}

TEST_F(ConfigManagerTest, AddAndRemoveKeys) {
    AutoVibez::Data::ConfigFile config;

    // Test adding keys
    config.add("test_string", "value");
    config.add("test_int", 42);
    config.add("test_bool", true);

    EXPECT_EQ(config.read<std::string>("test_string"), "value");
    EXPECT_EQ(config.read<int>("test_int"), 42);
    EXPECT_EQ(config.read<bool>("test_bool"), true);

    // Test removing keys
    config.remove("test_string");
    EXPECT_FALSE(config.keyExists("test_string"));
    EXPECT_TRUE(config.keyExists("test_int"));
    EXPECT_TRUE(config.keyExists("test_bool"));
}
