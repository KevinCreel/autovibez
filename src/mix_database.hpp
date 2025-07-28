#pragma once

#include "mix_metadata.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Manages SQLite database operations for mix metadata and user data
 */
class MixDatabase {
public:
    MixDatabase(const std::string& db_path);
    ~MixDatabase();
    
    /**
     * @brief Initialize the database and create tables
     * @return True if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Add a mix to the database
     * @param mix Mix to add
     * @return True if successful, false otherwise
     */
    bool addMix(const Mix& mix);
    
    /**
     * @brief Update a mix in the database
     * @param mix Mix to update
     * @return True if successful, false otherwise
     */
    bool updateMix(const Mix& mix);
    
    /**
     * @brief Get a mix by ID
     * @param id Mix ID
     * @return Mix object, or empty mix if not found
     */
    Mix getMixById(const std::string& id);
    
    /**
     * @brief Get all mixes in the database
     * @return Vector of all mixes
     */
    std::vector<Mix> getAllMixes();
    
    /**
     * @brief Get mixes by genre
     * @param genre Genre to filter by
     * @return Vector of mixes in the genre
     */
    std::vector<Mix> getMixesByGenre(const std::string& genre);
    
    /**
     * @brief Get mixes by artist
     * @param artist Artist to filter by
     * @return Vector of mixes by the artist
     */
    std::vector<Mix> getMixesByArtist(const std::string& artist);
    
    /**
     * @brief Get a random mix
     * @return Random mix, or empty mix if none available
     */
    Mix getRandomMix();
    
    /**
     * @brief Get a smart random mix with prioritization
     * Prioritizes: 1) Favorites, 2) Least recently played, 3) Fewest plays
     * @return Smart random mix, or empty mix if none available
     */
    Mix getSmartRandomMix();
    Mix getSmartRandomMix(const std::string& exclude_mix_id);
    
    /**
     * @brief Get the next mix in database order
     * @param current_mix_id ID of current mix (empty for first mix)
     * @return Next mix in database, or first mix if current is empty
     */
    Mix getNextMix(const std::string& current_mix_id = "");
    
    /**
     * @brief Get a random mix by genre
     * @param genre Genre to filter by
     * @return Random mix in the genre, or empty mix if none available
     */
    Mix getRandomMixByGenre(const std::string& genre);
    
    /**
     * @brief Get a random mix by artist
     * @param artist Artist to filter by
     * @return Random mix by the artist, or empty mix if none available
     */
    Mix getRandomMixByArtist(const std::string& artist);
    

    
    /**
     * @brief Toggle favorite status for a mix
     * @param mix_id Mix ID
     * @return True if successful, false otherwise
     */
    bool toggleFavorite(const std::string& mix_id);
    
    /**
     * @brief Update play count and last played time
     * @param mix_id Mix ID
     * @return True if successful, false otherwise
     */
    bool updatePlayStats(const std::string& mix_id);
    
    /**
     * @brief Set local file path for a mix
     * @param mix_id Mix ID
     * @param local_path Local file path
     * @return True if successful, false otherwise
     */
    bool setLocalPath(const std::string& mix_id, const std::string& local_path);
    
    /**
     * @brief Get mixes that are downloaded locally
     * @return Vector of downloaded mixes
     */
    std::vector<Mix> getDownloadedMixes();
    
    /**
     * @brief Get favorite mixes
     * @return Vector of favorite mixes
     */
    std::vector<Mix> getFavoriteMixes();
    
    /**
     * @brief Get recently played mixes
     * @param limit Maximum number of mixes to return
     * @return Vector of recently played mixes
     */
    std::vector<Mix> getRecentlyPlayed(int limit = 10);
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const { return last_error; }
    
    /**
     * @brief Check if the last operation was successful
     * @return True if successful, false otherwise
     */
    bool isSuccess() const { return success; }

private:
    sqlite3* db;
    std::string db_path;
    std::string last_error;
    bool success;
    
    /**
     * @brief Create database tables
     * @return True if successful, false otherwise
     */
    bool createTables();
    
    /**
     * @brief Convert a mix to database row
     * @param mix Mix to convert
     * @return SQL insert statement
     */
    std::string mixToInsertSql(const Mix& mix);
    
    /**
     * @brief Convert database row to mix
     * @param stmt SQLite statement
     * @return Mix object
     */
    Mix rowToMix(sqlite3_stmt* stmt);
    
    /**
     * @brief Execute a SQL query
     * @param sql SQL query to execute
     * @return True if successful, false otherwise
     */
    bool executeQuery(const std::string& sql);
    
    /**
     * @brief Execute a SQL query and return vector of mixes
     * @param sql SQL query to execute
     * @return Vector of mixes
     */
    std::vector<Mix> executeQueryForMixes(const char* sql);
    
    /**
     * @brief Execute a SQL query and return single mix
     * @param sql SQL query to execute
     * @return Mix object
     */
    Mix executeQueryForSingleMix(const char* sql);
}; 