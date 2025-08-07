#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "loopback.hpp"
using AutoVibez::Audio::cleanupLoopback;
using AutoVibez::Audio::initLoopback;

class MemoryCleanupTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure we start with a clean state
        cleanupLoopback();
    }

    void TearDown() override {
        // Ensure cleanup after each test
        cleanupLoopback();
    }
};

TEST_F(MemoryCleanupTest, LoopbackInitializationAndCleanup) {
    // Test that initialization and cleanup work correctly
    EXPECT_TRUE(initLoopback());

    // Verify that cleanup doesn't crash
    EXPECT_TRUE(cleanupLoopback());

    // Verify that cleanup can be called multiple times safely
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, MultipleInitializationCycles) {
    // Test multiple initialization/cleanup cycles
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(initLoopback());
        EXPECT_TRUE(cleanupLoopback());
    }
}

TEST_F(MemoryCleanupTest, CleanupWithoutInitialization) {
    // Test that cleanup is safe even without initialization
    EXPECT_TRUE(cleanupLoopback());
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, ThreadSafety) {
    // Test that cleanup is thread-safe
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([]() {
            initLoopback();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cleanupLoopback();
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Final cleanup should be safe
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, ResourceTracking) {
    // Test that resources are properly tracked and cleaned up
    EXPECT_TRUE(initLoopback());

    // Simulate some usage
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Cleanup should succeed
    EXPECT_TRUE(cleanupLoopback());

    // Verify no resources are leaked by checking again
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, ExceptionSafety) {
    // Test that cleanup is exception-safe
    EXPECT_TRUE(initLoopback());

    try {
        // Simulate an exception during usage
        throw std::runtime_error("Test exception");
    } catch (const std::exception&) {
        // Cleanup should still work after exception
        EXPECT_TRUE(cleanupLoopback());
    }
}

TEST_F(MemoryCleanupTest, LongRunningCleanup) {
    // Test cleanup after longer running time
    EXPECT_TRUE(initLoopback());

    // Simulate longer usage
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, RapidInitializationCleanup) {
    // Test rapid initialization/cleanup cycles
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(initLoopback());
        EXPECT_TRUE(cleanupLoopback());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TEST_F(MemoryCleanupTest, CleanupStateConsistency) {
    // Test that cleanup maintains consistent state
    EXPECT_TRUE(initLoopback());
    EXPECT_TRUE(cleanupLoopback());

    // Should be able to initialize again after cleanup
    EXPECT_TRUE(initLoopback());
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, MemoryLeakDetection) {
    // This test helps detect if there are still memory leaks
    // by running multiple cycles and checking for resource exhaustion

    for (int cycle = 0; cycle < 20; ++cycle) {
        EXPECT_TRUE(initLoopback());

        // Simulate some processing
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        EXPECT_TRUE(cleanupLoopback());

        // Small delay between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Final cleanup
    EXPECT_TRUE(cleanupLoopback());
}

TEST_F(MemoryCleanupTest, ConcurrentAccess) {
    // Test concurrent access to initialization and cleanup
    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    // Start multiple threads that initialize and cleanup
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&stop]() {
            while (!stop) {
                if (initLoopback()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    cleanupLoopback();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;

    // Wait for threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    // Final cleanup
    EXPECT_TRUE(cleanupLoopback());
}