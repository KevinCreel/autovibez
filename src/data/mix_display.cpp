#include "mix_display.hpp"
#include "console_output.hpp"
#include <sstream>
#include <iomanip>

MixDisplay::MixDisplay() {
}

MixDisplay::~MixDisplay() {
}

void MixDisplay::displayMixInfo(const Mix& mix) {
    const char* color_cyan = "\033[36m";
    const char* color_green = "\033[32m";
    const char* color_yellow = "\033[33m";
    const char* color_reset = "\033[0m";
    
    // SUPER SEXY "Now playing" message with colors and emojis!
    const char* color_purple = "\033[35m";
    // const char* color_blue = "\033[34m"; // Unused variable
    
    ConsoleOutput::output("\n");
    ConsoleOutput::output("%s🎵 %sNow playing:%s %s%s%s", color_purple, color_green, color_reset, color_yellow, mix.title.c_str(), color_reset);
    
    ConsoleOutput::output("%s%s🎧 Artist:%s %s%s%s", color_reset, color_cyan, color_reset, color_yellow, mix.artist.c_str(), color_reset);
    ConsoleOutput::output("%s%s🎼 Genre:%s %s%s%s", color_reset, color_cyan, color_reset, color_yellow, mix.genre.c_str(), color_reset);
    ConsoleOutput::output("%s%s⏱️  Duration:%s %s%s%s", color_reset, color_cyan, color_reset, color_yellow, formatTime(mix.duration_seconds).c_str(), color_reset);
    ConsoleOutput::output("%s%s📅 Added:%s %s%s%s", color_reset, color_cyan, color_reset, color_yellow, mix.date_added.c_str(), color_reset);
    ConsoleOutput::output("%s%s🎯 Plays:%s %s%d%s", color_reset, color_cyan, color_reset, color_yellow, mix.play_count, color_reset);
    ConsoleOutput::output("%s%s❤️  Favorite:%s %s%s%s", color_reset, color_cyan, color_reset, color_yellow, mix.is_favorite ? "Yes" : "No", color_reset);
}

void MixDisplay::displayPlaybackStatus(const Mix& mix, int position, int duration, int volume) {
    const char* color_cyan = "\033[36m";
    const char* color_green = "\033[32m";
    const char* color_yellow = "\033[33m";
    const char* color_reset = "\033[0m";
    
    ConsoleOutput::output("\n");
    ConsoleOutput::output("%s▶️  %sNow Playing:%s %s%s%s", color_cyan, color_green, color_reset, color_yellow, mix.title.c_str(), color_reset);
    ConsoleOutput::output("%s⏱️  %sPosition:%s %s%s / %s%s", color_cyan, color_green, color_reset, color_yellow, formatTime(position).c_str(), formatTime(duration).c_str(), color_reset);
    ConsoleOutput::output("%s🔊 %sVolume:%s %s%d%%%s", color_cyan, color_green, color_reset, color_yellow, volume, color_reset);
}

void MixDisplay::displayDownloadProgress(const Mix& mix, int progress, size_t downloaded_bytes, size_t total_bytes) {
    const char* color_cyan = "\033[36m";
    const char* color_green = "\033[32m";
    const char* color_yellow = "\033[33m";
    const char* color_reset = "\033[0m";
    
    ConsoleOutput::output("\n");
    ConsoleOutput::output("%s📥 %sDownloading:%s %s%s%s", color_cyan, color_green, color_reset, color_yellow, mix.title.c_str(), color_reset);
    ConsoleOutput::output("%s⏳ %sProgress:%s [%s%s%s] %d%% (%s / %s)", 
           color_cyan, color_green, color_reset, 
           color_yellow, std::string(progress/5, '#').c_str(), color_reset,
           progress, formatFileSize(downloaded_bytes).c_str(), formatFileSize(total_bytes).c_str());
}

void MixDisplay::displayCacheStatus(size_t used_mb, size_t total_mb, int mix_count) {
    const char* color_cyan = "\033[36m";
    const char* color_green = "\033[32m";
    const char* color_yellow = "\033[33m";
    const char* color_reset = "\033[0m";
    
    ConsoleOutput::output("\n");
    ConsoleOutput::output("%s💾 %sCache Status:%s %s%zu MB%s / %s%zu MB%s (%d mixes)", 
           color_cyan, color_green, color_reset, 
           color_yellow, used_mb, color_reset, color_yellow, total_mb, color_reset, mix_count);
}

std::string MixDisplay::formatTime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int remaining_seconds = seconds % 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << ":" 
            << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << remaining_seconds;
    } else {
        oss << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << remaining_seconds;
    }
    return oss.str();
}

std::string MixDisplay::formatFileSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit_index < 3) {
        size /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit_index];
    return oss.str();
} 