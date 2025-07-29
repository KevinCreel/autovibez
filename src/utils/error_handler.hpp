#pragma once

#include <string>
#include <exception>
#include <functional>
#include <memory>
#include <system_error>

/**
 * Custom exception classes for AutoVibez
 */
namespace AutoVibez {

class Error : public std::exception {
public:
    Error() : message_("") {} // Default constructor
    explicit Error(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    const std::string& getMessage() const { return message_; }
    
private:
    std::string message_;
};

class AudioError : public Error {
public:
    explicit AudioError(const std::string& message) : Error("Audio Error: " + message) {}
};

class ConfigError : public Error {
public:
    explicit ConfigError(const std::string& message) : Error("Configuration Error: " + message) {}
};

class DatabaseError : public Error {
public:
    explicit DatabaseError(const std::string& message) : Error("Database Error: " + message) {}
};

class NetworkError : public Error {
public:
    explicit NetworkError(const std::string& message) : Error("Network Error: " + message) {}
};

class FileError : public Error {
public:
    explicit FileError(const std::string& message) : Error("File Error: " + message) {}
};

/**
 * Error handling utilities
 */
class ErrorHandler {
public:
    using ErrorCallback = std::function<void(const Error&)>;
    
    // Set global error callback
    static void setErrorCallback(ErrorCallback callback);
    
    // Handle error with optional recovery
    static void handleError(const Error& error, bool canRecover = true);
    
    // Handle system errors
    static void handleSystemError(const std::system_error& error);
    
    // Handle unknown exceptions
    static void handleUnknownException(const std::exception& error);
    
    // Check if error is recoverable
    static bool isRecoverable(const Error& error);
    
    // Get error severity level
    enum class Severity {
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };
    
    static Severity getSeverity(const Error& error);
    
private:
    static ErrorCallback errorCallback_;
    static bool hasErrorCallback_;
};

/**
 * RAII error context for better error reporting
 */
class ErrorContext {
public:
    explicit ErrorContext(const std::string& context);
    ~ErrorContext();
    
    // Add additional context information
    void addContext(const std::string& key, const std::string& value);
    
    // Get current context
    std::string getContext() const;
    
private:
    std::string context_;
    
public:
    static thread_local std::string currentContext_;
};

/**
 * Result type for operations that can fail
 */
template<typename T>
class Result {
public:
    // Success constructor
    explicit Result(T value) : value_(std::move(value)), hasError_(false) {}
    
    // Error constructor
    explicit Result(Error error) : error_(std::move(error)), hasError_(true) {}
    
    // Check if successful
    bool isSuccess() const { return !hasError_; }
    
    // Check if failed
    bool isError() const { return hasError_; }
    
    // Get value (throws if error)
    const T& getValue() const {
        if (hasError_) {
            throw error_;
        }
        return value_;
    }
    
    // Get value or default
    T getValueOr(T defaultValue) const {
        return hasError_ ? defaultValue : value_;
    }
    
    // Get error (throws if success)
    const Error& getError() const {
        if (!hasError_) {
            throw std::runtime_error("Result is not an error");
        }
        return error_;
    }
    
    // Map success value
    template<typename F>
    auto map(F&& func) const -> Result<decltype(func(std::declval<T>()))> {
        if (hasError_) {
            return Result<decltype(func(std::declval<T>()))>(error_);
        }
        try {
            return Result<decltype(func(std::declval<T>()))>(func(value_));
        } catch (const std::exception& e) {
            return Result<decltype(func(std::declval<T>()))>(Error(e.what()));
        }
    }
    
    // Flat map (monadic bind)
    template<typename F>
    auto flatMap(F&& func) const -> decltype(func(std::declval<T>())) {
        if (hasError_) {
            return decltype(func(std::declval<T>()))(error_);
        }
        try {
            return func(value_);
        } catch (const std::exception& e) {
            return decltype(func(std::declval<T>()))(Error(e.what()));
        }
    }
    
private:
    T value_;
    Error error_;
    bool hasError_;
};

// Specialization for void
template<>
class Result<void> {
public:
    // Success constructor
    Result() : hasError_(false), error_("") {}
    
    // Error constructor
    explicit Result(Error error) : error_(std::move(error)), hasError_(true) {}
    
    bool isSuccess() const { return !hasError_; }
    bool isError() const { return hasError_; }
    
    void getValue() const {
        if (hasError_) {
            throw error_;
        }
    }
    
    const Error& getError() const {
        if (!hasError_) {
            throw std::runtime_error("Result is not an error");
        }
        return error_;
    }
    
private:
    Error error_;
    bool hasError_;
};

} // namespace AutoVibez 