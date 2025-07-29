#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Simple UI System using SDL2_ttf for proper text rendering
class UISystem {
public:
    UISystem();
    ~UISystem();
    
    void init(SDL_Window* window);
    void render();
    bool handleMouseClick(float mouseX, float mouseY);
    void handleMouseMove(float mouseX, float mouseY);
    void handleMouseDrag(float mouseX, float mouseY);
    void toggleControlPanel();
    void toggleHelpOverlay();
    void showMixStatus(const Mix& mix);
    void updateMixStatus(const Mix& mix, int position, int duration, int volume);
    void updateVolume(int volume);
    
    // Simple button class
    class Button {
    public:
        Button(float x, float y, float width, float height, const std::string& text);
        void render(SDL_Renderer* renderer, TTF_Font* font);
        bool handleClick(float mouseX, float mouseY);
        void setCallback(std::function<void()> callback) { onClick = callback; }
        
    private:
        float x, y, width, height;
        std::string text;
        std::function<void()> onClick;
        bool hovered = false;
    };
    
    // Add buttons to the UI
    void addButton(std::shared_ptr<Button> button);
    
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    std::vector<std::shared_ptr<Button>> buttons;
    
    void renderText(const std::string& text, float x, float y, SDL_Color color);
    void renderRect(float x, float y, float width, float height, SDL_Color color);
}; 