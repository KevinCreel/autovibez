#include <gtest/gtest.h>
#include "console_output.hpp"
#include <thread>
#include <sstream>
#include <iostream>

class ConsoleOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Capture cout output for testing
        original_cout = std::cout.rdbuf();
        test_output = new std::stringstream();
        std::cout.rdbuf(test_output->rdbuf());
        
        // Reset console output to default state
        ConsoleOutput::setColoredOutput(true);
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

TEST_F(ConsoleOutputTest, BasicOutput) {
    ConsoleOutput::output(ConsoleOutput::Type::INFO, "Test message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("Test message"), std::string::npos);
    
    EXPECT_EQ(output.find("[INFO]"), std::string::npos);
    EXPECT_EQ(output.find("[14:"), std::string::npos);
}

TEST_F(ConsoleOutputTest, FormatStringOutput) {
    ConsoleOutput::output(ConsoleOutput::Type::SUCCESS, "Formatted: %d + %d = %d", 1, 2, 3);
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Formatted: 1 + 2 = 3"), std::string::npos);
    EXPECT_EQ(output.find("[SUCCESS]"), std::string::npos);
}

TEST_F(ConsoleOutputTest, DifferentTypes) {
    ConsoleOutput::output(ConsoleOutput::Type::WARNING, "Warning message");
    ConsoleOutput::output(ConsoleOutput::Type::ERROR, "Error message");
    ConsoleOutput::output(ConsoleOutput::Type::PLAYBACK, "Playback message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("Warning message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
    EXPECT_NE(output.find("Playback message"), std::string::npos);
    
    EXPECT_EQ(output.find("[WARNING]"), std::string::npos);
    EXPECT_EQ(output.find("[ERROR]"), std::string::npos);
    EXPECT_EQ(output.find("[PLAYBACK]"), std::string::npos);
}

TEST_F(ConsoleOutputTest, SimplePrint) {
    ConsoleOutput::print("Simple message");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Simple message"), std::string::npos);
}

TEST_F(ConsoleOutputTest, FormatPrint) {
    ConsoleOutput::print("Formatted: %s", "test");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Formatted: test"), std::string::npos);
}

TEST_F(ConsoleOutputTest, ColoredOutput) {
    ConsoleOutput::setColoredOutput(true);
    ConsoleOutput::output(ConsoleOutput::Type::INFO, "Colored message");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("\033["), std::string::npos);
}

TEST_F(ConsoleOutputTest, NonColoredOutput) {
    ConsoleOutput::setColoredOutput(false);
    ConsoleOutput::output(ConsoleOutput::Type::INFO, "Non-colored message");
    
    std::string output = getOutput();
    EXPECT_EQ(output.find("\033["), std::string::npos);
}

TEST_F(ConsoleOutputTest, PresetConvenienceMethods) {
    ConsoleOutput::printNextPreset("Test Preset");
    ConsoleOutput::printPreviousPreset("Previous Preset");
    ConsoleOutput::printRandomPreset("Random Preset");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("⏭️  Next preset: Test Preset"), std::string::npos);
    EXPECT_NE(output.find("⏮️  Previous preset: Previous Preset"), std::string::npos);
    EXPECT_NE(output.find("🎨 Loaded random preset: Random Preset"), std::string::npos);
}

TEST_F(ConsoleOutputTest, PauseResumeMethods) {
    ConsoleOutput::printPause();
    ConsoleOutput::printResume();
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("⏸️  Preset paused"), std::string::npos);
    EXPECT_NE(output.find("▶️  Preset resumed"), std::string::npos);
}

TEST_F(ConsoleOutputTest, ThreadSafety) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            ConsoleOutput::output(ConsoleOutput::Type::INFO, "Thread %d message", i);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::string output = getOutput();
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_NE(output.find("Thread " + std::to_string(i) + " message"), std::string::npos);
    }
}

TEST_F(ConsoleOutputTest, EmptyMessage) {
    EXPECT_NO_THROW(ConsoleOutput::output(ConsoleOutput::Type::INFO, ""));
    
    std::string output = getOutput();
    EXPECT_EQ(output.find("[INFO]"), std::string::npos);
    EXPECT_EQ(output.find("["), std::string::npos);
}

TEST_F(ConsoleOutputTest, NullFormatString) {
    // Should not crash with null format string
    EXPECT_NO_THROW(ConsoleOutput::output(ConsoleOutput::Type::INFO, nullptr));
    EXPECT_NO_THROW(ConsoleOutput::print(nullptr));
} 