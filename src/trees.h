#pragma once

#include <flecs.h>
#include <fmt/core.h>

#include <iostream>

#include "components.h"
#include "tilemap.h"

void spawnTrees(flecs::world& ecs, int count, const Tilemap& map) {
    auto dist_w = std::uniform_int_distribution<int>(0, map.dim.x);
    auto dist_h = std::uniform_int_distribution<int>(0, map.dim.y);

    for (int i = 0; i < count; i++) {
        Position pos;
        do {
            pos = Position(Vec2I(dist_w(gen), dist_h(gen)));
        } while (map[pos] != Tilemap::Grass);
        ecs.entity().add<TreeTag>().set<Position>(pos);
    }
}

void renderTrees(flecs::world& ecs, const Tilemap& map) {
    ecs.each([&map](TreeTag, const Position& pos) {
        auto worldPos = map.tileToWorld(pos);
        textDrawer.draw(
            {.pos   = worldPos - Vec2(20, 10),
             .size  = 20,
             .color = sf::Color(150, 105, 25)},
            "T"
        );
    });
}