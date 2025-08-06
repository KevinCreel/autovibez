#include "help_overlay.hpp"
#include "setup.hpp"
#include "constants.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl2.h>
#include <iostream>

namespace AutoVibez {
namespace UI {

HelpOverlay::HelpOverlay() {
}

HelpOverlay::~HelpOverlay() {
    if (_visible && _imguiReady) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
    
    // Free cursors
    if (_blankCursor) {
        SDL_FreeCursor(_blankCursor);
        _blankCursor = nullptr;
    }
    // Note: Don't free _originalCursor as it's managed by SDL
}

void HelpOverlay::init(SDL_Window* window, SDL_GLContext glContext) {
    if (_initialized) {
        return; // Already initialized
    }
    
    _window = window;
    _glContext = glContext;
    
    // Create a blank cursor (1x1 transparent pixel)
    Uint8 blankData[4] = {0, 0, 0, 0}; // Transparent
    Uint8 blankMask[4] = {0, 0, 0, 0}; // Transparent
    _blankCursor = SDL_CreateCursor(blankData, blankMask, 1, 1, 0, 0);
    
    // Store the original cursor
    _originalCursor = SDL_GetCursor();
    
    _initialized = true;
    // Note: ImGui will be initialized on first render to avoid conflicts
}

void HelpOverlay::render() {
    if (!_visible) return;
    
    // Auto-load mix data if not already loaded
    if (_mixTableData.empty()) {
        // This will be populated by the app when needed
        // For now, we'll leave it empty and let the app handle it
    }
    
    // Handle deferred texture rebinding at the start of render cycle
    if (_needsDeferredTextureRebind) {
        // Execute the texture rebind
        if (_imguiReady) {
            ImGui_ImplOpenGL2_DestroyFontsTexture();
            ImGui_ImplOpenGL2_CreateFontsTexture();
        }
        
        _needsDeferredTextureRebind = false;
    }
    
    // Lazy initialize ImGui on first render
    if (!_imguiReady) {
        // Ensure we have the OpenGL context
        SDL_GL_MakeCurrent(_window, _glContext);
        
        // Setup Dear ImGui context with minimal configuration
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // Add default font with explicit atlas building
        io.Fonts->AddFontDefault();
        io.FontGlobalScale = 1.0f;
        
        // Explicitly build the font atlas
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        // Set INI file path to user config directory
        std::string configDir = getConfigDirectory();
        if (!configDir.empty()) {
            std::string iniPath = configDir + "/imgui.ini";
            io.IniFilename = strdup(iniPath.c_str());
        }
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL2_Init();
        
        // Explicitly create the font texture
        ImGui_ImplOpenGL2_CreateFontsTexture();
        
        _imguiReady = true;
    }
    
    // Save current OpenGL state and isolate ImGui rendering
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    
    // Completely isolate ImGui's texture state
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Use a very simple rendering approach
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Force texture rebinding if needed
    if (_needsTextureRebind) {
        ImGui_ImplOpenGL2_DestroyFontsTexture();
        ImGui_ImplOpenGL2_CreateFontsTexture();
        _needsTextureRebind = false;
    }
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Helper function to create aligned text with consistent spacing
    auto renderAlignedText = [](const std::string& label, const std::string& value, const ImVec4& valueColor) {
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, valueColor);
        ImGui::TextUnformatted(value.c_str());
        ImGui::PopStyleColor();
    };
    
    // Helper function to create aligned key binding
    auto renderKeyBinding = [](const std::string& key, const std::string& description) {
        ImGui::TextUnformatted(key.c_str());
        ImGui::SameLine();
        ImGui::TextUnformatted(" - ");
        ImGui::SameLine();
        ImGui::TextUnformatted(description.c_str());
    };
    
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    
    // Set up the main window with semi-transparent background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::SetNextWindowBgAlpha(0.85f); // Semi-transparent instead of opaque

    ImGui::Begin("AutoVibez Help", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Set larger font size
    ImGui::SetWindowFontScale(1.0f);

    // Add some padding at the top
    ImGui::Spacing();
    ImGui::Spacing();

    // Title with gradient-like effect and better styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize("AUTOVIBEZ CONTROLS").x) * 0.5f);
    ImGui::TextUnformatted("AUTOVIBEZ CONTROLS");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Decorative line under the title
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.8f, 1.0f, 0.8f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Current Status Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
    ImGui::TextUnformatted("CURRENT STATUS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Subtle line under section header
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 1.0f, 0.8f, 0.4f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    
    // Calculate the maximum label width for alignment
    float maxLabelWidth = 0.0f;
    std::vector<std::string> labels = {"Preset:", "Now playing:", "Genre:", "Volume:", "Device:", "Beat Sensitivity:"};
    for (const auto& label : labels) {
        float width = ImGui::CalcTextSize(("  " + label).c_str()).x;
        maxLabelWidth = std::max(maxLabelWidth, width);
    }
    
    // Current preset
    if (!_currentPreset.empty()) {
        std::string label = "  Preset:";
        float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
        ImGui::TextUnformatted(_currentPreset.c_str());
        ImGui::PopStyleColor();
    }
    
    // Current mix
    if (!_currentArtist.empty() && !_currentTitle.empty()) {
        std::string label = "  Now playing:";
        float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::TextUnformatted(_currentArtist.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted(" - ");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::TextUnformatted(_currentTitle.c_str());
        ImGui::PopStyleColor();
    }
    
    // Current genre
    if (!_currentGenre.empty()) {
        std::string label = "  Genre:";
        float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::TextUnformatted(_currentGenre.c_str());
        ImGui::PopStyleColor();
    }
    
    // Volume level
    if (_volumeLevel >= 0) {
        std::string label = "  Volume:";
        float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::TextUnformatted((std::to_string(_volumeLevel) + "%").c_str());
        ImGui::PopStyleColor();
    }
    
    // Audio device
    if (!_audioDevice.empty()) {
        std::string label = "  Device:";
        float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::TextUnformatted(label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::TextUnformatted(_audioDevice.c_str());
        ImGui::PopStyleColor();
    }
    
    // Beat sensitivity
    std::string label = "  Beat Sensitivity:";
    float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (maxLabelWidth - labelWidth));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
    ImGui::TextUnformatted((std::to_string(_beatSensitivity).substr(0, 4)).c_str());
    ImGui::PopStyleColor();
    
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Decorative line
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 1.0f, 0.8f, 0.6f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Mix Management Section with improved styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
    ImGui::TextUnformatted("MIX MANAGEMENT");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Subtle line under section header
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.6f, 0.0f, 0.4f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    renderKeyBinding("  N           ", "Play next mix");
    renderKeyBinding("  F           ", "Toggle favorite");
    renderKeyBinding("  V           ", "List favorite mixes");
    renderKeyBinding("  L           ", "List available mixes");
    renderKeyBinding("  G           ", "Play random mix in current genre");
    renderKeyBinding("  Shift+G     ", "Switch to random genre");
    renderKeyBinding("  Ctrl+G      ", "Show available genres");
    renderKeyBinding("  SPACE       ", "Load random mix");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Decorative line
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.6f, 0.0f, 0.6f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Audio Controls Section with improved styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::TextUnformatted("AUDIO CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Subtle line under section header
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.4f, 0.8f, 1.0f, 0.4f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    renderKeyBinding("  P           ", "Pause/Resume playback");
    renderKeyBinding("  Up/Down     ", "Volume up/down");
    renderKeyBinding("  Tab         ", "Cycle through audio devices");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Decorative line
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.4f, 0.8f, 1.0f, 0.6f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Visualizer Controls Section with improved styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
    ImGui::TextUnformatted("VISUALIZER CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Subtle line under section header
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.8f, 0.4f, 1.0f, 0.4f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    renderKeyBinding("  H           ", "Toggle this help overlay");
    renderKeyBinding("  F11         ", "Toggle fullscreen mode");
    renderKeyBinding("  R           ", "Load random preset");
    renderKeyBinding("  [ / ]       ", "Previous/Next preset");
    renderKeyBinding("  B / J       ", "Increase/Decrease beat sensitivity");
    renderKeyBinding("  Mouse Wheel ", "Next/Prev preset");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Decorative line
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.8f, 0.4f, 1.0f, 0.6f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Application Section with improved styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::TextUnformatted("APPLICATION");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Subtle line under section header
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.4f, 0.4f, 0.4f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    renderKeyBinding("  Ctrl+Q      ", "Quit application");
    ImGui::PopStyleColor();
    
    // Mix Table Section (always shown if data is available)
    if (!_mixTableData.empty()) {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Decorative line
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.4f, 0.4f, 0.6f));
        ImGui::Separator();
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Mix Table Header
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::TextUnformatted("MIX DATABASE TABLE");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        // Subtle line under section header
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.2f, 0.8f, 0.2f, 0.4f));
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        // Table header
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        
        // Calculate column widths based on longest values
        float artistWidth = ImGui::CalcTextSize("Artist").x;
        float titleWidth = ImGui::CalcTextSize("Title").x;
        float genreWidth = ImGui::CalcTextSize("Genre").x;
        float durationWidth = ImGui::CalcTextSize("Duration").x;
        float playsWidth = ImGui::CalcTextSize("Plays").x;
        float favoriteWidth = ImGui::CalcTextSize("Favorite").x;
        
        // Find the longest values in each column
        for (const auto& mix : _mixTableData) {
            float artistTextWidth = ImGui::CalcTextSize(mix.artist.c_str()).x;
            float titleTextWidth = ImGui::CalcTextSize(mix.title.c_str()).x;
            float genreTextWidth = ImGui::CalcTextSize(mix.genre.c_str()).x;
            
            // Duration formatting
            int minutes = mix.duration_seconds / 60;
            int seconds = mix.duration_seconds % 60;
            std::string duration = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
            float durationTextWidth = ImGui::CalcTextSize(duration.c_str()).x;
            
            // Plays
            std::string plays = std::to_string(mix.play_count);
            float playsTextWidth = ImGui::CalcTextSize(plays.c_str()).x;
            
            // Favorite
            std::string favorite = mix.is_favorite ? "YES" : "NO";
            float favoriteTextWidth = ImGui::CalcTextSize(favorite.c_str()).x;
            
            // Update maximum widths
            artistWidth = std::max(artistWidth, artistTextWidth);
            titleWidth = std::max(titleWidth, titleTextWidth);
            genreWidth = std::max(genreWidth, genreTextWidth);
            durationWidth = std::max(durationWidth, durationTextWidth);
            playsWidth = std::max(playsWidth, playsTextWidth);
            favoriteWidth = std::max(favoriteWidth, favoriteTextWidth);
        }
        
        // Add some padding to each column
        float padding = 40.0f;  // Increased from 20.0f to 40.0f for more spacing
        artistWidth += padding;
        titleWidth += padding;
        genreWidth += padding;
        durationWidth += padding;
        playsWidth += padding;
        favoriteWidth += padding;
        
        // Calculate column positions
        float startX = ImGui::GetCursorPosX();
        float artistX = startX;
        float titleX = artistX + artistWidth;
        float genreX = titleX + titleWidth;
        float durationX = genreX + genreWidth;
        float playsX = durationX + durationWidth;
        float favoriteX = playsX + playsWidth;
        
        // Header row
        ImGui::SetCursorPosX(artistX);
        ImGui::TextUnformatted("  Artist");
        ImGui::SameLine();
        ImGui::SetCursorPosX(titleX);
        ImGui::TextUnformatted("Title");
        ImGui::SameLine();
        ImGui::SetCursorPosX(genreX);
        ImGui::TextUnformatted("Genre");
        ImGui::SameLine();
        ImGui::SetCursorPosX(durationX);
        ImGui::TextUnformatted("Duration");
        ImGui::SameLine();
        ImGui::SetCursorPosX(playsX);
        ImGui::TextUnformatted("Plays");
        ImGui::SameLine();
        ImGui::SetCursorPosX(favoriteX);
        ImGui::TextUnformatted("Favorite");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Table data
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
        
        // Sort the data: favorites first, then by artist
        std::vector<AutoVibez::Data::Mix> sortedMixes = _mixTableData;
        
        // Apply filter if showing favorites only
        if (_showFavoritesOnly) {
            sortedMixes.erase(
                std::remove_if(sortedMixes.begin(), sortedMixes.end(),
                    [](const AutoVibez::Data::Mix& mix) { return !mix.is_favorite; }),
                sortedMixes.end()
            );
        }
        
        std::sort(sortedMixes.begin(), sortedMixes.end(), 
            [](const AutoVibez::Data::Mix& a, const AutoVibez::Data::Mix& b) {
                // First sort by favorite (favorites first)
                if (a.is_favorite != b.is_favorite) {
                    return a.is_favorite > b.is_favorite;
                }
                // Then by artist
                if (a.artist != b.artist) {
                    return a.artist < b.artist;
                }
                // Finally by title
                return a.title < b.title;
            });
        
        for (const auto& mix : sortedMixes) {
            // Artist
            std::string artist = "  " + mix.artist;
            ImGui::SetCursorPosX(artistX);
            ImGui::TextUnformatted(artist.c_str());
            
            // Title
            std::string title = mix.title;
            ImGui::SameLine();
            ImGui::SetCursorPosX(titleX);
            ImGui::TextUnformatted(title.c_str());
            
            // Genre
            std::string genre = mix.genre;
            ImGui::SameLine();
            ImGui::SetCursorPosX(genreX);
            ImGui::TextUnformatted(genre.c_str());
            
            // Duration (format as MM:SS)
            int minutes = mix.duration_seconds / 60;
            int seconds = mix.duration_seconds % 60;
            std::string duration = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
            ImGui::SameLine();
            ImGui::SetCursorPosX(durationX);
            ImGui::TextUnformatted(duration.c_str());
            
            // Plays
            ImGui::SameLine();
            ImGui::SetCursorPosX(playsX);
            ImGui::TextUnformatted(std::to_string(mix.play_count).c_str());
            
            // Favorite (use text instead of emoji)
            ImGui::SameLine();
            ImGui::SetCursorPosX(favoriteX);
            if (mix.is_favorite) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                ImGui::TextUnformatted("YES");
                ImGui::PopStyleColor();
            } else {
                ImGui::TextUnformatted("NO");
            }
        }
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
    
    // Render the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    
    // Restore OpenGL state
    glPopMatrix();
    glPopAttrib();
}

void HelpOverlay::toggle() {
    _visible = !_visible;
    
    if (_visible) {
        // Only hide cursor in fullscreen mode
        if (_isFullscreen) {
            _cursorWasVisible = SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
    } else {
        // Only restore cursor if we're not in fullscreen mode
        if (!_isFullscreen) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            if (_cursorWasVisible) {
                SDL_ShowCursor(SDL_ENABLE);
            }
        }
        // In fullscreen mode, keep relative mouse mode enabled (cursor hidden)
    }
}

void HelpOverlay::setFullscreenState(bool isFullscreen) {
    _isFullscreen = isFullscreen;
}

void HelpOverlay::setCursorVisibility(bool visible) {
    if (visible) {
        SDL_SetCursor(_originalCursor);
    } else {
        SDL_SetCursor(_blankCursor);
    }
}

void HelpOverlay::rebuildFontAtlas() {
    if (_imguiReady) {
        // Ensure we have the OpenGL context
        SDL_GL_MakeCurrent(_window, _glContext);
        
        // Rebuild the font atlas
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        io.Fonts->AddFontDefault();
        
        // Build the font atlas
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        // This forces ImGui to recreate its font texture
        ImGui_ImplOpenGL2_DestroyFontsTexture();
        ImGui_ImplOpenGL2_CreateFontsTexture();
    }
}

void HelpOverlay::reinitializeImGui() {
    // Ensure we have the OpenGL context
    SDL_GL_MakeCurrent(_window, _glContext);
    
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    
    // Shutdown existing ImGui if it exists
    if (_imguiReady) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        _imguiReady = false;
    }
    
    // Setup Dear ImGui context with minimal configuration
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Add default font with explicit atlas building
    io.Fonts->AddFontDefault();
    io.FontGlobalScale = 1.0f;
    
    // Explicitly build the font atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    // Set INI file path to user config directory
    std::string configDir = getConfigDirectory();
    if (!configDir.empty()) {
        std::string iniPath = configDir + "/imgui.ini";
        io.IniFilename = strdup(iniPath.c_str());
    }
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
    ImGui_ImplOpenGL2_Init();
    
    // Explicitly create the font texture
    ImGui_ImplOpenGL2_CreateFontsTexture();
    
    _imguiReady = true;
    
    // Restore OpenGL state
    glPopMatrix();
    glPopAttrib();
}

void HelpOverlay::triggerTextureRebind() {
    _needsTextureRebind = true;
}

void HelpOverlay::triggerDeferredTextureRebind() {
    _needsDeferredTextureRebind = true;
}

void HelpOverlay::triggerCompleteReinitialization() {
    // Ensure we have the OpenGL context
    SDL_GL_MakeCurrent(_window, _glContext);
    
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    
    // Shutdown existing ImGui if it exists
    if (_imguiReady) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        _imguiReady = false;
    }
    
    // Setup Dear ImGui context with minimal configuration
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Add default font with explicit atlas building
    io.Fonts->AddFontDefault();
    io.FontGlobalScale = 1.0f;
    
    // Explicitly build the font atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    // Set INI file path to user config directory
    std::string configDir = getConfigDirectory();
    if (!configDir.empty()) {
        std::string iniPath = configDir + "/imgui.ini";
        io.IniFilename = strdup(iniPath.c_str());
    }
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
    ImGui_ImplOpenGL2_Init();
    
    // Explicitly create the font texture
    ImGui_ImplOpenGL2_CreateFontsTexture();
    
    _imguiReady = true;
    
    // Restore OpenGL state
    glPopMatrix();
    glPopAttrib();
}

// Dynamic information methods
void HelpOverlay::setCurrentPreset(const std::string& preset) {
    _currentPreset = preset;
}

void HelpOverlay::setCurrentMix(const std::string& artist, const std::string& title, const std::string& genre) {
    _currentArtist = artist;
    _currentTitle = title;
    _currentGenre = genre;
}

void HelpOverlay::setVolumeLevel(int volume) {
    _volumeLevel = volume;
}

void HelpOverlay::setAudioDevice(const std::string& device) {
    _audioDevice = device;
}

void HelpOverlay::setBeatSensitivity(float sensitivity) {
    _beatSensitivity = sensitivity;
}

// Mix table methods
void HelpOverlay::setMixTableData(const std::vector<AutoVibez::Data::Mix>& mixes) {
    _mixTableData = mixes;
}

void HelpOverlay::toggleMixTableFilter() {
    _showFavoritesOnly = !_showFavoritesOnly;
}

} // namespace UI
} // namespace AutoVibez 