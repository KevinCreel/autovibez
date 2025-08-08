#include "sqlite_connection.hpp"

#include <utility>

namespace AutoVibez {
namespace Data {

// SqliteStatement Implementation
SqliteStatement::SqliteStatement(sqlite3_stmt* stmt, sqlite3* db) : stmt_(stmt), db_(db), executed_(false) {}

SqliteStatement::~SqliteStatement() {
    cleanup();
}

SqliteStatement::SqliteStatement(SqliteStatement&& other) noexcept
    : stmt_(other.stmt_), db_(other.db_), executed_(other.executed_) {
    other.stmt_ = nullptr;
    other.db_ = nullptr;
    other.executed_ = false;
}

SqliteStatement& SqliteStatement::operator=(SqliteStatement&& other) noexcept {
    if (this != &other) {
        cleanup();
        stmt_ = other.stmt_;
        db_ = other.db_;
        executed_ = other.executed_;
        other.stmt_ = nullptr;
        other.db_ = nullptr;
        other.executed_ = false;
    }
    return *this;
}

void SqliteStatement::bindText(int index, const std::string& value) {
    if (stmt_) {
        sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_STATIC);
    }
}

void SqliteStatement::bindInt(int index, int value) {
    if (stmt_) {
        sqlite3_bind_int(stmt_, index, value);
    }
}

bool SqliteStatement::execute() {
    if (!stmt_)
        return false;
    executed_ = true;
    int result = sqlite3_step(stmt_);
    return result == SQLITE_DONE;
}

bool SqliteStatement::step() {
    if (!stmt_)
        return false;
    executed_ = true;
    int result = sqlite3_step(stmt_);
    return result == SQLITE_ROW;
}

std::string SqliteStatement::getText(int column) const {
    if (!stmt_ || !executed_)
        return "";
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    return text ? reinterpret_cast<const char*>(text) : "";
}

std::string SqliteStatement::getText(const std::string& columnName) const {
    int column = getColumnIndex(columnName);
    return column >= 0 ? getText(column) : "";
}

int SqliteStatement::getInt(int column) const {
    if (!stmt_ || !executed_)
        return 0;
    return sqlite3_column_int(stmt_, column);
}

int SqliteStatement::getInt(const std::string& columnName) const {
    int column = getColumnIndex(columnName);
    return column >= 0 ? getInt(column) : 0;
}

bool SqliteStatement::isNull(int column) const {
    if (!stmt_ || !executed_)
        return true;
    return sqlite3_column_type(stmt_, column) == SQLITE_NULL;
}

bool SqliteStatement::isNull(const std::string& columnName) const {
    int column = getColumnIndex(columnName);
    return column >= 0 ? isNull(column) : true;
}

int SqliteStatement::getChanges() const {
    return db_ ? sqlite3_changes(db_) : 0;
}

void SqliteStatement::cleanup() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

int SqliteStatement::getColumnIndex(const std::string& columnName) const {
    if (!stmt_) {
        return -1;
    }

    int columnCount = sqlite3_column_count(stmt_);
    for (int i = 0; i < columnCount; ++i) {
        const char* name = sqlite3_column_name(stmt_, i);
        if (name && columnName == name) {
            return i;
        }
    }

    return -1;  // Column not found
}

// SqliteConnection Implementation
SqliteConnection::SqliteConnection(const std::string& db_path) : db_(nullptr), db_path_(db_path) {}

SqliteConnection::~SqliteConnection() {
    cleanup();
}

SqliteConnection::SqliteConnection(SqliteConnection&& other) noexcept
    : db_(other.db_), db_path_(std::move(other.db_path_)) {
    other.db_ = nullptr;
}

SqliteConnection& SqliteConnection::operator=(SqliteConnection&& other) noexcept {
    if (this != &other) {
        cleanup();
        db_ = other.db_;
        db_path_ = std::move(other.db_path_);
        other.db_ = nullptr;
    }
    return *this;
}

bool SqliteConnection::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    return rc == SQLITE_OK;
}

bool SqliteConnection::execute(const std::string& sql) {
    if (!db_)
        return false;
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (err_msg) {
        sqlite3_free(err_msg);
    }
    return rc == SQLITE_OK;
}

std::unique_ptr<IStatement> SqliteConnection::prepare(const std::string& sql) {
    if (!db_)
        return nullptr;

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    return std::make_unique<SqliteStatement>(stmt, db_);
}

std::string SqliteConnection::getLastError() const {
    return db_ ? sqlite3_errmsg(db_) : "Database not initialized";
}

bool SqliteConnection::beginTransaction() {
    return execute("BEGIN TRANSACTION");
}

bool SqliteConnection::commitTransaction() {
    return execute("COMMIT");
}

bool SqliteConnection::rollbackTransaction() {
    return execute("ROLLBACK");
}

void SqliteConnection::cleanup() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

}  // namespace Data
}  // namespace AutoVibez
