#include "utils/error_handler.hpp"

#include <gtest/gtest.h>

// Test class that inherits from ErrorHandler
class TestErrorHandler : public AutoVibez::Utils::ErrorHandler {
public:
    void simulateSuccess() {
        setSuccess(true);
    }

    void simulateError(const std::string& error) {
        setError(error);
    }

    void simulateFailure() {
        setSuccess(false);
    }
};

TEST(ErrorHandlerTest, InitialState) {
    TestErrorHandler handler;

    // Should start in success state
    EXPECT_TRUE(handler.isSuccess());
    EXPECT_TRUE(handler.getLastError().empty());
}

TEST(ErrorHandlerTest, SetAndGetError) {
    TestErrorHandler handler;

    // Set an error
    handler.simulateError("Test error message");

    // Should reflect error state
    EXPECT_FALSE(handler.isSuccess());
    EXPECT_EQ(handler.getLastError(), "Test error message");
}

TEST(ErrorHandlerTest, ClearError) {
    TestErrorHandler handler;

    // Set an error
    handler.simulateError("Test error message");
    EXPECT_FALSE(handler.isSuccess());

    // Clear the error
    handler.clearError();

    // Should be back to success state
    EXPECT_TRUE(handler.isSuccess());
    EXPECT_TRUE(handler.getLastError().empty());
}

TEST(ErrorHandlerTest, SetSuccess) {
    TestErrorHandler handler;

    // Set an error first
    handler.simulateError("Test error message");
    EXPECT_FALSE(handler.isSuccess());

    // Set success
    handler.simulateSuccess();

    // Should be in success state
    EXPECT_TRUE(handler.isSuccess());
    EXPECT_TRUE(handler.getLastError().empty());
}

TEST(ErrorHandlerTest, EdgeCases) {
    TestErrorHandler handler;

    // Test empty error message
    handler.simulateError("");
    EXPECT_FALSE(handler.isSuccess());
    EXPECT_TRUE(handler.getLastError().empty());

    // Test very long error message
    std::string longError(1000, 'A');
    handler.simulateError(longError);
    EXPECT_FALSE(handler.isSuccess());
    EXPECT_EQ(handler.getLastError(), longError);

    // Test special characters in error message
    handler.simulateError("Error with special chars: \n\t\r\"'");
    EXPECT_FALSE(handler.isSuccess());
    EXPECT_EQ(handler.getLastError(), "Error with special chars: \n\t\r\"'");

    // Test multiple error state changes
    handler.simulateError("First error");
    handler.simulateError("Second error");
    EXPECT_FALSE(handler.isSuccess());
    EXPECT_EQ(handler.getLastError(), "Second error");
}
