#include "mp3_analyzer.hpp"

#include <SDL2/SDL_mixer.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/taglib.h>
#include <taglib/textidentificationframe.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#include "constants.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"

using AutoVibez::Audio::MP3Analyzer;

namespace AutoVibez {
namespace Audio {

MP3Analyzer::MP3Analyzer() {
    last_error.clear();

    // TagLib will handle MP3 files with stream size issues gracefully
    // The "Xing stream size off" warning is common and doesn't affect functionality
}

MP3Analyzer::~MP3Analyzer() {}

MP3Metadata MP3Analyzer::analyzeFile(const std::string& file_path) {
    MP3Metadata metadata;
    // Note: ID should be set by caller based on URL, not generated from file path

    // Suppress TagLib warnings for this analysis
    // The "Xing stream size off" warning is common and doesn't affect functionality
    std::string original_error = last_error;
    last_error.clear();

    // Temporarily redirect stderr to suppress TagLib warnings
    std::streambuf* original_stderr = nullptr;
    std::ofstream null_stream;
    FILE* original_stderr_file = nullptr;

    if (!_verbose) {
#ifdef _WIN32
        // On Windows, we can't easily redirect stderr, so we'll just proceed
        // without redirecting (the library will handle its own output)
#else
        // On Unix-like systems, redirect stderr to /dev/null using freopen
        original_stderr_file = freopen("/dev/null", "w", stderr);
        if (!original_stderr_file) {
            // If freopen fails, try using a null stream
            null_stream.open("/dev/null");
            if (!null_stream.is_open()) {
                // Last resort: just proceed without redirecting
            }
        }
#endif
    }

    // Use TagLib to extract metadata
    TagLib::MPEG::File f(file_path.c_str());
    if (!f.isValid()) {
        last_error = "Failed to open MP3 file: " + file_path;
        return metadata;
    }

    // Get ID3v2 tag (preferred)
    TagLib::ID3v2::Tag* tag = f.ID3v2Tag();
    if (tag) {
        // Extract title
        if (!tag->title().isEmpty()) {
            metadata.title = tag->title().to8Bit();
        }

        // Extract artist
        if (!tag->artist().isEmpty()) {
            metadata.artist = tag->artist().to8Bit();
        }

        // Extract genre
        if (!tag->genre().isEmpty()) {
            metadata.genre = tag->genre().to8Bit();
        }

        // Extract description (use title if no description)
        if (!tag->comment().isEmpty()) {
            metadata.description = tag->comment().to8Bit();
        } else if (!metadata.title.empty()) {
            metadata.description = metadata.title;
        }

        // Extract tags (use genre as tag)
        if (!metadata.genre.empty()) {
            metadata.tags.push_back(metadata.genre);
        }
    }

    // Fallback to ID3v1 if ID3v2 is not available
    if (metadata.title.empty() && metadata.artist.empty()) {
        TagLib::Tag* v1tag = f.tag();
        if (v1tag) {
            if (!v1tag->title().isEmpty()) {
                metadata.title = v1tag->title().to8Bit();
            }
            if (!v1tag->artist().isEmpty()) {
                metadata.artist = v1tag->artist().to8Bit();
            }
            if (!v1tag->genre().isEmpty()) {
                metadata.genre = v1tag->genre().to8Bit();
            }
            if (!v1tag->comment().isEmpty()) {
                metadata.description = v1tag->comment().to8Bit();
            } else if (!metadata.title.empty()) {
                metadata.description = metadata.title;
            }
            if (!metadata.genre.empty()) {
                metadata.tags.push_back(metadata.genre);
            }
        }
    }

    // Fallback to filename if no metadata found
    if (metadata.title.empty()) {
        std::filesystem::path path(file_path);
        std::string filename = path.stem().string();
        metadata.title = filename;
        metadata.description = filename;
    }

    if (metadata.artist.empty()) {
        metadata.artist = "Unknown Artist";
    }

    if (metadata.genre.empty()) {
        metadata.genre = "Electronic";
    }

    // Get audio properties for duration and format info
    TagLib::AudioProperties* properties = f.audioProperties();
    if (properties) {
        metadata.duration_seconds = properties->length();
        metadata.bitrate = properties->bitrate();
        metadata.sample_rate = properties->sampleRate();
        metadata.channels = properties->channels();
    }

    // Get file size
    std::filesystem::path path(file_path);
    metadata.file_size = std::filesystem::file_size(path);

    metadata.format = "MP3";
    metadata.date_added = AutoVibez::Utils::DateTimeUtils::getCurrentDateTime();

    // Restore original error state if no new errors occurred
    if (last_error.empty()) {
        last_error = original_error;
    }

    // Restore stderr
#ifdef _WIN32
    // On Windows, we don't need to restore stderr since we didn't redirect it
#else
    if (original_stderr_file) {
        freopen("/dev/stderr", "w", stderr);
    } else if (null_stream.is_open()) {
        std::cerr.rdbuf(original_stderr);
        null_stream.close();
    }
#endif

    return metadata;
}

}  // namespace Audio
}  // namespace AutoVibez