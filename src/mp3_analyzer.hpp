#ifndef MP3_ANALYZER_HPP
#define MP3_ANALYZER_HPP

#include <string>
#include <vector>
#include <chrono>

struct MP3Metadata {
    std::string id;
    std::string title;
    std::string artist;
    std::string genre;
    std::string description;
    std::vector<std::string> tags;
    std::string url;
    std::string local_path;
    int duration_seconds;
    int bitrate;
    int sample_rate;
    int channels;
    long file_size;
    std::string format;
    std::string date_added;
    std::string last_played;
    int play_count;

    bool is_favorite;
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

#endif // MP3_ANALYZER_HPP 