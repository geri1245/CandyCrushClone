#pragma once

#include "SpriteAnimation.h"
#include "Texture.h"
#include "Vec2.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Screen {
public:
    static constexpr int ScreenWidth = 1024;
    static constexpr int ScreenHeight = 560;

    static std::unique_ptr<Screen> GetScreen();
    ~Screen();

    void TerminateWithMessage(const std::string& errorText);

    void BeginFrame() const;
    void DrawCell(Vec2 coords, int cellType, int sourceSize, int destinationSize) const;
    void DrawDestroyAnimation(Vec2 coords, int size, double progress);
    void DrawTexture(const Texture& texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect) const;
    void Present() const;

    void DrawButton(const std::string& text, const SDL_Rect& coords, bool isHovered) const;
    void DrawText(const std::string& text, const SDL_Rect& textRect, bool useLargeFont, SDL_Color color = { 255, 255, 255 }) const;
    void DrawBackgroundRectangle(const SDL_Rect& rect, SDL_Color color = { 50, 50, 50, 100 }) const;

    Texture LoadImage(const std::string& filePath) const;

private:
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    SDL_Texture* _renderTarget = nullptr;

    std::vector<Texture> _assetImages;
    Texture _backgroundImage;
    Texture _menuButton;
    TTF_Font* _bigFont = nullptr;
    TTF_Font* _smallFont = nullptr;

    std::unique_ptr<SpriteAnimation> _gravityAnimation;

    bool Initialize();
    bool LoadAssets();
};
