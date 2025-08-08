#include "smart_mix_selector.hpp"

#include <chrono>
#include <tuple>

#include "constants.hpp"
#include "json_utils.hpp"

namespace AutoVibez {
namespace Data {

SmartMixSelector::SmartMixSelector(std::shared_ptr<IDatabaseConnection> connection, const SmartSelectionConfig& config)
    : connection_(connection), config_(config) {
    // Initialize random number generator with current time
    auto now = std::chrono::high_resolution_clock::now();
    auto seed = static_cast<unsigned int>(now.time_since_epoch().count());
    rng_.seed(seed);
}

Mix SmartMixSelector::getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre) {
    if (!connection_) {
        return Mix();
    }

    // Get counts for decision making
    SelectionCriteria criteria;
    criteria.exclude_mix_id = exclude_mix_id;
    criteria.downloaded_only = true;  // Prefer downloaded mixes

    auto [total_mixes, favorite_mixes, preferred_genre_mixes] = getMixCounts(criteria);

    if (total_mixes == 0) {
        // Fallback to any available mix
        criteria.downloaded_only = false;
        return getRandomMix(exclude_mix_id);
    }

    // Decide selection strategy based on probabilities
    bool prefer_genre = !preferred_genre.empty() && (preferred_genre_mixes > 0) &&
                        (getRandomPercentage() < config_.preferred_genre_probability);

    bool prefer_favorites =
        !prefer_genre && (favorite_mixes > 0) && (getRandomPercentage() < config_.favorite_mix_probability);

    // Try genre preference first
    if (prefer_genre) {
        SelectionCriteria genre_criteria = criteria;
        genre_criteria.genre = preferred_genre;

        std::string query = buildSmartSelectionQuery(genre_criteria);
        std::vector<std::string> params;
        params.push_back(preferred_genre);
        if (!exclude_mix_id.empty()) {
            params.push_back(exclude_mix_id);
        }

        Mix result = executeSingleMixQuery(query, params);
        if (!result.id.empty()) {
            return result;
        }
    }

    // Try favorites if genre didn't work
    if (prefer_favorites) {
        SelectionCriteria fav_criteria = criteria;
        fav_criteria.favorites_only = true;

        std::string query = buildSmartSelectionQuery(fav_criteria);
        std::vector<std::string> params;
        if (!exclude_mix_id.empty()) {
            params.push_back(exclude_mix_id);
        }

        Mix result = executeSingleMixQuery(query, params);
        if (!result.id.empty()) {
            return result;
        }
    }

    // Fallback to any downloaded mix
    std::string query = buildSmartSelectionQuery(criteria);
    std::vector<std::string> params;
    if (!exclude_mix_id.empty()) {
        params.push_back(exclude_mix_id);
    }

    Mix result = executeSingleMixQuery(query, params);
    if (!result.id.empty()) {
        return result;
    }

    // Final fallback to any available mix
    return getRandomMix(exclude_mix_id);
}

Mix SmartMixSelector::getRandomMix(const std::string& exclude_mix_id) {
    SelectionCriteria criteria;
    criteria.exclude_mix_id = exclude_mix_id;
    criteria.downloaded_only = true;  // Try downloaded first

    std::string query = MixQueryBuilder::buildQuery(criteria, OrderBy::Random);
    std::vector<std::string> params;
    if (!exclude_mix_id.empty()) {
        params.push_back(exclude_mix_id);
    }

    Mix result = executeSingleMixQuery(query, params);
    if (!result.id.empty()) {
        return result;
    }

    // Fallback to any available mix
    criteria.downloaded_only = false;
    query = MixQueryBuilder::buildQuery(criteria, OrderBy::Random);
    return executeSingleMixQuery(query, params);
}

Mix SmartMixSelector::getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id) {
    SelectionCriteria criteria;
    criteria.genre = genre;
    criteria.exclude_mix_id = exclude_mix_id;

    std::string query = MixQueryBuilder::buildQuery(criteria, OrderBy::Random);
    std::vector<std::string> params = {genre};
    if (!exclude_mix_id.empty()) {
        params.push_back(exclude_mix_id);
    }

    return executeSingleMixQuery(query, params);
}

Mix SmartMixSelector::getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id) {
    SelectionCriteria criteria;
    criteria.artist = artist;
    criteria.exclude_mix_id = exclude_mix_id;

    std::string query = MixQueryBuilder::buildQuery(criteria, OrderBy::Random);
    std::vector<std::string> params = {artist};
    if (!exclude_mix_id.empty()) {
        params.push_back(exclude_mix_id);
    }

    return executeSingleMixQuery(query, params);
}

Mix SmartMixSelector::getNextMix(const std::string& current_mix_id) {
    std::string query;
    std::vector<std::string> params;

    if (current_mix_id.empty()) {
        query = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1";
    } else {
        query = "SELECT * FROM mixes WHERE id > ? AND is_deleted = 0 ORDER BY id LIMIT 1";
        params.push_back(current_mix_id);
    }

    Mix result = executeSingleMixQuery(query, params);
    if (!result.id.empty()) {
        return result;
    }

    // Wrap around to first mix
    query = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1";
    return executeSingleMixQuery(query, {});
}

Mix SmartMixSelector::getPreviousMix(const std::string& current_mix_id) {
    std::string query;
    std::vector<std::string> params;

    if (current_mix_id.empty()) {
        query = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1";
    } else {
        query = "SELECT * FROM mixes WHERE id < ? AND is_deleted = 0 ORDER BY id DESC LIMIT 1";
        params.push_back(current_mix_id);
    }

    Mix result = executeSingleMixQuery(query, params);
    if (!result.id.empty()) {
        return result;
    }

    // Wrap around to last mix
    query = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1";
    return executeSingleMixQuery(query, {});
}

void SmartMixSelector::setSeed(unsigned int seed) {
    rng_.seed(seed);
}

std::tuple<int, int, int> SmartMixSelector::getMixCounts(const SelectionCriteria& criteria) const {
    // Build count query
    std::string count_query = "SELECT COUNT(*) as total, SUM(CASE WHEN is_favorite = 1 THEN 1 ELSE 0 END) as favorites";
    std::vector<std::string> params;

    if (!criteria.genre.empty()) {
        count_query += ", SUM(CASE WHEN genre COLLATE NOCASE = ? COLLATE NOCASE THEN 1 ELSE 0 END) as preferred";
        params.push_back(criteria.genre);
    }

    count_query += " FROM mixes WHERE is_deleted = 0";

    if (criteria.downloaded_only) {
        count_query += " AND local_path IS NOT NULL AND local_path != ''";
    }

    if (!criteria.exclude_mix_id.empty()) {
        count_query += " AND id != ?";
        params.push_back(criteria.exclude_mix_id);
    }

    auto stmt = connection_->prepare(count_query);
    if (!stmt) {
        return {0, 0, 0};
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        stmt->bindText(static_cast<int>(i + 1), params[i]);
    }

    if (stmt->step()) {
        int total = stmt->getInt(0);
        int favorites = stmt->getInt(1);
        int preferred_genre = (!criteria.genre.empty()) ? stmt->getInt(2) : 0;
        return {total, favorites, preferred_genre};
    }

    return {0, 0, 0};
}

Mix SmartMixSelector::executeSingleMixQuery(const std::string& query,
                                            const std::vector<std::string>& parameters) const {
    auto stmt = connection_->prepare(query);
    if (!stmt) {
        return Mix();
    }

    // Bind parameters
    for (size_t i = 0; i < parameters.size(); ++i) {
        stmt->bindText(static_cast<int>(i + 1), parameters[i]);
    }

    if (stmt->step()) {
        return statementToMix(*stmt);
    }

    return Mix();
}

std::string SmartMixSelector::buildSmartSelectionQuery(const SelectionCriteria& criteria) const {
    MixQueryBuilder builder;
    builder.select();

    if (!criteria.include_deleted) {
        builder.whereNotDeleted();
    }

    if (!criteria.genre.empty()) {
        builder.whereGenre();
    }

    if (!criteria.artist.empty()) {
        builder.whereArtist();
    }

    if (!criteria.exclude_mix_id.empty()) {
        builder.whereNotId();
    }

    if (criteria.favorites_only) {
        builder.whereFavorites();
    }

    if (criteria.downloaded_only) {
        builder.whereDownloaded();
    }

    // Smart ordering: prefer unplayed, then least played, then random
    std::string query = builder.build();

    if (config_.prefer_unplayed || config_.prefer_least_played) {
        // Replace the ORDER BY clause with smart ordering
        size_t order_pos = query.find("ORDER BY");
        if (order_pos != std::string::npos) {
            query = query.substr(0, order_pos);
        }

        query += " ORDER BY ";
        if (config_.prefer_unplayed) {
            query += "CASE WHEN last_played IS NULL THEN 0 ELSE 1 END, ";
        }
        if (config_.prefer_least_played) {
            query += "last_played ASC, play_count ASC, ";
        }
        query += "RANDOM() LIMIT 1";
    } else {
        // Just add limit if not already present
        if (query.find("LIMIT") == std::string::npos) {
            query += " LIMIT 1";
        }
    }

    return query;
}

int SmartMixSelector::getRandomPercentage() const {
    std::uniform_int_distribution<int> dist(0, 99);
    return dist(rng_);
}

Mix SmartMixSelector::statementToMix(IStatement& stmt) const {
    Mix mix;

    // Use the constants for column indices
    using namespace Constants::DatabaseColumns;

    if (!stmt.isNull(MIX_ID)) {
        mix.id = stmt.getText(MIX_ID);
    }
    if (!stmt.isNull(MIX_TITLE)) {
        mix.title = stmt.getText(MIX_TITLE);
    }
    if (!stmt.isNull(MIX_ARTIST)) {
        mix.artist = stmt.getText(MIX_ARTIST);
    }
    if (!stmt.isNull(MIX_GENRE)) {
        mix.genre = stmt.getText(MIX_GENRE);
    }
    if (!stmt.isNull(MIX_URL)) {
        mix.url = stmt.getText(MIX_URL);
    }
    if (!stmt.isNull(MIX_LOCAL_PATH)) {
        mix.local_path = stmt.getText(MIX_LOCAL_PATH);
    }

    mix.duration_seconds = stmt.getInt(MIX_DURATION_SECONDS);

    if (!stmt.isNull(MIX_TAGS)) {
        std::string tags_json = stmt.getText(MIX_TAGS);
        mix.tags = AutoVibez::Utils::JsonUtils::jsonArrayToVector(tags_json);
    }

    if (!stmt.isNull(MIX_DESCRIPTION)) {
        mix.description = stmt.getText(MIX_DESCRIPTION);
    }
    if (!stmt.isNull(MIX_DATE_ADDED)) {
        mix.date_added = stmt.getText(MIX_DATE_ADDED);
    }
    if (!stmt.isNull(MIX_LAST_PLAYED)) {
        mix.last_played = stmt.getText(MIX_LAST_PLAYED);
    }

    mix.play_count = stmt.getInt(MIX_PLAY_COUNT);
    mix.is_favorite = stmt.getInt(MIX_IS_FAVORITE) != 0;
    mix.is_deleted = stmt.getInt(MIX_IS_DELETED) != 0;

    return mix;
}

}  // namespace Data
}  // namespace AutoVibez
