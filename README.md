# AutoVibez - Audio Visualizer

AutoVibez is an audio visualizer application based on the ProjectM visualizer engine. It provides real-time audio visualization with support for Milkdrop-style presets.

## Features

- Real-time audio visualization using ProjectM-4 engine
- Support for Milkdrop-style presets (.milk files)
- Audio input from microphone or system audio (PulseAudio monitor sources)
- Fullscreen and windowed modes with F11 toggle
- Preset shuffling and manual navigation
- Touch/mouse interaction support
- Configurable settings via config file
- Audio device cycling with Tab key
- Pause/resume preset transitions
- Help overlay with H key
- FPS counter display
- Window sizing and positioning
- **Included presets and textures** - No need to install projectM separately
- **Crossfade between mixes** - Smooth transitions between audio tracks

## Dependencies

- CMake 3.16 or higher
- C++17 compiler
- SDL2
- ProjectM-4
- GLM (OpenGL Mathematics)
- OpenGL

## Building

### Prerequisites

Install the required dependencies:

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential libsdl2-dev libprojectm-dev libglm-dev

# Arch Linux
sudo pacman -S cmake gcc sdl2 projectm glm

# macOS (using Homebrew)
brew install cmake sdl2 projectm glm
```

### Build Instructions

1. Clone the repository:
```bash
git clone <repository-url>
cd autovibez
```

2. Set up assets (optional, for development):
```bash
./setup_assets.sh
```

3. Create build directory and build:
```bash
mkdir build
cd build
cmake ..
make
```

4. Install the application (optional):
```bash
sudo make install
```
This will install AutoVibez to the system's standard locations.

5. Run the application:
```bash
./autovibez
```

## Usage

### Controls

- **H**: Toggle help overlay
- **F11**: Toggle fullscreen mode
- **P**: Pause/resume preset transitions
- **Tab**: Cycle through audio devices
- **Space**: Lock/unlock current preset
- **R**: Random preset
- **Y**: Toggle shuffle mode
- **Left/Right Arrow**: Previous/Next preset
- **Up/Down Arrow**: Adjust beat sensitivity
- **A**: Toggle aspect correction
- **Ctrl+Q**: Quit application
- **Mouse Wheel**: Previous/Next preset
- **Mouse**: Touch interaction (click and drag)

### Configuration

The application follows XDG Base Directory Specification and looks for configuration in the following order:
1. `$AUTOVIBEZ_CONFIG` environment variable (if set)
2. `$XDG_CONFIG_HOME/autovibez/config.inp` (or `~/.config/autovibez/config.inp` if not set)
3. System-wide default location
4. `./config/config.inp` (local configuration)

### Data Directory

AutoVibez uses a **single source of truth** (`PathManager`) for all directory management, providing XDG-compliant, cross-platform directory resolution:

**Linux/Unix:**
- **Config**: `$XDG_CONFIG_HOME/autovibez/` (or `~/.config/autovibez/` if not set)
- **Data**: `$XDG_DATA_HOME/autovibez/` (or `~/.local/share/autovibez/` if not set)
- **Cache**: `$XDG_CACHE_HOME/autovibez/` (or `~/.cache/autovibez/` if not set)
- **State**: `$XDG_STATE_HOME/autovibez/` (or `~/.local/state/autovibez/` if not set)

**macOS:**
- **Config**: `~/Library/Application Support/autovibez/config/`
- **Data**: `~/Library/Application Support/autovibez/assets/`
- **Cache**: `~/Library/Caches/autovibez/`
- **State**: `~/Library/Application Support/autovibez/state/`

**Windows:**
- **Config**: `%APPDATA%/autovibez/config/`
- **Data**: `%APPDATA%/autovibez/assets/`
- **Cache**: `%APPDATA%/autovibez/cache/`
- **State**: `%APPDATA%/autovibez/state/`

The application automatically creates these directories if they don't exist and respects your XDG environment variables.

### Using PathManager

The `PathManager` class provides a comprehensive interface for all path operations:

```cpp
// Core directories
std::string config_dir = PathManager::getConfigDirectory();
std::string data_dir = PathManager::getDataDirectory();
std::string cache_dir = PathManager::getCacheDirectory();
std::string state_dir = PathManager::getStateDirectory();

// File paths
std::string db_path = PathManager::getDatabasePath();
std::string mixes_dir = PathManager::getMixesDirectory();
std::string presets = PathManager::getPresetsDirectory();
std::string textures = PathManager::getTexturesDirectory();

// Search paths (in priority order)
std::vector<std::string> config_paths = PathManager::getConfigFileSearchPaths();
std::vector<std::string> preset_paths = PathManager::getPresetSearchPaths();
std::vector<std::string> texture_paths = PathManager::getTextureSearchPaths();

// Utilities
std::string config_file = PathManager::findConfigFile();
std::string expanded = PathManager::expandTilde("~/path");
bool exists = PathManager::pathExists("/some/path");
PathManager::ensureDirectoryExists("/some/directory");
```

### Presets

AutoVibez automatically uses your existing ProjectM presets through symlinks. The application looks for presets in:
- System-wide preset directory (symlinked to your ProjectM presets)
- Custom path specified in `preset_path` config option

## Configuration Options

### Core Settings
- `preset_path`: Directory containing preset files
- `texture_path`: Directory containing texture files
- `Window Width/Height`: Initial window size
- `FPS`: Target frame rate
- `Fullscreen`: Start in fullscreen mode (true/false)

### Audio Settings
- `audio_device_index`: Default audio device index (0 = system default)
- `Beat Sensitivity`: Audio reactivity sensitivity (0.0 - 5.0)
- `Hard Cuts Enabled`: Enable hard cuts on audio peaks (true/false)
- `Hard Cut Sensitivity`: Audio sensitivity for hard cuts (0.0 - 5.0)

### Visualization Settings
- `Mesh X/Y`: Mesh resolution for visualizations
- `Preset Duration`: How long each preset plays (seconds)
- `Smooth Transition Duration`: Preset transition duration (seconds)
- `Aspect Correction`: Enable aspect ratio correction (true/false)
- `show_fps`: Show FPS counter (true/false)

## Audio Devices

The application automatically detects available audio capture devices. You can:
- **Press Tab** to cycle through available audio devices
- **Set default device** using `audio_device_index` in config
- **Use PulseAudio monitor sources** for system audio capture
- **See device names** in the console when switching

Common audio devices include:
- Monitor of Built-in Audio (system audio)
- Monitor of [Graphics Card] (GPU audio)
- Microphone inputs
- Webcam audio

## Development

The application is structured as follows:

- `src/main.cpp`: Application entry point
- `src/autovibez_app.cpp/hpp`: Main application class
- `src/audio_capture.cpp/hpp`: Audio capture functionality
- `src/config_manager.cpp/hpp`: Configuration management
- `config/`: Configuration files

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

AutoVibez is based on ProjectM-4 and uses the same preset format as Milkdrop.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Troubleshooting

### No Audio Input
- Check that your audio device is properly configured
- Ensure the device has output volume greater than 0
- Try switching audio devices with the 'A' key

### No Presets Found
- Check the `preset_path` setting in your configuration
- Ensure preset files are in the correct directory
- Verify preset files are valid Milkdrop format

### Build Issues
- Ensure all dependencies are installed
- Check that CMake version is 3.16 or higher
- Verify C++17 support in your compiler 