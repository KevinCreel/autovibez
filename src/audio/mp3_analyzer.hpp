#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "base_metadata.hpp"
#include "datetime_utils.hpp"
#include "error_handler.hpp"
#include "uuid_utils.hpp"

namespace AutoVibez::Audio {

struct MP3Metadata : public Data::BaseMetadata {
    int bitrate = 0;
    int sample_rate = 0;
    int channels = 0;
    long file_size = 0;
    std::string format;
    std::string url;
};

class MP3Analyzer : public ::AutoVibez::Utils::ErrorHandler {
public:
    MP3Analyzer();
    ~MP3Analyzer();

    MP3Metadata analyzeFile(const std::string& file_path);
    std::string getLastError() const {
        return last_error;
    }
    void setVerbose(bool verbose) {
        _verbose = verbose;
    }

private:
    bool _verbose = false;
};

}  // namespace AutoVibez::Audio