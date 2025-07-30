#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include "logger.hpp"
#include "fixtures/test_fixtures.hpp"

using namespace AutoVibez::Utils;
using Logger = AutoVibez::Utils::Logger;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
        log_file_path = test_dir + "/test.log";
        
        // Reset logger to default state
        Logger::getInstance().setLogLevel(Logger::Level::INFO);
        Logger::getInstance().setOutputTarget(Logger::OutputTarget::CONSOLE);
        Logger::getInstance().setColoredOutput(false);
        Logger::getInstance().setLogFilePath("");
        Logger::getInstance().clearErrorHistory();
    }
    
    void TearDown() override {
        Logger::getInstance().close();
        TestFixtures::cleanupTestFiles({test_dir});
    }
    
    std::string test_dir;
    std::string log_file_path;
};

TEST_F(LoggerTest, ConstructorAndSingleton) {
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();
    
    EXPECT_EQ(&logger1, &logger2);
}

TEST_F(LoggerTest, LogLevelConfiguration) {
    Logger& logger = Logger::getInstance();
    
    logger.setLogLevel(Logger::Level::DEBUG);
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::DEBUG);
    
    logger.setLogLevel(Logger::Level::ERROR);
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::ERROR);
}

TEST_F(LoggerTest, OutputTargetConfiguration) {
    Logger& logger = Logger::getInstance();
    
    logger.setOutputTarget(Logger::OutputTarget::FILE);
    EXPECT_EQ(logger.getOutputTarget(), Logger::OutputTarget::FILE);
    
    logger.setOutputTarget(Logger::OutputTarget::BOTH);
    EXPECT_EQ(logger.getOutputTarget(), Logger::OutputTarget::BOTH);
}

TEST_F(LoggerTest, LogFilePathConfiguration) {
    Logger& logger = Logger::getInstance();
    
    logger.setLogFilePath(log_file_path);
    EXPECT_EQ(logger.getLogFilePath(), log_file_path);
    
    // Check that file was created
    EXPECT_TRUE(std::filesystem::exists(log_file_path));
}

TEST_F(LoggerTest, FileSizeConfiguration) {
    Logger& logger = Logger::getInstance();
    
    logger.setMaxFileSize(1024);
    EXPECT_EQ(logger.getMaxFileSize(), 1024);
    
    logger.setMaxFiles(10);
    EXPECT_EQ(logger.getMaxFiles(), 10);
}

TEST_F(LoggerTest, ColoredOutputConfiguration) {
    Logger& logger = Logger::getInstance();
    
    logger.setColoredOutput(false);
    EXPECT_FALSE(logger.isColoredOutputEnabled());
    
    logger.setColoredOutput(true);
    EXPECT_TRUE(logger.isColoredOutputEnabled());
}

TEST_F(LoggerTest, BasicLogging) {
    Logger& logger = Logger::getInstance();
    
    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.info("Test message");
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    EXPECT_THAT(output, ::testing::HasSubstr("Test message"));
    EXPECT_THAT(output, ::testing::HasSubstr("INFO"));
}

TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::WARNING);
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    EXPECT_THAT(output, ::testing::Not(::testing::HasSubstr("Debug message")));
    EXPECT_THAT(output, ::testing::Not(::testing::HasSubstr("Info message")));
    EXPECT_THAT(output, ::testing::HasSubstr("Warning message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Error message"));
}

TEST_F(LoggerTest, FileLogging) {
    Logger& logger = Logger::getInstance();
    logger.setOutputTarget(Logger::OutputTarget::FILE);
    logger.setLogFilePath(log_file_path);
    
    logger.info("File test message");
    logger.flush();
    
    std::ifstream log_file(log_file_path);
    std::string content((std::istreambuf_iterator<char>(log_file)),
                        std::istreambuf_iterator<char>());
    
    EXPECT_THAT(content, ::testing::HasSubstr("File test message"));
}

TEST_F(LoggerTest, BothConsoleAndFileLogging) {
    Logger& logger = Logger::getInstance();
    logger.setOutputTarget(Logger::OutputTarget::BOTH);
    logger.setLogFilePath(log_file_path);
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.info("Both test message");
    logger.flush();
    
    std::cout.rdbuf(old_cout);
    std::string console_output = buffer.str();
    
    std::ifstream log_file(log_file_path);
    std::string file_content((std::istreambuf_iterator<char>(log_file)),
                            std::istreambuf_iterator<char>());
    
    EXPECT_THAT(console_output, ::testing::HasSubstr("Both test message"));
    EXPECT_THAT(file_content, ::testing::HasSubstr("Both test message"));
}

TEST_F(LoggerTest, StructuredLoggingWithContext) {
    Logger& logger = Logger::getInstance();
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    Logger::LogContext context("TestComponent", "testFunction", "test.cpp", 42);
    logger.info("Context test message", context);
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    EXPECT_THAT(output, ::testing::HasSubstr("TestComponent::testFunction"));
    EXPECT_THAT(output, ::testing::HasSubstr("test.cpp:42"));
}

TEST_F(LoggerTest, ErrorTracking) {
    Logger& logger = Logger::getInstance();
    
    Logger::LogContext context("TestComponent", "testFunction");
    std::string error_id = logger.logError("TestError", "Test error message", context);
    
    EXPECT_FALSE(error_id.empty());
    EXPECT_THAT(error_id, ::testing::StartsWith("ERR_"));
    
    Logger::ErrorInfo error_info = logger.getErrorInfo(error_id);
    EXPECT_EQ(error_info.error_id, error_id);
    EXPECT_EQ(error_info.error_type, "TestError");
    EXPECT_EQ(error_info.message, "Test error message");
    EXPECT_EQ(error_info.context.component, "TestComponent");
    EXPECT_EQ(error_info.context.function, "testFunction");
}

TEST_F(LoggerTest, ErrorTrackingWithMetadata) {
    Logger& logger = Logger::getInstance();
    
    std::unordered_map<std::string, std::string> metadata = {
        {"user_id", "12345"},
        {"session_id", "abc123"},
        {"operation", "download"}
    };
    
    Logger::LogContext context("TestComponent", "testFunction");
    std::string error_id = logger.logErrorWithMetadata("TestError", "Test error message", 
                                                      metadata, context);
    
    Logger::ErrorInfo error_info = logger.getErrorInfo(error_id);
    EXPECT_EQ(error_info.metadata["user_id"], "12345");
    EXPECT_EQ(error_info.metadata["session_id"], "abc123");
    EXPECT_EQ(error_info.metadata["operation"], "download");
}

TEST_F(LoggerTest, RecentErrors) {
    Logger& logger = Logger::getInstance();
    
    // Log multiple errors
    for (int i = 0; i < 5; ++i) {
        logger.logError("TestError", "Error " + std::to_string(i));
    }
    
    std::vector<Logger::ErrorInfo> recent_errors = logger.getRecentErrors(3);
    EXPECT_EQ(recent_errors.size(), 3);
    
    // Check that they're sorted by timestamp (most recent first)
    for (size_t i = 1; i < recent_errors.size(); ++i) {
        EXPECT_GE(recent_errors[i-1].timestamp, recent_errors[i].timestamp);
    }
}

TEST_F(LoggerTest, PerformanceMonitoring) {
    Logger& logger = Logger::getInstance();
    
    logger.startTimer("test_timer");
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    double duration = logger.endTimer("test_timer");
    EXPECT_GT(duration, 0.0);
    
    // Test invalid timer
    double invalid_duration = logger.endTimer("nonexistent_timer");
    EXPECT_EQ(invalid_duration, -1.0);
}

TEST_F(LoggerTest, PerformanceLogging) {
    Logger& logger = Logger::getInstance();
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.logPerformance("test_operation", 123.45);
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    EXPECT_THAT(output, ::testing::HasSubstr("test_operation"));
    EXPECT_THAT(output, ::testing::HasSubstr("123.45ms"));
}

TEST_F(LoggerTest, Statistics) {
    Logger& logger = Logger::getInstance();
    
    // Log some messages
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    
    EXPECT_EQ(logger.getLogCount(Logger::Level::DEBUG), 1);
    EXPECT_EQ(logger.getLogCount(Logger::Level::INFO), 1);
    EXPECT_EQ(logger.getLogCount(Logger::Level::WARNING), 1);
    EXPECT_EQ(logger.getLogCount(Logger::Level::ERROR), 1);
    
    EXPECT_GT(logger.getAverageLogTime(), 0.0);
}

TEST_F(LoggerTest, ErrorCount) {
    Logger& logger = Logger::getInstance();
    
    EXPECT_EQ(logger.getErrorCount(), 0);
    
    logger.logError("TestError", "Test message");
    EXPECT_EQ(logger.getErrorCount(), 1);
    
    logger.logError("TestError2", "Test message 2");
    EXPECT_EQ(logger.getErrorCount(), 2);
    
    logger.clearErrorHistory();
    EXPECT_EQ(logger.getErrorCount(), 0);
}

TEST_F(LoggerTest, FileRotation) {
    Logger& logger = Logger::getInstance();
    logger.setOutputTarget(Logger::OutputTarget::FILE);
    logger.setLogFilePath(log_file_path);
    logger.setMaxFileSize(100); // Small size for testing
    logger.setMaxFiles(3);
    
    // Write enough data to trigger rotation
    for (int i = 0; i < 10; ++i) {
        logger.info("This is a test message that should trigger file rotation when enough data is written");
    }
    logger.flush();
    
    // Check that rotation files were created
    std::filesystem::path log_path(log_file_path);
    std::string backup_file = log_path.parent_path().string() + "/" + 
                             log_path.stem().string() + ".1" + log_path.extension().string();
    
    EXPECT_TRUE(std::filesystem::exists(backup_file));
}

TEST_F(LoggerTest, ThreadSafety) {
    Logger& logger = Logger::getInstance();
    logger.setOutputTarget(Logger::OutputTarget::FILE);
    logger.setLogFilePath(log_file_path);
    
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int messages_per_thread = 100;
    
    // Start multiple threads logging simultaneously
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.info("Thread " + std::to_string(i) + " message " + std::to_string(j));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    logger.flush();
    
    // Verify all messages were logged
    std::ifstream log_file(log_file_path);
    std::string content((std::istreambuf_iterator<char>(log_file)),
                        std::istreambuf_iterator<char>());
    
    // Count total expected messages
    int expected_messages = num_threads * messages_per_thread;
    int actual_messages = 0;
    std::string::size_type pos = 0;
    while ((pos = content.find("Thread ", pos)) != std::string::npos) {
        actual_messages++;
        pos++;
    }
    
    EXPECT_EQ(actual_messages, expected_messages);
}

TEST_F(LoggerTest, LogLevels) {
    Logger& logger = Logger::getInstance();
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.setLogLevel(Logger::Level::TRACE);
    
    logger.trace("Trace message");
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    logger.fatal("Fatal message");
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    EXPECT_THAT(output, ::testing::HasSubstr("Trace message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Debug message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Info message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Warning message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Error message"));
    EXPECT_THAT(output, ::testing::HasSubstr("Fatal message"));
}

TEST_F(LoggerTest, TimestampFormat) {
    Logger& logger = Logger::getInstance();
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.info("Timestamp test");
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    // Check for timestamp format: YYYY-MM-DD HH:MM:SS.mmm
    EXPECT_THAT(output, ::testing::MatchesRegex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]"));
}

TEST_F(LoggerTest, ThreadIdInLogs) {
    Logger& logger = Logger::getInstance();
    
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    logger.info("Thread ID test");
    
    std::cout.rdbuf(old_cout);
    std::string output = buffer.str();
    
    // Check for thread ID format
    EXPECT_THAT(output, ::testing::MatchesRegex("\\[\\d+\\]"));
} 