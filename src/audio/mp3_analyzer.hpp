#ifndef MP3_ANALYZER_HPP
#define MP3_ANALYZER_HPP

#include <string>
#include <vector>
#include <chrono>
#include "uuid_utils.hpp"
#include "datetime_utils.hpp"
#include "base_metadata.hpp"

namespace AutoVibez {
namespace Audio {

struct MP3Metadata : public Data::BaseMetadata {
    int bitrate = 0;
    int sample_rate = 0;
    int channels = 0;
    long file_size = 0;
    std::string format;
    std::string url;
};

class MP3Analyzer {
public:
    MP3Analyzer();
    ~MP3Analyzer();
    
    MP3Metadata analyzeFile(const std::string& file_path);
    std::string getLastError() const { return last_error; }
    void setVerbose(bool verbose) { _verbose = verbose; }
    
private:
    std::string last_error;
    bool _verbose = false;
};

} // namespace Audio
} // namespace AutoVibez

#endif // MP3_ANALYZER_HPP 