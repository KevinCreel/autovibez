#pragma once

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <vector>

/**
 * @brief Mock ProjectM class for testing
 */
class MockProjectM {
public:
    MOCK_METHOD(bool, init, (int width, int height), ());
    MOCK_METHOD(void, destroy, (), ());
    MOCK_METHOD(bool, renderFrame, (), ());
    MOCK_METHOD(void, addPCMData, (const float* data, int num_samples), ());
    MOCK_METHOD(bool, loadPreset, (const std::string& preset_path), ());
    MOCK_METHOD(bool, loadRandomPreset, (), ());
    MOCK_METHOD(std::string, getCurrentPresetName, (), (const));
    MOCK_METHOD(int, getPresetCount, (), (const));
    MOCK_METHOD(std::vector<std::string>, getPresetList, (), (const));
    MOCK_METHOD(bool, setTextureSearchPaths, (const std::vector<std::string>& paths), ());
    MOCK_METHOD(void, resetTextures, (), ());
    MOCK_METHOD(bool, setPresetPath, (const std::string& path), ());
    MOCK_METHOD(std::string, getPresetPath, (), (const));
    MOCK_METHOD(bool, setTexturePath, (const std::string& path), ());
    MOCK_METHOD(std::string, getTexturePath, (), (const));
    MOCK_METHOD(bool, setBeatSensitivity, (float sensitivity), ());
    MOCK_METHOD(float, getBeatSensitivity, (), (const));
    MOCK_METHOD(bool, setFPS, (int fps), ());
    MOCK_METHOD(int, getFPS, (), (const));
    MOCK_METHOD(bool, setMeshSize, (int x, int y), ());
    MOCK_METHOD(void, getMeshSize, (int& x, int& y), (const));
    MOCK_METHOD(bool, setAspectCorrection, (bool enabled), ());
    MOCK_METHOD(bool, getAspectCorrection, (), (const));
    MOCK_METHOD(bool, isInitialized, (), (const));
    MOCK_METHOD(std::string, getLastError, (), (const));
    MOCK_METHOD(bool, isSuccess, (), (const));

    // Helper methods for testing
    void setMockPresetCount(int count) {
        mock_preset_count = count;
    }

    void setMockPresetList(const std::vector<std::string>& presets) {
        mock_preset_list = presets;
    }

    void setMockCurrentPresetName(const std::string& name) {
        mock_current_preset = name;
    }

    void setMockBeatSensitivity(float sensitivity) {
        mock_beat_sensitivity = sensitivity;
    }

    void setMockFPS(int fps) {
        mock_fps = fps;
    }

    void setMockMeshSize(int x, int y) {
        mock_mesh_x = x;
        mock_mesh_y = y;
    }

    void setMockIsInitialized(bool initialized) {
        mock_initialized = initialized;
    }

private:
    int mock_preset_count = 0;
    std::vector<std::string> mock_preset_list;
    std::string mock_current_preset = "Mock Preset";
    float mock_beat_sensitivity = 2.0f;
    int mock_fps = 60;
    int mock_mesh_x = 32;
    int mock_mesh_y = 24;
    bool mock_initialized = false;
};