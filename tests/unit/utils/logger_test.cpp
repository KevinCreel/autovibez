#include "utils/logger.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

using namespace AutoVibez::Utils;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing test log files
        testLogFile_ = "/tmp/autovibez_logger_test.log";
        if (std::filesystem::exists(testLogFile_)) {
            std::filesystem::remove(testLogFile_);
        }
    }

    void TearDown() override {
        // Clean up test log file
        if (std::filesystem::exists(testLogFile_)) {
            std::filesystem::remove(testLogFile_);
        }
    }

    std::string readLogFile() const {
        if (!std::filesystem::exists(testLogFile_)) {
            return "";
        }

        std::ifstream file(testLogFile_);
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        return content;
    }

    std::string testLogFile_;
};

TEST_F(LoggerTest, InitialState) {
    Logger logger(false);  // Disable file logging for tests

    EXPECT_TRUE(logger.isSuccess());
    EXPECT_TRUE(logger.getLastError().empty());
}

TEST_F(LoggerTest, ErrorHandling) {
    Logger logger(false);  // Disable file logging for tests

    const std::string errorMessage = "Test error message";
    logger.setError(errorMessage);

    EXPECT_FALSE(logger.isSuccess());
    EXPECT_EQ(logger.getLastError(), errorMessage);
}

TEST_F(LoggerTest, ClearError) {
    Logger logger(false);  // Disable file logging for tests

    logger.setError("Test error");
    EXPECT_FALSE(logger.isSuccess());

    logger.clearError();
    EXPECT_TRUE(logger.isSuccess());
    EXPECT_TRUE(logger.getLastError().empty());
}

// Note: setSuccess is protected in ErrorHandler and not part of public Logger API

TEST_F(LoggerTest, LogLevels) {
    Logger logger(false);  // Disable file logging for tests

    // Test that all log levels can be called without crashing
    logger.logInfo("Info message");
    logger.logDebug("Debug message");
    logger.logWarning("Warning message");
    logger.logError("Error message");

    // Error should affect state
    EXPECT_FALSE(logger.isSuccess());
    EXPECT_EQ(logger.getLastError(), "Error message");
}

TEST_F(LoggerTest, MultipleErrors) {
    Logger logger(false);  // Disable file logging for tests

    logger.logError("First error");
    EXPECT_EQ(logger.getLastError(), "First error");

    logger.logError("Second error");
    EXPECT_EQ(logger.getLastError(), "Second error");

    EXPECT_FALSE(logger.isSuccess());
}

TEST_F(LoggerTest, ThreadSafety) {
    Logger logger(false);  // Disable file logging for tests
    const int numThreads = 10;
    const int messagesPerThread = 100;

    std::vector<std::thread> threads;

    // Launch multiple threads that log concurrently
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                logger.logInfo("Thread " + std::to_string(i) + " message " + std::to_string(j));
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Logger should still be in a valid state
    EXPECT_TRUE(logger.isSuccess());
}

TEST_F(LoggerTest, ErrorStateThreadSafety) {
    Logger logger(false);  // Disable file logging for tests
    std::atomic<int> errorCount{0};

    const int numThreads = 5;
    std::vector<std::thread> threads;

    // Launch threads that set errors concurrently
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, &errorCount, i]() {
            logger.setError("Error from thread " + std::to_string(i));
            errorCount++;
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Should be in error state
    EXPECT_FALSE(logger.isSuccess());
    EXPECT_FALSE(logger.getLastError().empty());
    EXPECT_EQ(errorCount.load(), numThreads);
}

TEST_F(LoggerTest, EmptyMessages) {
    Logger logger(false);  // Disable file logging for tests

    // Test empty messages don't crash
    logger.logInfo("");
    logger.logDebug("");
    logger.logWarning("");
    logger.setError("");

    EXPECT_FALSE(logger.isSuccess());
    EXPECT_TRUE(logger.getLastError().empty());
}

TEST_F(LoggerTest, LongMessages) {
    Logger logger(false);  // Disable file logging for tests

    // Test very long messages
    std::string longMessage(10000, 'A');
    logger.setError(longMessage);

    EXPECT_FALSE(logger.isSuccess());
    EXPECT_EQ(logger.getLastError(), longMessage);
}

TEST_F(LoggerTest, SpecialCharacters) {
    Logger logger(false);  // Disable file logging for tests

    const std::string specialMessage = "Error with special chars: \n\t\r\"'";
    logger.setError(specialMessage);

    EXPECT_FALSE(logger.isSuccess());
    EXPECT_EQ(logger.getLastError(), specialMessage);
}

TEST_F(LoggerTest, RapidLogging) {
    Logger logger(false);  // Disable file logging for tests

    // Test rapid consecutive logging
    for (int i = 0; i < 1000; ++i) {
        logger.logInfo("Rapid message " + std::to_string(i));
    }

    EXPECT_TRUE(logger.isSuccess());
}
