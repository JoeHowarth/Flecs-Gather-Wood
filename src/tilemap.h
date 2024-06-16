#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "components.h"
#include "util.h"

struct Tilemap {
    enum TileType {
        Water,
        Grass,
    };
    static inline int     renderDim = 100;
    Vec2I                 dim;
    std::vector<TileType> tiles;

    void render(sf::RenderTarget& window) {
        for (int i = 0; i < tiles.size(); i++) {
            const int x = i % dim.x;
            const int y = i / dim.x;

            sf::RectangleShape tile;
            tile.setSize({(float)renderDim, (float)renderDim});
            tile.setPosition(x * renderDim, y * renderDim);
            switch (tiles[i]) {
                case Grass:
                    tile.setFillColor(sf::Color::Green);
                    break;
                case Water:
                    tile.setFillColor(sf::Color::Blue);
                    break;
            }
            tile.setOutlineColor(sf::Color(50, 50, 50));
            tile.setOutlineThickness(1.f);
            window.draw(tile);
        }
    }

    TileType operator[](const Position pos) const {
        return get(pos);
    }

    TileType get(Position pos) const {
        return tiles[pos.v.y * dim.x + pos.v.x];
    }

    Vec2I worldToTile(Vec2 pos) const {
        return {
            static_cast<int>(pos.x / renderDim),
            static_cast<int>(pos.y / renderDim)
        };
    }

    Vec2 tileToWorld(Vec2I pos, bool center = true) const {
        // Center the tile
        const float offset = center ? 0.5f : 0.f;
        return {
            (static_cast<float>(pos.x) + offset) * renderDim,
            (static_cast<float>(pos.y) + offset) * renderDim
        };
    }
    Vec2 tileToWorld(Position pos, bool center = true) const {
        return tileToWorld(pos.v, center);
    }
};

Position randomTile(Tilemap::TileType type, const Tilemap& map) {
    auto     dist_w = std::uniform_int_distribution<int>(0, map.dim.x);
    auto     dist_h = std::uniform_int_distribution<int>(0, map.dim.y);
    Position pos;
    for (int i = 0; i < 1000; ++i) {
        pos = Position(Vec2I(dist_w(gen), dist_h(gen)));
        if (map[pos] == type) {
            return pos;
        }
    }
    std::cerr << "Failed to find a tile of type " << type << std::endl;
    throw std::runtime_error("Failed to find a tile of type");
}