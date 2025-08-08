#include "data/mix_database.hpp"

#include <gtest/gtest.h>

#include <filesystem>

#include "data/mix_metadata.hpp"

class MixDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "autovibez_db_test";
        std::filesystem::create_directories(tempDir);
        dbPath = tempDir / "test.db";
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;
    std::string dbPath;
};

TEST_F(MixDatabaseTest, InitializeDatabase) {
    AutoVibez::Data::MixDatabase db(dbPath);

    // Should initialize successfully
    EXPECT_TRUE(db.initialize());
    EXPECT_TRUE(db.isSuccess());
}

TEST_F(MixDatabaseTest, AddAndGetMix) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Create a test mix
    AutoVibez::Data::Mix mix;
    mix.id = "test-mix-1";
    mix.title = "Test Mix";
    mix.artist = "Test Artist";
    mix.genre = "Techno";
    mix.duration_seconds = 3600;
    mix.local_path = "/path/to/mix.mp3";

    // Add mix to database
    EXPECT_TRUE(db.addMix(mix));
    EXPECT_TRUE(db.isSuccess());

    // Retrieve mix from database
    auto retrievedMix = db.getMixById("test-mix-1");
    EXPECT_EQ(retrievedMix.id, "test-mix-1");
    EXPECT_EQ(retrievedMix.title, "Test Mix");
    EXPECT_EQ(retrievedMix.artist, "Test Artist");
    EXPECT_EQ(retrievedMix.genre, "Techno");
}

TEST_F(MixDatabaseTest, QueryByGenre) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add multiple mixes with different genres
    AutoVibez::Data::Mix mix1;
    mix1.id = "test-mix-1";
    mix1.title = "Techno Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.duration_seconds = 3600;
    mix1.local_path = "/path/to/mix1.mp3";

    AutoVibez::Data::Mix mix2;
    mix2.id = "test-mix-2";
    mix2.title = "House Mix 1";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.duration_seconds = 3600;
    mix2.local_path = "/path/to/mix2.mp3";

    AutoVibez::Data::Mix mix3;
    mix3.id = "test-mix-3";
    mix3.title = "Techno Mix 2";
    mix3.artist = "Artist 3";
    mix3.genre = "Techno";
    mix3.duration_seconds = 3600;
    mix3.local_path = "/path/to/mix3.mp3";

    // Add all mixes
    EXPECT_TRUE(db.addMix(mix1));
    EXPECT_TRUE(db.addMix(mix2));
    EXPECT_TRUE(db.addMix(mix3));

    // Query by genre
    auto technoMixes = db.getMixesByGenre("Techno");
    EXPECT_EQ(technoMixes.size(), 2);

    auto houseMixes = db.getMixesByGenre("House");
    EXPECT_EQ(houseMixes.size(), 1);

    auto tranceMixes = db.getMixesByGenre("Trance");
    EXPECT_EQ(tranceMixes.size(), 0);
}

// ============================================================================
// SOFT DELETE TESTS
// ============================================================================

TEST_F(MixDatabaseTest, SoftDeleteMarksMixAsDeleted) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Create and add a test mix
    AutoVibez::Data::Mix mix;
    mix.id = "test-mix-1";
    mix.title = "Test Mix";
    mix.artist = "Test Artist";
    mix.genre = "Techno";
    mix.duration_seconds = 3600;
    mix.local_path = "/path/to/mix.mp3";
    mix.is_deleted = false;

    EXPECT_TRUE(db.addMix(mix));

    // Verify mix is initially not deleted
    auto retrievedMix = db.getMixById("test-mix-1");
    EXPECT_FALSE(retrievedMix.is_deleted);

    // Soft delete the mix
    EXPECT_TRUE(db.softDeleteMix("test-mix-1"));

    // Verify mix is now marked as deleted
    auto deletedMix = db.getMixById("test-mix-1");
    EXPECT_TRUE(deletedMix.is_deleted);
}

TEST_F(MixDatabaseTest, SoftDeleteReturnsFalseForNonexistentMix) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Try to soft delete a mix that doesn't exist
    EXPECT_FALSE(db.softDeleteMix("nonexistent-mix"));
}

TEST_F(MixDatabaseTest, GetAllMixesExcludesSoftDeleted) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add multiple mixes
    AutoVibez::Data::Mix mix1;
    mix1.id = "test-mix-1";
    mix1.title = "Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.duration_seconds = 3600;
    mix1.local_path = "/path/to/mix1.mp3";

    AutoVibez::Data::Mix mix2;
    mix2.id = "test-mix-2";
    mix2.title = "Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.duration_seconds = 3600;
    mix2.local_path = "/path/to/mix2.mp3";

    EXPECT_TRUE(db.addMix(mix1));
    EXPECT_TRUE(db.addMix(mix2));

    // Verify both mixes are returned initially
    auto allMixes = db.getAllMixes();
    EXPECT_EQ(allMixes.size(), 2);

    // Soft delete one mix
    EXPECT_TRUE(db.softDeleteMix("test-mix-1"));

    // Verify only non-deleted mix is returned
    allMixes = db.getAllMixes();
    EXPECT_EQ(allMixes.size(), 1);
    EXPECT_EQ(allMixes[0].id, "test-mix-2");
}

TEST_F(MixDatabaseTest, GetNextMixSkipsSoftDeletedMixes) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add three mixes in sequence
    AutoVibez::Data::Mix mix1;
    mix1.id = "mix-1";
    mix1.title = "Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.duration_seconds = 3600;
    mix1.local_path = "/path/to/mix1.mp3";

    AutoVibez::Data::Mix mix2;
    mix2.id = "mix-2";
    mix2.title = "Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.duration_seconds = 3600;
    mix2.local_path = "/path/to/mix2.mp3";

    AutoVibez::Data::Mix mix3;
    mix3.id = "mix-3";
    mix3.title = "Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Trance";
    mix3.duration_seconds = 3600;
    mix3.local_path = "/path/to/mix3.mp3";

    EXPECT_TRUE(db.addMix(mix1));
    EXPECT_TRUE(db.addMix(mix2));
    EXPECT_TRUE(db.addMix(mix3));

    // Soft delete the middle mix
    EXPECT_TRUE(db.softDeleteMix("mix-2"));

    // Get next mix after mix-1 should skip mix-2 and return mix-3
    auto nextMix = db.getNextMix("mix-1");
    EXPECT_EQ(nextMix.id, "mix-3");
}

TEST_F(MixDatabaseTest, GetPreviousMixSkipsSoftDeletedMixes) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add three mixes in sequence
    AutoVibez::Data::Mix mix1;
    mix1.id = "mix-1";
    mix1.title = "Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.duration_seconds = 3600;
    mix1.local_path = "/path/to/mix1.mp3";

    AutoVibez::Data::Mix mix2;
    mix2.id = "mix-2";
    mix2.title = "Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "House";
    mix2.duration_seconds = 3600;
    mix2.local_path = "/path/to/mix2.mp3";

    AutoVibez::Data::Mix mix3;
    mix3.id = "mix-3";
    mix3.title = "Mix 3";
    mix3.artist = "Artist 3";
    mix3.genre = "Trance";
    mix3.duration_seconds = 3600;
    mix3.local_path = "/path/to/mix3.mp3";

    EXPECT_TRUE(db.addMix(mix1));
    EXPECT_TRUE(db.addMix(mix2));
    EXPECT_TRUE(db.addMix(mix3));

    // Soft delete the middle mix
    EXPECT_TRUE(db.softDeleteMix("mix-2"));

    // Get previous mix before mix-3 should skip mix-2 and return mix-1
    auto prevMix = db.getPreviousMix("mix-3");
    EXPECT_EQ(prevMix.id, "mix-1");
}

TEST_F(MixDatabaseTest, GetMixesByGenreExcludesSoftDeleted) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add multiple mixes with same genre
    AutoVibez::Data::Mix mix1;
    mix1.id = "techno-mix-1";
    mix1.title = "Techno Mix 1";
    mix1.artist = "Artist 1";
    mix1.genre = "Techno";
    mix1.duration_seconds = 3600;
    mix1.local_path = "/path/to/mix1.mp3";

    AutoVibez::Data::Mix mix2;
    mix2.id = "techno-mix-2";
    mix2.title = "Techno Mix 2";
    mix2.artist = "Artist 2";
    mix2.genre = "Techno";
    mix2.duration_seconds = 3600;
    mix2.local_path = "/path/to/mix2.mp3";

    EXPECT_TRUE(db.addMix(mix1));
    EXPECT_TRUE(db.addMix(mix2));

    // Verify both mixes are returned initially
    auto technoMixes = db.getMixesByGenre("Techno");
    EXPECT_EQ(technoMixes.size(), 2);

    // Soft delete one mix
    EXPECT_TRUE(db.softDeleteMix("techno-mix-1"));

    // Verify only non-deleted mix is returned
    technoMixes = db.getMixesByGenre("Techno");
    EXPECT_EQ(technoMixes.size(), 1);
    EXPECT_EQ(technoMixes[0].id, "techno-mix-2");
}

TEST_F(MixDatabaseTest, GetRandomMixByGenreExcludesSoftDeleted) {
    AutoVibez::Data::MixDatabase db(dbPath);
    EXPECT_TRUE(db.initialize());

    // Add mix with unique genre
    AutoVibez::Data::Mix mix;
    mix.id = "unique-mix";
    mix.title = "Unique Mix";
    mix.artist = "Artist";
    mix.genre = "Experimental";
    mix.duration_seconds = 3600;
    mix.local_path = "/path/to/mix.mp3";

    EXPECT_TRUE(db.addMix(mix));

    // Verify mix can be retrieved by genre
    auto randomMix = db.getRandomMixByGenre("Experimental");
    EXPECT_EQ(randomMix.id, "unique-mix");

    // Soft delete the mix
    EXPECT_TRUE(db.softDeleteMix("unique-mix"));

    // Verify no mix is returned for that genre
    randomMix = db.getRandomMixByGenre("Experimental");
    EXPECT_TRUE(randomMix.id.empty());
}
