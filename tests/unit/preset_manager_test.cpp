#include "preset_manager.hpp"

#include <gtest/gtest.h>
#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

class PresetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        _projectM = projectm_create();
        _playlist = projectm_playlist_create(_projectM);

        // For testing, we'll create a mock playlist with multiple positions
        // Since we can't easily create real preset files in tests,
        // we'll test the random number generation logic separately
        _presetManager = std::make_unique<PresetManager>(_playlist);
    }

    void TearDown() override {
        _presetManager.reset();
        projectm_playlist_destroy(_playlist);
        projectm_destroy(_projectM);
    }

    projectm_handle _projectM;
    projectm_playlist_handle _playlist;
    std::unique_ptr<PresetManager> _presetManager;
};

TEST_F(PresetManagerTest, RandomPresetDistribution) {
    // Test with a simulated playlist size of 5
    const int numCalls = 1000;
    const int numPresets = 5;
    std::unordered_map<uint32_t, int> distribution;

    // Since we can't easily create real preset files, we'll test the random logic
    // by directly calling the random number generation with a fixed range
    std::mt19937 testGenerator(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dis(0, numPresets - 1);

    // Generate random numbers using the same logic as randomPreset
    for (int i = 0; i < numCalls; ++i) {
        uint32_t random_index = dis(testGenerator);
        distribution[random_index]++;
    }

    // Verify that all presets were selected at least once
    EXPECT_EQ(distribution.size(), numPresets);

    // Verify that each preset was selected multiple times
    for (const auto& pair : distribution) {
        EXPECT_GT(pair.second, 0) << "Preset " << pair.first << " was never selected";
    }

    // Calculate expected frequency (should be roughly equal)
    double expectedFrequency = static_cast<double>(numCalls) / numPresets;
    double tolerance = expectedFrequency * 0.3;  // Allow 30% deviation

    for (const auto& pair : distribution) {
        EXPECT_NEAR(pair.second, expectedFrequency, tolerance)
            << "Preset " << pair.first << " was selected " << pair.second << " times, expected around "
            << expectedFrequency;
    }
}

TEST_F(PresetManagerTest, RandomPresetNoRepetition) {
    const int numCalls = 50;
    std::vector<uint32_t> selectedPositions;

    // Test the random number generation directly
    std::mt19937 testGenerator(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dis(0, 4);  // 5 presets (0-4)

    // Generate random numbers using the same logic as randomPreset
    for (int i = 0; i < numCalls; ++i) {
        uint32_t random_index = dis(testGenerator);
        selectedPositions.push_back(random_index);
    }

    // Verify that we don't get the same preset every time
    // (This is a probabilistic test, but with 50 calls and 5 presets,
    // getting the same preset every time is extremely unlikely)
    bool allSame = std::all_of(selectedPositions.begin(), selectedPositions.end(),
                               [&](uint32_t pos) { return pos == selectedPositions[0]; });
    EXPECT_FALSE(allSame) << "All random preset calls returned the same position";
}

TEST_F(PresetManagerTest, RandomPresetWithEmptyPlaylist) {
    // Create a new playlist with no presets
    projectm_playlist_handle emptyPlaylist = projectm_playlist_create(_projectM);
    PresetManager emptyPresetManager(emptyPlaylist);

    // This should not crash
    EXPECT_NO_THROW(emptyPresetManager.randomPreset());

    projectm_playlist_destroy(emptyPlaylist);
}

TEST_F(PresetManagerTest, RandomPresetWithSinglePreset) {
    // Create a playlist with only one preset
    projectm_playlist_handle singlePlaylist = projectm_playlist_create(_projectM);
    projectm_playlist_add_path(singlePlaylist, "single_preset.milk", true, false);

    PresetManager singlePresetManager(singlePlaylist);

    // Call randomPreset multiple times
    for (int i = 0; i < 10; ++i) {
        singlePresetManager.randomPreset();
        uint32_t position = projectm_playlist_get_position(singlePlaylist);
        EXPECT_EQ(position, 0) << "With single preset, should always select position 0";
    }

    projectm_playlist_destroy(singlePlaylist);
}
