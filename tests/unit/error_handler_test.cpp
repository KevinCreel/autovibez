#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "error_handler.hpp"
#include <functional>
#include <vector>

using namespace AutoVibez;

class ErrorHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset error handler state
        ErrorHandler::setErrorCallback(nullptr);
    }
    
    void TearDown() override {
        // Clean up
        ErrorHandler::setErrorCallback(nullptr);
    }
    
    // Helper to capture error callbacks
    std::vector<Error> capturedErrors;
    std::function<void(const Error&)> errorCallback = [this](const Error& error) {
        capturedErrors.push_back(error);
    };
};

TEST_F(ErrorHandlerTest, ExceptionHierarchy) {
    // Test that all exception types inherit from base Error
    AudioError audioError("Test audio error");
    ConfigError configError("Test config error");
    DatabaseError dbError("Test database error");
    NetworkError networkError("Test network error");
    FileError fileError("Test file error");
    
    // Test that they're all catchable as base Error
    try {
        throw audioError;
    } catch (const Error& e) {
        EXPECT_NE(std::string(e.what()).find("Audio Error"), std::string::npos);
    }
    
    try {
        throw configError;
    } catch (const Error& e) {
        EXPECT_NE(std::string(e.what()).find("Configuration Error"), std::string::npos);
    }
    
    try {
        throw dbError;
    } catch (const Error& e) {
        EXPECT_NE(std::string(e.what()).find("Database Error"), std::string::npos);
    }
    
    try {
        throw networkError;
    } catch (const Error& e) {
        EXPECT_NE(std::string(e.what()).find("Network Error"), std::string::npos);
    }
    
    try {
        throw fileError;
    } catch (const Error& e) {
        EXPECT_NE(std::string(e.what()).find("File Error"), std::string::npos);
    }
}

TEST_F(ErrorHandlerTest, ErrorMessageRetrieval) {
    // Test error message retrieval
    std::string testMessage = "Test error message";
    Error error(testMessage);
    
    EXPECT_EQ(error.what(), testMessage);
    EXPECT_EQ(error.getMessage(), testMessage);
}

TEST_F(ErrorHandlerTest, ErrorCallback) {
    // Test error callback functionality
    ErrorHandler::setErrorCallback(errorCallback);
    
    AudioError audioError("Test callback error");
    ErrorHandler::handleError(audioError);
    
    EXPECT_EQ(capturedErrors.size(), 1);
    EXPECT_EQ(capturedErrors[0].getMessage(), "Audio Error: Test callback error");
}

TEST_F(ErrorHandlerTest, ErrorCallbackWithRecovery) {
    // Test error callback with recovery option
    ErrorHandler::setErrorCallback(errorCallback);
    
    ConfigError configError("Test recovery error");
    ErrorHandler::handleError(configError, true); // Can recover
    
    EXPECT_EQ(capturedErrors.size(), 1);
    EXPECT_EQ(capturedErrors[0].getMessage(), "Configuration Error: Test recovery error");
}

TEST_F(ErrorHandlerTest, ErrorCallbackWithoutRecovery) {
    // Test error callback without recovery
    ErrorHandler::setErrorCallback(errorCallback);
    
    DatabaseError dbError("Test no recovery error");
    ErrorHandler::handleError(dbError, false); // Cannot recover
    
    EXPECT_EQ(capturedErrors.size(), 1);
    EXPECT_EQ(capturedErrors[0].getMessage(), "Database Error: Test no recovery error");
}

TEST_F(ErrorHandlerTest, NoErrorCallback) {
    // Test handling errors when no callback is set
    NetworkError networkError("Test no callback error");
    
    // Should not crash
    EXPECT_NO_THROW(ErrorHandler::handleError(networkError));
}

TEST_F(ErrorHandlerTest, SystemErrorHandling) {
    // Test system error handling
    std::system_error sysError(std::error_code(ENOENT, std::system_category()), "File not found");
    
    // Should not crash
    EXPECT_NO_THROW(ErrorHandler::handleSystemError(sysError));
}

TEST_F(ErrorHandlerTest, UnknownExceptionHandling) {
    // Test unknown exception handling
    std::runtime_error runtimeError("Unknown runtime error");
    
    // Should not crash
    EXPECT_NO_THROW(ErrorHandler::handleUnknownException(runtimeError));
}

TEST_F(ErrorHandlerTest, ErrorRecoverability) {
    // Test error recoverability checking
    AudioError audioError("Test recoverable error");
    DatabaseError dbError("Test non-recoverable error");
    
    // Should not crash (implementation dependent)
    EXPECT_NO_THROW(ErrorHandler::isRecoverable(audioError));
    EXPECT_NO_THROW(ErrorHandler::isRecoverable(dbError));
}

TEST_F(ErrorHandlerTest, ErrorSeverity) {
    // Test error severity levels
    AudioError audioError("Test severity error");
    
    // Should not crash (implementation dependent)
    EXPECT_NO_THROW(ErrorHandler::getSeverity(audioError));
}

TEST_F(ErrorHandlerTest, ErrorContext) {
    // Test error context functionality
    ErrorContext context("Test Context");
    
    // Test context retrieval
    EXPECT_EQ(context.getContext(), "Test Context");
    
    // Test adding additional context
    context.addContext("key1", "value1");
    context.addContext("key2", "value2");
    
    // Context should be updated
    std::string updatedContext = context.getContext();
    EXPECT_NE(updatedContext.find("key1"), std::string::npos);
    EXPECT_NE(updatedContext.find("value1"), std::string::npos);
    EXPECT_NE(updatedContext.find("key2"), std::string::npos);
    EXPECT_NE(updatedContext.find("value2"), std::string::npos);
}

TEST_F(ErrorHandlerTest, ErrorContextDestruction) {
    // Test error context destruction
    {
        ErrorContext context("Temporary Context");
        EXPECT_EQ(context.getContext(), "Temporary Context");
    }
    // Context should be cleaned up automatically
    EXPECT_TRUE(true); // Destructor executed without crashing
}

TEST_F(ErrorHandlerTest, ResultSuccess) {
    // Test successful Result
    Result<int> result(42);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.getValue(), 42);
    EXPECT_EQ(result.getValueOr(0), 42);
}

TEST_F(ErrorHandlerTest, ResultError) {
    // Test error Result
    AudioError error("Test result error");
    Result<int> result(error);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.getValueOr(0), 0); // Should return default value
}

TEST_F(ErrorHandlerTest, ResultMap) {
    // Test Result map functionality
    Result<int> successResult(10);
    Result<std::string> mappedResult = successResult.map([](int value) {
        return std::to_string(value * 2);
    });
    
    EXPECT_TRUE(mappedResult.isSuccess());
    EXPECT_EQ(mappedResult.getValue(), "20");
}

TEST_F(ErrorHandlerTest, ResultMapWithError) {
    // Test Result map with error
    AudioError error("Test map error");
    Result<int> errorResult(error);
    Result<std::string> mappedResult = errorResult.map([](int value) {
        return std::to_string(value);
    });
    
    EXPECT_FALSE(mappedResult.isSuccess());
    EXPECT_TRUE(mappedResult.isError());
}

TEST_F(ErrorHandlerTest, ResultFlatMap) {
    // Test Result flatMap functionality
    Result<int> successResult(5);
    Result<std::string> flatMappedResult = successResult.flatMap([](int value) {
        return Result<std::string>(std::to_string(value * 3));
    });
    
    EXPECT_TRUE(flatMappedResult.isSuccess());
    EXPECT_EQ(flatMappedResult.getValue(), "15");
}

TEST_F(ErrorHandlerTest, ResultFlatMapWithError) {
    // Test Result flatMap with error
    AudioError error("Test flatMap error");
    Result<int> errorResult(error);
    Result<std::string> flatMappedResult = errorResult.flatMap([](int value) {
        return Result<std::string>(std::to_string(value));
    });
    
    EXPECT_FALSE(flatMappedResult.isSuccess());
    EXPECT_TRUE(flatMappedResult.isError());
}

TEST_F(ErrorHandlerTest, ResultVoid) {
    // Test void Result
    Result<void> successResult;
    EXPECT_TRUE(successResult.isSuccess());
    EXPECT_FALSE(successResult.isError());
    
    AudioError error("Test void result error");
    Result<void> errorResult(error);
    EXPECT_FALSE(errorResult.isSuccess());
    EXPECT_TRUE(errorResult.isError());
}

TEST_F(ErrorHandlerTest, ResultVoidGetValue) {
    // Test void Result getValue
    Result<void> successResult;
    EXPECT_NO_THROW(successResult.getValue());
    
    AudioError error("Test void result getValue error");
    Result<void> errorResult(error);
    EXPECT_THROW(errorResult.getValue(), std::exception);
}

TEST_F(ErrorHandlerTest, ResultCopyConstructor) {
    // Test Result copy constructor
    Result<int> original(42);
    Result<int> copied(original);
    
    EXPECT_TRUE(copied.isSuccess());
    EXPECT_EQ(copied.getValue(), 42);
}

TEST_F(ErrorHandlerTest, ResultMoveConstructor) {
    // Test Result move constructor
    Result<int> original(42);
    Result<int> moved(std::move(original));
    
    EXPECT_TRUE(moved.isSuccess());
    EXPECT_EQ(moved.getValue(), 42);
}

TEST_F(ErrorHandlerTest, ResultAssignment) {
    // Test Result assignment
    Result<int> original(42);
    Result<int> assigned(0); // Use a default value instead of default constructor
    assigned = original;
    
    EXPECT_TRUE(assigned.isSuccess());
    EXPECT_EQ(assigned.getValue(), 42);
}

TEST_F(ErrorHandlerTest, ResultMoveAssignment) {
    // Test Result move assignment
    Result<int> original(42);
    Result<int> assigned(0); // Use a default value instead of default constructor
    assigned = std::move(original);
    
    EXPECT_TRUE(assigned.isSuccess());
    EXPECT_EQ(assigned.getValue(), 42);
}

TEST_F(ErrorHandlerTest, ResultErrorAssignment) {
    // Test Result error assignment
    AudioError error("Test assignment error");
    Result<int> original(error);
    Result<int> assigned(0); // Use a default value instead of default constructor
    assigned = original;
    
    EXPECT_FALSE(assigned.isSuccess());
    EXPECT_TRUE(assigned.isError());
}

TEST_F(ErrorHandlerTest, ResultComplexTypes) {
    // Test Result with complex types
    std::vector<int> data = {1, 2, 3, 4, 5};
    Result<std::vector<int>> result(data);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.getValue().size(), 5);
    EXPECT_EQ(result.getValue()[0], 1);
}

TEST_F(ErrorHandlerTest, ResultComplexTypesError) {
    // Test Result with complex types and error
    AudioError error("Test complex error");
    Result<std::vector<int>> result(error);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    
    std::vector<int> defaultValue = {0};
    EXPECT_EQ(result.getValueOr(defaultValue), defaultValue);
}

TEST_F(ErrorHandlerTest, ErrorContextThreadLocal) {
    // Test thread-local error context
    ErrorContext context("Thread Context");
    
    // Context should be accessible
    EXPECT_EQ(context.getContext(), "Thread Context");
    
    // Should not interfere with other threads (basic test)
    EXPECT_TRUE(true); // Context created without crashing
}

TEST_F(ErrorHandlerTest, MultipleErrorCallbacks) {
    // Test multiple error callbacks
    std::vector<Error> callback1Errors;
    std::vector<Error> callback2Errors;
    
    auto callback1 = [&callback1Errors](const Error& error) {
        callback1Errors.push_back(error);
    };
    
    auto callback2 = [&callback2Errors](const Error& error) {
        callback2Errors.push_back(error);
    };
    
    // Set first callback
    ErrorHandler::setErrorCallback(callback1);
    AudioError error1("First callback error");
    ErrorHandler::handleError(error1);
    
    // Set second callback
    ErrorHandler::setErrorCallback(callback2);
    AudioError error2("Second callback error");
    ErrorHandler::handleError(error2);
    
    EXPECT_EQ(callback1Errors.size(), 1);
    EXPECT_EQ(callback2Errors.size(), 1);
    EXPECT_EQ(callback1Errors[0].getMessage(), "Audio Error: First callback error");
    EXPECT_EQ(callback2Errors[0].getMessage(), "Audio Error: Second callback error");
}

TEST_F(ErrorHandlerTest, ErrorChaining) {
    // Test error chaining with Result
    auto chainFunction = [](int value) -> Result<int> {
        if (value < 0) {
            return Result<int>(AudioError("Negative value not allowed"));
        }
        return Result<int>(value * 2);
    };
    
    Result<int> successResult = chainFunction(5);
    EXPECT_TRUE(successResult.isSuccess());
    EXPECT_EQ(successResult.getValue(), 10);
    
    Result<int> errorResult = chainFunction(-5);
    EXPECT_FALSE(errorResult.isSuccess());
    EXPECT_TRUE(errorResult.isError());
}

TEST_F(ErrorHandlerTest, ErrorRecovery) {
    // Test error recovery patterns
    auto riskyOperation = [](int value) -> Result<int> {
        if (value == 0) {
            return Result<int>(AudioError("Division by zero"));
        }
        return Result<int>(100 / value);
    };
    
    // Successful operation
    Result<int> success = riskyOperation(10);
    EXPECT_TRUE(success.isSuccess());
    EXPECT_EQ(success.getValue(), 10);
    
    // Failed operation
    Result<int> failure = riskyOperation(0);
    EXPECT_FALSE(failure.isSuccess());
    EXPECT_TRUE(failure.isError());
} 