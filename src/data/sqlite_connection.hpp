#pragma once

#include <sqlite3.h>

#include <memory>
#include <string>

#include "database_interfaces.hpp"

namespace AutoVibez::Data {

/**
 * @brief RAII wrapper for SQLite statement
 */
class SqliteStatement : public IStatement {
public:
    explicit SqliteStatement(sqlite3_stmt* stmt, sqlite3* db);
    ~SqliteStatement() override;

    // Non-copyable but movable
    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;
    SqliteStatement(SqliteStatement&& other) noexcept;
    SqliteStatement& operator=(SqliteStatement&& other) noexcept;

    void bindText(int index, const std::string& value) override;
    void bindInt(int index, int value) override;
    bool execute() override;
    bool step() override;
    std::string getText(int column) const override;
    std::string getText(const std::string& columnName) const override;
    int getInt(int column) const override;
    int getInt(const std::string& columnName) const override;
    bool isNull(int column) const override;
    bool isNull(const std::string& columnName) const override;
    int getChanges() const override;

private:
    sqlite3_stmt* stmt_;
    sqlite3* db_;
    bool executed_;

    void cleanup();
    int getColumnIndex(const std::string& columnName) const;
};

/**
 * @brief SQLite implementation of database connection
 */
class SqliteConnection : public IDatabaseConnection {
public:
    explicit SqliteConnection(const std::string& db_path);
    ~SqliteConnection() override;

    // Non-copyable but movable
    SqliteConnection(const SqliteConnection&) = delete;
    SqliteConnection& operator=(const SqliteConnection&) = delete;
    SqliteConnection(SqliteConnection&& other) noexcept;
    SqliteConnection& operator=(SqliteConnection&& other) noexcept;

    bool initialize() override;
    bool execute(const std::string& sql) override;
    std::unique_ptr<IStatement> prepare(const std::string& sql) override;
    std::string getLastError() const override;
    bool beginTransaction() override;
    bool commitTransaction() override;
    bool rollbackTransaction() override;

private:
    sqlite3* db_;
    std::string db_path_;

    void cleanup();
};

}  // namespace AutoVibez::Data
