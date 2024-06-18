#pragma once

#include <SFML/Graphics.hpp>

#include "vectors.h"

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

using Drawable = std::variant<
    Line,
    LineStrip,
    sf::CircleShape,
    sf::RectangleShape,
    sf::ConvexShape,
    sf::Sprite,
    sf::Text>;

void drawDrawable(Drawable& drawable, sf::RenderTarget& target) {
    std::visit([&target](auto&& arg) { target.draw(arg); }, drawable);
}

template <typename T, typename Func>
concept CallableWith = requires(Func f, T t) {
    { f(t) } -> std::convertible_to<Vec2>;
};

struct LayeredDrawer {
    std::vector<std::vector<Drawable>> layers;
    const bool                         clearEveryFrame = false;

    LayeredDrawer(int numLayers = 1) : layers(numLayers) {}

    void draw(Drawable drawable, int layer = -1) {
        layer = layer == -1 ? this->layers.size() - 1 : layer;
        this->layers[layer].push_back(std::move(drawable));
    }

    void line(const Vec2 start, const Vec2 end, int layer = -1) {
        layer = layer == -1 ? this->layers.size() - 1 : layer;
        this->layers[layer].push_back(Line{start, end});
    }

    void lineStrip(
        const std::vector<Vec2>& points,
        sf::Color                color = sf::Color::Red,
        int                      layer = 0
    ) {
        this->layers[layer].push_back(LineStrip{points, color});
    }

    template <std::forward_iterator It, typename Func>
        requires CallableWith<
            typename std::iterator_traits<It>::value_type,
            Func>
    void lineStripMap(It begin, It end, Func toWorld, int layer = -1) {
        layer = layer == -1 ? this->layers.size() - 1 : layer;
        std::vector<Vec2> path;
        std::transform(begin, end, std::back_inserter(path), toWorld);
        this->lineStrip(path, sf::Color::Red, layer);
    }

    void point(const Vec2 point, int layer = -1) {
        layer = layer == -1 ? this->layers.size() - 1 : layer;
        sf::CircleShape circle(2);
        circle.setPosition(point);
        circle.setFillColor(sf::Color::Red);
        this->layers[layer].push_back(circle);
    }

    void clear(int layer) {
        this->layers[layer].clear();
    }

    void display(sf::RenderWindow& window) {
        for (auto& layer : this->layers) {
            for (const auto& drawable : layer) {
                std::visit(
                    [&window](auto&& arg) { window.draw(arg); }, drawable
                );
            }
            if (this->clearEveryFrame) {
                layer.clear();
            }
        }
    }
};