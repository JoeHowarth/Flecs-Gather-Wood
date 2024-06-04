#pragma once

#include <flecs.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <SFML/Graphics.hpp>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iomanip>  // Include this header for std::fixed and std::setprecision
#include <iostream>
#include <random>
#include <sstream>
#include <utility>

using Vec2I = sf::Vector2i;
using Vec2U = sf::Vector2u;
using Vec2  = sf::Vector2f;

// Note: Utility globals defined at bottom of the file

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

// template <typename... Args>
// void print(Args&&... args) {
//   std::stringstream ss;
//   (ss << ...
//       << std::forward<Args>(
//              args));  // Fold expression to handle multiple arguments
//   std::cout << ss.str() << std::endl;
// }

template <typename T>
T dbg(const T s) {
    std::cout << s << std::endl;
    return s;
}

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

template <>
struct fmt::formatter<flecs::entity> : fmt::ostream_formatter {};

/**** Drawing ******/

sf::Text makeText(
    const std::string& str,
    const Vec2&        pos,
    sf::Font&          font,
    uint8_t            size  = 12,
    sf::Color          color = sf::Color::White
) {
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(pos);
    return text;
}

template <typename Renderable, typename... Args>
void drawText(
    Renderable& window, const Vec2& pos, sf::Font& font, Args&&... args
) {
    std::stringstream ss;
    (ss << ... << std::forward<Args>(args)
    );  // Fold expression to handle multiple arguments
    window.draw(makeText(ss.str(), pos, font));
}

sf::Font loadFont(const std::string& path) {
    sf::Font font;
    if (!font.loadFromFile(path)) {
        throw std::runtime_error(
            "Failed to load font: " + path + "\n. Current working directory: " +
            std::filesystem::current_path().string() + "\n"
        );
    }
    return font;
}

struct Line : public sf::Drawable {
    sf::Vertex vertices[2];

    Line(const Vec2 start, const Vec2 end) {
        this->vertices[0] = start;
        this->vertices[1] = end;
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw((sf::Vertex*)vertices, 2, sf::Lines, states);
    }
};

struct LineStrip : public sf::Drawable {
    std::vector<sf::Vertex> vertices;

    LineStrip(const std::vector<Vec2>& points) {
        LineStrip(points, sf::Color::Red);
    }

    LineStrip(const std::vector<Vec2>& points, sf::Color color) {
        for (const auto& point : points) {
            this->vertices.push_back(sf::Vertex(point, color));
        }
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(vertices.data(), vertices.size(), sf::LineStrip, states);
    }
};

template <typename T, typename Func>
concept CallableWith = requires(Func f, T t) {
    { f(t) } -> std::convertible_to<Vec2>;
};

struct LayeredDrawer {
    std::vector<std::vector<std::unique_ptr<sf::Drawable>>> layers;
    const bool clearEveryFrame = false;

    LayeredDrawer(int numLayers = 1) : layers(numLayers) {}

    void draw(std::unique_ptr<sf::Drawable> drawable, int layer = 0) {
        this->layers[layer].push_back(std::move(drawable));
    }

    void line(const Vec2 start, const Vec2 end, int layer = 0) {
        this->layers[layer].push_back(std::make_unique<Line>(start, end));
    }

    void lineStrip(
        const std::vector<Vec2>& points,
        sf::Color                color = sf::Color::Red,
        int                      layer = 0
    ) {
        this->layers[layer].push_back(std::make_unique<LineStrip>(points, color)
        );
    }

    template <std::forward_iterator It, typename Func>
        requires CallableWith<
            typename std::iterator_traits<It>::value_type,
            Func>
    void lineStripMap(It begin, It end, Func toWorld) {
        std::vector<Vec2> path;
        std::transform(begin, end, std::back_inserter(path), toWorld);
        this->lineStrip(path, sf::Color::Red);
    }

    void point(const Vec2 point, int layer = 0) {
        std::unique_ptr<sf::CircleShape> circle =
            std::make_unique<sf::CircleShape>(2);
        circle->setPosition(point);
        circle->setFillColor(sf::Color::Red);
        this->layers[layer].push_back(
            std::unique_ptr<sf::Drawable>(std::move(circle))
        );
    }

    void clear(int layer) {
        this->layers[layer].clear();
    }

    void display(sf::RenderWindow& window) {
        for (auto& layer : this->layers) {
            for (const auto& drawable : layer) {
                window.draw(*drawable);
            }
            if (this->clearEveryFrame) {
                layer.clear();
            }
        }
    }
};

struct TextDrawer {
    struct Text {
        Vec2        pos;
        std::string str;
        uint8_t     size  = 12;
        sf::Color   color = sf::Color::White;
    };

    struct Opts {
        Vec2      pos;
        uint8_t   size  = 12;
        sf::Color color = sf::Color::White;
    };

    sf::Font          font;
    std::vector<Text> texts;

    TextDrawer(const std::string& fontPath) : font(loadFont(fontPath)) {}

    template <typename... Args>
    void draw(const Vec2& pos, Args&&... args) {
        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        this->texts.push_back({pos, ss.str()});
    }

    template <typename... Args>
    void draw(const Opts& opts, Args&&... args) {
        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        this->texts.push_back(
            {.pos   = opts.pos,
             .str   = ss.str(),
             .size  = opts.size,
             .color = opts.color}
        );
    }

    void display(sf::RenderWindow& window) {
        for (const auto& text : this->texts) {
            window.draw(
                makeText(text.str, text.pos, font, text.size, text.color)
            );
        }
        this->texts.clear();
    }
};

auto now() {
    return std::chrono::high_resolution_clock::now();
}

/**** Bad Globals ****/
TextDrawer    textDrawer("./open-sans/OpenSans-Bold.ttf");
LayeredDrawer debugDrawer(1);
const int     SIM_DEBUG_LAYER = 0;

std::random_device rd;         // Seed
std::mt19937       gen(rd());  // Mersenne Twister engine