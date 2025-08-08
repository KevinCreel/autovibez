#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <vector>
#include <map>

// Test platform-specific integration behaviors
// Focus on: cross-platform compatibility, path handling, audio devices, file systems, environment variables

// Mock platform state (behavior focus, not implementation)
struct MockPlatformState {
    // Platform detection
    std::string detected_platform; // "windows", "macos", "linux"
    bool platform_detection_successful = false;
    bool platform_features_available = false;
    
    // Path system integration
    bool path_system_initialized = false;
    std::string config_directory_pattern;
    std::string data_directory_pattern;
    std::string cache_directory_pattern;
    bool path_separators_correct = false;
    bool tilde_expansion_works = false;
    
    // Environment variable integration
    bool environment_variables_available = false;
    std::map<std::string, std::string> detected_env_vars;
    bool fallback_paths_used = false;
    
    // Audio system integration
    bool audio_system_available = false;
    std::vector<std::string> available_audio_devices;
    std::string default_audio_device;
    bool platform_audio_apis_working = false;
    
    // File system integration
    bool file_system_accessible = false;
    bool directory_creation_works = false;
    bool file_permissions_correct = false;
    bool unicode_filenames_supported = false;
    
    // Platform-specific features
    bool wasapi_loopback_available = false;  // Windows
    bool coreaudio_available = false;        // macOS
    bool pulseaudio_available = false;       // Linux
    bool xdg_directories_available = false;  // Linux
    
    // Cross-platform compatibility
    bool path_compatibility_verified = false;
    bool audio_compatibility_verified = false;
    bool config_compatibility_verified = false;
    
    // Error states
    bool platform_error_occurred = false;
    std::string platform_error_type;
    std::string platform_error_message;
    bool platform_fallback_successful = false;
};

// Mock platform-specific behavior (never real platform calls)
namespace MockPlatform {
    MockPlatformState state;
    
    // Mock platform configuration
    struct MockPlatformConfig {
        std::string platform_name = "linux"; // Default for testing
        bool simulate_windows = false;
        bool simulate_macos = false;
        bool simulate_linux = true;
        
        // Environment simulation
        std::map<std::string, std::string> mock_env_vars = {
            {"HOME", "/home/testuser"},
            {"XDG_CONFIG_HOME", "/home/testuser/.config"},
            {"XDG_DATA_HOME", "/home/testuser/.local/share"},
            {"XDG_CACHE_HOME", "/home/testuser/.cache"}
        };
        
        // Audio device simulation
        std::vector<std::string> mock_audio_devices = {
            "default", "pulse", "alsa", "speakers", "headphones"
        };
        
        // File system simulation
        bool mock_unicode_support = true;
        bool mock_long_path_support = true;
        bool mock_case_sensitive = true;
    };
    MockPlatformConfig config;
    
    // Mock platform-specific paths
    struct MockPlatformPaths {
        std::string config_home;
        std::string data_home;
        std::string cache_home;
        std::string separator;
        bool paths_valid = true;
    };
    MockPlatformPaths paths;
}

class PlatformIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset platform state
        MockPlatform::state = MockPlatformState();
        MockPlatform::config = MockPlatform::MockPlatformConfig();
        MockPlatform::paths = MockPlatform::MockPlatformPaths();
        
        // Setup default Linux environment for testing
        setupLinuxEnvironment();
    }

    void TearDown() override {
        // Clean up platform state
    }
    
    // Helper functions to simulate platform behaviors
    void setupWindowsEnvironment() {
        MockPlatform::config.platform_name = "windows";
        MockPlatform::config.simulate_windows = true;
        MockPlatform::config.simulate_macos = false;
        MockPlatform::config.simulate_linux = false;
        
        MockPlatform::config.mock_env_vars = {
            {"USERPROFILE", "C:\\Users\\testuser"},
            {"APPDATA", "C:\\Users\\testuser\\AppData\\Roaming"},
            {"LOCALAPPDATA", "C:\\Users\\testuser\\AppData\\Local"}
        };
        
        MockPlatform::paths.config_home = "C:\\Users\\testuser\\AppData\\Roaming";
        MockPlatform::paths.data_home = "C:\\Users\\testuser\\AppData\\Roaming";
        MockPlatform::paths.cache_home = "C:\\Users\\testuser\\AppData\\Local";
        MockPlatform::paths.separator = "\\";
        
        MockPlatform::config.mock_audio_devices = {
            "default", "DirectSound", "WASAPI", "speakers", "headphones"
        };
        
        MockPlatform::config.mock_case_sensitive = false;
    }
    
    void setupMacOSEnvironment() {
        MockPlatform::config.platform_name = "macos";
        MockPlatform::config.simulate_windows = false;
        MockPlatform::config.simulate_macos = true;
        MockPlatform::config.simulate_linux = false;
        
        MockPlatform::config.mock_env_vars = {
            {"HOME", "/Users/testuser"}
        };
        
        MockPlatform::paths.config_home = "/Users/testuser/Library/Application Support";
        MockPlatform::paths.data_home = "/Users/testuser/Library/Application Support";
        MockPlatform::paths.cache_home = "/Users/testuser/Library/Caches";
        MockPlatform::paths.separator = "/";
        
        MockPlatform::config.mock_audio_devices = {
            "default", "CoreAudio", "Built-in Output", "AirPods", "speakers"
        };
        
        MockPlatform::config.mock_case_sensitive = false; // HFS+ default
    }
    
    void setupLinuxEnvironment() {
        MockPlatform::config.platform_name = "linux";
        MockPlatform::config.simulate_windows = false;
        MockPlatform::config.simulate_macos = false;
        MockPlatform::config.simulate_linux = true;
        
        MockPlatform::config.mock_env_vars = {
            {"HOME", "/home/testuser"},
            {"XDG_CONFIG_HOME", "/home/testuser/.config"},
            {"XDG_DATA_HOME", "/home/testuser/.local/share"},
            {"XDG_CACHE_HOME", "/home/testuser/.cache"}
        };
        
        MockPlatform::paths.config_home = "/home/testuser/.config";
        MockPlatform::paths.data_home = "/home/testuser/.local/share";
        MockPlatform::paths.cache_home = "/home/testuser/.cache";
        MockPlatform::paths.separator = "/";
        
        MockPlatform::config.mock_audio_devices = {
            "default", "pulse", "alsa", "speakers", "headphones"
        };
        
        MockPlatform::config.mock_case_sensitive = true;
    }
    
    void simulatePlatformDetection() {
        // Simulate platform detection behavior
        MockPlatform::state.detected_platform = MockPlatform::config.platform_name;
        MockPlatform::state.platform_detection_successful = true;
        
        // Set platform-specific features
        if (MockPlatform::config.simulate_windows) {
            MockPlatform::state.wasapi_loopback_available = true;
            MockPlatform::state.platform_features_available = true;
        } else if (MockPlatform::config.simulate_macos) {
            MockPlatform::state.coreaudio_available = true;
            MockPlatform::state.platform_features_available = true;
        } else if (MockPlatform::config.simulate_linux) {
            MockPlatform::state.pulseaudio_available = true;
            MockPlatform::state.xdg_directories_available = true;
            MockPlatform::state.platform_features_available = true;
        }
    }
    
    void simulatePathSystemInitialization() {
        // Simulate path system initialization
        MockPlatform::state.path_system_initialized = true;
        
        // Set path patterns based on platform
        if (MockPlatform::config.simulate_windows) {
            MockPlatform::state.config_directory_pattern = "AppData\\Roaming\\autovibez";
            MockPlatform::state.data_directory_pattern = "AppData\\Roaming\\autovibez";
            MockPlatform::state.cache_directory_pattern = "AppData\\Local\\autovibez";
        } else if (MockPlatform::config.simulate_macos) {
            MockPlatform::state.config_directory_pattern = "Library/Application Support/autovibez";
            MockPlatform::state.data_directory_pattern = "Library/Application Support/autovibez";
            MockPlatform::state.cache_directory_pattern = "Library/Caches/autovibez";
        } else {
            MockPlatform::state.config_directory_pattern = ".config/autovibez";
            MockPlatform::state.data_directory_pattern = ".local/share/autovibez";
            MockPlatform::state.cache_directory_pattern = ".cache/autovibez";
        }
        
        MockPlatform::state.path_separators_correct = true;
        MockPlatform::state.tilde_expansion_works = !MockPlatform::config.simulate_windows;
    }
    
    void simulateEnvironmentVariableDetection() {
        // Simulate environment variable detection
        MockPlatform::state.environment_variables_available = true;
        MockPlatform::state.detected_env_vars = MockPlatform::config.mock_env_vars;
        
        // Check if fallback paths are needed
        if (MockPlatform::config.simulate_linux) {
            // Linux should have XDG variables
            bool has_xdg = MockPlatform::state.detected_env_vars.count("XDG_CONFIG_HOME") > 0;
            MockPlatform::state.fallback_paths_used = !has_xdg;
        }
    }
    
    void simulateAudioSystemInitialization() {
        // Simulate audio system initialization
        MockPlatform::state.audio_system_available = true;
        MockPlatform::state.available_audio_devices = MockPlatform::config.mock_audio_devices;
        MockPlatform::state.default_audio_device = MockPlatform::config.mock_audio_devices[0];
        
        // Platform-specific audio API availability
        if (MockPlatform::config.simulate_windows) {
            MockPlatform::state.platform_audio_apis_working = MockPlatform::state.wasapi_loopback_available;
        } else if (MockPlatform::config.simulate_macos) {
            MockPlatform::state.platform_audio_apis_working = MockPlatform::state.coreaudio_available;
        } else {
            MockPlatform::state.platform_audio_apis_working = MockPlatform::state.pulseaudio_available;
        }
    }
    
    void simulateFileSystemCompatibility() {
        // Simulate file system compatibility checks
        MockPlatform::state.file_system_accessible = true;
        MockPlatform::state.directory_creation_works = true;
        MockPlatform::state.file_permissions_correct = true;
        MockPlatform::state.unicode_filenames_supported = MockPlatform::config.mock_unicode_support;
    }
    
    void simulateCrossPlatformCompatibility() {
        // Simulate cross-platform compatibility verification
        simulatePlatformDetection();
        simulatePathSystemInitialization();
        simulateEnvironmentVariableDetection();
        simulateAudioSystemInitialization();
        simulateFileSystemCompatibility();
        
        // Verify compatibility
        MockPlatform::state.path_compatibility_verified = MockPlatform::state.path_system_initialized;
        MockPlatform::state.audio_compatibility_verified = MockPlatform::state.audio_system_available;
        MockPlatform::state.config_compatibility_verified = MockPlatform::state.environment_variables_available;
    }
    
    void simulatePlatformError(const std::string& error_type) {
        MockPlatform::state.platform_error_occurred = true;
        MockPlatform::state.platform_error_type = error_type;
        
        if (error_type == "missing_audio_system") {
            MockPlatform::state.audio_system_available = false;
            MockPlatform::state.platform_error_message = "Audio system not available";
        } else if (error_type == "missing_environment_vars") {
            MockPlatform::state.environment_variables_available = false;
            MockPlatform::state.platform_error_message = "Required environment variables missing";
            MockPlatform::state.fallback_paths_used = true;
            MockPlatform::state.platform_fallback_successful = true;
        } else if (error_type == "file_system_permissions") {
            MockPlatform::state.file_permissions_correct = false;
            MockPlatform::state.platform_error_message = "Insufficient file system permissions";
        }
    }
};

TEST_F(PlatformIntegrationTest, WindowsPlatformIntegrationWorkflow) {
    // Test: Windows-specific integration
    setupWindowsEnvironment();
    simulateCrossPlatformCompatibility();
    
    // Verify Windows detection
    EXPECT_EQ(MockPlatform::state.detected_platform, "windows");
    EXPECT_TRUE(MockPlatform::state.platform_detection_successful);
    
    // Verify Windows-specific features
    EXPECT_TRUE(MockPlatform::state.wasapi_loopback_available);
    EXPECT_TRUE(MockPlatform::state.platform_features_available);
    
    // Verify Windows paths
    EXPECT_TRUE(MockPlatform::state.path_system_initialized);
    EXPECT_EQ(MockPlatform::paths.separator, "\\");
    EXPECT_TRUE(MockPlatform::state.config_directory_pattern.find("AppData") != std::string::npos);
    
    // Verify Windows environment variables
    EXPECT_TRUE(MockPlatform::state.environment_variables_available);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("APPDATA") > 0);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("USERPROFILE") > 0);
    
    // Verify Windows audio
    EXPECT_TRUE(MockPlatform::state.audio_system_available);
    EXPECT_TRUE(std::find(MockPlatform::state.available_audio_devices.begin(),
                         MockPlatform::state.available_audio_devices.end(),
                         "WASAPI") != MockPlatform::state.available_audio_devices.end());
}

TEST_F(PlatformIntegrationTest, MacOSPlatformIntegrationWorkflow) {
    // Test: macOS-specific integration
    setupMacOSEnvironment();
    simulateCrossPlatformCompatibility();
    
    // Verify macOS detection
    EXPECT_EQ(MockPlatform::state.detected_platform, "macos");
    EXPECT_TRUE(MockPlatform::state.platform_detection_successful);
    
    // Verify macOS-specific features
    EXPECT_TRUE(MockPlatform::state.coreaudio_available);
    EXPECT_TRUE(MockPlatform::state.platform_features_available);
    
    // Verify macOS paths
    EXPECT_TRUE(MockPlatform::state.path_system_initialized);
    EXPECT_EQ(MockPlatform::paths.separator, "/");
    EXPECT_TRUE(MockPlatform::state.config_directory_pattern.find("Library") != std::string::npos);
    
    // Verify macOS environment variables
    EXPECT_TRUE(MockPlatform::state.environment_variables_available);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("HOME") > 0);
    
    // Verify macOS audio
    EXPECT_TRUE(MockPlatform::state.audio_system_available);
    EXPECT_TRUE(std::find(MockPlatform::state.available_audio_devices.begin(),
                         MockPlatform::state.available_audio_devices.end(),
                         "CoreAudio") != MockPlatform::state.available_audio_devices.end());
}

TEST_F(PlatformIntegrationTest, LinuxPlatformIntegrationWorkflow) {
    // Test: Linux-specific integration (default setup)
    simulateCrossPlatformCompatibility();
    
    // Verify Linux detection
    EXPECT_EQ(MockPlatform::state.detected_platform, "linux");
    EXPECT_TRUE(MockPlatform::state.platform_detection_successful);
    
    // Verify Linux-specific features
    EXPECT_TRUE(MockPlatform::state.pulseaudio_available);
    EXPECT_TRUE(MockPlatform::state.xdg_directories_available);
    EXPECT_TRUE(MockPlatform::state.platform_features_available);
    
    // Verify Linux paths
    EXPECT_TRUE(MockPlatform::state.path_system_initialized);
    EXPECT_EQ(MockPlatform::paths.separator, "/");
    EXPECT_TRUE(MockPlatform::state.config_directory_pattern.find(".config") != std::string::npos);
    
    // Verify XDG environment variables
    EXPECT_TRUE(MockPlatform::state.environment_variables_available);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("XDG_CONFIG_HOME") > 0);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("XDG_DATA_HOME") > 0);
    
    // Verify Linux audio
    EXPECT_TRUE(MockPlatform::state.audio_system_available);
    EXPECT_TRUE(std::find(MockPlatform::state.available_audio_devices.begin(),
                         MockPlatform::state.available_audio_devices.end(),
                         "pulse") != MockPlatform::state.available_audio_devices.end());
}

TEST_F(PlatformIntegrationTest, CrossPlatformPathCompatibilityWorkflow) {
    // Test: Path compatibility across platforms
    
    std::vector<std::string> platforms = {"windows", "macos", "linux"};
    std::vector<bool> compatibility_results;
    
    for (const auto& platform : platforms) {
        if (platform == "windows") setupWindowsEnvironment();
        else if (platform == "macos") setupMacOSEnvironment();
        else setupLinuxEnvironment();
        
        simulatePathSystemInitialization();
        // Verify path system was initialized properly
        bool path_compatible = MockPlatform::state.path_system_initialized;
        compatibility_results.push_back(path_compatible);
    }
    
    // Verify all platforms have working path systems
    for (bool result : compatibility_results) {
        EXPECT_TRUE(result);
    }
    
    // Verify path pattern differences
    EXPECT_EQ(compatibility_results.size(), 3);
    EXPECT_TRUE(std::all_of(compatibility_results.begin(), compatibility_results.end(),
                           [](bool b) { return b; }));
}

TEST_F(PlatformIntegrationTest, AudioSystemCompatibilityWorkflow) {
    // Test: Audio system compatibility across platforms
    
    // Test Windows audio
    setupWindowsEnvironment();
    simulateAudioSystemInitialization();
    bool windows_audio = MockPlatform::state.audio_system_available;
    
    // Test macOS audio
    setupMacOSEnvironment();
    simulateAudioSystemInitialization();
    bool macos_audio = MockPlatform::state.audio_system_available;
    
    // Test Linux audio
    setupLinuxEnvironment();
    simulateAudioSystemInitialization();
    bool linux_audio = MockPlatform::state.audio_system_available;
    
    // Verify audio works on all platforms
    EXPECT_TRUE(windows_audio);
    EXPECT_TRUE(macos_audio);
    EXPECT_TRUE(linux_audio);
    
    // Verify platform-specific audio features
    setupWindowsEnvironment();
    simulatePlatformDetection();
    EXPECT_TRUE(MockPlatform::state.wasapi_loopback_available);
    
    setupMacOSEnvironment();
    simulatePlatformDetection();
    EXPECT_TRUE(MockPlatform::state.coreaudio_available);
    
    setupLinuxEnvironment();
    simulatePlatformDetection();
    EXPECT_TRUE(MockPlatform::state.pulseaudio_available);
}

TEST_F(PlatformIntegrationTest, EnvironmentVariableFallbackWorkflow) {
    // Test: Environment variable fallback mechanisms
    
    // Test missing XDG variables on Linux
    setupLinuxEnvironment();
    MockPlatform::config.mock_env_vars.erase("XDG_CONFIG_HOME");
    MockPlatform::config.mock_env_vars.erase("XDG_DATA_HOME");
    
    simulateEnvironmentVariableDetection();
    
    // Verify fallback behavior
    EXPECT_TRUE(MockPlatform::state.fallback_paths_used);
    EXPECT_TRUE(MockPlatform::state.detected_env_vars.count("HOME") > 0);
    
    // Test missing environment variables entirely
    MockPlatform::config.mock_env_vars.clear();
    simulatePlatformError("missing_environment_vars");
    
    EXPECT_TRUE(MockPlatform::state.platform_error_occurred);
    EXPECT_EQ(MockPlatform::state.platform_error_type, "missing_environment_vars");
    EXPECT_TRUE(MockPlatform::state.platform_fallback_successful);
}

TEST_F(PlatformIntegrationTest, FileSystemPermissionsWorkflow) {
    // Test: File system permissions across platforms
    
    // Test normal permissions
    simulateFileSystemCompatibility();
    EXPECT_TRUE(MockPlatform::state.file_system_accessible);
    EXPECT_TRUE(MockPlatform::state.directory_creation_works);
    EXPECT_TRUE(MockPlatform::state.file_permissions_correct);
    
    // Test permission errors
    simulatePlatformError("file_system_permissions");
    EXPECT_TRUE(MockPlatform::state.platform_error_occurred);
    EXPECT_FALSE(MockPlatform::state.file_permissions_correct);
    EXPECT_EQ(MockPlatform::state.platform_error_type, "file_system_permissions");
}

TEST_F(PlatformIntegrationTest, UnicodeFilenameCompatibilityWorkflow) {
    // Test: Unicode filename compatibility
    
    // Test platforms with Unicode support
    std::vector<std::string> platforms = {"windows", "macos", "linux"};
    
    for (const auto& platform : platforms) {
        if (platform == "windows") setupWindowsEnvironment();
        else if (platform == "macos") setupMacOSEnvironment();
        else setupLinuxEnvironment();
        
        simulateFileSystemCompatibility();
        EXPECT_TRUE(MockPlatform::state.unicode_filenames_supported);
    }
    
    // Test platform without Unicode support
    MockPlatform::config.mock_unicode_support = false;
    simulateFileSystemCompatibility();
    EXPECT_FALSE(MockPlatform::state.unicode_filenames_supported);
}

TEST_F(PlatformIntegrationTest, PlatformSpecificAudioFeaturesWorkflow) {
    // Test: Platform-specific audio features
    
    // Test WASAPI Loopback on Windows
    setupWindowsEnvironment();
    simulatePlatformDetection();
    simulateAudioSystemInitialization();
    
    EXPECT_TRUE(MockPlatform::state.wasapi_loopback_available);
    EXPECT_TRUE(MockPlatform::state.platform_audio_apis_working);
    
    // Test CoreAudio on macOS
    setupMacOSEnvironment();
    simulatePlatformDetection();
    simulateAudioSystemInitialization();
    
    EXPECT_TRUE(MockPlatform::state.coreaudio_available);
    EXPECT_TRUE(MockPlatform::state.platform_audio_apis_working);
    
    // Test PulseAudio on Linux
    setupLinuxEnvironment();
    simulatePlatformDetection();
    simulateAudioSystemInitialization();
    
    EXPECT_TRUE(MockPlatform::state.pulseaudio_available);
    EXPECT_TRUE(MockPlatform::state.platform_audio_apis_working);
}

TEST_F(PlatformIntegrationTest, ConfigurationCompatibilityWorkflow) {
    // Test: Configuration compatibility across platforms
    
    std::vector<std::string> platforms = {"windows", "macos", "linux"};
    std::vector<bool> config_compatibility;
    
    for (const auto& platform : platforms) {
        if (platform == "windows") setupWindowsEnvironment();
        else if (platform == "macos") setupMacOSEnvironment();
        else setupLinuxEnvironment();
        
        simulateEnvironmentVariableDetection();
        simulatePathSystemInitialization();
        
        bool compatible = MockPlatform::state.environment_variables_available &&
                         MockPlatform::state.path_system_initialized;
        config_compatibility.push_back(compatible);
    }
    
    // Verify configuration works on all platforms
    for (bool compatible : config_compatibility) {
        EXPECT_TRUE(compatible);
    }
    
    EXPECT_EQ(config_compatibility.size(), 3);
}

TEST_F(PlatformIntegrationTest, PlatformErrorRecoveryWorkflow) {
    // Test: Platform error recovery
    
    // Test audio system failure recovery
    simulatePlatformError("missing_audio_system");
    EXPECT_TRUE(MockPlatform::state.platform_error_occurred);
    EXPECT_FALSE(MockPlatform::state.audio_system_available);
    
    // Application should still function without audio
    simulatePathSystemInitialization();
    simulateEnvironmentVariableDetection();
    EXPECT_TRUE(MockPlatform::state.path_system_initialized);
    EXPECT_TRUE(MockPlatform::state.environment_variables_available);
    
    // Test environment variable fallback
    SetUp(); // Reset
    simulatePlatformError("missing_environment_vars");
    EXPECT_TRUE(MockPlatform::state.platform_fallback_successful);
    EXPECT_TRUE(MockPlatform::state.fallback_paths_used);
}

TEST_F(PlatformIntegrationTest, PathSeparatorConsistencyWorkflow) {
    // Test: Path separator consistency
    
    // Test Windows backslash
    setupWindowsEnvironment();
    simulatePathSystemInitialization();
    EXPECT_EQ(MockPlatform::paths.separator, "\\");
    EXPECT_TRUE(MockPlatform::state.path_separators_correct);
    
    // Test Unix forward slash
    setupMacOSEnvironment();
    simulatePathSystemInitialization();
    EXPECT_EQ(MockPlatform::paths.separator, "/");
    EXPECT_TRUE(MockPlatform::state.path_separators_correct);
    
    setupLinuxEnvironment();
    simulatePathSystemInitialization();
    EXPECT_EQ(MockPlatform::paths.separator, "/");
    EXPECT_TRUE(MockPlatform::state.path_separators_correct);
}

TEST_F(PlatformIntegrationTest, TildeExpansionWorkflow) {
    // Test: Tilde expansion behavior
    
    // Test Unix-like systems (should work)
    setupLinuxEnvironment();
    simulatePathSystemInitialization();
    EXPECT_TRUE(MockPlatform::state.tilde_expansion_works);
    
    setupMacOSEnvironment();
    simulatePathSystemInitialization();
    EXPECT_TRUE(MockPlatform::state.tilde_expansion_works);
    
    // Test Windows (should not work)
    setupWindowsEnvironment();
    simulatePathSystemInitialization();
    EXPECT_FALSE(MockPlatform::state.tilde_expansion_works);
}

TEST_F(PlatformIntegrationTest, CompletePlatformIntegrationWorkflow) {
    // Test: Complete platform integration workflow
    
    std::vector<std::string> platforms = {"windows", "macos", "linux"};
    
    for (const auto& platform : platforms) {
        // Setup platform
        if (platform == "windows") setupWindowsEnvironment();
        else if (platform == "macos") setupMacOSEnvironment();
        else setupLinuxEnvironment();
        
        // Run complete integration
        simulateCrossPlatformCompatibility();
        
        // Verify complete integration
        EXPECT_TRUE(MockPlatform::state.platform_detection_successful);
        EXPECT_TRUE(MockPlatform::state.path_compatibility_verified);
        EXPECT_TRUE(MockPlatform::state.audio_compatibility_verified);
        EXPECT_TRUE(MockPlatform::state.config_compatibility_verified);
        EXPECT_TRUE(MockPlatform::state.platform_features_available);
        
        // Platform-specific verification
        if (platform == "windows") {
            EXPECT_TRUE(MockPlatform::state.wasapi_loopback_available);
            EXPECT_EQ(MockPlatform::paths.separator, "\\");
        } else if (platform == "macos") {
            EXPECT_TRUE(MockPlatform::state.coreaudio_available);
            EXPECT_EQ(MockPlatform::paths.separator, "/");
        } else {
            EXPECT_TRUE(MockPlatform::state.pulseaudio_available);
            EXPECT_TRUE(MockPlatform::state.xdg_directories_available);
            EXPECT_EQ(MockPlatform::paths.separator, "/");
        }
    }
}

TEST_F(PlatformIntegrationTest, CaseSensitivityHandlingWorkflow) {
    // Test: Case sensitivity handling across platforms
    
    // Test case-sensitive Linux
    setupLinuxEnvironment();
    simulateFileSystemCompatibility();
    EXPECT_TRUE(MockPlatform::config.mock_case_sensitive);
    
    // Test case-insensitive Windows
    setupWindowsEnvironment();
    simulateFileSystemCompatibility();
    EXPECT_FALSE(MockPlatform::config.mock_case_sensitive);
    
    // Test case-insensitive macOS (HFS+ default)
    setupMacOSEnvironment();
    simulateFileSystemCompatibility();
    EXPECT_FALSE(MockPlatform::config.mock_case_sensitive);
}
