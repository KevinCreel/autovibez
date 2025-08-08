#include "mp3_analyzer.hpp"

#include <SDL2/SDL_mixer.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/taglib.h>
#include <taglib/textidentificationframe.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>

#include "constants.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"
#include "path_utils.hpp"

using AutoVibez::Audio::MP3Analyzer;

namespace AutoVibez {
namespace Audio {

MP3Analyzer::MP3Analyzer() {
    // TagLib will handle MP3 files with stream size issues gracefully
    // The "Xing stream size off" warning is common and doesn't affect functionality
}

MP3Analyzer::~MP3Analyzer() {}

MP3Metadata MP3Analyzer::analyzeFile(const std::string& file_path) {
    MP3Metadata metadata;
    clearError();

    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        setError("File does not exist: " + file_path);
        return metadata;
    }

    // Check if file is empty or too small to be a valid MP3
    std::filesystem::path path(file_path);
    if (std::filesystem::file_size(path) < Constants::MIN_MP3_FILE_SIZE) {
        setError("File is too small to be a valid MP3: " + file_path);
        return metadata;
    }

    // Use TagLib to extract metadata
    TagLib::MPEG::File f(file_path.c_str());
    if (!f.isValid()) {
        setError("Invalid or corrupted MP3 file: " + file_path);
        return metadata;
    }

    // Get audio properties to validate it's actually an MP3
    TagLib::AudioProperties* properties = f.audioProperties();
    if (!properties) {
        setError("Unable to read audio properties from file: " + file_path);
        return metadata;
    }

    // Basic validation that this is actually an MP3
    if (properties->length() <= 0) {
        setError("Invalid MP3 file - no audio data found: " + file_path);
        return metadata;
    }

    // Extract audio properties for duration and format info
    metadata.duration_seconds = properties->length();
    metadata.bitrate = properties->bitrate();
    metadata.sample_rate = properties->sampleRate();
    metadata.channels = properties->channels();

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
        std::string filename = AutoVibez::Utils::PathUtils::getFilenameWithoutExtension(file_path);
        metadata.title = filename;
        metadata.description = filename;
    }

    if (metadata.artist.empty()) {
        metadata.artist = StringConstants::UNKNOWN_ARTIST;
    }

    if (metadata.genre.empty()) {
        metadata.genre = StringConstants::DEFAULT_GENRE;
    }

    // Get file size
    metadata.file_size = std::filesystem::file_size(file_path);

    metadata.format = StringConstants::MP3_FORMAT;
    metadata.date_added = AutoVibez::Utils::DateTimeUtils::getCurrentDateTime();

    return metadata;
}

}  // namespace Audio
}  // namespace AutoVibez