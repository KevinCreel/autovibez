#pragma once

#include <memory>
#include <string>
#include <vector>

namespace AutoVibez {
namespace Data {

/**
 * @brief Interface for database statement execution with RAII
 */
class IStatement {
public:
    virtual ~IStatement() = default;

    /**
     * @brief Bind text parameter to statement
     * @param index Parameter index (1-based)
     * @param value Text value to bind
     */
    virtual void bindText(int index, const std::string& value) = 0;

    /**
     * @brief Bind integer parameter to statement
     * @param index Parameter index (1-based)
     * @param value Integer value to bind
     */
    virtual void bindInt(int index, int value) = 0;

    /**
     * @brief Execute the statement
     * @return True if successful, false otherwise
     */
    virtual bool execute() = 0;

    /**
     * @brief Step through result set
     * @return True if row available, false if no more rows
     */
    virtual bool step() = 0;

    /**
     * @brief Get text from current row by column index
     * @param column Column index (0-based)
     * @return Text value or empty string if null
     */
    virtual std::string getText(int column) const = 0;

    /**
     * @brief Get text from current row by column name
     * @param columnName Column name
     * @return Text value or empty string if null
     */
    virtual std::string getText(const std::string& columnName) const = 0;

    /**
     * @brief Get integer from current row by column index
     * @param column Column index (0-based)
     * @return Integer value or 0 if null
     */
    virtual int getInt(int column) const = 0;

    /**
     * @brief Get integer from current row by column name
     * @param columnName Column name
     * @return Integer value or 0 if null
     */
    virtual int getInt(const std::string& columnName) const = 0;

    /**
     * @brief Check if column value is null by column index
     * @param column Column index (0-based)
     * @return True if null, false otherwise
     */
    virtual bool isNull(int column) const = 0;

    /**
     * @brief Check if column value is null by column name
     * @param columnName Column name
     * @return True if null, false otherwise
     */
    virtual bool isNull(const std::string& columnName) const = 0;

    /**
     * @brief Get number of rows affected by last operation
     * @return Number of affected rows
     */
    virtual int getChanges() const = 0;
};

/**
 * @brief Interface for database connection operations
 */
class IDatabaseConnection {
public:
    virtual ~IDatabaseConnection() = default;

    /**
     * @brief Initialize the database connection
     * @return True if successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Execute a simple SQL statement
     * @param sql SQL statement to execute
     * @return True if successful, false otherwise
     */
    virtual bool execute(const std::string& sql) = 0;

    /**
     * @brief Prepare a SQL statement for execution
     * @param sql SQL statement to prepare
     * @return Prepared statement or nullptr on error
     */
    virtual std::unique_ptr<IStatement> prepare(const std::string& sql) = 0;

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    virtual std::string getLastError() const = 0;

    /**
     * @brief Begin a transaction
     * @return True if successful, false otherwise
     */
    virtual bool beginTransaction() = 0;

    /**
     * @brief Commit a transaction
     * @return True if successful, false otherwise
     */
    virtual bool commitTransaction() = 0;

    /**
     * @brief Rollback a transaction
     * @return True if successful, false otherwise
     */
    virtual bool rollbackTransaction() = 0;
};

/**
 * @brief Result of validation operations
 */
struct ValidationResult {
    bool isValid;
    std::string errorMessage;

    ValidationResult(bool valid = true, const std::string& error = "") : isValid(valid), errorMessage(error) {}

    operator bool() const {
        return isValid;
    }
};

}  // namespace Data
}  // namespace AutoVibez
