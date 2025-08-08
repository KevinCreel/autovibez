#include "mix_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "base_metadata.hpp"

using AutoVibez::Data::Mix;
using AutoVibez::Data::MixManager;

class MixManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directories
        test_dir = std::filesystem::temp_directory_path() / "autovibez_test";
        db_dir = test_dir / "db";
        data_dir = test_dir / "data";

        std::filesystem::create_directories(db_dir);
        std::filesystem::create_directories(data_dir);

        db_path = (db_dir / "test_mixes.db").string();
        data_path = data_dir.string();
    }

    void TearDown() override {
        // Clean up test directories
        std::filesystem::remove_all(test_dir);
    }

    Mix createMockMix(const std::string& id, const std::string& url, const std::string& title = "Test Mix",
                      const std::string& artist = "Test Artist", const std::string& genre = "Electronic") {
        Mix mix;
        mix.id = id;
        mix.url = url;
        mix.title = title;
        mix.artist = artist;
        mix.genre = genre;
        mix.is_favorite = false;
        mix.play_count = 0;
        return mix;
    }

    std::filesystem::path test_dir;
    std::filesystem::path db_dir;
    std::filesystem::path data_dir;
    std::string db_path;
    std::string data_path;
};

TEST_F(MixManagerTest, ConstructorInitialization) {
    MixManager manager(db_path, data_path);

    // Should be initialized without error
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, InitializeSuccess) {
    MixManager manager(db_path, data_path);

    bool result = manager.initialize();

    EXPECT_TRUE(result);
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, InitializeWithInvalidPaths) {
    MixManager manager("/invalid/path/db.db", "/invalid/path/data");

    bool result = manager.initialize();

    // Should fail with invalid paths
    EXPECT_FALSE(result);
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, VolumeControl) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Test setting volume
    bool result = manager.setVolume(50, false);
    EXPECT_TRUE(result);

    int volume = manager.getVolume();
    EXPECT_EQ(volume, 50);
}

TEST_F(MixManagerTest, VolumeControlBoundaryValues) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Test minimum volume
    bool result = manager.setVolume(0, false);
    EXPECT_TRUE(result);
    EXPECT_EQ(manager.getVolume(), 0);

    // Test maximum volume
    result = manager.setVolume(100, false);
    EXPECT_TRUE(result);
    EXPECT_EQ(manager.getVolume(), 100);
}

TEST_F(MixManagerTest, PlaybackState) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Initially not playing
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_FALSE(manager.isPaused());
    EXPECT_FALSE(manager.hasFinished());
}

TEST_F(MixManagerTest, CrossfadeControl) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Initially crossfade is disabled
    EXPECT_FALSE(manager.isCrossfadeEnabled());
    EXPECT_FALSE(manager.isCrossfading());

    // Enable crossfade
    manager.setCrossfadeEnabled(true);
    EXPECT_TRUE(manager.isCrossfadeEnabled());

    // Disable crossfade
    manager.setCrossfadeEnabled(false);
    EXPECT_FALSE(manager.isCrossfadeEnabled());
}

TEST_F(MixManagerTest, CrossfadeDuration) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Test default duration
    int default_duration = manager.getCrossfadeDuration();
    EXPECT_GT(default_duration, 0);

    // Set custom duration
    manager.setCrossfadeDuration(5000);
    EXPECT_EQ(manager.getCrossfadeDuration(), 5000);
}

TEST_F(MixManagerTest, ErrorStateManagement) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Initially no error
    EXPECT_TRUE(manager.isSuccess());
    EXPECT_TRUE(manager.getLastError().empty());

    // Trigger an operation on nonexistent mix - should not set error
    manager.toggleFavorite("nonexistent_id");

    // Should still be successful (silent failure)
    EXPECT_TRUE(manager.isSuccess());
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, GetMixByIdEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty mix when database is empty
    Mix found_mix = manager.getMixById("nonexistent_id");
    EXPECT_TRUE(found_mix.id.empty());
}

TEST_F(MixManagerTest, GetAllMixesEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty vector when database is empty
    std::vector<Mix> all_mixes = manager.getAllMixes();
    EXPECT_TRUE(all_mixes.empty());
}

TEST_F(MixManagerTest, GetMixesByGenreEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty vector when database is empty
    std::vector<Mix> electronic_mixes = manager.getMixesByGenre("Electronic");
    EXPECT_TRUE(electronic_mixes.empty());
}

TEST_F(MixManagerTest, GetRandomMixEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty mix when database is empty
    Mix random_mix = manager.getRandomMix("id1");
    EXPECT_TRUE(random_mix.id.empty());
}

TEST_F(MixManagerTest, GetRandomMixByGenreEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty mix when database is empty
    Mix random_electronic = manager.getRandomMixByGenre("Electronic");
    EXPECT_TRUE(random_electronic.id.empty());
}

TEST_F(MixManagerTest, ToggleFavoriteNonexistentMix) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // SQLite considers UPDATE with no rows affected as successful
    bool result = manager.toggleFavorite("nonexistent_id");
    EXPECT_TRUE(result);
}

TEST_F(MixManagerTest, UpdatePlayStatsNonexistentMix) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // SQLite considers UPDATE with no rows affected as successful
    bool result = manager.updatePlayStats("nonexistent_id");
    EXPECT_TRUE(result);
}

TEST_F(MixManagerTest, GetAvailableGenresEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return empty vector when database is empty
    std::vector<std::string> genres = manager.getAvailableGenres();
    EXPECT_TRUE(genres.empty());
}

TEST_F(MixManagerTest, GetRandomGenreEmptyDatabase) {
    MixManager manager(db_path, data_path);
    ASSERT_TRUE(manager.initialize());

    // Should return fallback genre when database is empty
    std::string random_genre = manager.getRandomGenre();
    EXPECT_FALSE(random_genre.empty());
    EXPECT_EQ(random_genre, "techno");  // Fallback value
}
