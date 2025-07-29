#include "mix_manager.hpp"
#include "mix_database.hpp"
#include "mix_metadata.hpp"
#include "mix_downloader.hpp"
#include "mix_player.hpp"
#include "console_output.hpp"
#include <iostream>
#include <filesystem>
#include <random>
#include <fstream>
#include <set>
#include <sstream>
#include <SDL2/SDL.h>
#include <algorithm>

// URL decode function
std::string urlDecode(const std::string& encoded) {
    std::string result;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int value;
            std::string hex = encoded.substr(i + 1, 2);
            std::istringstream iss(hex);
            iss >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }
    return result;
}

// Convert seconds to human-readable format (HH:MM:SS)
std::string formatDuration(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) {
        return std::to_string(hours) + ":" + 
               (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + 
               (secs < 10 ? "0" : "") + std::to_string(secs);
    } else {
        return std::to_string(minutes) + ":" + 
               (secs < 10 ? "0" : "") + std::to_string(secs);
    }
}

MixManager::MixManager(const std::string& db_path, const std::string& cache_dir)
    : db_path(db_path), cache_dir(cache_dir), success(true) {
}

MixManager::~MixManager() {
}

bool MixManager::initialize() {
    std::filesystem::create_directories(cache_dir);
    
    // Clean up any corrupted files
    cleanupCorruptedFiles();
    
    database = std::make_unique<MixDatabase>(db_path);
    if (!database->initialize()) {
        last_error = "Failed to initialize database: " + database->getLastError();
        success = false;
        return false;
    }
    
    metadata = std::make_unique<MixMetadata>();
    
    downloader = std::make_unique<MixDownloader>(cache_dir);
    
    mp3_analyzer = std::make_unique<MP3Analyzer>();
    
    player = std::make_unique<MixPlayer>();
    
    return true;
}

bool MixManager::loadMixMetadata(const std::string& yaml_url) {
    if (!metadata) {
        last_error = "Metadata parser not initialized";
        return false;
    }
    
    std::vector<Mix> mixes = metadata->loadFromYaml(yaml_url);
    
    if (!metadata->isSuccess()) {
        last_error = "Failed to load metadata: " + metadata->getLastError();
        return false;
    }
    
    try {
        syncMixesWithDatabase(mixes);
    } catch (const std::exception& e) {
        last_error = "Database sync failed: " + std::string(e.what());
        return false;
    }
    
    return true;
}

bool MixManager::checkForNewMixes(const std::string& yaml_url) {
    if (!metadata) {
        last_error = "Metadata parser not initialized";
        return false;
    }
    
    // Load current mixes from remote YAML
    std::vector<Mix> new_mixes = metadata->loadFromYaml(yaml_url);
    
    if (!metadata->isSuccess()) {
        last_error = "Failed to check for new mixes: " + metadata->getLastError();
        return false;
    }
    
    // Get existing mix IDs from database
    std::vector<Mix> existing_mixes = database ? database->getAllMixes() : std::vector<Mix>();
    std::set<std::string> existing_ids;
    for (const auto& mix : existing_mixes) {
        existing_ids.insert(mix.id);
    }
    
    // Find new mixes that aren't in the database
    std::vector<Mix> new_mixes_to_add;
    for (const auto& mix : new_mixes) {
        if (existing_ids.find(mix.id) == existing_ids.end()) {
            new_mixes_to_add.push_back(mix);
        }
    }
    
    if (!new_mixes_to_add.empty()) {
        ConsoleOutput::output("🆕 Found %zu new mixes, downloading in background...", new_mixes_to_add.size());
        ConsoleOutput::output("");
        
        // Add new mixes to available_mixes for background download
        available_mixes.insert(available_mixes.end(), new_mixes_to_add.begin(), new_mixes_to_add.end());
        
        // Start background download of new mixes (silently)
        for (const auto& mix : new_mixes_to_add) {
            downloadMixBackground(mix);
        }
    }
    
    return true;
}

// Audio functionality
bool MixManager::downloadAndPlayMix(const Mix& mix) {
    // Check if already downloaded
    if (!downloader->isMixDownloaded(mix.id)) {
        if (!downloadMix(mix)) {
            return false;
        }
    }
    
    // Use crossfade if enabled and we're currently playing
    if (_crossfade_enabled && isPlaying() && !_crossfade_active) {
        return startCrossfade(mix);
    }
    
    return playMix(mix);
}

bool MixManager::startCrossfade(const Mix& new_mix, int crossfade_duration_ms) {
    if (!player) {
        last_error = "Player not initialized";
        return false;
    }
    
    // Store current mix as old mix
    _crossfade_old_mix = current_mix;
    _crossfade_new_mix = new_mix;
    _crossfade_duration_ms = crossfade_duration_ms;
    _crossfade_active = true;
    _crossfade_progress = 0;
    _crossfade_start_time = SDL_GetTicks();
    
    // Store current volume
    _old_volume = getVolume();
    _new_volume = _old_volume; // Start new mix at same volume
    
    // Start the new mix at 0 volume
    if (!playMix(new_mix)) {
        _crossfade_active = false;
        return false;
    }
    
    // Set new mix volume to 0 initially
    player->setVolume(0, true);
    
    return true;
}

void MixManager::updateCrossfade() {
    if (!_crossfade_active || !player) {
        return;
    }
    
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - _crossfade_start_time;
    
    if (elapsed >= static_cast<Uint32>(_crossfade_duration_ms)) {
        // Crossfade complete
        _crossfade_active = false;
        _crossfade_progress = 100;
        
        // Ensure final volumes are correct
        player->setVolume(_new_volume, true);
        
        return;
    }
    
    // Calculate progress (0-100)
    _crossfade_progress = (elapsed * 100) / _crossfade_duration_ms;
    
    // Calculate new volume level
    int new_volume = (_new_volume * _crossfade_progress / 100);
    
    // Apply volume with suppressed output
    player->setVolume(new_volume, true);
    
    // Update current mix reference
    current_mix = _crossfade_new_mix;
}

bool MixManager::playMix(const Mix& mix) {
    if (!player) {
        last_error = "Player not initialized";
        return false;
    }
    
    std::string local_path = downloader->getLocalPath(mix.id);
    
    if (!downloader->isMixDownloaded(mix.id)) {
        last_error = "Mix not downloaded: " + mix.title;
        return false;
    }
    
    // Validate that the file is actually a valid MP3 before attempting playback
    if (!player->isValidMP3File(local_path)) {
        last_error = "Mix file is corrupted or invalid: " + mix.title;
                    ConsoleOutput::output("⚠️  Corrupted file detected: %s", local_path.c_str());
        
        // Clean up the corrupted file
        try {
            std::filesystem::remove(local_path);
            ConsoleOutput::output("🗑️  Removed corrupted file: %s", std::filesystem::path(local_path).filename().c_str());
        } catch (const std::exception& e) {
            ConsoleOutput::output("⚠️  Failed to remove corrupted file: %s", e.what());
        }
        
        return false;
    }
    
    if (player->playMix(local_path, mix.title)) {
        current_mix = mix;
        updatePlayStats(mix.id);
        setLocalPath(mix.id, local_path);
        return true;
    } else {
        last_error = "Failed to play mix: " + player->getLastError();
        return false;
    }
}

bool MixManager::togglePause() {
    if (!player) {
        last_error = "Player not initialized";
        return false;
    }
    return player->togglePause();
}

bool MixManager::stop() {
    if (!player) {
        last_error = "Player not initialized";
        return false;
    }
    return player->stop();
}

bool MixManager::setVolume(int volume) {
    return setVolume(volume, false);
}

bool MixManager::setVolume(int volume, bool suppress_output) {
    if (!player) {
        last_error = "Player not initialized";
        return false;
    }
    
    bool result = player->setVolume(volume, suppress_output);
    
    return result;
}

int MixManager::getVolume() const {
    return player ? player->getVolume() : 0;
}

bool MixManager::isPlaying() const {
    return player && player->isPlaying();
}

bool MixManager::isPaused() const {
    return player && player->isPaused();
}

bool MixManager::hasFinished() {
    return player && player->hasFinished();
}

int MixManager::getCurrentPosition() const {
    return player ? player->getCurrentPosition() : 0;
}

int MixManager::getDuration() const {
    return player ? player->getDuration() : 0;
}

// Cache management
bool MixManager::clearCache() {
    if (!std::filesystem::exists(cache_dir)) {
        return true;
    }
    
    try {
        std::filesystem::remove_all(cache_dir);
        std::filesystem::create_directories(cache_dir);
        ConsoleOutput::output("🗑️  Cache cleared");
        ConsoleOutput::output("");
        return true;
    } catch (const std::exception& e) {
        last_error = "Failed to clear cache: " + std::string(e.what());
        return false;
    }
}

size_t MixManager::getCacheSize() const {
    if (!std::filesystem::exists(cache_dir)) {
        return 0;
    }
    
    size_t total_size = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(cache_dir)) {
        if (entry.is_regular_file()) {
            total_size += entry.file_size();
        }
    }
    return total_size;
}

bool MixManager::cleanupCorruptedFiles() {
    if (!std::filesystem::exists(cache_dir)) {
        return true;
    }
    
    int cleaned_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(cache_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            std::string file_path = entry.path().string();
            
            // Check if file is actually an MP3
            std::ifstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                continue;
            }
            
            char header[10];
            file.read(header, 10);
            
            if (file.gcount() < 10) {
                // File too small, likely corrupted
                std::filesystem::remove(file_path);
                cleaned_count++;
                continue;
            }
            
            // Check for MP3 sync word or ID3 header
            bool is_valid_mp3 = false;
            if ((header[0] & 0xFF) == 0xFF && 
                ((header[1] & 0xE0) == 0xE0 || (header[1] & 0xF0) == 0xF0)) {
                is_valid_mp3 = true;
            } else if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
                is_valid_mp3 = true;
            }
            
            if (!is_valid_mp3) {
                // Not a valid MP3, remove it
                std::filesystem::remove(file_path);
                cleaned_count++;
                ConsoleOutput::output("🗑️  Removed corrupted file: %s", std::filesystem::path(file_path).filename().c_str());
            }
        }
    }
    
    if (cleaned_count > 0) {
        ConsoleOutput::output("🧹 Cleaned up %d corrupted files", cleaned_count);
        ConsoleOutput::output("");
    }
    
    return true;
}

// Private helper methods
bool MixManager::downloadMix(const Mix& mix) {
    if (!downloader) {
        last_error = "Downloader not initialized";
        return false;
    }
    
    if (downloader->downloadMix(mix)) {
        setLocalPath(mix.id, downloader->getLocalPath(mix.id));
        return true;
    } else {
        last_error = "Download failed: " + downloader->getLastError();
        return false;
    }
}

bool MixManager::downloadAndAnalyzeMix(const Mix& mix) {
    // Extract filename from URL for prettier display
    std::string filename = mix.url;
    size_t last_slash = filename.find_last_of('/');
    if (last_slash != std::string::npos) {
        filename = filename.substr(last_slash + 1);
        filename = urlDecode(filename);
    }
    
            ConsoleOutput::output("🔄 Downloading and analyzing mix: %s", filename.c_str());
    
    // Check if mix is already in database
    if (database && database->getMixById(mix.id).id == mix.id) {
        Mix existing_mix = database->getMixById(mix.id);
        ConsoleOutput::output("📋 Mix already in database: %s", existing_mix.title.c_str());
        return true;
    }
    
    // Step 1: Download the mix
    if (!downloader->downloadMix(mix)) {
        last_error = "Failed to download mix: " + downloader->getLastError();
        return false;
    }
    
    // Step 2: Get the local file path
    std::string local_path = downloader->getLocalPath(mix.id);
    
    // Step 3: Analyze the downloaded file to extract ALL metadata
    MP3Metadata mp3_metadata = mp3_analyzer->analyzeFile(local_path);
    if (mp3_metadata.title.empty() && mp3_metadata.artist.empty()) {
        last_error = "Failed to analyze MP3 file: " + mp3_analyzer->getLastError();
        return false;
    }
    
    // Step 4: Create complete mix with extracted metadata
    Mix updated_mix;
    updated_mix.id = mix.id; // Use original ID from YAML
    updated_mix.title = mp3_metadata.title;
    updated_mix.artist = mp3_metadata.artist;
    updated_mix.genre = mp3_metadata.genre;
    updated_mix.url = mix.url; // Keep original URL
    updated_mix.duration_seconds = mp3_metadata.duration_seconds;
    updated_mix.local_path = local_path;
    updated_mix.description = mp3_metadata.description;
    updated_mix.tags = mp3_metadata.tags;

    updated_mix.play_count = 0;
    updated_mix.is_favorite = false;
    updated_mix.date_added = mp3_analyzer->getCurrentDateTime();
    updated_mix.last_played = "";
    
    // Step 5: Add the mix to the database with complete metadata
    if (database) {
        database->addMix(updated_mix);
        ConsoleOutput::output("✅ Added mix to database with complete metadata: %s", updated_mix.title.c_str());
        ConsoleOutput::output("");
    }
    
        ConsoleOutput::output("✅ Successfully downloaded and analyzed: %s", updated_mix.title.c_str());
        ConsoleOutput::output("");
        ConsoleOutput::output("   🎧 Artist: %s", updated_mix.artist.c_str());
        ConsoleOutput::output("   🎼 Genre: %s", updated_mix.genre.c_str());
        ConsoleOutput::output("   ⏱️  Duration: %s", formatDuration(updated_mix.duration_seconds).c_str());
        ConsoleOutput::output("   📁 Format: %s", mp3_metadata.format.c_str());
        ConsoleOutput::output("   💾 File size: %ld bytes", mp3_metadata.file_size);
    
    return true;
}

// Database delegation methods
Mix MixManager::getRandomMix() {
    return database ? database->getRandomMix() : Mix();
}

Mix MixManager::getSmartRandomMix() {
    return database ? database->getSmartRandomMix() : Mix();
}

Mix MixManager::getSmartRandomMix(const std::string& exclude_mix_id) {
    return database ? database->getSmartRandomMix(exclude_mix_id) : Mix();
}

Mix MixManager::getSmartRandomMix(const std::string& exclude_mix_id, const std::string& preferred_genre) {
    return database ? database->getSmartRandomMix(exclude_mix_id, preferred_genre) : Mix();
}

Mix MixManager::getNextMix(const std::string& current_mix_id) {
    return database ? database->getNextMix(current_mix_id) : Mix();
}

Mix MixManager::getRandomAvailableMix() {
    if (available_mixes.empty()) {
        return Mix();
    }
    
    // Use a simple random selection
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, available_mixes.size() - 1);
    
    return available_mixes[dis(gen)];
}

Mix MixManager::getRandomAvailableMixByGenre(const std::string& genre) {
    if (available_mixes.empty()) {
        return Mix();
    }
    
    // Filter available mixes by genre (case-insensitive)
    std::vector<Mix> genre_mixes;
    std::string genre_lower = genre;
    std::transform(genre_lower.begin(), genre_lower.end(), genre_lower.begin(), ::tolower);
    
    for (const auto& mix : available_mixes) {
        if (!mix.genre.empty()) {
            std::string mix_genre_lower = mix.genre;
            std::transform(mix_genre_lower.begin(), mix_genre_lower.end(), mix_genre_lower.begin(), ::tolower);
            if (mix_genre_lower == genre_lower) {
                genre_mixes.push_back(mix);
            }
        }
    }
    
    if (genre_mixes.empty()) {
        return Mix();
    }
    
    // Use a simple random selection from genre mixes
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, genre_mixes.size() - 1);
    
    return genre_mixes[dis(gen)];
}

std::vector<Mix> MixManager::getAvailableMixes() {
    return available_mixes;
}

Mix MixManager::getRandomMixByGenre(const std::string& genre) {
    if (!database) {
        last_error = "Database not initialized";
        return Mix();
    }
    
    return database->getRandomMixByGenre(genre);
}

Mix MixManager::getRandomMixByArtist(const std::string& artist) {
    if (!database) {
        last_error = "Database not initialized";
        return Mix();
    }
    return database->getRandomMixByArtist(artist);
}

Mix MixManager::getMixById(const std::string& id) {
    if (!database) {
        last_error = "Database not initialized";
        return Mix();
    }
    return database->getMixById(id);
}

std::vector<Mix> MixManager::getAllMixes() {
    return database ? database->getAllMixes() : std::vector<Mix>();
}

std::vector<Mix> MixManager::getMixesByGenre(const std::string& genre) {
    return database ? database->getMixesByGenre(genre) : std::vector<Mix>();
}

std::vector<Mix> MixManager::getMixesByArtist(const std::string& artist) {
    return database ? database->getMixesByArtist(artist) : std::vector<Mix>();
}

std::vector<Mix> MixManager::getDownloadedMixes() {
    return database ? database->getDownloadedMixes() : std::vector<Mix>();
}

std::vector<Mix> MixManager::getFavoriteMixes() {
    return database ? database->getFavoriteMixes() : std::vector<Mix>();
}

Mix MixManager::getRandomFavoriteMix() {
    if (!database) {
        return Mix();
    }
    
    std::vector<Mix> favorite_mixes = database->getFavoriteMixes();
    if (favorite_mixes.empty()) {
        return Mix();
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, favorite_mixes.size() - 1);
    return favorite_mixes[dis(gen)];
}



bool MixManager::toggleFavorite(const std::string& mix_id) {
    return database ? database->toggleFavorite(mix_id) : false;
}

bool MixManager::updatePlayStats(const std::string& mix_id) {
    return database ? database->updatePlayStats(mix_id) : false;
}

bool MixManager::setLocalPath(const std::string& mix_id, const std::string& local_path) {
    return database ? database->setLocalPath(mix_id, local_path) : false;
}

bool MixManager::downloadMixBackground(const Mix& mix) {
    return downloadAndAnalyzeMix(mix);
}

void MixManager::syncMixesWithDatabase(const std::vector<Mix>& mixes) {
    if (!database) { 
        return; 
    }
    
    // Store the mixes for later download, but don't add to database yet
    // The database will only be updated after successful download and analysis
    available_mixes = mixes;
    
} 

std::vector<std::string> MixManager::getAvailableGenres() {
    if (!database) {
        return std::vector<std::string>();
    }
    
    // Get all mixes and extract unique genres
    std::vector<Mix> all_mixes = database->getAllMixes();
    std::set<std::string> genres;
    
    for (const auto& mix : all_mixes) {
        if (!mix.genre.empty()) {
            // Use original casing from database
            genres.insert(mix.genre);
        }
    }
    
    _available_genres = std::vector<std::string>(genres.begin(), genres.end());
    return _available_genres;
}

std::string MixManager::getCurrentGenre() const {
    return _current_genre;
}

void MixManager::setCurrentGenre(const std::string& genre) {
    // Use case-insensitive matching to find the actual genre name
    std::string actual_genre = findGenreCaseInsensitive(genre);
    if (!actual_genre.empty()) {
        _current_genre = actual_genre;
    } else {
        _current_genre = genre;
    }
}

std::string MixManager::getNextGenre() {
    if (_available_genres.empty()) {
        getAvailableGenres();
    }
    
    if (_available_genres.empty()) {
        return "techno"; // fallback
    }
    
    // Find current genre in list
    auto it = std::find(_available_genres.begin(), _available_genres.end(), _current_genre);
    if (it == _available_genres.end()) {
        _current_genre = _available_genres[0];
        return _current_genre;
    }
    
    // Move to next genre
    ++it;
    if (it == _available_genres.end()) {
        it = _available_genres.begin(); // wrap around
    }
    
    _current_genre = *it;
    return _current_genre;
}

std::string MixManager::getRandomGenre() {
    if (_available_genres.empty()) {
        getAvailableGenres();
    }
    
    if (_available_genres.empty()) {
        return "techno"; // fallback
    }
    
    // Pick random genre
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, _available_genres.size() - 1);
    
    _current_genre = _available_genres[dis(gen)];
    return _current_genre;
}

std::string MixManager::findGenreCaseInsensitive(const std::string& target_genre) {
    if (!database) {
        return "";
    }
    
    // Get all mixes and extract unique genres
    std::vector<Mix> all_mixes = database->getAllMixes();
    std::set<std::string> genres;
    
    for (const auto& mix : all_mixes) {
        if (!mix.genre.empty()) {
            genres.insert(mix.genre);
        }
    }
    
    // Convert target to lowercase for comparison
    std::string target_lower = target_genre;
    std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);
    
    // Find matching genre (case-insensitive)
    for (const auto& genre : genres) {
        std::string genre_lower = genre;
        std::transform(genre_lower.begin(), genre_lower.end(), genre_lower.begin(), ::tolower);
        
        if (genre_lower == target_lower) {
            return genre; // Return the actual genre name from database
        }
    }
    
    return ""; // Not found
} 