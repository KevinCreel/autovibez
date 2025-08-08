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
constexpr int VOLUME_STEP_SIZE = 10;

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
constexpr int BLANK_CURSOR_SIZE = 4;
constexpr int CURSOR_DIMENSIONS = 1;
constexpr int CURSOR_HOTSPOT = 0;

// Smart selection probabilities (percentages)
constexpr int PREFERRED_GENRE_PROBABILITY = 80;  // 80% chance to prefer genre
constexpr int FAVORITE_MIX_PROBABILITY = 70;     // 70% chance to prefer favorites
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

// Regex patterns
constexpr const char* URL_REGEX_PATTERN = R"((https?|ftp)://[^\s/$.?#].[^\s]*)";
constexpr const char* DATETIME_REGEX_PATTERN = R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})";
}  // namespace StringConstants