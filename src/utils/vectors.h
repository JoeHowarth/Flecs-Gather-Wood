#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <SFML/Graphics.hpp>
#include <random>

using Vec2I = sf::Vector2i;
using Vec2U = sf::Vector2u;
using Vec2  = sf::Vector2f;

/**** Math ****/

Vec2 vec(float x, float y) {
    return {x, y};
}

float to_radians(float degrees) {
    return degrees * (3.14159265f / 180.f);
}
Vec2 move_forward(float degrees, float distance) {
    float radians = to_radians(degrees - 90);
    return {std::cos(radians) * distance, std::sin(radians) * distance};
}

float crossProduct(const Vec2& v1, const Vec2& v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

// Function to calculate the magnitude of a vector
float magnitude(const Vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
float magnitude(const Vec2I& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
float magnitude(const Vec2U& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
int magnitude2(const Vec2I& v) {
    return v.x * v.x + v.y * v.y;
}

// Function to normalize a vector (get the unit vector)
Vec2 normalize(const Vec2& v) {
    float mag = magnitude(v);
    if (mag == 0) {
        return {0, 0};  // Avoid division by zero
    }
    return {v.x / mag, v.y / mag};
}

float randomFloat(float min, float max) {
    static std::default_random_engine     engine;  // Default random engine
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}

// Function to generate a random Vec2 within the given range
Vec2 randomVector2f(float minX, float maxX, float minY, float maxY) {
    return {randomFloat(minX, maxX), randomFloat(minY, maxY)};
}

Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
    return a + (b - a) * t;
}

template <typename T>
bool inBounds(T a, T b) {
    return a.x >= 0 && a.x < b.x && a.y >= 0 && a.y < b.y;
}

template <typename T>
std::pair<T, T> toPair(const sf::Vector2<T>& v) {
    return {v.x, v.y};
}

/**** Printing ****/

namespace sf {

std::ostream& operator<<(std::ostream& os, const Vector2u& vec) {
    os << std::fixed << std::setprecision(1);
    os << "(" << std::setw(4) << vec.x << ", " << std::setw(4) << vec.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Vector2i& vec) {
    os << std::fixed << std::setprecision(1);
    os << "(" << std::setw(4) << vec.x << ", " << std::setw(4) << vec.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Vector2f& vec) {
    os << std::fixed << std::setprecision(1);
    os << "(" << std::setw(6) << vec.x << ", " << std::setw(6) << vec.y << ")";
    return os;
}

}  // namespace sf

template <>
struct fmt::formatter<sf::Vector2f> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<sf::Vector2i> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<sf::Vector2u> : fmt::ostream_formatter {};