#pragma once

#include "mix_metadata.hpp"
#include "mix_database.hpp"
#include "mix_downloader.hpp"
#include "mix_player.hpp"
#include "mp3_analyzer.hpp"
#include <future>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace AutoVibez {
namespace Data {

// Callback function type for first mix added
using FirstMixAddedCallback = std::function<void(const Mix&)>;

/**
 * @brief Main orchestrator for mix management functionality
 */
class MixManager {
public:
    MixManager(const std::string& db_path, const std::string& data_dir);
    ~MixManager();
    
    // Core functionality
    bool initialize();
    bool loadMixMetadata(const std::string& yaml_url);
    bool checkForNewMixes(const std::string& yaml_url);
    bool downloadAndAnalyzeMix(const Mix& mix);
    void syncMixesWithDatabase(const std::vector<Mix>& mixes);
    
    // Callback for first mix added
    void setFirstMixAddedCallback(FirstMixAddedCallback callback) { _first_mix_callback = callback; }
    
    // Mix retrieval - direct database access
    Mix getMixById(const std::string& id);
    std::vector<Mix> getAllMixes();
    std::vector<Mix> getMixesByGenre(const std::string& genre);
    std::vector<Mix> getMixesByArtist(const std::string& artist);
    std::vector<Mix> getDownloadedMixes();
    std::vector<Mix> getFavoriteMixes();
    
    // High-level mix selection with smart logic
    Mix getSmartRandomMix();
    Mix getSmartRandomMix(const std::string& exclude_mix_id);
    Mix getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre);
    
    // Convenience methods for common operations
    Mix getRandomMix();
    Mix getRandomMix(const std::string& exclude_mix_id);
    Mix getRandomMixByGenre(const std::string& genre);
    Mix getRandomMixByGenre(const std::string& genre, const std::string& exclude_mix_id);
    Mix getRandomMixByArtist(const std::string& artist);
    Mix getRandomMixByArtist(const std::string& artist, const std::string& exclude_mix_id);
    Mix getRandomFavoriteMix();
    Mix getRandomFavoriteMix(const std::string& exclude_mix_id);
    Mix getNextMix(const std::string& current_mix_id = "");
    Mix getPreviousMix(const std::string& current_mix_id = "");
    
    // Audio control methods
    bool setVolume(int volume);
    bool setVolume(int volume, bool suppress_output);
    int getVolume() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool hasFinished();
    int getCurrentPosition() const;
    int getDuration() const;
    
    // User data update methods
    bool toggleFavorite(const std::string& mix_id);
    bool updatePlayStats(const std::string& mix_id);
    bool setLocalPath(const std::string& mix_id, const std::string& local_path);
    
    // Available mixes (not yet downloaded)
    Mix getRandomAvailableMix();
    Mix getRandomAvailableMix(const std::string& exclude_mix_id);
    Mix getRandomAvailableMixByGenre(const std::string& genre);
    Mix getRandomAvailableMixByGenre(const std::string& genre, const std::string& exclude_mix_id);
    std::vector<Mix> getAvailableMixes();
    

    
    // Audio functionality
    bool downloadAndPlayMix(const Mix& mix);
    bool playMix(const Mix& mix);
    bool togglePause();
    /**
     * @brief Stop playback
     * @return True if successful, false otherwise
     */
    bool stop();
    

    

    
    // Crossfade functionality
    bool startCrossfade(const Mix& new_mix, int crossfade_duration_ms = 3000);
    void updateCrossfade();
    bool isCrossfading() const { return _crossfade_active; }
    int getCrossfadeProgress() const { return _crossfade_progress; }
    void setCrossfadeEnabled(bool enabled) { _crossfade_enabled = enabled; }
    bool isCrossfadeEnabled() const { return _crossfade_enabled; }
    void setCrossfadeDuration(int duration_ms) { _crossfade_duration_ms = duration_ms; }
    int getCrossfadeDuration() const { return _crossfade_duration_ms; }
    
    // Mix files management
    bool clearMixFiles();
    size_t getMixFilesSize() const;
    bool cleanupCorruptedMixFiles();
    bool cleanupMissingFiles(); // New method to remove database entries for missing files
    bool validateDatabaseFileConsistency(); // New method to check database-file consistency
    
    // Background downloads
    bool downloadMixBackground(const Mix& mix);
    void cleanupCompletedDownloads();
    bool cleanupInconsistentIds();
    bool downloadMissingMixesBackground(); // New method to download missing mixes
    
    /**
     * @brief Get the last error message
     * @return Last error message
     */
    std::string getLastError() const { return last_error; }
    
    /**
     * @brief Check if the last operation was successful
     * @return True if successful, false otherwise
     */
    bool isSuccess() const { return success; }
    
    // Test access methods
    AutoVibez::Data::MixDatabase* getDatabase() { return database.get(); }
    AutoVibez::Data::MixDownloader* getDownloader() { return downloader.get(); }

    // Mix retrieval by genre
    std::vector<std::string> getAvailableGenres();
    std::string getCurrentGenre() const;
    void setCurrentGenre(const std::string& genre);
    std::string getNextGenre();
    std::string getRandomGenre();
    
    /**
     * @brief Find a genre by case-insensitive matching
     * @param target_genre The genre to find (case-insensitive)
     * @return The actual genre name from database, or empty string if not found
     */
    std::string findGenreCaseInsensitive(const std::string& target_genre);
    
private:
    std::unique_ptr<MixDatabase> database;
    std::unique_ptr<MixMetadata> metadata;
    std::unique_ptr<MixDownloader> downloader;
    std::unique_ptr<AutoVibez::Audio::MixPlayer> player;
    std::unique_ptr<AutoVibez::Audio::MP3Analyzer> mp3_analyzer;
    std::string db_path;
    std::string data_dir;
    std::string last_error;
    bool success;
    Mix current_mix;
    std::vector<Mix> available_mixes;
    std::vector<std::future<bool>> _download_futures;
    std::string _current_genre;
    
    // Crossfade state
    bool _crossfade_enabled{false};
    bool _crossfade_active{false};
    int _crossfade_duration_ms{3000};
    int _crossfade_progress{0};
    Mix _crossfade_new_mix;
    Mix _crossfade_old_mix;
    Uint32 _crossfade_start_time{0};
    int _old_volume{0};
    int _new_volume{0};
    bool _suppress_volume_output{false};
    
    // Genre management
    std::vector<std::string> _available_genres;

    // Callback for first mix added
    FirstMixAddedCallback _first_mix_callback;
};

} // namespace Data
} // namespace AutoVibez 