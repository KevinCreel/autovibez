#include "mix_database.hpp"

#include <chrono>
#include <random>

#include "constants.hpp"
#include "json_utils.hpp"
#include "mix_query_builder.hpp"
#include "path_manager.hpp"
#include "sqlite_connection.hpp"

namespace AutoVibez {
namespace Data {

MixDatabase::MixDatabase(const std::string& db_path) : db_path_(db_path) {
    connection_ = std::make_shared<SqliteConnection>(db_path);
    validator_ = std::make_unique<MixValidator>();
    // Smart selector will be created after connection is initialized
}

MixDatabase::MixDatabase(std::shared_ptr<IDatabaseConnection> connection) 
    : connection_(connection) {
    validator_ = std::make_unique<MixValidator>();
    // Smart selector will be created after connection is initialized
}

MixDatabase::~MixDatabase() = default;

bool MixDatabase::initialize() {
    if (!connection_->initialize()) {
        setError("Failed to initialize database: " + connection_->getLastError());
        return false;
    }

    if (!createTables()) {
        return false;
    }

    // Initialize smart selector now that connection is ready
    selector_ = std::make_unique<SmartMixSelector>(connection_);

    return true;
}

bool MixDatabase::createTables() {
    // Create the main table
    if (!connection_->execute(StringConstants::CREATE_MIXES_TABLE)) {
        setError("Failed to create tables: " + connection_->getLastError());
        return false;
    }

    // Try to add is_deleted column (will fail silently if it already exists)
    connection_->execute(StringConstants::ALTER_ADD_IS_DELETED);

    return true;
}

bool MixDatabase::addMix(const Mix& mix) {
    auto validation_result = validator_->validate(mix);
    if (!validation_result) {
        setError(validation_result.errorMessage);
        return false;
    }

    auto stmt = connection_->prepare(StringConstants::INSERT_OR_REPLACE_MIX);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    bindMixToStatement(*stmt, mix, false);

    if (!stmt->execute()) {
        setError("Failed to insert mix: " + connection_->getLastError());
        return false;
    }

    return true;
}

bool MixDatabase::updateMix(const Mix& mix) {
    auto validation_result = validator_->validate(mix);
    if (!validation_result) {
        setError(validation_result.errorMessage);
        return false;
    }

    auto stmt = connection_->prepare(StringConstants::UPDATE_MIX);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    bindMixToStatement(*stmt, mix, true);

    if (!stmt->execute()) {
        setError("Failed to update mix: " + connection_->getLastError());
        return false;
    }

    return true;
}

bool MixDatabase::deleteMix(const std::string& id) {
    if (!connection_) {
        setError("Database not initialized");
        return false;
    }

    auto stmt = connection_->prepare(StringConstants::DELETE_MIX);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    stmt->bindText(1, id);

    if (!stmt->execute()) {
        setError("Failed to delete mix: " + connection_->getLastError());
        return false;
    }

    if (stmt->getChanges() == 0) {
        setError("No mix found with id: " + id);
        return false;
    }

    return true;
}

Mix MixDatabase::getMixById(const std::string& id) {
    return executeQueryForSingleMix(StringConstants::SELECT_MIX_BY_ID, {id});
}

std::vector<Mix> MixDatabase::getAllMixes() {
    return executeQueryForMixes(StringConstants::SELECT_ALL_MIXES);
}

std::vector<Mix> MixDatabase::getMixesByGenre(const std::string& genre) {
    return executeQueryForMixes(StringConstants::SELECT_MIXES_BY_GENRE, {genre});
}

std::vector<Mix> MixDatabase::getMixesByArtist(const std::string& artist) {
    return executeQueryForMixes(StringConstants::SELECT_MIXES_BY_ARTIST, {artist});
}

Mix MixDatabase::getRandomMix(const std::string& exclude_mix_id) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getRandomMix(exclude_mix_id);
}

Mix MixDatabase::getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getSmartRandomMix(exclude_mix_id, preferred_genre);
}

Mix MixDatabase::getNextMix(const std::string& current_mix_id) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getNextMix(current_mix_id);
}

Mix MixDatabase::getPreviousMix(const std::string& current_mix_id) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getPreviousMix(current_mix_id);
}

Mix MixDatabase::getRandomMixByGenre(const std::string& genre) {
    return getRandomMixByGenre(genre, "");
}

Mix MixDatabase::getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getRandomMixByGenre(genre, exclude_mix_id);
}

Mix MixDatabase::getRandomMixByArtist(const std::string& artist) {
    return getRandomMixByArtist(artist, "");
}

Mix MixDatabase::getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id) {
    if (!selector_) {
        setError("Smart selector not initialized");
        return Mix();
    }
    return selector_->getRandomMixByArtist(artist, exclude_mix_id);
}

bool MixDatabase::toggleFavorite(const std::string& mix_id) {
    auto stmt = connection_->prepare(StringConstants::TOGGLE_FAVORITE);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    stmt->bindText(1, mix_id);
    return stmt->execute();
}

bool MixDatabase::softDeleteMix(const std::string& mix_id) {
    auto stmt = connection_->prepare(StringConstants::SOFT_DELETE_MIX);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    stmt->bindText(1, mix_id);
    
    if (!stmt->execute()) {
        setError("Failed to soft delete mix: " + connection_->getLastError());
        return false;
    }

    return stmt->getChanges() > 0;
}

bool MixDatabase::updatePlayStats(const std::string& mix_id) {
    auto stmt = connection_->prepare(StringConstants::UPDATE_PLAY_STATS);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    stmt->bindText(1, mix_id);
    return stmt->execute();
}

bool MixDatabase::setLocalPath(const std::string& mix_id, const std::string& local_path) {
    auto stmt = connection_->prepare(StringConstants::SET_LOCAL_PATH);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return false;
    }

    stmt->bindText(1, local_path);
    stmt->bindText(2, mix_id);
    
    return stmt->execute();
}

std::vector<Mix> MixDatabase::getDownloadedMixes() {
    return executeQueryForMixes(StringConstants::SELECT_DOWNLOADED_MIXES);
}

std::vector<Mix> MixDatabase::getFavoriteMixes() {
    return executeQueryForMixes(StringConstants::SELECT_FAVORITE_MIXES);
}

std::vector<Mix> MixDatabase::getRecentlyPlayed(int limit) {
    auto stmt = connection_->prepare(StringConstants::SELECT_RECENTLY_PLAYED);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return std::vector<Mix>();
    }

    stmt->bindInt(1, limit);

    std::vector<Mix> mixes;
    while (stmt->step()) {
        try {
            Mix mix = statementToMix(*stmt);
            mixes.push_back(mix);
        } catch (const std::exception& e) {
            // Skip malformed rows
        }
    }

    return mixes;
}

Mix MixDatabase::statementToMix(IStatement& stmt) {
    Mix mix;
    
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

std::vector<Mix> MixDatabase::executeQueryForMixes(const std::string& query, const std::vector<std::string>& parameters) {
    auto stmt = connection_->prepare(query);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
        return std::vector<Mix>();
    }

    // Bind parameters
    for (size_t i = 0; i < parameters.size(); ++i) {
        stmt->bindText(static_cast<int>(i + 1), parameters[i]);
    }

    std::vector<Mix> mixes;
    while (stmt->step()) {
        try {
            Mix mix = statementToMix(*stmt);
            mixes.push_back(mix);
        } catch (const std::exception& e) {
            // Skip malformed rows
        }
    }

    return mixes;
}

Mix MixDatabase::executeQueryForSingleMix(const std::string& query, const std::vector<std::string>& parameters) {
    auto stmt = connection_->prepare(query);
    if (!stmt) {
        setError("Failed to prepare statement: " + connection_->getLastError());
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

void MixDatabase::bindMixToStatement(IStatement& stmt, const Mix& mix, bool include_id) {
    std::string tags_json = AutoVibez::Utils::JsonUtils::vectorToJsonArray(mix.tags);
    
    int param_index = 1;
    
    if (!include_id) {
        // For INSERT statements, bind all fields
        stmt.bindText(param_index++, mix.id);
    }
    
    stmt.bindText(param_index++, mix.title);
    stmt.bindText(param_index++, mix.artist);
    stmt.bindText(param_index++, mix.genre);
    stmt.bindText(param_index++, mix.url);
    stmt.bindText(param_index++, mix.local_path);
    stmt.bindInt(param_index++, mix.duration_seconds);
    stmt.bindText(param_index++, tags_json);
    stmt.bindText(param_index++, mix.description);
    stmt.bindText(param_index++, mix.date_added);
    stmt.bindText(param_index++, mix.last_played);
    stmt.bindInt(param_index++, mix.play_count);
    stmt.bindInt(param_index++, mix.is_favorite ? 1 : 0);
    stmt.bindInt(param_index++, mix.is_deleted ? 1 : 0);
    
    if (include_id) {
        // For UPDATE statements, bind ID at the end
        stmt.bindText(param_index, mix.id);
    }
}

}  // namespace Data
}  // namespace AutoVibez
