#include <flecs.h>
#include <fmt/core.h>

#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <random>

#include "components.h"
#include "gather_wood_behavior.h"
// #include "htn/htn.h"
#include "htn/htn2.h"
#include "pathfinder.h"
#include "tilemap.h"
#include "trees.h"
#include "utils/util.h"
#include "workers.h"

Tilemap makeTilemap() {
    using Tilemap::Grass;
    using Tilemap::Water;
    auto G = Tilemap::Grass;
    auto W = Tilemap::Water;

    return {
        {10, 10},
        {
            W, G, G, G, W, W, G, G, G, W,  // don't format
            W, G, G, G, G, G, G, G, G, G,  // don't format
            G, G, G, G, G, G, G, G, G, G,  // don't format
            W, G, G, G, W, W, G, G, G, W,  // don't format
            W, G, G, G, W, G, G, G, G, G,  // don't format
            W, G, G, G, W, G, G, G, G, G,  // don't format
            W, G, G, G, W, G, G, G, G, G,  // don't format
            W, G, G, G, W, G, G, G, G, G,  // don't format
            W, G, G, G, W, G, G, G, G, G,  // don't format
            W, G, G, G, W, W, G, G, G, W,  // don't format

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

    ecs.each([](const Count& count, const Position& pos) {
        fmt::println("Wood at {}, count: {}", pos.v, count.v);
    });
}

int main() {
    htn_main2();
    return 0;

    auto      window = sf::RenderWindow{{1920u, 1080u}, "Watchem Gatherum"};
    sf::View  view   = initWindow(window);
    sf::Clock frameClock;
    sf::Clock simulationClock;
    const int SIM_TICK_MS = 200;

    flecs::world ecs;
    Tilemap      map        = makeTilemap();
    Pathfinder   pathfinder = pathfinderFromTilemap(map);

    ecs.set<flecs::Rest>({});

    registerComponents(ecs);
    spawnWorkers(ecs, 3, map);
    spawnTrees(ecs, 10, map);

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
    // view.setCenter(0, 0);
    view.setSize(
        static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)
    );
    window.setView(view);
    return view;
}