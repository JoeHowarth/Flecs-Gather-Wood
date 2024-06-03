#include <flecs.h>
#include <fmt/core.h>

#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <random>

#include "components.h"
#include "newtype.h"
#include "pathfinder.h"
#include "tilemap.h"
#include "trees.h"
#include "util.h"
#include "workers.h"

Tilemap makeTilemap() {
    using Tilemap::Grass;
    using Tilemap::Water;

    return {
        {5, 5},
        {
            Water, Grass, Grass, Grass, Water,  // don't format
            Water, Grass, Grass, Grass, Grass,  // don't format
            Grass, Grass, Grass, Grass, Grass,  // don't format
            Water, Grass, Grass, Grass, Water,  // don't format
            Water, Grass, Grass, Grass, Water,  // don't format
        }
    };
}

sf::View   initWindow(sf::RenderWindow& window);
Pathfinder pathfinderFromTilemap(const Tilemap& map);
void       simulationUpdate(
          flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
      );

int main() {
    auto      window = sf::RenderWindow{{1920u, 1080u}, "Watchem Gatherum"};
    sf::View  view   = initWindow(window);
    sf::Clock frameClock;
    sf::Clock simulationClock;

    flecs::world ecs;
    Tilemap      map        = makeTilemap();
    Pathfinder   pathfinder = pathfinderFromTilemap(map);

    registerComponents(ecs);
    spawnWorkers(ecs, 2, map);
    spawnTrees(ecs, 15, map);

    for (int frame = -1; window.isOpen(); ++frame) {
        sf::Time deltaTime = frameClock.restart();
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

        if (simulationClock.getElapsedTime().asMilliseconds() > 500) {
            simulationUpdate(ecs, map, pathfinder);
            simulationClock.restart();
        };

        map.render(window);

        renderWorkers(ecs, map);
        renderTrees(ecs, map);

        textDrawer.display(window);
        debugDrawer.display(window);
        window.display();
    }

    // std::cout << "Hello, World!" << std::endl;
}

void simulationUpdate(
    flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
) {
    Tick* tick = ecs.get_mut<Tick>();
    tick->v += 1;
    fmt::println("[simulationUpdate] tick: {}\n", tick->v);

    assignTasks(ecs, map);
    updateMoveTo(ecs, map, pathfinder);
}

Pathfinder pathfinderFromTilemap(const Tilemap& map) {
    std::vector<unsigned char> pathmap(map.tiles.size());
    std::transform(
        map.tiles.begin(), map.tiles.end(), pathmap.begin(),
        [](Tilemap::TileType t) { return t == Tilemap::Grass ? 0 : 1; }
    );
    return Pathfinder{.map = pathmap, .mapDim = map.dim};
}

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