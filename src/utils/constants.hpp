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
constexpr float MIN_BEAT_SENSITIVITY = 0.0f;
constexpr float MAX_BEAT_SENSITIVITY = 5.0f;
constexpr float BEAT_SENSITIVITY_STEP = 0.1f;

// File validation
constexpr int MIN_MP3_FILE_SIZE = 1024;  // Minimum valid MP3 file size in bytes

// Crossfade
constexpr int DEFAULT_CROSSFADE_DURATION_MS = 3000;
constexpr int MAX_CROSSFADE_DURATION_MS = 10000;
constexpr int MIN_CROSSFADE_DURATION_MS = 500;

// Database
constexpr int MAX_RETRIES = 3;
constexpr int DEFAULT_TIMEOUT_SECONDS = 30;
constexpr int MAX_DOWNLOAD_RETRIES = 5;

// Download
constexpr int MIN_DOWNLOAD_SPEED_BYTES_PER_SEC = 1000;  // 1KB/s minimum
constexpr int DEFAULT_DOWNLOAD_TIMEOUT_SECONDS = 60;
constexpr int DOWNLOAD_TIMEOUT_SECONDS = 300;  // 5 minutes timeout
constexpr int DOWNLOAD_LOW_SPEED_TIME_SECONDS = 60;  // 60 seconds
constexpr int MAX_FILENAME_LENGTH = 200;

// UUID
constexpr int UUID_BYTE_LENGTH = 16;
constexpr int UUID_FORMAT_POSITIONS[] = {4, 6, 8, 10};

// Time/Duration
constexpr int SECONDS_PER_HOUR = 3600;
constexpr int SECONDS_PER_MINUTE = 60;
constexpr int MINUTES_PER_HOUR = 60;
constexpr int MINUTES_PER_SECOND = 60;
constexpr int HOURS_PER_DAY = 24;
constexpr int DAYS_PER_WEEK = 7;

// Time formatting
constexpr int TIME_FORMAT_PADDING = 10;  // For "0" padding in time display
constexpr int PROGRESS_PERCENTAGE_MAX = 100;

// Database
constexpr int SQLITE_BIND_START_INDEX = 1;
constexpr int SQLITE_COLUMN_START_INDEX = 0;
constexpr int SQLITE_TEXT_BINDING = -1;
constexpr int SQLITE_STATIC_BINDING = SQLITE_STATIC;

// Audio/Media
constexpr int HEX_BASE = 16;
constexpr int HEX_ALPHA_OFFSET = 10;
constexpr int BITS_PER_BYTE = 8;
constexpr int MIN_RESPONSE_LENGTH = 10;

// UI/Window
constexpr int DEFAULT_WINDOW_SIZE = 512;
constexpr int DEFAULT_MESH_X = 32;
constexpr int DEFAULT_MESH_Y = 24;
constexpr int DEFAULT_PRESET_DURATION = 30;
constexpr int DEFAULT_HARD_CUT_DURATION = 60;
constexpr int DEFAULT_FPS_VALUE = 60;
constexpr int DEFAULT_CHECK_INTERVAL_MS = 5000;

// UUID formatting positions
constexpr int UUID_POSITION_1 = 4;
constexpr int UUID_POSITION_2 = 6;
constexpr int UUID_POSITION_3 = 8;
constexpr int UUID_POSITION_4 = 10;

// UI/Display
constexpr int DEFAULT_WINDOW_WIDTH = 1024;
constexpr int DEFAULT_WINDOW_HEIGHT = 768;
constexpr int DEFAULT_FPS = 60;
constexpr int FRAME_DELAY_MS = 1000 / DEFAULT_FPS;  // ~16.67ms
constexpr float UI_PADDING = 40.0f;
constexpr int BLANK_CURSOR_SIZE = 4;
constexpr int CURSOR_DIMENSIONS = 1;
constexpr int CURSOR_HOTSPOT = 0;

// Smart selection probabilities (percentages)
constexpr int PREFERRED_GENRE_PROBABILITY = 80;  // 80% chance to prefer genre
constexpr int FAVORITE_MIX_PROBABILITY = 70;     // 70% chance to prefer favorites

// Configuration defaults
constexpr int DEFAULT_SEEK_INCREMENT_SECONDS = 60;
constexpr int DEFAULT_VOLUME_STEP_PERCENT = 10;
constexpr int DEFAULT_CACHE_SIZE_MB = 100;

// Test constants
constexpr int TEST_ITERATIONS = 1000;
constexpr int TEST_SLEEP_MS = 100;
constexpr int TEST_CACHE_SIZE_MB = 100;
}  // namespace Constants

// String constants
namespace StringConstants {
// File extensions
constexpr const char* MP3_EXTENSION = ".mp3";
constexpr const char* MILK_EXTENSION = ".milk";
constexpr const char* JPG_EXTENSION = ".jpg";
constexpr const char* PNG_EXTENSION = ".png";
constexpr const char* DB_EXTENSION = ".db";
constexpr const char* YAML_EXTENSION = ".yaml";
constexpr const char* JSON_EXTENSION = ".json";

// Protocols
constexpr const char* HTTP_PROTOCOL = "http://";
constexpr const char* HTTPS_PROTOCOL = "https://";
constexpr const char* FILE_PROTOCOL = "file://";
constexpr const char* FTP_PROTOCOL = "ftp://";

// Error messages
constexpr const char* UNKNOWN_ARTIST = "Unknown Artist";
constexpr const char* UNKNOWN_TRACK = "Unknown Track";
constexpr const char* DEFAULT_GENRE = "Electronic";
constexpr const char* MP3_FORMAT = "MP3";
constexpr const char* ID3_TAG = "ID3";

// UI strings
constexpr const char* APP_TITLE = "AutoVibez";
constexpr const char* LOCKED_INDICATOR = " [locked]";
constexpr const char* DEFAULT_AUDIO_DEVICE = "Default Device";

// Configuration keys
constexpr const char* CROSSFADE_DURATION_KEY = "crossfade_duration_ms";
constexpr const char* CROSSFADE_ENABLED_KEY = "crossfade_enabled";
constexpr const char* PREFERRED_GENRE_KEY = "preferred_genre";
constexpr const char* MIXES_URL_KEY = "mixes_url";

// System paths
constexpr const char* DEV_NULL = "/dev/null";
constexpr const char* DEV_STDERR = "/dev/stderr";
constexpr const char* PROGRAM_DATA_PATH = "C:/ProgramData";

// Date/Time formats
constexpr const char* DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S";
constexpr const char* DATE_FORMAT = "%Y-%m-%d";
constexpr const char* TIME_FORMAT = "%H:%M:%S";

// Regex patterns
constexpr const char* URL_REGEX_PATTERN = R"((https?|ftp)://[^\s/$.?#].[^\s]*)";
constexpr const char* DATETIME_REGEX_PATTERN = R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})";
}  // namespace StringConstants