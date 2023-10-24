#include "Texture.h"

#include <utility>

Texture::Texture(SDL_Texture* texture)
    : _texture(texture)
{
}

Texture::~Texture()
{
    ReleaseIfNotEmpty();
}

Texture::Texture(Texture&& other) noexcept
{
    *this = std::move(other);
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    ReleaseIfNotEmpty();

    _texture = other._texture;
    other._texture = nullptr;

    return *this;
}

SDL_Texture* Texture::operator*()
{
    return _texture;
}

SDL_Texture* Texture::operator*() const
{
    return _texture;
}

Texture::operator bool() const
{
    return _texture != nullptr;
}

void Texture::ReleaseIfNotEmpty()
{
    if (_texture) {
        SDL_DestroyTexture(_texture);
    }
}
