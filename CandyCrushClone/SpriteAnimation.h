#pragma once

#include "Texture.h"
#include "Vec2.h"

#include <SDL.h>

#include <string>
#include <vector>

class Screen;

class SpriteAnimation {
public:
    SpriteAnimation(const std::string& animationDirectory, const Screen& screen);

    void Draw(Vec2 location, int frameSize, double progress);

private:
    const Screen* _screen;

    std::vector<Texture> _textures;

    void LoadAssets(const std::string& animationDirectory);
};
