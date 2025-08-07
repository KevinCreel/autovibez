#include "mix_manager.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

#include "gtest/gtest.h"
#include "mix_database.hpp"
#include "mix_downloader.hpp"
#include "mix_metadata.hpp"
#include "mp3_analyzer.hpp"
#include "path_manager.hpp"

using namespace AutoVibez::Data;

class MixManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directories
        temp_dir = std::filesystem::temp_directory_path() / "autovibez_test";
        db_path = temp_dir / "test_mixes.db";
        data_dir = temp_dir / "mixes";

        std::filesystem::create_directories(temp_dir);
        std::filesystem::create_directories(data_dir);
    }

    void TearDown() override {
        // Clean up temporary files
        std::filesystem::remove_all(temp_dir);
    }

    std::filesystem::path temp_dir;
    std::filesystem::path db_path;
    std::filesystem::path data_dir;
};

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_AllFilesExist) {
    // Create test database with mix entries
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());

    // Create test files
    std::vector<std::string> test_files = {(data_dir / "test1.mp3").string(), (data_dir / "test2.mp3").string(),
                                           (data_dir / "test3.mp3").string()};

    // Create dummy MP3 files
    for (const auto& file_path : test_files) {
        std::ofstream file(file_path, std::ios::binary);
        file.write("\xFF\xFB\x90\x00", 4);                    // MP3 header
        file.write(std::string(1000, '\x00').c_str(), 1000);  // Dummy data
        file.close();
    }

    // Add mixes to database
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Test";
    mix1.url = "http://example.com/1";
    mix1.local_path = test_files[0];
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Test";
    mix2.url = "http://example.com/2";
    mix2.local_path = test_files[1];
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Test";
    mix3.url = "http://example.com/3";
    mix3.local_path = test_files[2];
    mix3.duration_seconds = 300;

    ASSERT_TRUE(db.addMix(mix1));
    ASSERT_TRUE(db.addMix(mix2));
    ASSERT_TRUE(db.addMix(mix3));

    // Test validation
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_SomeFilesMissing) {
    // Create test files (only 2 out of 3)
    std::vector<std::string> test_files = {(data_dir / "test1.mp3").string(), (data_dir / "test2.mp3").string(),
                                           (data_dir / "test3.mp3").string()};

    // Create dummy MP3 files (only first two)
    for (int i = 0; i < 2; ++i) {
        std::ofstream file(test_files[i], std::ios::binary);
        file.write("\xFF\xFB\x90\x00", 4);                    // MP3 header
        file.write(std::string(1000, '\x00').c_str(), 1000);  // Dummy data
        file.close();
    }

    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add mixes to database through the manager
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Test";
    mix1.url = "http://example.com/1";
    mix1.local_path = test_files[0];
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Test";
    mix2.url = "http://example.com/2";
    mix2.local_path = test_files[1];
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Test";
    mix3.url = "http://example.com/3";
    mix3.local_path = test_files[2];
    mix3.duration_seconds = 300;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));

    // Verify that the file doesn't exist
    EXPECT_FALSE(std::filesystem::exists(test_files[2]));

    // Test validation (should fail)
    EXPECT_FALSE(manager.validateDatabaseFileConsistency());
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_AllFilesMissing) {
    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add mixes to database through the manager (no files created)
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Test";
    mix1.url = "http://example.com/1";
    mix1.local_path = (data_dir / "test1.mp3").string();
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Test";
    mix2.url = "http://example.com/2";
    mix2.local_path = (data_dir / "test2.mp3").string();
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Test";
    mix3.url = "http://example.com/3";
    mix3.local_path = (data_dir / "test3.mp3").string();
    mix3.duration_seconds = 300;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));

    // Verify that the files don't exist
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test1.mp3").string()));
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test2.mp3").string()));
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test3.mp3").string()));

    // Test validation (should fail)
    EXPECT_FALSE(manager.validateDatabaseFileConsistency());
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_EmptyDatabase) {
    // Create empty database
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());

    // Test validation (should pass for empty database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_MixesWithoutLocalPath) {
    // Create test database with mix entries (no local_path)
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());

    // Add mixes to database without local_path
    Mix mix1, mix2;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Test";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Test";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    ASSERT_TRUE(db.addMix(mix1));
    ASSERT_TRUE(db.addMix(mix2));

    // Test validation (should pass for mixes without local_path)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, CleanupMissingFiles_RemovesMissingEntries) {
    // Create test files (only 1 out of 3)
    std::vector<std::string> test_files = {(data_dir / "test1.mp3").string(), (data_dir / "test2.mp3").string(),
                                           (data_dir / "test3.mp3").string()};

    // Create dummy MP3 file (only first one)
    std::ofstream file(test_files[0], std::ios::binary);
    file.write("\xFF\xFB\x90\x00", 4);                    // MP3 header
    file.write(std::string(1000, '\x00').c_str(), 1000);  // Dummy data
    file.close();

    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add mixes to database through the manager
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Test";
    mix1.url = "http://example.com/1";
    mix1.local_path = test_files[0];
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Test";
    mix2.url = "http://example.com/2";
    mix2.local_path = test_files[1];
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Test";
    mix3.url = "http://example.com/3";
    mix3.local_path = test_files[2];
    mix3.duration_seconds = 300;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));

    // Verify initial state
    auto all_mixes = db->getAllMixes();
    EXPECT_EQ(all_mixes.size(), 3);

    // Test cleanup
    EXPECT_TRUE(manager.cleanupMissingFiles());

    // Verify cleanup removed missing entries
    all_mixes = db->getAllMixes();
    EXPECT_EQ(all_mixes.size(), 1);
    EXPECT_EQ(all_mixes[0].id, "test1");
}

TEST_F(MixManagerTest, GetRandomGenreDifferentFromCurrent) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add test mixes with different genres
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Drum & Bass";
    mix3.url = "http://example.com/3";
    mix3.local_path = "";
    mix3.duration_seconds = 300;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));

    // Set current genre to Techno
    manager.setCurrentGenre("Techno");
    EXPECT_EQ(manager.getCurrentGenre(), "Techno");

    // Get random genre (should be different from current)
    std::string newGenre = manager.getRandomGenre();

    // Should be different from Techno
    EXPECT_NE(newGenre, "Techno");
    EXPECT_TRUE(newGenre == "House" || newGenre == "Drum & Bass");

    // Current genre should be updated
    EXPECT_EQ(manager.getCurrentGenre(), newGenre);
}

TEST_F(MixManagerTest, GetRandomGenreDifferentFromCurrent_SingleGenre) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add test mixes with only one genre
    Mix mix1, mix2;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Techno";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));

    // Set current genre to Techno
    manager.setCurrentGenre("Techno");
    EXPECT_EQ(manager.getCurrentGenre(), "Techno");

    // Get random genre (should return the same genre since there's only one)
    std::string newGenre = manager.getRandomGenre();

    // Should be Techno since there's only one genre
    EXPECT_EQ(newGenre, "Techno");
    EXPECT_EQ(manager.getCurrentGenre(), "Techno");
}

TEST_F(MixManagerTest, GetRandomMixWithExclusion) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add test mixes
    Mix mix1, mix2, mix3;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Drum & Bass";
    mix3.url = "http://example.com/3";
    mix3.local_path = "";
    mix3.duration_seconds = 300;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));

    // Get random mix excluding test1
    Mix randomMix = manager.getRandomMix("test1");

    // Should not be test1
    EXPECT_NE(randomMix.id, "test1");
    EXPECT_TRUE(randomMix.id == "test2" || randomMix.id == "test3");

    // Get random mix excluding test2
    randomMix = manager.getRandomMix("test2");

    // Should not be test2
    EXPECT_NE(randomMix.id, "test2");
    EXPECT_TRUE(randomMix.id == "test1" || randomMix.id == "test3");
}

TEST_F(MixManagerTest, GetRandomMixWithExclusion_SingleMix) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add only one test mix
    Mix mix1;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    // Add mix to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));

    // Get random mix excluding test1 (should return empty since there's only one mix)
    Mix randomMix = manager.getRandomMix("test1");

    // Should be empty since we're excluding the only mix
    EXPECT_TRUE(randomMix.id.empty());
}

TEST_F(MixManagerTest, AllRandomMixMethodsExcludeCurrentMix) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add test mixes with different genres and artists
    Mix mix1, mix2, mix3, mix4;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 1";
    mix3.genre = "Techno";
    mix3.url = "http://example.com/3";
    mix3.local_path = "";
    mix3.duration_seconds = 300;

    mix4.id = "test4";
    mix4.title = "Test Mix 4";
    mix4.artist = "Artist 3";
    mix4.genre = "Drum & Bass";
    mix4.url = "http://example.com/4";
    mix4.local_path = "";
    mix4.duration_seconds = 360;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));
    ASSERT_TRUE(db->addMix(mix4));

    // Test getRandomMix with exclusion
    Mix randomMix = manager.getRandomMix("test1");
    EXPECT_NE(randomMix.id, "test1");

    // Test getRandomMixByGenre with exclusion
    randomMix = manager.getRandomMixByGenre("Techno", "test1");
    EXPECT_NE(randomMix.id, "test1");
    EXPECT_EQ(randomMix.genre, "Techno");

    // Test getRandomMixByArtist with exclusion
    randomMix = manager.getRandomMixByArtist("Artist 1", "test1");
    EXPECT_NE(randomMix.id, "test1");
    EXPECT_EQ(randomMix.artist, "Artist 1");

    // Test getRandomFavoriteMix with exclusion (first make test1 a favorite)
    db->toggleFavorite("test1");
    db->toggleFavorite("test2");
    randomMix = manager.getRandomFavoriteMix("test1");
    EXPECT_NE(randomMix.id, "test1");
    EXPECT_EQ(randomMix.id, "test2");  // Should be the only other favorite

    // Test getRandomAvailableMix with exclusion (add to available mixes)
    std::vector<Mix> available_mixes = {mix1, mix2, mix3, mix4};
    manager.syncMixesWithDatabase(available_mixes);

    randomMix = manager.getRandomAvailableMix("test1");
    EXPECT_NE(randomMix.id, "test1");

    // Test getRandomAvailableMixByGenre with exclusion
    randomMix = manager.getRandomAvailableMixByGenre("Techno", "test1");
    EXPECT_NE(randomMix.id, "test1");
    EXPECT_EQ(randomMix.genre, "Techno");
}

TEST_F(MixManagerTest, RapidMixSelectionPreventsRepetition) {
    // Initialize MixManager
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());

    // Add test mixes with different genres
    Mix mix1, mix2, mix3, mix4, mix5;
    mix1.id = "test1";
    mix1.title = "Test Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.url = "http://example.com/1";
    mix1.local_path = "";
    mix1.duration_seconds = 180;

    mix2.id = "test2";
    mix2.title = "Test Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Techno";
    mix2.url = "http://example.com/2";
    mix2.local_path = "";
    mix2.duration_seconds = 240;

    mix3.id = "test3";
    mix3.title = "Test Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Techno";
    mix3.url = "http://example.com/3";
    mix3.local_path = "";
    mix3.duration_seconds = 300;

    mix4.id = "test4";
    mix4.title = "Test Mix 4";
    mix4.artist = "Artist 4";
    mix4.genre = "Techno";
    mix4.url = "http://example.com/4";
    mix4.local_path = "";
    mix4.duration_seconds = 360;

    mix5.id = "test5";
    mix5.title = "Test Mix 5";
    mix5.artist = "Artist 5";
    mix5.genre = "Techno";
    mix5.url = "http://example.com/5";
    mix5.local_path = "";
    mix5.duration_seconds = 420;

    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));
    ASSERT_TRUE(db->addMix(mix4));
    ASSERT_TRUE(db->addMix(mix5));

    // Simulate rapid G key presses (random mix in current genre)
    // Each call should exclude the previous selection
    std::vector<std::string> selected_mixes;

    // First selection (no exclusion needed for first call)
    Mix first_mix = manager.getRandomMixByGenre("Techno");
    EXPECT_FALSE(first_mix.id.empty());
    selected_mixes.push_back(first_mix.id);

    // Second selection (should exclude first_mix)
    Mix second_mix = manager.getRandomMixByGenre("Techno", first_mix.id);
    EXPECT_FALSE(second_mix.id.empty());
    EXPECT_NE(second_mix.id, first_mix.id);
    selected_mixes.push_back(second_mix.id);

    // Third selection (should exclude second_mix)
    Mix third_mix = manager.getRandomMixByGenre("Techno", second_mix.id);
    EXPECT_FALSE(third_mix.id.empty());
    EXPECT_NE(third_mix.id, second_mix.id);
    selected_mixes.push_back(third_mix.id);

    // Fourth selection (should exclude third_mix)
    Mix fourth_mix = manager.getRandomMixByGenre("Techno", third_mix.id);
    EXPECT_FALSE(fourth_mix.id.empty());
    EXPECT_NE(fourth_mix.id, third_mix.id);
    selected_mixes.push_back(fourth_mix.id);

    // Fifth selection (should exclude fourth_mix)
    Mix fifth_mix = manager.getRandomMixByGenre("Techno", fourth_mix.id);
    EXPECT_FALSE(fifth_mix.id.empty());
    EXPECT_NE(fifth_mix.id, fourth_mix.id);
    selected_mixes.push_back(fifth_mix.id);

    // Verify no consecutive repetitions
    for (size_t i = 1; i < selected_mixes.size(); ++i) {
        EXPECT_NE(selected_mixes[i], selected_mixes[i - 1])
            << "Mix " << selected_mixes[i] << " was selected twice in a row";
    }

    // Verify all selections are from the correct genre
    for (const auto& mix_id : selected_mixes) {
        Mix mix = db->getMixById(mix_id);
        EXPECT_EQ(mix.genre, "Techno");
    }
}
