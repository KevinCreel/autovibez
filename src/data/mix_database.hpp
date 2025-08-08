#pragma once

#include <memory>
#include <string>
#include <vector>

#include "database_interfaces.hpp"
#include "error_handler.hpp"
#include "mix_metadata.hpp"
#include "mix_validator.hpp"
#include "smart_mix_selector.hpp"

namespace AutoVibez::Data {

/**
 * @brief Manages SQLite database operations for mix metadata and user data
 */
class MixDatabase : public AutoVibez::Utils::ErrorHandler {
public:
    explicit MixDatabase(const std::string& db_path);
    explicit MixDatabase(std::shared_ptr<IDatabaseConnection> connection);
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
     * @brief Delete a mix from the database
     * @param id Mix ID to delete
     * @return True if successful, false otherwise
     */
    bool deleteMix(const std::string& id);

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
    Mix getRandomMix(const std::string& exclude_mix_id);
    Mix getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre);

    /**
     * @brief Get the next mix in database order
     * @param current_mix_id ID of current mix (empty for first mix)
     * @return Next mix in database, or first mix if current is empty
     */
    Mix getNextMix(const std::string& current_mix_id = "");

    /**
     * @brief Get the previous mix in database order
     * @param current_mix_id ID of current mix (empty for last mix)
     * @return Previous mix in database, or last mix if current is empty
     */
    Mix getPreviousMix(const std::string& current_mix_id = "");

    /**
     * @brief Get a random mix by genre
     * @param genre Genre to filter by
     * @return Random mix in the genre, or empty mix if none available
     */
    Mix getRandomMixByGenre(const std::string& genre);
    Mix getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id);
    Mix getRandomMixByArtist(const std::string& artist);
    Mix getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id);

    /**
     * @brief Toggle favorite status for a mix
     * @param mix_id Mix ID
     * @return True if successful, false otherwise
     */
    bool toggleFavorite(const std::string& mix_id);

    /**
     * @brief Soft delete a mix (mark as deleted without removing from database)
     * @param mix_id Mix ID
     * @return True if successful, false otherwise
     */
    bool softDeleteMix(const std::string& mix_id);

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
    std::string getLastError() const {
        return last_error;
    }

    /**
     * @brief Check if the last operation was successful
     * @return True if successful, false otherwise
     */
    bool isSuccess() const {
        return success;
    }

private:
    std::shared_ptr<IDatabaseConnection> connection_;
    std::unique_ptr<MixValidator> validator_;
    std::unique_ptr<SmartMixSelector> selector_;
    std::string db_path_;

    /**
     * @brief Create database tables
     * @return True if successful, false otherwise
     */
    bool createTables();

    /**
     * @brief Convert database row to mix
     * @param stmt Database statement
     * @return Mix object
     */
    Mix statementToMix(IStatement& stmt);

    /**
     * @brief Execute a query and return vector of mixes
     * @param query SQL query to execute
     * @param parameters Query parameters
     * @return Vector of mixes
     */
    std::vector<Mix> executeQueryForMixes(const std::string& query, const std::vector<std::string>& parameters = {});

    /**
     * @brief Execute a query and return single mix
     * @param query SQL query to execute
     * @param parameters Query parameters
     * @return Mix object
     */
    Mix executeQueryForSingleMix(const std::string& query, const std::vector<std::string>& parameters = {});

    /**
     * @brief Bind mix data to prepared statement
     * @param stmt Statement to bind to
     * @param mix Mix data to bind
     * @param include_id Whether to bind ID (for updates vs inserts)
     */
    void bindMixToStatement(IStatement& stmt, const Mix& mix, bool include_id = false);
};

}  // namespace AutoVibez::Data