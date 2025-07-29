#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "mix_metadata.hpp"

/**
 * @brief Test fixtures and utilities for autovibez tests
 */
class TestFixtures {
public:
    /**
     * @brief Create a temporary test directory
     * @return Path to the temporary directory
     */
    static std::string createTempTestDir();
    
    /**
     * @brief Create a test config file
     * @param config_path Path where to create the config file
     * @param content Config file content
     * @return True if successful
     */
    static bool createTestConfigFile(const std::string& config_path, const std::string& content);
    
    /**
     * @brief Create a test YAML file with mix data
     * @param yaml_path Path where to create the YAML file
     * @param mixes Vector of mixes to include
     * @return True if successful
     */
    static bool createTestYamlFile(const std::string& yaml_path, const std::vector<Mix>& mixes);
    
    /**
     * @brief Create a test SQLite database
     * @param db_path Path where to create the database
     * @return True if successful
     */
    static bool createTestDatabase(const std::string& db_path);
    
    /**
     * @brief Create a sample mix for testing
     * @param id Mix ID (optional, will generate if empty)
     * @return Sample Mix object
     */
    static Mix createSampleMix(const std::string& id = "");
    
    /**
     * @brief Create multiple sample mixes for testing
     * @param count Number of mixes to create
     * @return Vector of sample Mix objects
     */
    static std::vector<Mix> createSampleMixes(size_t count);
    
    /**
     * @brief Clean up test files and directories
     * @param paths Vector of paths to clean up
     */
    static void cleanupTestFiles(const std::vector<std::string>& paths);
    
    /**
     * @brief Get sample config content
     * @return Sample config file content
     */
    static std::string getSampleConfigContent();
    
    /**
     * @brief Get sample YAML content
     * @return Sample YAML file content
     */
    static std::string getSampleYamlContent();
    
    /**
     * @brief Generate a unique test ID
     * @return Unique test ID string
     */
    static std::string generateTestId();
    
    /**
     * @brief Create a test MP3 file for testing
     * @param file_path Path where to create the MP3 file
     * @return True if successful
     */
    static bool createTestMP3File(const std::string& file_path);

private:
    static int test_counter;
}; 