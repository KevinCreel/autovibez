#include <gtest/gtest.h>
#include "logger.hpp"
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Capture cout output for testing
        original_cout = std::cout.rdbuf();
        test_output = new std::stringstream();
        std::cout.rdbuf(test_output->rdbuf());
        
        // Reset logger to default state
        Logger::getInstance().setLogLevel(Logger::Level::INFO);
        Logger::getInstance().setColoredOutput(true);
    }
    
    void TearDown() override {
        // Restore cout
        std::cout.rdbuf(original_cout);
        delete test_output;
    }
    
    std::string getOutput() {
        std::string output = test_output->str();
        test_output->str(""); // Clear for next test
        return output;
    }
    
    std::streambuf* original_cout;
    std::stringstream* test_output;
};

TEST_F(LoggerTest, SingletonPattern) {
    Logger& instance1 = Logger::getInstance();
    Logger& instance2 = Logger::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(LoggerTest, LogLevelSetting) {
    Logger& logger = Logger::getInstance();
    
    // Test default level
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::INFO);
    
    // Test setting different levels
    logger.setLogLevel(Logger::Level::DEBUG);
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::DEBUG);
    
    logger.setLogLevel(Logger::Level::WARNING);
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::WARNING);
    
    logger.setLogLevel(Logger::Level::ERROR);
    EXPECT_EQ(logger.getLogLevel(), Logger::Level::ERROR);
}

TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();
    
    // Set to WARNING level - should filter out DEBUG and INFO
    logger.setLogLevel(Logger::Level::WARNING);
    
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    
    std::string output = getOutput();
    
    EXPECT_EQ(output.find("Debug message"), std::string::npos);
    EXPECT_EQ(output.find("Info message"), std::string::npos);
    
    EXPECT_NE(output.find("Warning message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
}

TEST_F(LoggerTest, ColoredOutput) {
    Logger& logger = Logger::getInstance();
    
    // Test colored output
    logger.setColoredOutput(true);
    logger.info("Colored message");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("\033["), std::string::npos);
    
    // Test non-colored output
    logger.setColoredOutput(false);
    logger.info("Non-colored message");
    
    output = getOutput();
    EXPECT_EQ(output.find("\033["), std::string::npos);
}

TEST_F(LoggerTest, MessageFormatting) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::DEBUG);
    
    logger.info("Test message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("["), std::string::npos);
    EXPECT_NE(output.find("]"), std::string::npos);
    
    EXPECT_NE(output.find("[INFO]"), std::string::npos);
    
    EXPECT_NE(output.find("Test message"), std::string::npos);
}

TEST_F(LoggerTest, FormatStringLogging) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::INFO);
    
    EXPECT_NO_THROW(logger.info("Formatted message: %d + %d = %d", 1, 2, 3));
    
    SUCCEED();
}

TEST_F(LoggerTest, ThreadSafety) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::INFO);
    
    const int num_threads = 10;
    const int messages_per_thread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.info("Thread %d message %d", i, j);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::string output = getOutput();
    
    int message_count = 0;
    size_t pos = 0;
    while ((pos = output.find("Thread ", pos)) != std::string::npos) {
        message_count++;
        pos++;
    }
    
    EXPECT_EQ(message_count, num_threads * messages_per_thread);
}

TEST_F(LoggerTest, DifferentLogLevels) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::DEBUG);
    
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("Debug message"), std::string::npos);
    EXPECT_NE(output.find("Info message"), std::string::npos);
    EXPECT_NE(output.find("Warning message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
}

TEST_F(LoggerTest, EmptyMessage) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::INFO);
    
    EXPECT_NO_THROW(logger.info(""));
    
    SUCCEED();
}

TEST_F(LoggerTest, SpecialCharacters) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::INFO);
    
    logger.info("Message with special chars: !@#$%^&*()");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Message with special chars: !@#$%^&*()"), std::string::npos);
}

TEST_F(LoggerTest, LongMessage) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(Logger::Level::INFO);
    
    std::string long_message(1000, 'x');
    EXPECT_NO_THROW(logger.info("Long message: %s", long_message.c_str()));
    
    SUCCEED();
} 