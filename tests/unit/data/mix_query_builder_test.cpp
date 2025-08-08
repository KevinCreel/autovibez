#include "mix_query_builder.hpp"

#include <gtest/gtest.h>

#include "sqlite_connection.hpp"

using namespace AutoVibez::Data;

class MixQueryBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        builder = std::make_unique<MixQueryBuilder>();
        connection = std::make_shared<SqliteConnection>(":memory:");
        ASSERT_TRUE(connection->initialize());

        // Create test table
        ASSERT_TRUE(
            connection->execute("CREATE TABLE mixes ("
                                "id TEXT PRIMARY KEY, "
                                "title TEXT, "
                                "artist TEXT, "
                                "genre TEXT, "
                                "url TEXT, "
                                "local_path TEXT, "
                                "duration_seconds INTEGER, "
                                "tags TEXT, "
                                "description TEXT, "
                                "date_added TEXT, "
                                "last_played TEXT, "
                                "play_count INTEGER DEFAULT 0, "
                                "is_favorite INTEGER DEFAULT 0, "
                                "is_deleted INTEGER DEFAULT 0"
                                ")"));

        // Insert test data
        insertTestData();
    }

    void TearDown() override {}

    void insertTestData() {
        connection->execute(
            "INSERT INTO mixes (id, title, artist, genre, local_path, is_favorite, is_deleted, last_played) VALUES "
            "('mix1', 'Electronic Mix 1', 'Artist A', 'Electronic', '/path/1.mp3', 1, 0, '2023-01-01'), "
            "('mix2', 'House Mix 1', 'Artist B', 'House', NULL, 0, 0, NULL), "
            "('mix3', 'Electronic Mix 2', 'Artist A', 'Electronic', '/path/3.mp3', 0, 0, '2023-01-02'), "
            "('mix4', 'Deleted Mix', 'Artist C', 'Techno', '/path/4.mp3', 0, 1, NULL)");
    }

    // Helper to execute a query and count results
    int countResults(const std::string& query, const std::vector<std::string>& params = {}) {
        auto stmt = connection->prepare(query);
        if (!stmt)
            return -1;

        for (size_t i = 0; i < params.size(); ++i) {
            stmt->bindText(static_cast<int>(i + 1), params[i]);
        }

        int count = 0;
        while (stmt->step()) {
            count++;
        }
        return count;
    }

    // Helper to verify query is executable
    bool canExecuteQuery(const std::string& query) {
        auto stmt = connection->prepare(query);
        return stmt != nullptr;
    }

    std::unique_ptr<MixQueryBuilder> builder;
    std::shared_ptr<SqliteConnection> connection;
};

TEST_F(MixQueryBuilderTest, BasicSelectQueryReturnsAllMixes) {
    std::string query = builder->select().build();

    // Query should be valid
    EXPECT_TRUE(canExecuteQuery(query));

    // Basic select returns ALL mixes including deleted ones (4 total)
    EXPECT_EQ(countResults(query), 4);
}

TEST_F(MixQueryBuilderTest, WhereNotDeletedFiltersDeletedMixes) {
    std::string query = builder->select().whereNotDeleted().build();

    // Should only return non-deleted mixes (3 out of 4)
    EXPECT_EQ(countResults(query), 3);

    // Verify deleted mix is excluded by checking with include deleted
    connection->execute("INSERT INTO mixes (id, title, is_deleted) VALUES ('temp', 'temp', 1)");
    EXPECT_EQ(countResults(query), 3);  // Still 3, new deleted mix not included
}

TEST_F(MixQueryBuilderTest, SelectWithMultipleWhereConditions) {
    std::string query = builder->select().whereNotDeleted().whereGenre().whereFavorites().build();

    EXPECT_NE(query.find("WHERE"), std::string::npos);
    EXPECT_NE(query.find("is_deleted = 0"), std::string::npos);
    EXPECT_NE(query.find("genre COLLATE NOCASE = ? COLLATE NOCASE"), std::string::npos);
    EXPECT_NE(query.find("is_favorite = 1"), std::string::npos);
    EXPECT_NE(query.find("AND"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, OrderByTitleSortsResultsAlphabetically) {
    std::string query = builder->select().whereNotDeleted().orderBy(OrderBy::Title).build();

    auto stmt = connection->prepare(query);
    ASSERT_NE(stmt, nullptr);

    std::vector<std::string> titles;
    while (stmt->step()) {
        titles.push_back(stmt->getText("title"));  // title column
    }

    // Should get results in alphabetical order
    EXPECT_EQ(titles.size(), 3);
    EXPECT_TRUE(titles[0] <= titles[1]);  // First <= Second
    EXPECT_TRUE(titles[1] <= titles[2]);  // Second <= Third
}

TEST_F(MixQueryBuilderTest, OrderByLastPlayedDescendingShowsRecentFirst) {
    std::string query = builder->select().whereNotDeleted().orderBy(OrderBy::LastPlayed, false).build();

    auto stmt = connection->prepare(query);
    ASSERT_NE(stmt, nullptr);

    std::vector<std::string> last_played_dates;
    while (stmt->step()) {
        if (!stmt->isNull("last_played")) {  // last_played column
            last_played_dates.push_back(stmt->getText("last_played"));
        }
    }

    // Should get recent dates first (mix3: 2023-01-02, mix1: 2023-01-01)
    EXPECT_GE(last_played_dates.size(), 2);
    if (last_played_dates.size() >= 2) {
        EXPECT_GE(last_played_dates[0], last_played_dates[1]);
    }
}

TEST_F(MixQueryBuilderTest, LimitRestrictsNumberOfResults) {
    std::string query = builder->select().whereNotDeleted().limit(2).build();

    // Should return exactly 2 results even though we have 3 non-deleted mixes
    EXPECT_EQ(countResults(query), 2);

    // Verify with different limit
    query = builder->reset().select().whereNotDeleted().limit(1).build();
    EXPECT_EQ(countResults(query), 1);
}

TEST_F(MixQueryBuilderTest, ComplexQuery) {
    std::string query =
        builder->select().whereNotDeleted().whereGenre().whereDownloaded().orderBy(OrderBy::Random).limit(1).build();

    EXPECT_NE(query.find("SELECT * FROM mixes"), std::string::npos);
    EXPECT_NE(query.find("WHERE"), std::string::npos);
    EXPECT_NE(query.find("is_deleted = 0"), std::string::npos);
    EXPECT_NE(query.find("genre COLLATE NOCASE = ? COLLATE NOCASE"), std::string::npos);
    EXPECT_NE(query.find("local_path IS NOT NULL AND local_path != ''"), std::string::npos);
    EXPECT_NE(query.find("ORDER BY RANDOM()"), std::string::npos);
    EXPECT_NE(query.find("LIMIT 1"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, QueryWithParametersExecutesSuccessfully) {
    std::string query = builder->select().whereGenre().whereArtist().whereId().build();

    // Query should be valid and executable
    EXPECT_TRUE(canExecuteQuery(query));

    // Should find the Electronic mix by Artist A with specific ID
    EXPECT_EQ(countResults(query, {"Electronic", "Artist A", "mix1"}), 1);

    // Should find no results for non-existent combination
    EXPECT_EQ(countResults(query, {"Electronic", "Nonexistent Artist", "mix1"}), 0);
}

TEST_F(MixQueryBuilderTest, QueryWithMixedConditionsFiltersProperly) {
    std::string query = builder->select().whereNotDeleted().whereFavorites().whereDownloaded().whereGenre().build();

    // Query should be valid and executable
    EXPECT_TRUE(canExecuteQuery(query));

    // Should find only non-deleted, favorite, downloaded mixes of specified genre
    // mix1 is favorite (1), not deleted (0), downloaded (has local_path), Electronic
    EXPECT_EQ(countResults(query, {"Electronic"}), 1);

    // Should find no House mixes that meet all criteria (mix2 is not favorite)
    EXPECT_EQ(countResults(query, {"House"}), 0);
}

TEST_F(MixQueryBuilderTest, ResetBuilderAllowsReuseForDifferentQueries) {
    // Build first query
    std::string query1 = builder->select().whereGenre().orderBy(OrderBy::Title).limit(10).build();
    EXPECT_TRUE(canExecuteQuery(query1));

    // Reset and build different query
    builder->reset();
    std::string query2 = builder->select().whereFavorites().build();
    EXPECT_TRUE(canExecuteQuery(query2));

    // Queries should be different
    EXPECT_NE(query1, query2);

    // Second query should find only favorites
    EXPECT_EQ(countResults(query2), 1);  // Only mix1 is favorite

    // After reset, empty build should return empty string
    builder->reset();
    std::string empty_query = builder->build();
    EXPECT_TRUE(empty_query.empty());
}

TEST_F(MixQueryBuilderTest, BuildQueryFromCriteria) {
    SelectionCriteria criteria;
    criteria.genre = "Electronic";
    criteria.favorites_only = true;
    criteria.downloaded_only = true;
    criteria.exclude_mix_id = "exclude-123";
    criteria.limit = 5;

    std::string query = MixQueryBuilder::buildQuery(criteria, OrderBy::Title);

    EXPECT_NE(query.find("SELECT * FROM mixes"), std::string::npos);
    EXPECT_NE(query.find("is_deleted = 0"), std::string::npos);
    EXPECT_NE(query.find("genre COLLATE NOCASE = ? COLLATE NOCASE"), std::string::npos);
    EXPECT_NE(query.find("id != ?"), std::string::npos);
    EXPECT_NE(query.find("is_favorite = 1"), std::string::npos);
    EXPECT_NE(query.find("local_path IS NOT NULL"), std::string::npos);
    EXPECT_NE(query.find("ORDER BY title ASC"), std::string::npos);
    EXPECT_NE(query.find("LIMIT 5"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, BuildQueryWithIncludeDeleted) {
    SelectionCriteria criteria;
    criteria.include_deleted = true;
    criteria.genre = "Electronic";

    std::string query = MixQueryBuilder::buildQuery(criteria);

    // Should not have is_deleted = 0 condition
    EXPECT_EQ(query.find("is_deleted = 0"), std::string::npos);
    EXPECT_NE(query.find("genre COLLATE NOCASE = ? COLLATE NOCASE"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, EmptyCriteria) {
    SelectionCriteria criteria;
    std::string query = MixQueryBuilder::buildQuery(criteria);

    EXPECT_NE(query.find("SELECT * FROM mixes"), std::string::npos);
    EXPECT_NE(query.find("WHERE is_deleted = 0"), std::string::npos);
    EXPECT_NE(query.find("ORDER BY title ASC"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, OrderByVariants) {
    std::string query;

    query = builder->reset().select().orderBy(OrderBy::Artist).build();
    EXPECT_NE(query.find("ORDER BY artist ASC, title ASC"), std::string::npos);

    query = builder->reset().select().orderBy(OrderBy::Genre).build();
    EXPECT_NE(query.find("ORDER BY genre ASC, title ASC"), std::string::npos);

    query = builder->reset().select().orderBy(OrderBy::PlayCount).build();
    EXPECT_NE(query.find("ORDER BY play_count ASC"), std::string::npos);

    query = builder->reset().select().orderBy(OrderBy::DateAdded).build();
    EXPECT_NE(query.find("ORDER BY date_added ASC"), std::string::npos);

    query = builder->reset().select().orderBy(OrderBy::Random).build();
    EXPECT_NE(query.find("ORDER BY RANDOM()"), std::string::npos);

    query = builder->reset().select().orderBy(OrderBy::None).build();
    EXPECT_EQ(query.find("ORDER BY"), std::string::npos);
}

TEST_F(MixQueryBuilderTest, ZeroLimitReturnsAllResults) {
    std::string query = builder->select().whereNotDeleted().limit(0).build();

    // Zero limit should return all results (3 non-deleted mixes)
    EXPECT_EQ(countResults(query), 3);
}

TEST_F(MixQueryBuilderTest, NegativeLimitReturnsAllResults) {
    std::string query = builder->select().whereNotDeleted().limit(-5).build();

    // Negative limit should return all results (3 non-deleted mixes)
    EXPECT_EQ(countResults(query), 3);
}

TEST_F(MixQueryBuilderTest, WhereConditionVariants) {
    std::string query;

    query = builder->reset().select().whereId().build();
    EXPECT_NE(query.find("WHERE id = ?"), std::string::npos);

    query = builder->reset().select().whereNotId().build();
    EXPECT_NE(query.find("WHERE id != ?"), std::string::npos);

    query = builder->reset().select().whereArtist().build();
    EXPECT_NE(query.find("WHERE artist = ?"), std::string::npos);

    query = builder->reset().select().whereHasBeenPlayed().build();
    EXPECT_NE(query.find("WHERE last_played IS NOT NULL"), std::string::npos);
}
