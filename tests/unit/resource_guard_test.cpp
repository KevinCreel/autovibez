#include <gtest/gtest.h>
#include "resource_guard.hpp"
#include <atomic>

class ResourceGuardTest : public ::testing::Test {
protected:
    void SetUp() override {
        cleanup_called = false;
        cleanup_count = 0;
    }
    
    std::atomic<bool> cleanup_called{false};
    std::atomic<int> cleanup_count{0};
};

TEST_F(ResourceGuardTest, BasicCleanup) {
    {
        ResourceGuard guard([this]() {
            cleanup_called = true;
            cleanup_count++;
        });
        
        EXPECT_FALSE(cleanup_called);
        EXPECT_EQ(cleanup_count, 0);
    }
    
    // After scope ends, cleanup should be called
    EXPECT_TRUE(cleanup_called);
    EXPECT_EQ(cleanup_count, 1);
}

TEST_F(ResourceGuardTest, MultipleGuards) {
    {
        ResourceGuard guard1([this]() { cleanup_count++; });
        ResourceGuard guard2([this]() { cleanup_count++; });
        ResourceGuard guard3([this]() { cleanup_count++; });
        
        EXPECT_EQ(cleanup_count, 0);
    }
    
    // All three guards should have called cleanup
    EXPECT_EQ(cleanup_count, 3);
}

TEST_F(ResourceGuardTest, EarlyRelease) {
    {
        ResourceGuard guard([this]() { cleanup_count++; });
        
        EXPECT_EQ(cleanup_count, 0);
        
        // Release early
        guard.release();
        EXPECT_EQ(cleanup_count, 1);
        
        // Should not call cleanup again when going out of scope
    }
    
    EXPECT_EQ(cleanup_count, 1);
}

TEST_F(ResourceGuardTest, MoveConstructor) {
    {
        ResourceGuard guard1([this]() { cleanup_count++; });
        
        // Move to new guard
        ResourceGuard guard2 = std::move(guard1);
        
        EXPECT_EQ(cleanup_count, 0);
    }
    
    // Only one cleanup should be called
    EXPECT_EQ(cleanup_count, 1);
}

TEST_F(ResourceGuardTest, MoveAssignment) {
    {
        ResourceGuard guard1([this]() { cleanup_count++; });
        ResourceGuard guard2([this]() { cleanup_count++; });
        
        // Move assign - guard1's cleanup should be called
        guard2 = std::move(guard1);
        
        EXPECT_EQ(cleanup_count, 1); // guard1's cleanup should be called
    }
    
    // guard2's cleanup should be called when it goes out of scope
    EXPECT_EQ(cleanup_count, 2);
}

TEST_F(ResourceGuardTest, EmptyCleanupFunction) {
    {
        ResourceGuard guard([]() {
            // Empty cleanup function
        });
        
        // Should not crash
    }
    
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ResourceGuardTest, ExceptionInCleanup) {
    {
        ResourceGuard guard([]() {
            throw std::runtime_error("Cleanup exception");
        });
        
        // Should not crash when cleanup throws
    }
    
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ResourceGuardTest, MultipleReleases) {
    {
        ResourceGuard guard([this]() { cleanup_count++; });
        
        guard.release();
        EXPECT_EQ(cleanup_count, 1);
        
        // Second release should be safe
        guard.release();
        EXPECT_EQ(cleanup_count, 1); // Should not call cleanup again
    }
    
    EXPECT_EQ(cleanup_count, 1);
}

TEST_F(ResourceGuardTest, IsActive) {
    {
        ResourceGuard guard([this]() { cleanup_count++; });
        
        EXPECT_TRUE(guard.isActive());
        
        guard.release();
        EXPECT_FALSE(guard.isActive());
    }
    
    EXPECT_EQ(cleanup_count, 1);
}

TEST_F(ResourceGuardTest, NestedScopes) {
    {
        ResourceGuard outer([this]() { cleanup_count++; });
        
        {
            ResourceGuard inner([this]() { cleanup_count++; });
            EXPECT_EQ(cleanup_count, 0);
        }
        
        // Inner should be cleaned up
        EXPECT_EQ(cleanup_count, 1);
    }
    
    // Outer should be cleaned up
    EXPECT_EQ(cleanup_count, 2);
}

TEST_F(ResourceGuardTest, LambdaCapture) {
    int local_var = 42;
    
    {
        ResourceGuard guard([&local_var]() {
            local_var = 100; // Modify captured variable
        });
        
        EXPECT_EQ(local_var, 42);
    }
    
    // Cleanup should have modified the variable
    EXPECT_EQ(local_var, 100);
}

TEST_F(ResourceGuardTest, ComplexCleanup) {
    std::vector<int> vec = {1, 2, 3};
    
    {
        ResourceGuard guard([&vec]() {
            vec.clear();
            vec.push_back(999);
        });
        
        EXPECT_EQ(vec.size(), 3);
    }
    
    // Cleanup should have modified the vector
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 999);
} 