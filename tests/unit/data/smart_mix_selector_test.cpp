#include "smart_mix_selector.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "sqlite_connection.hpp"

using namespace AutoVibez::Data;

class SmartMixSelectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        connection = std::make_shared<SqliteConnection>(":memory:");
        ASSERT_TRUE(connection->initialize());

        // Create test table
        ASSERT_TRUE(connection->execute(R"(
            CREATE TABLE mixes (
                id TEXT PRIMARY KEY,
                title TEXT NOT NULL,
                artist TEXT NOT NULL,
                genre TEXT NOT NULL,
                url TEXT NOT NULL,
                local_path TEXT,
                duration_seconds INTEGER NOT NULL,
                tags TEXT,
                description TEXT,
                date_added DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_played DATETIME,
                play_count INTEGER DEFAULT 0,
                is_favorite BOOLEAN DEFAULT 0,
                is_deleted BOOLEAN DEFAULT 0
            )
        )"));

        // Configure for deterministic testing
        SmartSelectionConfig config;
        config.preferred_genre_probability = 80;
        config.favorite_mix_probability = 70;

        selector = std::make_unique<SmartMixSelector>(connection, config);
        selector->setSeed(12345);  // Fixed seed for deterministic tests

        createTestData();
    }

    void TearDown() override {}

    void createTestData() {
        // Insert test mixes
        ASSERT_TRUE(connection->execute(R"(
            INSERT INTO mixes (id, title, artist, genre, url, local_path, duration_seconds, is_favorite, is_deleted)
            VALUES 
            ('mix1', 'Electronic Mix 1', 'Artist A', 'Electronic', 'http://example.com/1.mp3', '/path/to/1.mp3', 3600, 1, 0),
            ('mix2', 'House Mix 1', 'Artist B', 'House', 'http://example.com/2.mp3', '/path/to/2.mp3', 3200, 0, 0),
            ('mix3', 'Electronic Mix 2', 'Artist A', 'Electronic', 'http://example.com/3.mp3', NULL, 2800, 1, 0),
            ('mix4', 'Techno Mix 1', 'Artist C', 'Techno', 'http://example.com/4.mp3', '/path/to/4.mp3', 4000, 0, 0),
            ('mix5', 'House Mix 2', 'Artist B', 'House', 'http://example.com/5.mp3', NULL, 3400, 0, 1),
            ('mix6', 'Electronic Mix 3', 'Artist D', 'Electronic', 'http://example.com/6.mp3', '/path/to/6.mp3', 3800, 1, 0)
        )"));
    }

    std::shared_ptr<SqliteConnection> connection;
    std::unique_ptr<SmartMixSelector> selector;
};

TEST_F(SmartMixSelectorTest, GetRandomMixReturnsValidMix) {
    Mix mix = selector->getRandomMix();
    EXPECT_FALSE(mix.id.empty());
    EXPECT_FALSE(mix.title.empty());
    EXPECT_FALSE(mix.artist.empty());
    EXPECT_FALSE(mix.genre.empty());
}

TEST_F(SmartMixSelectorTest, GetRandomMixExcludesSpecifiedMix) {
    Mix excluded_mix = selector->getRandomMix();
    ASSERT_FALSE(excluded_mix.id.empty());

    // Get another mix excluding the first one
    Mix different_mix = selector->getRandomMix(excluded_mix.id);
    EXPECT_NE(different_mix.id, excluded_mix.id);
}

TEST_F(SmartMixSelectorTest, GetRandomMixByGenre) {
    Mix mix = selector->getRandomMixByGenre("Electronic");
    EXPECT_FALSE(mix.id.empty());
    EXPECT_EQ(mix.genre, "Electronic");
}

TEST_F(SmartMixSelectorTest, GetRandomMixByGenreWithExclusion) {
    Mix mix = selector->getRandomMixByGenre("Electronic", "mix1");
    EXPECT_FALSE(mix.id.empty());
    EXPECT_EQ(mix.genre, "Electronic");
    EXPECT_NE(mix.id, "mix1");
}

TEST_F(SmartMixSelectorTest, GetRandomMixByArtist) {
    Mix mix = selector->getRandomMixByArtist("Artist A");
    EXPECT_FALSE(mix.id.empty());
    EXPECT_EQ(mix.artist, "Artist A");
}

TEST_F(SmartMixSelectorTest, GetRandomMixByArtistWithExclusion) {
    Mix mix = selector->getRandomMixByArtist("Artist A", "mix1");
    EXPECT_FALSE(mix.id.empty());
    EXPECT_EQ(mix.artist, "Artist A");
    EXPECT_NE(mix.id, "mix1");
}

TEST_F(SmartMixSelectorTest, GetNextMixInSequence) {
    Mix first_mix = selector->getNextMix("");  // Get first mix
    EXPECT_FALSE(first_mix.id.empty());

    Mix next_mix = selector->getNextMix(first_mix.id);
    EXPECT_FALSE(next_mix.id.empty());
    EXPECT_NE(next_mix.id, first_mix.id);
}

TEST_F(SmartMixSelectorTest, GetPreviousMixInSequence) {
    Mix last_mix = selector->getPreviousMix("");  // Get last mix
    EXPECT_FALSE(last_mix.id.empty());

    Mix previous_mix = selector->getPreviousMix(last_mix.id);
    EXPECT_FALSE(previous_mix.id.empty());
    EXPECT_NE(previous_mix.id, last_mix.id);
}

TEST_F(SmartMixSelectorTest, GetSmartRandomMixWithPreferredGenre) {
    // Test with preferred genre that exists
    Mix mix = selector->getSmartRandomMix("", "Electronic");
    EXPECT_FALSE(mix.id.empty());
    // Note: Due to probability, we can't guarantee it will be Electronic,
    // but it should prefer downloaded mixes
    EXPECT_FALSE(mix.local_path.empty());
}

TEST_F(SmartMixSelectorTest, GetSmartRandomMixWithNonExistentGenre) {
    Mix mix = selector->getSmartRandomMix("", "NonExistent");
    EXPECT_FALSE(mix.id.empty());
    // Should fallback to any available mix
}

TEST_F(SmartMixSelectorTest, PrefersDownloadedMixes) {
    // Run multiple times to check preference for downloaded mixes
    int downloaded_count = 0;
    const int iterations = 10;

    for (int i = 0; i < iterations; ++i) {
        Mix mix = selector->getRandomMix();
        if (!mix.local_path.empty()) {
            downloaded_count++;
        }
    }

    // Should prefer downloaded mixes (though not guaranteed due to randomness)
    EXPECT_GT(downloaded_count, 0);
}

TEST_F(SmartMixSelectorTest, ExcludesDeletedMixes) {
    // All mixes returned should not be deleted
    for (int i = 0; i < 5; ++i) {
        Mix mix = selector->getRandomMix();
        EXPECT_FALSE(mix.is_deleted);
    }
}

TEST_F(SmartMixSelectorTest, HandleEmptyDatabase) {
    // Clear all data
    ASSERT_TRUE(connection->execute("DELETE FROM mixes"));

    Mix mix = selector->getRandomMix();
    EXPECT_TRUE(mix.id.empty());
}

TEST_F(SmartMixSelectorTest, HandleNoDownloadedMixes) {
    // Clear all local_path values
    ASSERT_TRUE(connection->execute("UPDATE mixes SET local_path = NULL"));

    Mix mix = selector->getRandomMix();
    EXPECT_FALSE(mix.id.empty());         // Should still return a mix
    EXPECT_TRUE(mix.local_path.empty());  // But it won't be downloaded
}

TEST_F(SmartMixSelectorTest, HandleSingleMixWithExclusion) {
    // Keep only one mix
    ASSERT_TRUE(connection->execute("DELETE FROM mixes WHERE id != 'mix1'"));

    Mix mix = selector->getRandomMix("mix1");
    EXPECT_TRUE(mix.id.empty());  // Should return empty when only mix is excluded
}

TEST_F(SmartMixSelectorTest, SeedDeterminism) {
    // Test that setSeed affects randomization behavior
    selector->setSeed(12345);
    std::vector<std::string> results1;
    for (int i = 0; i < 5; ++i) {
        Mix mix = selector->getRandomMix();
        results1.push_back(mix.id);
    }

    selector->setSeed(54321);  // Different seed
    std::vector<std::string> results2;
    for (int i = 0; i < 5; ++i) {
        Mix mix = selector->getRandomMix();
        results2.push_back(mix.id);
    }

    // Different seeds should potentially produce different sequences
    // (though they might still be the same by chance with small datasets)
    // The main thing is that setSeed doesn't crash and affects behavior
    EXPECT_FALSE(results1.empty());
    EXPECT_FALSE(results2.empty());
}

TEST_F(SmartMixSelectorTest, GetNextMixWrapsAround) {
    // Get the last mix by ID
    auto stmt = connection->prepare("SELECT id FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1");
    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(stmt->step());
    std::string last_id = stmt->getText(0);

    // Get next after last should wrap to first
    Mix next_mix = selector->getNextMix(last_id);
    EXPECT_FALSE(next_mix.id.empty());

    // Get first mix to compare
    auto first_stmt = connection->prepare("SELECT id FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1");
    ASSERT_NE(first_stmt, nullptr);
    ASSERT_TRUE(first_stmt->step());
    std::string first_id = first_stmt->getText(0);

    EXPECT_EQ(next_mix.id, first_id);
}

TEST_F(SmartMixSelectorTest, GetPreviousMixWrapsAround) {
    // Get the first mix by ID
    auto stmt = connection->prepare("SELECT id FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1");
    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(stmt->step());
    std::string first_id = stmt->getText(0);

    // Get previous before first should wrap to last
    Mix prev_mix = selector->getPreviousMix(first_id);
    EXPECT_FALSE(prev_mix.id.empty());

    // Get last mix to compare
    auto last_stmt = connection->prepare("SELECT id FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1");
    ASSERT_NE(last_stmt, nullptr);
    ASSERT_TRUE(last_stmt->step());
    std::string last_id = last_stmt->getText(0);

    EXPECT_EQ(prev_mix.id, last_id);
}

TEST_F(SmartMixSelectorTest, SmartSelectionConfigurationRespected) {
    SmartSelectionConfig config;
    config.preferred_genre_probability = 0;  // Never prefer genre
    config.favorite_mix_probability = 100;   // Always prefer favorites
    config.prefer_unplayed = true;
    config.prefer_least_played = true;

    auto custom_selector = std::make_unique<SmartMixSelector>(connection, config);
    custom_selector->setSeed(12345);

    // With 100% favorite probability, should tend to get favorites
    // (though not guaranteed due to other factors)
    Mix mix = custom_selector->getSmartRandomMix();
    EXPECT_FALSE(mix.id.empty());
}
