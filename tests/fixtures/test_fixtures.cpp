#include "test_fixtures.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <sqlite3.h>
#include <yaml-cpp/yaml.h>

int TestFixtures::test_counter = 0;

std::string TestFixtures::createTempTestDir() {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    temp_dir /= "autovibez_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    std::filesystem::create_directories(temp_dir);
    return temp_dir.string();
}

bool TestFixtures::createTestConfigFile(const std::string& config_path, const std::string& content) {
    std::ofstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

bool TestFixtures::createTestYamlFile(const std::string& yaml_path, const std::vector<Mix>& mixes) {
    YAML::Node root;
    YAML::Node mixes_node;
    
    for (const auto& mix : mixes) {
        YAML::Node mix_node;
        mix_node["id"] = mix.id;
        mix_node["title"] = mix.title;
        mix_node["artist"] = mix.artist;
        mix_node["genre"] = mix.genre;
        mix_node["url"] = mix.url;
        mix_node["duration_seconds"] = mix.duration_seconds;
        mix_node["description"] = mix.description;
        mix_node["tags"] = mix.tags;
        
        mixes_node.push_back(mix_node);
    }
    
    root["mixes"] = mixes_node;
    
    std::ofstream file(yaml_path);
    if (!file.is_open()) {
        return false;
    }
    file << root;
    file.close();
    return true;
}

bool TestFixtures::createTestDatabase(const std::string& db_path) {
    sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS mixes (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            artist TEXT NOT NULL,
            genre TEXT,
            url TEXT,
            local_path TEXT,
            duration_seconds INTEGER DEFAULT 0,
            description TEXT,
            date_added TEXT,
            last_played TEXT,
            play_count INTEGER DEFAULT 0,
            is_favorite BOOLEAN DEFAULT 0
        );
        
        CREATE TABLE IF NOT EXISTS mix_tags (
            mix_id TEXT,
            tag TEXT,
            FOREIGN KEY (mix_id) REFERENCES mixes(id)
        );
    )";
    
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_close(db);
    return true;
}

Mix TestFixtures::createSampleMix(const std::string& id) {
    Mix mix;
    mix.id = id.empty() ? generateTestId() : id;
    mix.title = "Test Mix " + mix.id;
    mix.artist = "Test Artist";
    mix.genre = "Electronic";
    mix.url = "https://example.com/mix_" + mix.id + ".mp3";
    mix.duration_seconds = 3600; // 1 hour
    mix.description = "A test mix for unit testing";
    mix.tags = {"test", "electronic", "dance"};
    mix.play_count = 0;
    mix.is_favorite = false;
    return mix;
}

std::vector<Mix> TestFixtures::createSampleMixes(size_t count) {
    std::vector<Mix> mixes;
    mixes.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        mixes.push_back(createSampleMix("test_mix_" + std::to_string(i)));
    }
    
    return mixes;
}

void TestFixtures::cleanupTestFiles(const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        try {
            if (std::filesystem::is_directory(path)) {
                std::filesystem::remove_all(path);
            } else {
                std::filesystem::remove(path);
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore cleanup errors in tests
        }
    }
}

std::string TestFixtures::getSampleConfigContent() {
    return R"(
# Test configuration file
audio_device = 0
preset_path = assets/presets
texture_path = assets/textures
mixes_url = https://example.com/mixes.yaml
cache_size_mb = 100
auto_download = true
seek_increment = 10
volume_step = 5
crossfade_enabled = true
crossfade_duration = 3
refresh_interval = 30
preferred_genre = Electronic
font_path = /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
show_fps = false
mesh_x = 32
mesh_y = 24
aspect_correction = true
fps = 60
)";
}

std::string TestFixtures::getSampleYamlContent() {
    return R"(
mixes:
  - id: test_mix_1
    title: "Test Mix 1"
    artist: "Test Artist 1"
    genre: "Electronic"
    url: "https://example.com/mix1.mp3"
    duration_seconds: 3600
    description: "A test mix for unit testing"
    tags: ["test", "electronic", "dance"]
    
  - id: test_mix_2
    title: "Test Mix 2"
    artist: "Test Artist 2"
    genre: "House"
    url: "https://example.com/mix2.mp3"
    duration_seconds: 2700
    description: "Another test mix for unit testing"
    tags: ["test", "house", "dance"]
)";
}

std::string TestFixtures::generateTestId() {
    return "test_" + std::to_string(++test_counter) + "_" + 
           std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}

bool TestFixtures::createTestMP3File(const std::string& file_path) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Create a minimal valid MP3 file with ID3v2 header
    // This is a simplified MP3 file structure for testing
    
    // ID3v2 header (10 bytes)
    const unsigned char id3_header[] = {
        0x49, 0x44, 0x33,  // "ID3"
        0x03, 0x00,         // Version 2.3
        0x00,               // Flags
        0x00, 0x00, 0x00, 0x00  // Size (0 for this test)
    };
    
    file.write(reinterpret_cast<const char*>(id3_header), sizeof(id3_header));
    
    // Add some dummy data to make it a "valid" file
    const char dummy_data[] = "This is a test MP3 file for unit testing";
    file.write(dummy_data, sizeof(dummy_data) - 1);
    
    file.close();
    return true;
} 