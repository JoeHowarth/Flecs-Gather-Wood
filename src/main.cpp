#include <flecs.h>
#include <fmt/core.h>

#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <random>

#include "components.h"
#include "gather_wood_behavior.h"
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

void simulationUpdate(
    flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
) {
    Tick* tick = ecs.get_mut<Tick>();
    tick->v += 1;
    fmt::println("\n[simulationUpdate] tick: {}", tick->v);

    gatherWoodBehaviorNaive(ecs, map, pathfinder);
}

int main() {
    auto      window = sf::RenderWindow{{1920u, 1080u}, "Watchem Gatherum"};
    sf::View  view   = initWindow(window);
    sf::Clock frameClock;
    sf::Clock simulationClock;
    const int SIM_TICK_MS = 500;

    flecs::world ecs;
    Tilemap      map        = makeTilemap();
    Pathfinder   pathfinder = pathfinderFromTilemap(map);

    ecs.set<flecs::Rest>({});

    registerComponents(ecs);
    spawnWorkers(ecs, 1, map);
    spawnTrees(ecs, 2, map);

    for (int frame = 0; window.isOpen(); ++frame) {
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

        if (simulationClock.getElapsedTime().asMilliseconds() > SIM_TICK_MS) {
            // Only clear the debug drawer every simulation tick since
            // it will only be written to during the simulation update
            // and otherwise it will be cleared every frame
            debugDrawer.clear(SIM_DEBUG_LAYER);

            simulationUpdate(ecs, map, pathfinder);
            simulationClock.restart();
        };

        map.render(window);

        renderWorkers(ecs, map);
        renderTrees(ecs, map);
        renderWood(ecs, map);

        ecs.progress(deltaTime.asSeconds());

        textDrawer.display(window);
        debugDrawer.display(window);
        window.display();
    }
}

Pathfinder pathfinderFromTilemap(const Tilemap& map) {
    std::vector<unsigned char> pathmap(map.tiles.size());
    std::transform(
        map.tiles.begin(), map.tiles.end(), pathmap.begin(),
        [](Tilemap::TileType t) { return t == Tilemap::Grass ? 1 : 0; }
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