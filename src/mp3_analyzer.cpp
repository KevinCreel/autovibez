#include "mp3_analyzer.hpp"
#include <SDL2/SDL_mixer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <functional>
#include <random>
#include <taglib/taglib.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/textidentificationframe.h>

MP3Analyzer::MP3Analyzer() {
    last_error.clear();
    
    // TagLib will handle MP3 files with stream size issues gracefully
    // The "Xing stream size off" warning is common and doesn't affect functionality
}

MP3Analyzer::~MP3Analyzer() {
}

std::string MP3Analyzer::generateIdFromFilename(const std::string& file_path) {
    // Generate a deterministic UUID based on filename hash
    std::hash<std::string> hasher;
    size_t hash = hasher(file_path);
    
    // Use the hash to generate a deterministic UUID v5 (name-based)
    unsigned char uuid_bytes[16];
    
    // Use first 16 bytes of hash (or repeat if shorter)
    for (int i = 0; i < 16; i++) {
        uuid_bytes[i] = (hash >> (i % 8 * 8)) & 0xFF;
    }
    
    // Set version (5) and variant bits for deterministic UUID
    uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x50;  // Version 5
    uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80;  // Variant 1
    
    // Convert to UUID string format
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            ss << "-";
        }
        ss << std::setw(2) << static_cast<int>(uuid_bytes[i]);
    }
    
    return ss.str();
}

std::string MP3Analyzer::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

MP3Metadata MP3Analyzer::analyzeFile(const std::string& file_path) {
    MP3Metadata metadata;
    metadata.id = generateIdFromFilename(file_path);
    
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
#else
        // On Unix-like systems, redirect stderr to /dev/null using freopen
        original_stderr_file = freopen("/dev/null", "w", stderr);
        if (!original_stderr_file) {
            // Fallback to stream redirection
            null_stream.open("/dev/null");
            if (null_stream.is_open()) {
                original_stderr = std::cerr.rdbuf();
                std::cerr.rdbuf(null_stream.rdbuf());
            }
        }
        if (!original_stderr_file) {
            // Fallback to stream redirection
            null_stream.open("/dev/null");
            if (null_stream.is_open()) {
                original_stderr = std::cerr.rdbuf();
                std::cerr.rdbuf(null_stream.rdbuf());
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
    metadata.date_added = getCurrentDateTime();
    
    // Restore original error state if no new errors occurred
    if (last_error.empty()) {
        last_error = original_error;
    }
    
    // Restore stderr
    if (!_verbose) {
        if (original_stderr_file) {
            freopen("/dev/stderr", "w", stderr);
        } else if (original_stderr) {
            std::cerr.rdbuf(original_stderr);
            null_stream.close();
        }
    }
    
    return metadata;
} 