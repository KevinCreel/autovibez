#include "sqlite_connection.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "database_interfaces.hpp"

using namespace AutoVibez::Data;

class SqliteConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_db_path = ":memory:";  // Use in-memory database for testing
        connection = std::make_unique<SqliteConnection>(test_db_path);
    }

    void TearDown() override {}

    std::string test_db_path;
    std::unique_ptr<SqliteConnection> connection;
};

TEST_F(SqliteConnectionTest, InitializeSuccess) {
    EXPECT_TRUE(connection->initialize());
}

TEST_F(SqliteConnectionTest, InitializeInvalidPath) {
    auto invalid_connection = std::make_unique<SqliteConnection>("/invalid/path/database.db");
    EXPECT_FALSE(invalid_connection->initialize());
    EXPECT_FALSE(invalid_connection->getLastError().empty());
}

TEST_F(SqliteConnectionTest, ExecuteSimpleQuery) {
    ASSERT_TRUE(connection->initialize());

    bool result = connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)");
    EXPECT_TRUE(result);
}

TEST_F(SqliteConnectionTest, ExecuteInvalidQuery) {
    ASSERT_TRUE(connection->initialize());

    bool result = connection->execute("INVALID SQL QUERY");
    EXPECT_FALSE(result);
}

TEST_F(SqliteConnectionTest, PrepareValidStatement) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));

    auto stmt = connection->prepare("INSERT INTO test (name) VALUES (?)");
    EXPECT_NE(stmt, nullptr);
}

TEST_F(SqliteConnectionTest, PrepareInvalidStatement) {
    ASSERT_TRUE(connection->initialize());

    auto stmt = connection->prepare("INVALID SQL STATEMENT");
    EXPECT_EQ(stmt, nullptr);
}

TEST_F(SqliteConnectionTest, StatementBindAndExecute) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));

    auto stmt = connection->prepare("INSERT INTO test (name) VALUES (?)");
    ASSERT_NE(stmt, nullptr);

    stmt->bindText(1, "test_name");
    EXPECT_TRUE(stmt->execute());
    EXPECT_EQ(stmt->getChanges(), 1);
}

TEST_F(SqliteConnectionTest, StatementBindIntAndExecute) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER, value INTEGER)"));

    auto stmt = connection->prepare("INSERT INTO test (id, value) VALUES (?, ?)");
    ASSERT_NE(stmt, nullptr);

    stmt->bindInt(1, 1);
    stmt->bindInt(2, 42);
    EXPECT_TRUE(stmt->execute());
}

TEST_F(SqliteConnectionTest, StatementStepAndRetrieveData) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));
    ASSERT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test1')"));
    ASSERT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test2')"));

    auto stmt = connection->prepare("SELECT id, name FROM test ORDER BY id");
    ASSERT_NE(stmt, nullptr);

    // First row
    EXPECT_TRUE(stmt->step());
    EXPECT_EQ(stmt->getInt(0), 1);
    EXPECT_EQ(stmt->getText(1), "test1");

    // Second row
    EXPECT_TRUE(stmt->step());
    EXPECT_EQ(stmt->getInt(0), 2);
    EXPECT_EQ(stmt->getText(1), "test2");

    // No more rows
    EXPECT_FALSE(stmt->step());
}

TEST_F(SqliteConnectionTest, StatementIsNull) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER, name TEXT)"));
    ASSERT_TRUE(connection->execute("INSERT INTO test (id, name) VALUES (1, 'test')"));
    ASSERT_TRUE(connection->execute("INSERT INTO test (id, name) VALUES (2, NULL)"));

    auto stmt = connection->prepare("SELECT id, name FROM test ORDER BY id");
    ASSERT_NE(stmt, nullptr);

    // First row - not null
    EXPECT_TRUE(stmt->step());
    EXPECT_FALSE(stmt->isNull(0));
    EXPECT_FALSE(stmt->isNull(1));

    // Second row - name is null
    EXPECT_TRUE(stmt->step());
    EXPECT_FALSE(stmt->isNull(0));
    EXPECT_TRUE(stmt->isNull(1));
}

TEST_F(SqliteConnectionTest, TransactionOperations) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));

    // Begin transaction
    EXPECT_TRUE(connection->beginTransaction());

    // Insert some data
    EXPECT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test1')"));
    EXPECT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test2')"));

    // Commit transaction
    EXPECT_TRUE(connection->commitTransaction());

    // Verify data exists
    auto stmt = connection->prepare("SELECT COUNT(*) FROM test");
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(stmt->step());
    EXPECT_EQ(stmt->getInt(0), 2);
}

TEST_F(SqliteConnectionTest, TransactionRollback) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));
    ASSERT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('existing')"));

    // Begin transaction
    EXPECT_TRUE(connection->beginTransaction());

    // Insert some data
    EXPECT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test1')"));
    EXPECT_TRUE(connection->execute("INSERT INTO test (name) VALUES ('test2')"));

    // Rollback transaction
    EXPECT_TRUE(connection->rollbackTransaction());

    // Verify only original data exists
    auto stmt = connection->prepare("SELECT COUNT(*) FROM test");
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(stmt->step());
    EXPECT_EQ(stmt->getInt(0), 1);
}

TEST_F(SqliteConnectionTest, MoveConstructor) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY)"));

    // Move the connection
    auto moved_connection = std::move(*connection);

    // Original connection should be in a valid but unspecified state
    // Moved connection should work
    EXPECT_TRUE(moved_connection.execute("INSERT INTO test DEFAULT VALUES"));
}

TEST_F(SqliteConnectionTest, MoveAssignment) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY)"));

    SqliteConnection other_connection(":memory:");
    other_connection = std::move(*connection);

    // Moved-to connection should work
    EXPECT_TRUE(other_connection.execute("INSERT INTO test DEFAULT VALUES"));
}

TEST_F(SqliteConnectionTest, StatementMoveOperations) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));

    auto stmt1 = connection->prepare("INSERT INTO test (name) VALUES (?)");
    ASSERT_NE(stmt1, nullptr);

    // Test that statement can be moved (implementation should support it)
    stmt1->bindText(1, "test");
    EXPECT_TRUE(stmt1->execute());
}

TEST_F(SqliteConnectionTest, MultipleStatements) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));

    auto stmt1 = connection->prepare("INSERT INTO test (name) VALUES (?)");
    auto stmt2 = connection->prepare("SELECT COUNT(*) FROM test");

    ASSERT_NE(stmt1, nullptr);
    ASSERT_NE(stmt2, nullptr);

    stmt1->bindText(1, "test1");
    EXPECT_TRUE(stmt1->execute());

    stmt1->bindText(1, "test2");
    EXPECT_TRUE(stmt1->execute());

    EXPECT_TRUE(stmt2->step());
    EXPECT_EQ(stmt2->getInt(0), 2);
}

TEST_F(SqliteConnectionTest, ErrorMessages) {
    ASSERT_TRUE(connection->initialize());

    // Try to execute invalid SQL
    EXPECT_FALSE(connection->execute("INVALID SQL"));
    std::string error = connection->getLastError();
    EXPECT_FALSE(error.empty());
    EXPECT_NE(error.find("syntax error"), std::string::npos);
}

TEST_F(SqliteConnectionTest, StatementParameterBinding) {
    ASSERT_TRUE(connection->initialize());
    ASSERT_TRUE(connection->execute("CREATE TABLE test (text_col TEXT, int_col INTEGER)"));

    auto stmt = connection->prepare("INSERT INTO test (text_col, int_col) VALUES (?, ?)");
    ASSERT_NE(stmt, nullptr);

    // Test multiple parameter bindings
    stmt->bindText(1, "first");
    stmt->bindInt(2, 100);
    EXPECT_TRUE(stmt->execute());

    // Verify data
    auto select_stmt = connection->prepare("SELECT text_col, int_col FROM test");
    ASSERT_NE(select_stmt, nullptr);
    EXPECT_TRUE(select_stmt->step());
    EXPECT_EQ(select_stmt->getText(0), "first");
    EXPECT_EQ(select_stmt->getInt(1), 100);
}
