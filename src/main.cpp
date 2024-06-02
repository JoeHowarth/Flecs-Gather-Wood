#include <flecs.h>
#include <fmt/core.h>

#include <SFML/Graphics.hpp>
#include <iostream>

#include "pathfinding/astar.h"

sf::View initWindow(sf::RenderWindow& window) {
    window.setFramerateLimit(144);
    sf::Vector2u windowSize = window.getSize();
    sf::View     view;
    view.setCenter(0, 0);
    view.setSize(
        static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)
    );
    window.setView(view);
    return view;
}

enum TileType {
    Water,
    Grass,
    Tree
};

std::array<TileType, 25> tiles = {
    Water, Grass, Grass, Grass, Water,  // don't format
    Water, Grass, Grass, Grass, Grass,  // don't format
    Grass, Grass, Grass, Grass, Grass,  // don't format
    Water, Grass, Tree,  Tree,  Water,  // don't format
    Water, Grass, Tree,  Grass, Water,  // don't format
};

int main() {
    TestSimplePath();
    TestPathWithObstacle();

    auto     window = sf::RenderWindow{{1920u, 1080u}, "Watchem Gatherum"};
    sf::View view   = initWindow(window);

    flecs::world ecs;

    ecs.component<sf::Vector2i>("Position");
    ecs.entity("Player").set<sf::Vector2i>({0, 0});

    while (window.isOpen()) {
        window.clear(sf::Color::Black);

        for (auto event = sf::Event{}; window.pollEvent(event);) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::Resized:
                    view.setSize(
                        static_cast<float>(event.size.width),
                        static_cast<float>(event.size.height)
                    );
                    window.setView(view);
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                    break;
                default:
                    break;
            }
        }

        auto w = tiles.size() / 5;
        for (int i = 0; i < tiles.size(); i++) {
            const int x = i % w;
            const int y = i / w;

            sf::RectangleShape tile;
            tile.setSize({100, 100});
            tile.setPosition(x * 100, y * 100);
            if (tiles[i] == 1) {
                tile.setFillColor(sf::Color::Green);
            } else {
                tile.setFillColor(sf::Color::Blue);
            }
            window.draw(tile);
        }

        ecs.each([=, &window](flecs::entity e, sf::Vector2i& pos) {
            pos.x = pos.x + 1;
            pos.y += pos.x / w;
            fmt::println("x: {}, y: {}", pos.x, pos.y);

            sf::RectangleShape player;
            player.setSize({100, 100});
            player.setPosition(pos.x * 100, pos.y * 100);
            player.setFillColor(sf::Color::Red);
            window.draw(player);
        });

        window.display();
    }

    // std::cout << "Hello, World!" << std::endl;
}