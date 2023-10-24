#pragma once

#include <SDL.h>

class Texture {
public:
    Texture() = default;
    explicit Texture(SDL_Texture* texture);
    ~Texture();

    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    SDL_Texture* operator*();
    SDL_Texture* operator*() const;
    operator bool() const;

private:
    SDL_Texture* _texture = nullptr;

    void ReleaseIfNotEmpty();
};
