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
    ConsoleOutput::output("Test message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("Test message"), std::string::npos);
    
    EXPECT_EQ(output.find("[INFO]"), std::string::npos);
    EXPECT_EQ(output.find("[14:"), std::string::npos);
}

TEST_F(ConsoleOutputTest, FormatStringOutput) {
    ConsoleOutput::output("Formatted: %d + %d = %d", 1, 2, 3);
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Formatted: 1 + 2 = 3"), std::string::npos);
    EXPECT_EQ(output.find("[SUCCESS]"), std::string::npos);
}

TEST_F(ConsoleOutputTest, DifferentTypes) {
    ConsoleOutput::output("Warning message");
    ConsoleOutput::output("Error message");
    ConsoleOutput::output("Playback message");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("Warning message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
    EXPECT_NE(output.find("Playback message"), std::string::npos);
    
    EXPECT_EQ(output.find("[WARNING]"), std::string::npos);
    EXPECT_EQ(output.find("[ERROR]"), std::string::npos);
    EXPECT_EQ(output.find("[PLAYBACK]"), std::string::npos);
}



TEST_F(ConsoleOutputTest, ColoredOutput) {
    ConsoleOutput::output("Colored message");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Colored message"), std::string::npos);
}

TEST_F(ConsoleOutputTest, NonColoredOutput) {
    ConsoleOutput::output("Non-colored message");
    
    std::string output = getOutput();
    EXPECT_NE(output.find("Non-colored message"), std::string::npos);
}

TEST_F(ConsoleOutputTest, PresetConvenienceMethods) {
    ConsoleOutput::output("⏭️  Next preset: %s", "Test Preset");
    ConsoleOutput::output("⏮️  Previous preset: %s", "Previous Preset");
    ConsoleOutput::output("🎨 Loaded random preset: %s", "Random Preset");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("⏭️  Next preset: Test Preset"), std::string::npos);
    EXPECT_NE(output.find("⏮️  Previous preset: Previous Preset"), std::string::npos);
    EXPECT_NE(output.find("🎨 Loaded random preset: Random Preset"), std::string::npos);
}

TEST_F(ConsoleOutputTest, PauseResumeMethods) {
    ConsoleOutput::output("⏸️  Preset paused");
    ConsoleOutput::output("▶️  Preset resumed");
    
    std::string output = getOutput();
    
    EXPECT_NE(output.find("⏸️  Preset paused"), std::string::npos);
    EXPECT_NE(output.find("▶️  Preset resumed"), std::string::npos);
}

TEST_F(ConsoleOutputTest, ThreadSafety) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            ConsoleOutput::output("Thread %d message", i);
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
    EXPECT_NO_THROW(ConsoleOutput::output(""));
    
    std::string output = getOutput();
    EXPECT_EQ(output.find("[INFO]"), std::string::npos);
    EXPECT_EQ(output.find("["), std::string::npos);
}

TEST_F(ConsoleOutputTest, NullFormatString) {
    // Should not crash with null format string
    EXPECT_NO_THROW(ConsoleOutput::output(nullptr));
} 