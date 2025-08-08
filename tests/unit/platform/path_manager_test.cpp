#include "platform/path_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>

class PathManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directories
        test_dir = std::filesystem::temp_directory_path() / "autovibez_test";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test directories
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::filesystem::path test_dir;
};

TEST_F(PathManagerTest, GetConfigDirectory) {
    // Test that config directory path is returned
    std::string config_dir = PathManager::getConfigDirectory();

    // Should not be empty
    EXPECT_FALSE(config_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(config_dir.find("autovibez") != std::string::npos);

    // Should contain config directory
    EXPECT_TRUE(config_dir.find("config") != std::string::npos);
}

TEST_F(PathManagerTest, GetAssetsDirectory) {
    // Test that assets directory path is returned
    std::string assets_dir = PathManager::getAssetsDirectory();

    // Should not be empty
    EXPECT_FALSE(assets_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(assets_dir.find("autovibez") != std::string::npos);

    // Should contain assets directory
    EXPECT_TRUE(assets_dir.find("assets") != std::string::npos);
}

TEST_F(PathManagerTest, GetDataDirectory) {
    // Test that data directory path is returned
    std::string data_dir = PathManager::getDataDirectory();

    // Should not be empty
    EXPECT_FALSE(data_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(data_dir.find("autovibez") != std::string::npos);
}

TEST_F(PathManagerTest, GetCacheDirectory) {
    // Test that cache directory path is returned
    std::string cache_dir = PathManager::getCacheDirectory();

    // Should not be empty
    EXPECT_FALSE(cache_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(cache_dir.find("autovibez") != std::string::npos);

    // Should contain cache directory
    EXPECT_TRUE(cache_dir.find("cache") != std::string::npos);
}

TEST_F(PathManagerTest, GetStateDirectory) {
    // Test that state directory path is returned
    std::string state_dir = PathManager::getStateDirectory();

    // Should not be empty
    EXPECT_FALSE(state_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(state_dir.find("autovibez") != std::string::npos);

    // Should contain state directory
    EXPECT_TRUE(state_dir.find("state") != std::string::npos);
}

TEST_F(PathManagerTest, GetDatabasePath) {
    // Test that database path is returned
    std::string db_path = PathManager::getDatabasePath();

    // Should not be empty
    EXPECT_FALSE(db_path.empty());

    // Should contain the app name
    EXPECT_TRUE(db_path.find("autovibez") != std::string::npos);

    // Should contain database filename
    EXPECT_TRUE(db_path.find("autovibez_mixes.db") != std::string::npos);
}

TEST_F(PathManagerTest, GetMixesDirectory) {
    // Test that mixes directory path is returned
    std::string mixes_dir = PathManager::getMixesDirectory();

    // Should not be empty
    EXPECT_FALSE(mixes_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(mixes_dir.find("autovibez") != std::string::npos);

    // Should contain mixes directory
    EXPECT_TRUE(mixes_dir.find("mixes") != std::string::npos);
}

TEST_F(PathManagerTest, GetFileMappingsPath) {
    // Test that file mappings path is returned
    std::string mappings_path = PathManager::getFileMappingsPath();

    // Should not be empty
    EXPECT_FALSE(mappings_path.empty());

    // Should contain the app name
    EXPECT_TRUE(mappings_path.find("autovibez") != std::string::npos);

    // Should contain file mappings filename
    EXPECT_TRUE(mappings_path.find("file_mappings.txt") != std::string::npos);
}

TEST_F(PathManagerTest, GetPresetsDirectory) {
    // Test that presets directory path is returned
    std::string presets_dir = PathManager::getPresetsDirectory();

    // Should not be empty
    EXPECT_FALSE(presets_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(presets_dir.find("autovibez") != std::string::npos);

    // Should contain presets directory
    EXPECT_TRUE(presets_dir.find("presets") != std::string::npos);
}

TEST_F(PathManagerTest, GetTexturesDirectory) {
    // Test that textures directory path is returned
    std::string textures_dir = PathManager::getTexturesDirectory();

    // Should not be empty
    EXPECT_FALSE(textures_dir.empty());

    // Should contain the app name
    EXPECT_TRUE(textures_dir.find("autovibez") != std::string::npos);

    // Should contain textures directory
    EXPECT_TRUE(textures_dir.find("textures") != std::string::npos);
}

TEST_F(PathManagerTest, ExpandTilde) {
    // Test tilde expansion
    std::string path_with_tilde = "~/test/path";
    std::string expanded = PathManager::expandTilde(path_with_tilde);

    // Should not be empty
    EXPECT_FALSE(expanded.empty());

    // Should not contain tilde
    EXPECT_TRUE(expanded.find("~") == std::string::npos);

    // Should contain the rest of the path
    EXPECT_TRUE(expanded.find("test/path") != std::string::npos);
}

TEST_F(PathManagerTest, ExpandTildeNoTilde) {
    // Test path without tilde
    std::string path_without_tilde = "/absolute/path";
    std::string result = PathManager::expandTilde(path_without_tilde);

    // Should return the same path
    EXPECT_EQ(result, path_without_tilde);
}

TEST_F(PathManagerTest, ExpandTildeEmptyPath) {
    // Test empty path
    std::string empty_path = "";
    std::string result = PathManager::expandTilde(empty_path);

    // Should return empty string
    EXPECT_EQ(result, empty_path);
}

TEST_F(PathManagerTest, GetConfigFileSearchPaths) {
    // Test that config file search paths are returned
    std::vector<std::string> search_paths = PathManager::getConfigFileSearchPaths();

    // Should not be empty
    EXPECT_FALSE(search_paths.empty());

    // Should contain config file name
    for (const auto& path : search_paths) {
        EXPECT_TRUE(path.find("config.inp") != std::string::npos);
    }
}

TEST_F(PathManagerTest, GetPresetSearchPaths) {
    // Test that preset search paths are returned
    std::vector<std::string> search_paths = PathManager::getPresetSearchPaths();

    // Should not be empty
    EXPECT_FALSE(search_paths.empty());

    // Should contain presets directory
    for (const auto& path : search_paths) {
        EXPECT_TRUE(path.find("presets") != std::string::npos);
    }
}

TEST_F(PathManagerTest, GetTextureSearchPaths) {
    // Test that texture search paths are returned
    std::vector<std::string> search_paths = PathManager::getTextureSearchPaths();

    // Should not be empty
    EXPECT_FALSE(search_paths.empty());

    // Should contain textures directory
    for (const auto& path : search_paths) {
        EXPECT_TRUE(path.find("textures") != std::string::npos);
    }
}

TEST_F(PathManagerTest, PathExists) {
    // Test path existence check
    std::string existing_path = test_dir.string();
    std::string non_existing_path = (test_dir / "nonexistent").string();

    // Should return true for existing path
    EXPECT_TRUE(PathManager::pathExists(existing_path));

    // Should return false for non-existing path
    EXPECT_FALSE(PathManager::pathExists(non_existing_path));
}

TEST_F(PathManagerTest, PathExistsEmptyPath) {
    // Test empty path
    EXPECT_FALSE(PathManager::pathExists(""));
}

TEST_F(PathManagerTest, EnsureDirectoryExists) {
    // Test directory creation
    std::string new_dir = (test_dir / "new_directory").string();

    // Directory should not exist initially
    EXPECT_FALSE(std::filesystem::exists(new_dir));

    // Create directory
    PathManager::ensureDirectoryExists(new_dir);

    // Directory should exist after creation
    EXPECT_TRUE(std::filesystem::exists(new_dir));
    EXPECT_TRUE(std::filesystem::is_directory(new_dir));
}

TEST_F(PathManagerTest, EnsureDirectoryExistsAlreadyExists) {
    // Test creating directory that already exists
    std::string existing_dir = test_dir.string();

    // Directory should exist initially
    EXPECT_TRUE(std::filesystem::exists(existing_dir));

    // Should not throw when creating existing directory
    EXPECT_NO_THROW({ PathManager::ensureDirectoryExists(existing_dir); });

    // Directory should still exist
    EXPECT_TRUE(std::filesystem::exists(existing_dir));
}

TEST_F(PathManagerTest, PlatformDetection) {
    // Test platform detection methods
    bool is_linux = PathManager::isLinux();
    bool is_macos = PathManager::isMacOS();
    bool is_windows = PathManager::isWindows();

    // Only one platform should be detected as true
    int platform_count = (is_linux ? 1 : 0) + (is_macos ? 1 : 0) + (is_windows ? 1 : 0);
    EXPECT_EQ(platform_count, 1);
}

TEST_F(PathManagerTest, FindConfigFile) {
    // Test config file finding
    std::string config_file = PathManager::findConfigFile();

    // In test environment, config file might not exist, so it could be empty
    // This is expected behavior - the method returns empty string if no config file is found
    if (!config_file.empty()) {
        // If a config file is found, it should contain the config file name
        EXPECT_TRUE(config_file.find("config.inp") != std::string::npos);
    }

    // Test that the method doesn't crash and returns a valid string (empty or not)
    EXPECT_NO_THROW({
        std::string result = PathManager::findConfigFile();
        // result can be empty, that's valid
    });
}

TEST_F(PathManagerTest, DirectoryConsistency) {
    // Test that all directory methods return consistent paths
    std::string config_dir = PathManager::getConfigDirectory();
    std::string data_dir = PathManager::getDataDirectory();
    std::string cache_dir = PathManager::getCacheDirectory();
    std::string state_dir = PathManager::getStateDirectory();

    // All should contain the app name
    EXPECT_TRUE(config_dir.find("autovibez") != std::string::npos);
    EXPECT_TRUE(data_dir.find("autovibez") != std::string::npos);
    EXPECT_TRUE(cache_dir.find("autovibez") != std::string::npos);
    EXPECT_TRUE(state_dir.find("autovibez") != std::string::npos);

    // All should be different paths
    EXPECT_NE(config_dir, data_dir);
    EXPECT_NE(config_dir, cache_dir);
    EXPECT_NE(config_dir, state_dir);
    EXPECT_NE(data_dir, cache_dir);
    EXPECT_NE(data_dir, state_dir);
    EXPECT_NE(cache_dir, state_dir);
}
