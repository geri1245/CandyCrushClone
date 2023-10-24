#include "Vec2.h"

#include <cmath>

int Vec2::DistanceSquared(const Vec2& other) const
{
    return int(std::pow(x - other.x, 2)) + int(std::pow(y - other.y, 2));
}

Vec2 Vec2::Lerp(const Vec2& other, double progress) const
{
    return Vec2 { int(std::lerp(x, other.x, progress)), int(std::lerp(y, other.y, progress)) };
}

Vec2 Vec2::KeepGreaterComponent() const
{
    return (abs(x) > abs(y)) ? Vec2 { x, 0 } : Vec2 { 0, y };
}

Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2 { lhs.x + rhs.x, lhs.y + rhs.y };
}

Vec2 operator*(const Vec2& point, int multiplier)
{
    return Vec2 { point.x * multiplier, point.y * multiplier };
}

Vec2 operator/(const Vec2& point, int dividend)
{
    return Vec2 { point.x / dividend, point.y / dividend };
}

Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
    return lhs + rhs * -1;
}