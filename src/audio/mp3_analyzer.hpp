#ifndef MP3_ANALYZER_HPP
#define MP3_ANALYZER_HPP

#include <string>
#include <vector>
#include <chrono>

namespace AutoVibez {
namespace Audio {

struct MP3Metadata {
    std::string id;
    std::string title;
    std::string artist;
    std::string genre;
    std::string description;
    std::vector<std::string> tags;
    std::string url;
    std::string local_path;
    int duration_seconds = 0;
    int bitrate = 0;
    int sample_rate = 0;
    int channels = 0;
    long file_size = 0;
    std::string format;
    std::string date_added;
    std::string last_played;
    int play_count = 0;
    bool is_favorite = false;
};

class MP3Analyzer {
public:
    MP3Analyzer();
    ~MP3Analyzer();
    
    MP3Metadata analyzeFile(const std::string& file_path);
    std::string generateIdFromFilename(const std::string& file_path);
    std::string getCurrentDateTime();
    std::string getLastError() const { return last_error; }
    void setVerbose(bool verbose) { _verbose = verbose; }
    
private:
    std::string last_error;
    bool _verbose = false;
};

} // namespace Audio
} // namespace AutoVibez

#endif // MP3_ANALYZER_HPP 