#pragma once

namespace Constants {
// Audio
constexpr int DEFAULT_SAMPLE_RATE = 44100;
constexpr int DEFAULT_CHANNELS = 2;
constexpr int DEFAULT_SAMPLES = 512;
constexpr int DEFAULT_BUFFER_SIZE = 2048;
constexpr int MAX_VOLUME = 100;
constexpr int MIN_VOLUME = 0;
constexpr int BITS_PER_SAMPLE = 16;
constexpr int ID3V2_HEADER_SIZE = 10;

// Beat sensitivity

// File validation
constexpr int MIN_MP3_FILE_SIZE = 1024;  // Minimum valid MP3 file size in bytes

// Crossfade
constexpr int DEFAULT_CROSSFADE_DURATION_MS = 3000;

// Database
constexpr int MAX_RETRIES = 3;
constexpr int DEFAULT_TIMEOUT_SECONDS = 30;

// Download
constexpr int MIN_DOWNLOAD_SPEED_BYTES_PER_SEC = 1000;  // 1KB/s minimum
constexpr int DOWNLOAD_TIMEOUT_SECONDS = 300;           // 5 minutes timeout
constexpr int DOWNLOAD_LOW_SPEED_TIME_SECONDS = 60;     // 60 seconds
constexpr int MAX_FILENAME_LENGTH = 200;

// UUID
constexpr int UUID_BYTE_LENGTH = 16;
constexpr int UUID_POSITION_1 = 4;
constexpr int UUID_POSITION_2 = 6;
constexpr int UUID_POSITION_3 = 8;
constexpr int UUID_POSITION_4 = 10;

// Time/Duration
constexpr int SECONDS_PER_MINUTE = 60;

// HTTP/Network
constexpr int HTTP_TIMEOUT_SECONDS = 30;
constexpr int HTTP_CONNECT_TIMEOUT_SECONDS = 10;

// Volume control
constexpr int VOLUME_STEP_SIZE = 1;

// UUID version constants
constexpr int UUID_VERSION_5 = 0x50;
constexpr int UUID_VARIANT_1 = 0x80;
constexpr int UUID_VERSION_MASK = 0x0F;
constexpr int UUID_VARIANT_MASK = 0x3F;

// Time formatting
constexpr int TIME_FORMAT_PADDING = 10;  // For "0" padding in time display
constexpr int PROGRESS_PERCENTAGE_MAX = 100;

// Audio/Media
constexpr int HEX_BASE = 16;
constexpr int HEX_ALPHA_OFFSET = 10;
constexpr int MIN_RESPONSE_LENGTH = 10;
constexpr int MIN_YAML_RESPONSE_LENGTH = 50;  // Minimum valid YAML file size

// SDL_mixer constants
constexpr int SDL_MIXER_MAX_VOLUME = 128;  // SDL_mixer's MIX_MAX_VOLUME

// UI/Window
constexpr int DEFAULT_WINDOW_SIZE = 512;
constexpr int DEFAULT_MESH_X = 32;
constexpr int DEFAULT_MESH_Y = 24;
constexpr int DEFAULT_PRESET_DURATION = 30;
constexpr int DEFAULT_HARD_CUT_DURATION = 60;
constexpr int DEFAULT_FPS_VALUE = 60;
constexpr int DEFAULT_CHECK_INTERVAL_MS = 5000;

// UI/Display
constexpr int FRAME_DELAY_MS = 1000 / DEFAULT_FPS_VALUE;  // ~16.67ms
constexpr float UI_PADDING = 40.0f;
constexpr float HELP_OVERLAY_ALPHA = 0.7f;  // Help overlay transparency (0.0 = fully transparent, 1.0 = opaque)
constexpr int BLANK_CURSOR_SIZE = 4;
constexpr int CURSOR_DIMENSIONS = 1;
constexpr int CURSOR_HOTSPOT = 0;

// Smart selection probabilities (percentages)
constexpr int PREFERRED_GENRE_PROBABILITY = 80;  // 80% chance to prefer genre
constexpr int FAVORITE_MIX_PROBABILITY = 70;     // 70% chance to prefer favorites

// DatabaseColumns constants removed - now using column name-based access
}  // namespace Constants

// String constants
namespace StringConstants {
// File extensions
constexpr const char* MP3_EXTENSION = ".mp3";

// Protocols
constexpr const char* FILE_PROTOCOL = "file://";

// Error messages
constexpr const char* UNKNOWN_ARTIST = "Unknown Artist";
constexpr const char* UNKNOWN_TRACK = "Unknown Track";
constexpr const char* DEFAULT_GENRE = "Electronic";
constexpr const char* MP3_FORMAT = "MP3";

// MixDownloader error messages
constexpr const char* EMPTY_URL_ERROR = "Mix URL is empty";
constexpr const char* INVALID_URL_ERROR = "Invalid URL format";
constexpr const char* CURL_INIT_ERROR = "Failed to initialize CURL";
constexpr const char* CURL_DOWNLOAD_ERROR = "Download failed";
constexpr const char* FILE_CREATE_ERROR = "Failed to create local file";
constexpr const char* FILE_COPY_ERROR = "Failed to copy local file";
constexpr const char* MP3_ANALYZER_REQUIRED_ERROR = "MP3Analyzer is required for title-based naming";
constexpr const char* INVALID_MIX_ID_ERROR = "Invalid mix ID format";

// UI strings
constexpr const char* APP_TITLE = "AutoVibez";
constexpr const char* LOCKED_INDICATOR = " [locked]";
constexpr const char* DEFAULT_AUDIO_DEVICE = "Default Device";

// Configuration keys
constexpr const char* CROSSFADE_DURATION_KEY = "crossfade_duration_ms";

// Configuration keys
constexpr const char* MESH_X_KEY = "Mesh X";
constexpr const char* MESH_Y_KEY = "Mesh Y";
constexpr const char* WINDOW_WIDTH_KEY = "Window Width";
constexpr const char* WINDOW_HEIGHT_KEY = "Window Height";
constexpr const char* PRESET_DURATION_KEY = "Preset Duration";
constexpr const char* HARD_CUT_DURATION_KEY = "Hard Cut Duration";
constexpr const char* FPS_KEY = "FPS";

// Time formatting
constexpr const char* TIME_SEPARATOR = ":";
constexpr const char* TIME_PADDING = "0";

// Date/Time formats
constexpr const char* DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S";
constexpr const char* DATE_FORMAT = "%Y-%m-%d";
constexpr const char* TIME_FORMAT = "%H:%M:%S";

// Database SQL queries
constexpr const char* CREATE_MIXES_TABLE = R"(
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

constexpr const char* ALTER_ADD_IS_DELETED = "ALTER TABLE mixes ADD COLUMN is_deleted BOOLEAN DEFAULT 0;";

constexpr const char* INSERT_OR_REPLACE_MIX = R"(
    INSERT OR REPLACE INTO mixes 
    (id, title, artist, genre, url, local_path, duration_seconds, tags, description, date_added, last_played, play_count, is_favorite, is_deleted)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
)";

constexpr const char* UPDATE_MIX = R"(
    UPDATE mixes SET title = ?, artist = ?, genre = ?, url = ?, local_path = ?, duration_seconds = ?, tags = ?, 
    description = ?, date_added = ?, last_played = ?, play_count = ?, is_favorite = ?, is_deleted = ? WHERE id = ?
)";

constexpr const char* SELECT_MIX_BY_ID = "SELECT * FROM mixes WHERE id = ?";
constexpr const char* SELECT_ALL_MIXES = "SELECT * FROM mixes WHERE is_deleted = 0 ORDER BY title";
constexpr const char* SELECT_MIXES_BY_GENRE =
    "SELECT * FROM mixes WHERE genre COLLATE NOCASE = ? COLLATE NOCASE AND is_deleted = 0 ORDER BY title";
constexpr const char* SELECT_MIXES_BY_ARTIST = "SELECT * FROM mixes WHERE artist = ? AND is_deleted = 0 ORDER BY title";
constexpr const char* SELECT_DOWNLOADED_MIXES =
    "SELECT * FROM mixes WHERE local_path IS NOT NULL AND local_path != '' AND is_deleted = 0 ORDER BY title";
constexpr const char* SELECT_FAVORITE_MIXES =
    "SELECT * FROM mixes WHERE is_favorite = 1 AND is_deleted = 0 ORDER BY title";
constexpr const char* SELECT_RECENTLY_PLAYED =
    "SELECT * FROM mixes WHERE last_played IS NOT NULL AND is_deleted = 0 ORDER BY last_played DESC LIMIT ?";

constexpr const char* DELETE_MIX = "DELETE FROM mixes WHERE id = ?";
constexpr const char* SOFT_DELETE_MIX = "UPDATE mixes SET is_deleted = 1 WHERE id = ?";
constexpr const char* TOGGLE_FAVORITE = "UPDATE mixes SET is_favorite = NOT is_favorite WHERE id = ?";
constexpr const char* UPDATE_PLAY_STATS =
    "UPDATE mixes SET play_count = play_count + 1, last_played = CURRENT_TIMESTAMP WHERE id = ?";
constexpr const char* SET_LOCAL_PATH = "UPDATE mixes SET local_path = ? WHERE id = ?";

// Regex patterns
constexpr const char* URL_REGEX_PATTERN = R"((https?|ftp)://[^\s/$.?#].[^\s]*)";
constexpr const char* DATETIME_REGEX_PATTERN = R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})";
}  // namespace StringConstants