#pragma once

#include <memory>
#include <random>
#include <string>

#include "database_interfaces.hpp"
#include "mix_metadata.hpp"
#include "mix_query_builder.hpp"

namespace AutoVibez::Data {

/**
 * @brief Smart selection strategy configuration
 */
struct SmartSelectionConfig {
    int preferred_genre_probability = 80;  // Percentage chance to prefer genre
    int favorite_mix_probability = 70;     // Percentage chance to prefer favorites
    bool prefer_unplayed = true;           // Prefer mixes that haven't been played
    bool prefer_least_played = true;       // Prefer mixes with lower play count

    SmartSelectionConfig() = default;
};

/**
 * @brief Handles smart mix selection logic with configurable strategies
 *
 * This class encapsulates the complex logic for selecting mixes based on
 * various criteria such as genre preference, favorites, play history, etc.
 */
class SmartMixSelector {
public:
    /**
     * @brief Constructor with database connection and optional config
     * @param connection Database connection for queries
     * @param config Smart selection configuration
     */
    explicit SmartMixSelector(std::shared_ptr<IDatabaseConnection> connection,
                              const SmartSelectionConfig& config = SmartSelectionConfig());

    /**
     * @brief Get a smart random mix based on criteria
     * @param exclude_mix_id Mix ID to exclude from selection
     * @param preferred_genre Preferred genre (empty for no preference)
     * @return Selected mix or empty mix if none available
     */
    Mix getSmartRandomMix(const std::string& exclude_mix_id = "", const std::string& preferred_genre = "");

    /**
     * @brief Get a random mix with basic filtering
     * @param exclude_mix_id Mix ID to exclude from selection
     * @return Selected mix or empty mix if none available
     */
    Mix getRandomMix(const std::string& exclude_mix_id = "");

    /**
     * @brief Get a random mix by genre
     * @param genre Genre to filter by
     * @param exclude_mix_id Mix ID to exclude from selection
     * @return Selected mix or empty mix if none available
     */
    Mix getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id = "");

    /**
     * @brief Get a random mix by artist
     * @param artist Artist to filter by
     * @param exclude_mix_id Mix ID to exclude from selection
     * @return Selected mix or empty mix if none available
     */
    Mix getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id = "");

    /**
     * @brief Get the next mix in sequence
     * @param current_mix_id Current mix ID (empty for first)
     * @return Next mix or empty mix if none available
     */
    Mix getNextMix(const std::string& current_mix_id = "");

    /**
     * @brief Get the previous mix in sequence
     * @param current_mix_id Current mix ID (empty for last)
     * @return Previous mix or empty mix if none available
     */
    Mix getPreviousMix(const std::string& current_mix_id = "");

    /**
     * @brief Set the random number generator seed for testing
     * @param seed Seed value
     */
    void setSeed(unsigned int seed);

private:
    std::shared_ptr<IDatabaseConnection> connection_;
    SmartSelectionConfig config_;
    mutable std::mt19937 rng_;

    /**
     * @brief Get mix counts for smart selection
     * @param criteria Selection criteria
     * @return Tuple of (total, favorites, preferred_genre)
     */
    std::tuple<int, int, int> getMixCounts(const SelectionCriteria& criteria) const;

    /**
     * @brief Execute a query and return a single mix
     * @param query SQL query
     * @param parameters Query parameters
     * @return Selected mix or empty mix
     */
    Mix executeSingleMixQuery(const std::string& query, const std::vector<std::string>& parameters) const;

    /**
     * @brief Build smart selection query with prioritization
     * @param criteria Base selection criteria
     * @return SQL query with smart ordering
     */
    std::string buildSmartSelectionQuery(const SelectionCriteria& criteria) const;

    /**
     * @brief Get random percentage (0-99)
     * @return Random percentage
     */
    int getRandomPercentage() const;

    /**
     * @brief Convert statement result to Mix object
     * @param stmt Database statement with result row
     * @return Mix object
     */
    Mix statementToMix(IStatement& stmt) const;
};

}  // namespace AutoVibez::Data
