#include "mix_manager.hpp"
#include "mix_database.hpp"
#include "mix_downloader.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"
#include "mp3_analyzer.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <memory>

using namespace AutoVibez::Data;

class MixManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directories
        temp_dir = std::filesystem::temp_directory_path() / "autovibez_test";
        db_path = temp_dir / "test_mixes.db";
        data_dir = temp_dir / "mixes";
        
        std::filesystem::create_directories(temp_dir);
        std::filesystem::create_directories(data_dir);
    }
    
    void TearDown() override {
        // Clean up temporary files
        std::filesystem::remove_all(temp_dir);
    }
    
    std::filesystem::path temp_dir;
    std::filesystem::path db_path;
    std::filesystem::path data_dir;
};

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_AllFilesExist) {
    // Create test database with mix entries
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());
    
    // Create test files
    std::vector<std::string> test_files = {
        (data_dir / "test1.mp3").string(),
        (data_dir / "test2.mp3").string(),
        (data_dir / "test3.mp3").string()
    };
    
    // Create dummy MP3 files
    for (const auto& file_path : test_files) {
        std::ofstream file(file_path, std::ios::binary);
        file.write("\xFF\xFB\x90\x00", 4); // MP3 header
        file.write(std::string(1000, '\x00').c_str(), 1000); // Dummy data
        file.close();
    }
    
    // Add mixes to database
    Mix mix1, mix2, mix3;
    mix1.id = "test1"; mix1.title = "Test Mix 1"; mix1.artist = "Artist 1"; 
    mix1.genre = "Test"; mix1.url = "http://example.com/1"; 
    mix1.local_path = test_files[0]; mix1.duration_seconds = 180;
    
    mix2.id = "test2"; mix2.title = "Test Mix 2"; mix2.artist = "Artist 2"; 
    mix2.genre = "Test"; mix2.url = "http://example.com/2"; 
    mix2.local_path = test_files[1]; mix2.duration_seconds = 240;
    
    mix3.id = "test3"; mix3.title = "Test Mix 3"; mix3.artist = "Artist 3"; 
    mix3.genre = "Test"; mix3.url = "http://example.com/3"; 
    mix3.local_path = test_files[2]; mix3.duration_seconds = 300;
    
    ASSERT_TRUE(db.addMix(mix1));
    ASSERT_TRUE(db.addMix(mix2));
    ASSERT_TRUE(db.addMix(mix3));
    
    // Test validation
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_SomeFilesMissing) {
    // Create test files (only 2 out of 3)
    std::vector<std::string> test_files = {
        (data_dir / "test1.mp3").string(),
        (data_dir / "test2.mp3").string(),
        (data_dir / "test3.mp3").string()
    };
    
    // Create dummy MP3 files (only first two)
    for (int i = 0; i < 2; ++i) {
        std::ofstream file(test_files[i], std::ios::binary);
        file.write("\xFF\xFB\x90\x00", 4); // MP3 header
        file.write(std::string(1000, '\x00').c_str(), 1000); // Dummy data
        file.close();
    }
    
    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    
    // Add mixes to database through the manager
    Mix mix1, mix2, mix3;
    mix1.id = "test1"; mix1.title = "Test Mix 1"; mix1.artist = "Artist 1"; 
    mix1.genre = "Test"; mix1.url = "http://example.com/1"; 
    mix1.local_path = test_files[0]; mix1.duration_seconds = 180;
    
    mix2.id = "test2"; mix2.title = "Test Mix 2"; mix2.artist = "Artist 2"; 
    mix2.genre = "Test"; mix2.url = "http://example.com/2"; 
    mix2.local_path = test_files[1]; mix2.duration_seconds = 240;
    
    mix3.id = "test3"; mix3.title = "Test Mix 3"; mix3.artist = "Artist 3"; 
    mix3.genre = "Test"; mix3.url = "http://example.com/3"; 
    mix3.local_path = test_files[2]; mix3.duration_seconds = 300;
    
    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));
    
    // Verify that the file doesn't exist
    EXPECT_FALSE(std::filesystem::exists(test_files[2]));
    
    // Test validation (should fail)
    EXPECT_FALSE(manager.validateDatabaseFileConsistency());
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_AllFilesMissing) {
    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    
    // Add mixes to database through the manager (no files created)
    Mix mix1, mix2, mix3;
    mix1.id = "test1"; mix1.title = "Test Mix 1"; mix1.artist = "Artist 1"; 
    mix1.genre = "Test"; mix1.url = "http://example.com/1"; 
    mix1.local_path = (data_dir / "test1.mp3").string(); mix1.duration_seconds = 180;
    
    mix2.id = "test2"; mix2.title = "Test Mix 2"; mix2.artist = "Artist 2"; 
    mix2.genre = "Test"; mix2.url = "http://example.com/2"; 
    mix2.local_path = (data_dir / "test2.mp3").string(); mix2.duration_seconds = 240;
    
    mix3.id = "test3"; mix3.title = "Test Mix 3"; mix3.artist = "Artist 3"; 
    mix3.genre = "Test"; mix3.url = "http://example.com/3"; 
    mix3.local_path = (data_dir / "test3.mp3").string(); mix3.duration_seconds = 300;
    
    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));
    
    // Verify that the files don't exist
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test1.mp3").string()));
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test2.mp3").string()));
    EXPECT_FALSE(std::filesystem::exists((data_dir / "test3.mp3").string()));
    
    // Test validation (should fail)
    EXPECT_FALSE(manager.validateDatabaseFileConsistency());
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_EmptyDatabase) {
    // Create empty database
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());
    
    // Test validation (should pass for empty database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, ValidateDatabaseFileConsistency_MixesWithoutLocalPath) {
    // Create test database with mix entries (no local_path)
    MixDatabase db(db_path.string());
    ASSERT_TRUE(db.initialize());
    
    // Add mixes to database without local_path
    Mix mix1, mix2;
    mix1.id = "test1"; mix1.title = "Test Mix 1"; mix1.artist = "Artist 1"; 
    mix1.genre = "Test"; mix1.url = "http://example.com/1"; 
    mix1.local_path = ""; mix1.duration_seconds = 180;
    
    mix2.id = "test2"; mix2.title = "Test Mix 2"; mix2.artist = "Artist 2"; 
    mix2.genre = "Test"; mix2.url = "http://example.com/2"; 
    mix2.local_path = ""; mix2.duration_seconds = 240;
    
    ASSERT_TRUE(db.addMix(mix1));
    ASSERT_TRUE(db.addMix(mix2));
    
    // Test validation (should pass for mixes without local_path)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.validateDatabaseFileConsistency());
}

TEST_F(MixManagerTest, CleanupMissingFiles_RemovesMissingEntries) {
    // Create test files (only 1 out of 3)
    std::vector<std::string> test_files = {
        (data_dir / "test1.mp3").string(),
        (data_dir / "test2.mp3").string(),
        (data_dir / "test3.mp3").string()
    };
    
    // Create dummy MP3 file (only first one)
    std::ofstream file(test_files[0], std::ios::binary);
    file.write("\xFF\xFB\x90\x00", 4); // MP3 header
    file.write(std::string(1000, '\x00').c_str(), 1000); // Dummy data
    file.close();
    
    // Initialize MixManager (this will create its own database)
    MixManager manager(db_path.string(), data_dir.string());
    ASSERT_TRUE(manager.initialize());
    
    // Add mixes to database through the manager
    Mix mix1, mix2, mix3;
    mix1.id = "test1"; mix1.title = "Test Mix 1"; mix1.artist = "Artist 1"; 
    mix1.genre = "Test"; mix1.url = "http://example.com/1"; 
    mix1.local_path = test_files[0]; mix1.duration_seconds = 180;
    
    mix2.id = "test2"; mix2.title = "Test Mix 2"; mix2.artist = "Artist 2"; 
    mix2.genre = "Test"; mix2.url = "http://example.com/2"; 
    mix2.local_path = test_files[1]; mix2.duration_seconds = 240;
    
    mix3.id = "test3"; mix3.title = "Test Mix 3"; mix3.artist = "Artist 3"; 
    mix3.genre = "Test"; mix3.url = "http://example.com/3"; 
    mix3.local_path = test_files[2]; mix3.duration_seconds = 300;
    
    // Add mixes to the manager's database
    auto* db = manager.getDatabase();
    ASSERT_TRUE(db->addMix(mix1));
    ASSERT_TRUE(db->addMix(mix2));
    ASSERT_TRUE(db->addMix(mix3));
    
    // Verify initial state
    auto all_mixes = db->getAllMixes();
    EXPECT_EQ(all_mixes.size(), 3);
    
    // Test cleanup
    EXPECT_TRUE(manager.cleanupMissingFiles());
    
    // Verify cleanup removed missing entries
    all_mixes = db->getAllMixes();
    EXPECT_EQ(all_mixes.size(), 1);
    EXPECT_EQ(all_mixes[0].id, "test1");
}
