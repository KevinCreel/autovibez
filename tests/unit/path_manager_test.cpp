#include <gtest/gtest.h>
#include "path_manager.hpp"
#include "constants.hpp"
#include <vector>
#include <string>

class PathManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Any setup needed for tests
    }
    
    void TearDown() override {
        // Any cleanup needed for tests
    }
};

// Test platform detection
TEST_F(PathManagerTest, PlatformDetection) {
    // At least one platform should be detected
    bool platform_detected = PathManager::isLinux() || 
                            PathManager::isMacOS() || 
                            PathManager::isWindows();
    EXPECT_TRUE(platform_detected);
    
    // Only one platform should be detected
    int platform_count = 0;
    if (PathManager::isLinux()) platform_count++;
    if (PathManager::isMacOS()) platform_count++;
    if (PathManager::isWindows()) platform_count++;
    EXPECT_EQ(platform_count, 1);
}

// Test core directory methods
TEST_F(PathManagerTest, CoreDirectories) {
    // All core directories should return non-empty paths
    EXPECT_FALSE(PathManager::getConfigDirectory().empty());
    EXPECT_FALSE(PathManager::getAssetsDirectory().empty());
    EXPECT_FALSE(PathManager::getDataDirectory().empty());
    EXPECT_FALSE(PathManager::getCacheDirectory().empty());
    EXPECT_FALSE(PathManager::getStateDirectory().empty());
    
    // Directories should be different (except data and assets which might be the same)
    EXPECT_NE(PathManager::getConfigDirectory(), PathManager::getCacheDirectory());
    EXPECT_NE(PathManager::getConfigDirectory(), PathManager::getStateDirectory());
    EXPECT_NE(PathManager::getCacheDirectory(), PathManager::getStateDirectory());
}

// Test file path methods
TEST_F(PathManagerTest, FilePaths) {
    // Database path should be in state directory
    std::string db_path = PathManager::getDatabasePath();
    EXPECT_FALSE(db_path.empty());
    EXPECT_TRUE(db_path.find("autovibez_mixes.db") != std::string::npos);
    
    // Mix cache should be in cache directory
    std::string mixes_dir = PathManager::getMixesDirectory();
    EXPECT_FALSE(mixes_dir.empty());
    EXPECT_TRUE(mixes_dir.find("mixes") != std::string::npos);
    
    // Presets and textures should be in assets directory
    std::string presets = PathManager::getPresetsDirectory();
    std::string textures = PathManager::getTexturesDirectory();
    EXPECT_FALSE(presets.empty());
    EXPECT_FALSE(textures.empty());
    EXPECT_TRUE(presets.find("presets") != std::string::npos);
    EXPECT_TRUE(textures.find("textures") != std::string::npos);
}

// Test search paths
TEST_F(PathManagerTest, SearchPaths) {
    // Config search paths should be in correct order
    std::vector<std::string> config_paths = PathManager::getConfigFileSearchPaths();
    EXPECT_FALSE(config_paths.empty());
    
    // Should include environment variable path if set
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env) {
        EXPECT_EQ(config_paths[0], std::string(config_env));
    }
    
    // Should include XDG config directory
    bool has_xdg_config = false;
    for (const auto& path : config_paths) {
        if (path.find(PathManager::getConfigDirectory()) != std::string::npos) {
            has_xdg_config = true;
            break;
        }
    }
    EXPECT_TRUE(has_xdg_config);
    
    // Preset search paths
    std::vector<std::string> preset_paths = PathManager::getPresetSearchPaths();
    EXPECT_FALSE(preset_paths.empty());
    
    // Texture search paths
    std::vector<std::string> texture_paths = PathManager::getTextureSearchPaths();
    EXPECT_FALSE(texture_paths.empty());
}

// Test utility methods
TEST_F(PathManagerTest, UtilityMethods) {
    // App name should be consistent
    EXPECT_EQ(PathManager::getAppName(), "autovibez");
    
    // Path existence checking should work
    EXPECT_TRUE(PathManager::pathExists("."));
    EXPECT_FALSE(PathManager::pathExists("/this/path/does/not/exist/12345"));
}

// Test XDG base directory methods
TEST_F(PathManagerTest, XDGBaseDirectories) {
    // Core directories should return non-empty paths
    EXPECT_FALSE(PathManager::getConfigDirectory().empty());
    EXPECT_FALSE(PathManager::getDataDirectory().empty());
    EXPECT_FALSE(PathManager::getCacheDirectory().empty());
    EXPECT_FALSE(PathManager::getStateDirectory().empty());
    
    // Search paths should include multiple paths
    std::vector<std::string> config_paths = PathManager::getConfigFileSearchPaths();
    EXPECT_FALSE(config_paths.empty());
    
    std::vector<std::string> preset_paths = PathManager::getPresetSearchPaths();
    EXPECT_FALSE(preset_paths.empty());
    
    std::vector<std::string> texture_paths = PathManager::getTextureSearchPaths();
    EXPECT_FALSE(texture_paths.empty());
}



// Test tilde expansion
TEST_F(PathManagerTest, TildeExpansion) {
    // Should expand tilde to home directory
    std::string expanded = PathManager::expandTilde("~/test");
    EXPECT_FALSE(expanded.empty());
    EXPECT_NE(expanded, "~/test"); // Should be expanded
    
    // Should not expand if no tilde
    std::string not_expanded = PathManager::expandTilde("/absolute/path");
    EXPECT_EQ(not_expanded, "/absolute/path");
    
    // Should handle empty string
    std::string empty = PathManager::expandTilde("");
    EXPECT_EQ(empty, "");
}

// Test directory creation
TEST_F(PathManagerTest, DirectoryCreation) {
    // Should not throw when creating directories
    EXPECT_NO_THROW(PathManager::ensureDirectoryExists(PathManager::getConfigDirectory()));
    EXPECT_NO_THROW(PathManager::ensureDirectoryExists(PathManager::getCacheDirectory()));
    EXPECT_NO_THROW(PathManager::ensureDirectoryExists(PathManager::getStateDirectory()));
}

// Test findConfigFile
TEST_F(PathManagerTest, FindConfigFile) {
    // Should return a path (even if empty) without throwing
    std::string config_file = PathManager::findConfigFile();
    // Note: This might be empty if no config file exists, which is valid
    // We just test that the method doesn't throw
    EXPECT_NO_THROW(PathManager::findConfigFile());
} 