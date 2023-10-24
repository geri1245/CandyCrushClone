#pragma once

#include <compare>

struct Vec2 {
    int x;
    int y;

    std::strong_ordering operator<=>(const Vec2& other) const = default;

    int DistanceSquared(const Vec2& other) const;
    Vec2 Lerp(const Vec2& other, double progress) const;
    Vec2 KeepGreaterComponent() const;
};

Vec2 operator+(const Vec2& lhs, const Vec2& rhs);
Vec2 operator*(const Vec2& point, int multiplier);
Vec2 operator/(const Vec2& point, int dividend);
Vec2 operator-(const Vec2& lhs, const Vec2& rhs);

// From cppreference
template <>
struct std::hash<Vec2> {
    std::size_t operator()(const Vec2& p) const noexcept
    {
        std::size_t h1 = std::hash<int> {}(p.x);
        std::size_t h2 = std::hash<int> {}(p.y);
        return h1 ^ (h2 << 1);
    }
};
