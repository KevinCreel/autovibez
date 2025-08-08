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
