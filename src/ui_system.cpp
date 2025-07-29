#include "ui_system.hpp"
#include "config_manager.hpp"
#include "setup.hpp"
#include <iostream>
#include <filesystem>

// UISystem implementation
UISystem::UISystem() {
}

UISystem::~UISystem() {
    if (font) {
        TTF_CloseFont(font);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
}

void UISystem::init(SDL_Window* window) {
    this->window = window;
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Initialize TTF
    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize TTF: " << TTF_GetError() << std::endl;
        return;
    }
    
    // Load font using configuration file approach
    std::string fontPath = getFontPathFromConfig();
    font = TTF_OpenFont(fontPath.c_str(), 16);
    if (!font) {
        std::cerr << "Failed to load font from config: " << TTF_GetError() << std::endl;
        std::cerr << "Trying fallback fonts..." << std::endl;
        
        // Fallback to system fonts
        std::vector<std::string> fallbackFonts = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/Library/Fonts/Arial.ttf",
            "C:/Windows/Fonts/arial.ttf"
        };
        
        for (const auto& fallbackPath : fallbackFonts) {
            font = TTF_OpenFont(fallbackPath.c_str(), 16);
            if (font) {
                std::cout << "✅ Loaded fallback font: " << fallbackPath << std::endl;
                break;
            }
        }
    } else {
        std::cout << "✅ Loaded font from config: " << fontPath << std::endl;
    }
    
    if (!font) {
        std::cerr << "Failed to load any font: " << TTF_GetError() << std::endl;
        return;
    }
    
    std::cout << "✅ UI System initialized successfully" << std::endl;
}

std::string UISystem::getFontPathFromConfig() {
    // Try to get font path from config file
    std::string configPath = findConfigFile();
    if (!configPath.empty()) {
        try {
            ConfigFile config(configPath);
            std::string fontPath = config.getFontPath();
            if (!fontPath.empty()) {
                return fontPath;
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not read font_path from config: " << e.what() << std::endl;
        }
    }
    
    // Return default font path if config doesn't specify one
    return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
}

void UISystem::render() {
    if (!renderer || !font) return;
    
    // Clear renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render all buttons
    for (auto& button : buttons) {
        button->render(renderer, font);
    }
    
    // Present renderer
    SDL_RenderPresent(renderer);
}

bool UISystem::handleMouseClick(float mouseX, float mouseY) {
    for (auto& button : buttons) {
        if (button->handleClick(mouseX, mouseY)) {
            return true;
        }
    }
    return false;
}

void UISystem::handleMouseMove(float mouseX, float mouseY) {
    // Handle mouse hover effects if needed
}

void UISystem::handleMouseDrag(float mouseX, float mouseY) {
    // Handle mouse drag effects if needed
}

void UISystem::toggleControlPanel() {
    // Toggle control panel visibility
}

void UISystem::toggleHelpOverlay() {
    // Toggle help overlay visibility
}

void UISystem::showMixStatus(const Mix& mix) {
    // Show mix status
}

void UISystem::updateMixStatus(const Mix& mix, int position, int duration, int volume) {
    // Update mix status
}

void UISystem::updateVolume(int volume) {
    // Update volume
}

void UISystem::addButton(std::shared_ptr<Button> button) {
    buttons.push_back(button);
}

void UISystem::renderText(const std::string& text, float x, float y, SDL_Color color) {
    if (!font || !renderer) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dest = {(int)x, (int)y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void UISystem::renderRect(float x, float y, float width, float height, SDL_Color color) {
    if (!renderer) return;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {(int)x, (int)y, (int)width, (int)height};
    SDL_RenderFillRect(renderer, &rect);
}

// Button implementation
UISystem::Button::Button(float x, float y, float width, float height, const std::string& text)
    : x(x), y(y), width(width), height(height), text(text) {
}

void UISystem::Button::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!renderer || !font) return;
    
    // Button background
    SDL_Color bgColor = hovered ? SDL_Color{100, 100, 100, 255} : SDL_Color{80, 80, 80, 255};
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect rect = {(int)x, (int)y, (int)width, (int)height};
    SDL_RenderFillRect(renderer, &rect);
    
    // Button border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Button text
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    // Center text in button
    int textX = (int)(x + (width - surface->w) / 2);
    int textY = (int)(y + (height - surface->h) / 2);
    SDL_Rect dest = {textX, textY, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

bool UISystem::Button::handleClick(float mouseX, float mouseY) {
    if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height) {
        if (onClick) {
            onClick();
        }
        return true;
    }
    return false;
} 