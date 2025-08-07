#include "config_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "fixtures/test_fixtures.hpp"
using AutoVibez::Data::ConfigFile;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
        config_path = test_dir + "/test_config.inp";
    }

    void TearDown() override {
        TestFixtures::cleanupTestFiles({test_dir});
    }

    std::string test_dir;
    std::string config_path;
};

TEST_F(ConfigManagerTest, ConstructorWithValidFile) {
    // Create a test config file
    std::string config_content = R"(
audio_device = 2
preset_path = assets/presets
texture_path = assets/textures
mixes_url = https://example.com/mixes.yaml
cache_size_mb = 200
auto_download = true
seek_increment = 15
volume_step = 10
crossfade_enabled = true
crossfade_duration = 5
refresh_interval = 60
preferred_genre = House
font_path = assets/fonts/DejaVuSans.ttf
show_fps = true
mesh_x = 64
mesh_y = 48
aspect_correction = false
fps = 30
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));

    // Test constructor
    EXPECT_NO_THROW({
        ConfigFile config(config_path);

        // Test reading values
        EXPECT_EQ(config.read<int>("audio_device"), 2);
        EXPECT_EQ(config.read<std::string>("preset_path"), "assets/presets");
        EXPECT_EQ(config.read<std::string>("texture_path"), "assets/textures");
        EXPECT_EQ(config.read<std::string>("mixes_url"), "https://example.com/mixes.yaml");
        EXPECT_EQ(config.read<int>("cache_size_mb"), 200);
        EXPECT_EQ(config.read<bool>("auto_download"), true);
        EXPECT_EQ(config.read<int>("seek_increment"), 15);
        EXPECT_EQ(config.read<int>("volume_step"), 10);
        EXPECT_EQ(config.read<bool>("crossfade_enabled"), true);
        EXPECT_EQ(config.read<int>("crossfade_duration"), 5);
        EXPECT_EQ(config.read<int>("refresh_interval"), 60);
        EXPECT_EQ(config.read<std::string>("preferred_genre"), "House");
        EXPECT_EQ(config.read<std::string>("font_path"), "assets/fonts/DejaVuSans.ttf");
        EXPECT_EQ(config.read<bool>("show_fps"), true);
        EXPECT_EQ(config.read<int>("mesh_x"), 64);
        EXPECT_EQ(config.read<int>("mesh_y"), 48);
        EXPECT_EQ(config.read<bool>("aspect_correction"), false);
        EXPECT_EQ(config.read<int>("fps"), 30);
    });
}

TEST_F(ConfigManagerTest, ConstructorWithNonExistentFile) {
    EXPECT_THROW({ ConfigFile config("/nonexistent/file.inp"); }, ConfigFile::file_not_found);
}

TEST_F(ConfigManagerTest, ReadWithDefaultValues) {
    std::string config_content = R"(
audio_device = 1
preset_path = /test/presets
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test reading with default values
    EXPECT_EQ(config.read<int>("audio_device", 0), 1);
    EXPECT_EQ(config.read<int>("nonexistent_key", 42), 42);
    EXPECT_EQ(config.read<std::string>("preset_path", "/default/presets"), "/test/presets");
    EXPECT_EQ(config.read<std::string>("nonexistent_string", "default"), "default");
    EXPECT_EQ(config.read<bool>("nonexistent_bool", true), true);
    EXPECT_EQ(config.read<bool>("nonexistent_bool", false), false);
}

TEST_F(ConfigManagerTest, ReadIntoMethod) {
    std::string config_content = R"(
audio_device = 3
preset_path = /custom/presets
mixes_url = https://custom.com/mixes.yaml
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test readInto method
    int audio_device;
    std::string preset_path;
    std::string mixes_url;
    std::string nonexistent;

    EXPECT_TRUE(config.readInto(audio_device, "audio_device"));
    EXPECT_EQ(audio_device, 3);

    EXPECT_TRUE(config.readInto(preset_path, "preset_path"));
    EXPECT_EQ(preset_path, "/custom/presets");

    EXPECT_TRUE(config.readInto(mixes_url, "mixes_url"));
    EXPECT_EQ(mixes_url, "https://custom.com/mixes.yaml");

    EXPECT_FALSE(config.readInto(nonexistent, "nonexistent_key"));
}

TEST_F(ConfigManagerTest, ReadIntoWithDefaultValues) {
    std::string config_content = R"(
audio_device = 5
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    int audio_device;
    int nonexistent_int;
    std::string nonexistent_string;
    bool nonexistent_bool;

    EXPECT_TRUE(config.readInto(audio_device, "audio_device", 0));
    EXPECT_EQ(audio_device, 5);

    EXPECT_FALSE(config.readInto(nonexistent_int, "nonexistent_int", 42));
    EXPECT_EQ(nonexistent_int, 42);

    EXPECT_FALSE(config.readInto(nonexistent_string, "nonexistent_string", std::string("default")));
    EXPECT_EQ(nonexistent_string, "default");

    EXPECT_FALSE(config.readInto(nonexistent_bool, "nonexistent_bool", true));
    EXPECT_EQ(nonexistent_bool, true);
}

TEST_F(ConfigManagerTest, KeyExistsMethod) {
    std::string config_content = R"(
audio_device = 1
preset_path = /test/presets
mixes_url = https://test.com/mixes.yaml
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    EXPECT_TRUE(config.keyExists("audio_device"));
    EXPECT_TRUE(config.keyExists("preset_path"));
    EXPECT_TRUE(config.keyExists("mixes_url"));
    EXPECT_FALSE(config.keyExists("nonexistent_key"));
}

TEST_F(ConfigManagerTest, AddAndRemoveMethods) {
    std::string config_content = R"(
audio_device = 1
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test adding new keys
    config.add("new_int", 42);
    config.add("new_string", "test_value");
    config.add("new_bool", true);

    EXPECT_EQ(config.read<int>("new_int"), 42);
    EXPECT_EQ(config.read<std::string>("new_string"), "test_value");
    EXPECT_EQ(config.read<bool>("new_bool"), true);

    // Test removing keys
    config.remove("new_int");
    EXPECT_FALSE(config.keyExists("new_int"));
    EXPECT_TRUE(config.keyExists("new_string"));
    EXPECT_TRUE(config.keyExists("new_bool"));

    config.remove("new_string");
    config.remove("new_bool");
    EXPECT_FALSE(config.keyExists("new_string"));
    EXPECT_FALSE(config.keyExists("new_bool"));
}

TEST_F(ConfigManagerTest, DelimiterAndCommentMethods) {
    std::string config_content = R"(
audio_device = 1 # This is a comment
preset_path = /test/presets
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    EXPECT_EQ(config.getDelimiter(), "=");
    EXPECT_EQ(config.getComment(), "#");

    // Test changing delimiter and comment
    config.setDelimiter(":");
    config.setComment("//");

    EXPECT_EQ(config.getDelimiter(), ":");
    EXPECT_EQ(config.getComment(), "//");
}

TEST_F(ConfigManagerTest, StreamOperators) {
    std::string config_content = R"(
audio_device = 1
preset_path = /test/presets
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test output stream operator
    std::ostringstream oss;
    oss << config;
    std::string output = oss.str();

    EXPECT_NE(output.find("audio_device = 1"), std::string::npos);
    EXPECT_NE(output.find("preset_path = /test/presets"), std::string::npos);
}

TEST_F(ConfigManagerTest, TemplateSpecializations) {
    std::string config_content = R"(
int_value = 42
float_value = 3.14
bool_value = true
string_value = test_string
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test different template specializations
    EXPECT_EQ(config.read<int>("int_value"), 42);
    EXPECT_FLOAT_EQ(config.read<float>("float_value"), 3.14f);
    EXPECT_EQ(config.read<bool>("bool_value"), true);
    EXPECT_EQ(config.read<std::string>("string_value"), "test_string");
}

TEST_F(ConfigManagerTest, ErrorHandling) {
    std::string config_content = R"(
audio_device = 1
preset_path = /test/presets
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test reading non-existent key without default
    EXPECT_THROW({ config.read<int>("nonexistent_key"); }, ConfigFile::key_not_found);

    EXPECT_THROW({ config.read<std::string>("nonexistent_key"); }, ConfigFile::key_not_found);
}

TEST_F(ConfigManagerTest, WhitespaceHandling) {
    std::string config_content = R"(
  audio_device  =  1  
  preset_path =  /test/presets  
  mixes_url  =  https://test.com/mixes.yaml  
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test that whitespace is properly handled
    EXPECT_EQ(config.read<int>("audio_device"), 1);
    EXPECT_EQ(config.read<std::string>("preset_path"), "/test/presets");
    EXPECT_EQ(config.read<std::string>("mixes_url"), "https://test.com/mixes.yaml");
}

TEST_F(ConfigManagerTest, CommentHandling) {
    std::string config_content = R"(
audio_device = 1 # This is a comment
preset_path = /test/presets # Another comment
# This is a full line comment
mixes_url = https://test.com/mixes.yaml
)";

    ASSERT_TRUE(TestFixtures::createTestConfigFile(config_path, config_content));
    ConfigFile config(config_path);

    // Test that comments are properly ignored
    EXPECT_EQ(config.read<int>("audio_device"), 1);
    EXPECT_EQ(config.read<std::string>("preset_path"), "/test/presets");
    EXPECT_EQ(config.read<std::string>("mixes_url"), "https://test.com/mixes.yaml");
    EXPECT_FALSE(config.keyExists("This"));
    EXPECT_FALSE(config.keyExists("This is a full line comment"));
}