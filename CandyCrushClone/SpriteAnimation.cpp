#include "SpriteAnimation.h"
#include "Screen.h"

#include <cmath>
#include <filesystem>

SpriteAnimation::SpriteAnimation(const std::string& animationDirectory, const Screen& screen)
    : _screen(&screen)
{
    LoadAssets(animationDirectory);
}

void SpriteAnimation::Draw(Vec2 location, int frameSize, double progress)
{
    SDL_Rect dstRect { location.x, location.y, frameSize, frameSize };

    auto indexToDraw = std::min(size_t(progress * _textures.size()), _textures.size() - 1);

    _screen->DrawTexture(_textures[indexToDraw], nullptr, &dstRect);
}

void SpriteAnimation::LoadAssets(const std::string& animationDirectory)
{
    std::vector<std::pair<std::string, Texture>> textures;

    auto assetPath = std::filesystem::current_path() / animationDirectory;
    for (auto const& dirEntry : std::filesystem::directory_iterator { assetPath }) {
        auto texture = _screen->LoadImage(dirEntry.path().string());
        textures.push_back({ dirEntry.path().filename().string(), std::move(texture) });
    }

    // Sort the iamges based on their name (as we expect the sprite animation frames to be in numbered order)
    std::sort(textures.begin(), textures.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

    _textures.reserve(textures.size());
    std::transform(textures.begin(), textures.end(), std::back_inserter(_textures), [](auto&& pair) { return std::move(pair.second); });
}
