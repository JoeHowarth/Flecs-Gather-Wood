#pragma once

#include <flecs.h>

#include "components.h"
#include "utils/util.h"

void renderWorkers(flecs::world& ecs, const Tilemap& map) {
    ecs.each([&map](WorkerTag, Position& pos) {
        auto worldPos = map.tileToWorld(pos);

        textDrawer.draw(
            {.pos = worldPos, .size = 20, .color = sf::Color(20, 20, 20)}, "W"
        );
    });
}

void spawnWorkers(flecs::world& ecs, int count, Tilemap map) {
    auto dist_w = std::uniform_int_distribution<int>(0, map.dim.x - 1);
    auto dist_h = std::uniform_int_distribution<int>(0, map.dim.y - 1);

    for (int i = 0; i < count; i++) {
        flecs::entity e = ecs.entity();
        e.add<WorkerTag>();
        e.set<Position>(randomTile(Tilemap::Grass, map));
        e.set<GatherWoodBehavior>({.state = Idle{}, .hasWood = false});
    }
}
