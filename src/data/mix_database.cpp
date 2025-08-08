#include "mix_database.hpp"

#include <chrono>
#include <random>

#include "constants.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"

namespace AutoVibez {
namespace Data {

MixDatabase::MixDatabase(const std::string& db_path) : db(nullptr), db_path(db_path) {}

MixDatabase::~MixDatabase() {
    if (db) {
        sqlite3_close(db);
    }
}

bool MixDatabase::initialize() {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        setError("Failed to open database: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    if (!createTables()) {
        return false;
    }

    return true;
}

bool MixDatabase::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS mixes (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            artist TEXT NOT NULL,
            genre TEXT NOT NULL,
            url TEXT NOT NULL,
            local_path TEXT,
            duration_seconds INTEGER NOT NULL,
        
            tags TEXT,
            description TEXT,
            date_added DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_played DATETIME,
            play_count INTEGER DEFAULT 0,
            is_favorite BOOLEAN DEFAULT 0,
            is_deleted BOOLEAN DEFAULT 0
        );
        
        CREATE INDEX IF NOT EXISTS idx_mixes_genre ON mixes(genre);
        CREATE INDEX IF NOT EXISTS idx_mixes_artist ON mixes(artist);
        CREATE INDEX IF NOT EXISTS idx_mixes_favorite ON mixes(is_favorite);
        CREATE INDEX IF NOT EXISTS idx_mixes_last_played ON mixes(last_played);
        CREATE INDEX IF NOT EXISTS idx_mixes_deleted ON mixes(is_deleted);
    )";

    // Add the is_deleted column to existing tables if it doesn't exist
    const char* alter_sql = R"(
        ALTER TABLE mixes ADD COLUMN is_deleted BOOLEAN DEFAULT 0;
    )";

    // Try to add the column (will fail silently if column already exists)
    sqlite3_exec(db, alter_sql, nullptr, nullptr, nullptr);

    return executeQuery(sql);
}

bool MixDatabase::validateMixData(const Mix& mix) {
    if (mix.id.empty() || mix.title.empty() || mix.artist.empty() || mix.genre.empty()) {
        setError("Invalid mix data: missing required fields (id, title, artist, genre)");
        return false;
    }

    if (mix.title == mix.id) {
        setError("Invalid mix data: title cannot be the same as id");
        return false;
    }

    if (mix.artist == "Unknown Artist" || mix.artist.empty()) {
        setError("Invalid mix data: artist cannot be 'Unknown Artist' or empty");
        return false;
    }

    if (mix.duration_seconds <= 0) {
        setError("Invalid mix data: duration must be greater than 0");
        return false;
    }

    return true;
}

bool MixDatabase::addMix(const Mix& mix) {
    if (!validateMixData(mix)) {
        return false;
    }

    const char* sql = R"(
        INSERT OR REPLACE INTO mixes 
        (id, title, artist, genre, url, local_path, duration_seconds, tags, description, date_added, last_played, play_count, is_favorite, is_deleted)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    std::string tags_json = "[";
    for (size_t i = 0; i < mix.tags.size(); ++i) {
        if (i > 0)
            tags_json += ",";
        tags_json += "\"" + mix.tags[i] + "\"";
    }
    tags_json += "]";

    sqlite3_bind_text(stmt, 1, mix.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, mix.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, mix.artist.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, mix.genre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, mix.url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, mix.local_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, mix.duration_seconds);

    sqlite3_bind_text(stmt, 8, tags_json.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, mix.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, mix.date_added.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, mix.last_played.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 12, mix.play_count);
    sqlite3_bind_int(stmt, 13, mix.is_favorite ? 1 : 0);
    sqlite3_bind_int(stmt, 14, mix.is_deleted ? 1 : 0);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        setError("Failed to insert mix: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    return true;
}

bool MixDatabase::updateMix(const Mix& mix) {
    if (!validateMixData(mix)) {
        return false;
    }

    const char* sql =
        "UPDATE mixes SET title = ?, artist = ?, genre = ?, url = ?, local_path = ?, duration_seconds = ?, tags = ?, "
        "description = ?, date_added = ?, last_played = ?, play_count = ?, is_favorite = ?, is_deleted = ? WHERE id = "
        "?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare update statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    std::string tags_json = "[";
    for (size_t i = 0; i < mix.tags.size(); ++i) {
        if (i > 0)
            tags_json += ",";
        tags_json += "\"" + mix.tags[i] + "\"";
    }
    tags_json += "]";

    sqlite3_bind_text(stmt, 1, mix.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, mix.artist.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, mix.genre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, mix.url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, mix.local_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, mix.duration_seconds);
    sqlite3_bind_text(stmt, 7, tags_json.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, mix.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, mix.date_added.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, mix.last_played.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 11, mix.play_count);
    sqlite3_bind_int(stmt, 12, mix.is_favorite ? 1 : 0);
    sqlite3_bind_int(stmt, 13, mix.is_deleted ? 1 : 0);
    sqlite3_bind_text(stmt, 14, mix.id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        setError("Failed to update mix: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    return true;
}

bool MixDatabase::deleteMix(const std::string& id) {
    if (!db) {
        setError("Database not initialized");
        return false;
    }

    const char* sql = "DELETE FROM mixes WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        setError("Failed to delete mix: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    int rows_affected = sqlite3_changes(db);
    if (rows_affected == 0) {
        setError("No mix found with id: " + id);
        return false;
    }

    return true;
}

Mix MixDatabase::getMixById(const std::string& id) {
    const char* sql = "SELECT * FROM mixes WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return Mix();
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    Mix mix;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        mix = rowToMix(stmt);
    }

    sqlite3_finalize(stmt);
    return mix;
}

std::vector<Mix> MixDatabase::getAllMixes() {
    const char* sql = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY title";
    return executeQueryForMixes(sql);
}

std::vector<Mix> MixDatabase::getMixesByGenre(const std::string& genre) {
    if (!db) {
        setError("Database not initialized");
        return std::vector<Mix>();
    }

    const char* sql =
        "SELECT * FROM mixes WHERE genre COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0 ORDER BY title";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return std::vector<Mix>();
    }

    sqlite3_bind_text(stmt, 1, genre.c_str(), -1, SQLITE_STATIC);

    std::vector<Mix> mixes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        mixes.push_back(rowToMix(stmt));
    }

    sqlite3_finalize(stmt);
    return mixes;
}

std::vector<Mix> MixDatabase::getMixesByArtist(const std::string& artist) {
    if (!db) {
        setError("Database not initialized");
        return std::vector<Mix>();
    }

    const char* sql = "SELECT * FROM mixes WHERE artist = ? AND is_deleted = 0 ORDER BY title";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return std::vector<Mix>();
    }

    sqlite3_bind_text(stmt, 1, artist.c_str(), -1, SQLITE_STATIC);

    std::vector<Mix> mixes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        mixes.push_back(rowToMix(stmt));
    }

    sqlite3_finalize(stmt);
    return mixes;
}

Mix MixDatabase::getRandomMix(const std::string& exclude_mix_id) {
    // First try to get a random mix that's actually downloaded (excluding current mix)
    std::string sql = "SELECT * FROM mixes WHERE local_path IS NOT NULL AND local_path != '' AND is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, 1, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix downloaded_mix;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            downloaded_mix = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);

        if (!downloaded_mix.id.empty()) {
            return downloaded_mix;
        }
    }

    sql = "SELECT * FROM mixes WHERE is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, 1, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix mix;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            mix = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return mix;
    }

    return Mix();
}

Mix MixDatabase::getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre) {
    int total_mixes = 0;
    int favorite_mixes = 0;
    int preferred_genre_mixes = 0;

    std::string count_sql = "SELECT COUNT(*) as total, SUM(CASE WHEN is_favorite = 1 THEN 1 ELSE 0 END) as favorites";
    if (!preferred_genre.empty()) {
        count_sql += ", SUM(CASE WHEN genre COLLATE NOCASE = ? COLLATE NOCASE THEN 1 ELSE 0 END) as preferred";
    }
    count_sql += " FROM mixes WHERE local_path IS NOT NULL AND local_path != '' AND is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        count_sql += " AND id != ?";
    }

    sqlite3_stmt* count_stmt;
    int rc = sqlite3_prepare_v2(db, count_sql.c_str(), -1, &count_stmt, nullptr);
    if (rc == SQLITE_OK) {
        int param_index = 1;
        if (!preferred_genre.empty()) {
            sqlite3_bind_text(count_stmt, param_index++, preferred_genre.c_str(), -1, SQLITE_STATIC);
        }
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(count_stmt, param_index, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        if (sqlite3_step(count_stmt) == SQLITE_ROW) {
            total_mixes = sqlite3_column_int(count_stmt, 0);
            favorite_mixes = sqlite3_column_int(count_stmt, 1);
            if (!preferred_genre.empty()) {
                preferred_genre_mixes = sqlite3_column_int(count_stmt, 2);
            }
        }
    }
    sqlite3_finalize(count_stmt);

    if (total_mixes == 0) {
        return getRandomMix("");
    }

    bool prefer_genre = !preferred_genre.empty() && (preferred_genre_mixes > 0) &&
                        (rand() % 100 < Constants::PREFERRED_GENRE_PROBABILITY);
    bool prefer_favorites =
        !prefer_genre && (favorite_mixes > 0) && (rand() % 100 < Constants::FAVORITE_MIX_PROBABILITY);

    if (prefer_genre) {
        std::string sql = R"(
            SELECT * FROM mixes 
            WHERE genre COLLATE NOCASE = ? COLLATE NOCASE
            AND local_path IS NOT NULL AND local_path != ''
            AND is_deleted = 0
        )";
        if (!exclude_mix_id.empty()) {
            sql += " AND id != ?";
        }
        sql += R"(
            ORDER BY 
                CASE WHEN last_played IS NULL THEN 0 ELSE 1 END,  -- NULL last_played first
                last_played ASC,  -- Oldest first
                play_count ASC,   -- Fewest plays first
                RANDOM()          -- Random tiebreaker
            LIMIT 1
        )";

        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            int param_index = 1;
            sqlite3_bind_text(stmt, param_index++, preferred_genre.c_str(), -1, SQLITE_STATIC);
            if (!exclude_mix_id.empty()) {
                sqlite3_bind_text(stmt, param_index, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
            }
            Mix mix;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                mix = rowToMix(stmt);
            }
            sqlite3_finalize(stmt);
            if (!mix.id.empty()) {
                return mix;
            }
        }
    }

    if (prefer_favorites) {
        // Get a favorite mix with smart prioritization (excluding current, preferring downloaded)
        std::string sql = R"(
            SELECT * FROM mixes 
            WHERE is_favorite = 1
            AND local_path IS NOT NULL AND local_path != ''
            AND is_deleted = 0
        )";
        if (!exclude_mix_id.empty()) {
            sql += " AND id != ?";
        }
        sql += R"(
            ORDER BY 
                CASE WHEN last_played IS NULL THEN 0 ELSE 1 END,  -- NULL last_played first
                last_played ASC,  -- Oldest first
                play_count ASC,   -- Fewest plays first
                RANDOM()          -- Random tiebreaker
            LIMIT 1
        )";

        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            int param_index = 1;
            if (!exclude_mix_id.empty()) {
                sqlite3_bind_text(stmt, param_index, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
            }
            Mix mix;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                mix = rowToMix(stmt);
            }
            sqlite3_finalize(stmt);
            if (!mix.id.empty()) {
                return mix;
            }
        }
    }

    std::string sql = "SELECT * FROM mixes WHERE local_path IS NOT NULL AND local_path != '' AND is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, 1, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix mix;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            mix = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        if (!mix.id.empty()) {
            return mix;
        }
    }

    sql = "SELECT * FROM mixes WHERE is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, 1, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix mix;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            mix = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return mix;
    }

    return Mix();
}

Mix MixDatabase::getNextMix(const std::string& current_mix_id) {
    if (!db) {
        setError("Database not initialized");
        return Mix();
    }

    std::string sql;
    if (current_mix_id.empty()) {
        sql = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1";
    } else {
        sql = "SELECT * FROM mixes WHERE id > ? AND is_deleted = 0 ORDER BY id LIMIT 1";
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return Mix();
    }

    if (!current_mix_id.empty()) {
        sqlite3_bind_text(stmt, 1, current_mix_id.c_str(), -1, SQLITE_STATIC);
    }

    Mix result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToMix(stmt);
    } else {
        sqlite3_finalize(stmt);
        sql = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id LIMIT 1";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = rowToMix(stmt);
            }
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

Mix MixDatabase::getPreviousMix(const std::string& current_mix_id) {
    if (!db) {
        setError("Database not initialized");
        return Mix();
    }

    std::string sql;
    if (current_mix_id.empty()) {
        // Get last mix
        sql = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1";
    } else {
        // Get previous mix before current
        sql = "SELECT * FROM mixes WHERE id < ? AND is_deleted = 0 ORDER BY id DESC LIMIT 1";
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return Mix();
    }

    if (!current_mix_id.empty()) {
        sqlite3_bind_text(stmt, 1, current_mix_id.c_str(), -1, SQLITE_STATIC);
    }

    Mix result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToMix(stmt);
    } else {
        sqlite3_finalize(stmt);
        sql = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY id DESC LIMIT 1";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = rowToMix(stmt);
            }
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

Mix MixDatabase::getRandomMixByGenre(const std::string& genre) {
    const char* sql =
        "SELECT * FROM mixes WHERE genre COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0 ORDER BY RANDOM() LIMIT "
        "1";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, genre.c_str(), -1, SQLITE_STATIC);
        Mix result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    return Mix();
}

Mix MixDatabase::getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id) {
    std::string sql = "SELECT * FROM mixes WHERE genre COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        int param_index = 1;
        sqlite3_bind_text(stmt, param_index++, genre.c_str(), -1, SQLITE_STATIC);
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, param_index, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    return Mix();
}

Mix MixDatabase::getRandomMixByArtist(const std::string& artist) {
    const char* sql =
        "SELECT * FROM mixes WHERE artist COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0 ORDER BY RANDOM() LIMIT "
        "1";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, artist.c_str(), -1, SQLITE_STATIC);
        Mix result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    return Mix();
}

Mix MixDatabase::getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id) {
    std::string sql = "SELECT * FROM mixes WHERE artist COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0";
    if (!exclude_mix_id.empty()) {
        sql += " AND id != ?";
    }
    sql += " ORDER BY RANDOM() LIMIT 1";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        int param_index = 1;
        sqlite3_bind_text(stmt, param_index++, artist.c_str(), -1, SQLITE_STATIC);
        if (!exclude_mix_id.empty()) {
            sqlite3_bind_text(stmt, param_index, exclude_mix_id.c_str(), -1, SQLITE_STATIC);
        }
        Mix result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = rowToMix(stmt);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    return Mix();
}

bool MixDatabase::toggleFavorite(const std::string& mix_id) {
    const char* sql = "UPDATE mixes SET is_favorite = NOT is_favorite WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, mix_id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool MixDatabase::softDeleteMix(const std::string& mix_id) {
    const char* sql = "UPDATE mixes SET is_deleted = 1 WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, mix_id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool MixDatabase::updatePlayStats(const std::string& mix_id) {
    const char* sql = "UPDATE mixes SET play_count = play_count + 1, last_played = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, mix_id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool MixDatabase::setLocalPath(const std::string& mix_id, const std::string& local_path) {
    const char* sql = "UPDATE mixes SET local_path = ? WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, local_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, mix_id.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<Mix> MixDatabase::getDownloadedMixes() {
    const char* sql =
        "SELECT * FROM mixes WHERE local_path IS NOT NULL AND local_path != '' AND is_deleted = 0 ORDER BY title";
    return executeQueryForMixes(sql);
}

std::vector<Mix> MixDatabase::getFavoriteMixes() {
    const char* sql = "SELECT * FROM mixes WHERE is_favorite = 1 AND is_deleted = 0 ORDER BY title";
    return executeQueryForMixes(sql);
}

std::vector<Mix> MixDatabase::getRecentlyPlayed(int limit) {
    const char* sql =
        "SELECT * FROM mixes WHERE last_played IS NOT NULL AND is_deleted = 0 ORDER BY last_played DESC LIMIT ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return std::vector<Mix>();
    }

    sqlite3_bind_int(stmt, 1, limit);

    std::vector<Mix> mixes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        mixes.push_back(rowToMix(stmt));
    }

    sqlite3_finalize(stmt);
    return mixes;
}

Mix MixDatabase::rowToMix(sqlite3_stmt* stmt) {
    Mix mix;

    // Check for null pointers before accessing
    if (sqlite3_column_text(stmt, 0)) {
        mix.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    if (sqlite3_column_text(stmt, 1)) {
        mix.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    if (sqlite3_column_text(stmt, 2)) {
        mix.artist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }
    if (sqlite3_column_text(stmt, 3)) {
        mix.genre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }
    if (sqlite3_column_text(stmt, 4)) {
        mix.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }

    if (sqlite3_column_text(stmt, 5)) {
        mix.local_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    mix.duration_seconds = sqlite3_column_int(stmt, 6);

    if (sqlite3_column_text(stmt, 7)) {
        // Parse tags JSON
        std::string tags_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        if (!tags_json.empty()) {
            try {
                // Simple JSON parsing for tags - extract comma-separated values
                // This is a basic implementation; for full JSON parsing, consider using a library like nlohmann/json
                std::string current_tag;
                for (char c : tags_json) {
                    if (c == '"' || c == '[' || c == ']' || c == '{' || c == '}') {
                        continue;  // Skip JSON syntax characters
                    }
                    if (c == ',' || c == ' ') {
                        if (!current_tag.empty()) {
                            mix.tags.push_back(current_tag);
                            current_tag.clear();
                        }
                    } else {
                        current_tag += c;
                    }
                }
                if (!current_tag.empty()) {
                    mix.tags.push_back(current_tag);
                }
            } catch (const std::exception& e) {
                // If JSON parsing fails, store the raw string
                mix.tags.push_back(tags_json);
            }
        }
    }

    if (sqlite3_column_text(stmt, 8)) {
        mix.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    }

    if (sqlite3_column_text(stmt, 9)) {
        mix.date_added = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
    }

    if (sqlite3_column_text(stmt, 10)) {
        mix.last_played = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
    }

    mix.play_count = sqlite3_column_int(stmt, 11);
    mix.is_favorite = sqlite3_column_int(stmt, 12) != 0;
    mix.is_deleted = sqlite3_column_int(stmt, 13) != 0;

    return mix;
}

std::vector<Mix> MixDatabase::executeQueryForMixes(const char* sql) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return std::vector<Mix>();
    }

    std::vector<Mix> mixes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        try {
            Mix mix = rowToMix(stmt);
            mixes.push_back(mix);
        } catch (const std::exception& e) {
        }
    }

    sqlite3_finalize(stmt);
    return mixes;
}

Mix MixDatabase::executeQueryForSingleMix(const char* sql) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return Mix();
    }

    Mix mix;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        mix = rowToMix(stmt);
    }

    sqlite3_finalize(stmt);
    return mix;
}

bool MixDatabase::executeQuery(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        setError("SQL error: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

}  // namespace Data
}  // namespace AutoVibez