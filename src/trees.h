#pragma once

#include <flecs.h>
#include <fmt/core.h>

#include <iostream>

#include "components.h"
#include "tilemap.h"

void spawnWood(flecs::world& ecs, const Position& pos) {
    bool found;
    ecs.each([&pos, &found](
                 flecs::entity e, Count& count, WoodTag, const Position& p
             ) {
        if (p == pos) {
            fmt::println("Incrementing wood count at {}, {}", pos.v, count.v);
            count.v += 1;
            found = true;
            return;
        }
    });
    if (found) return;
    ecs.entity().add<WoodTag>().set<Position>(pos).set<Count>(Count(1));
}

void renderWood(flecs::world& ecs, const Tilemap& map) {
    ecs.each([&map](WoodTag, const Count& count, const Position& pos) {
        // fmt::println("Rendering wood at {}, {}", pos.v, count.v);
        auto worldPos = map.tileToWorld(pos);
        if (count.v > 1) {
            textDrawer.draw(
                {.pos   = worldPos - Vec2(20, 10),
                 .size  = 20,
                 .color = sf::Color(150, 105, 25)},
                count.v, "W"
            );
        } else {
            textDrawer.draw(
                {.pos   = worldPos - Vec2(20, 10),
                 .size  = 20,
                 .color = sf::Color(150, 105, 25)},
                "W"
            );
        }
    });
}

void spawnTrees(flecs::world& ecs, int count, const Tilemap& map) {
    auto dist_w = std::uniform_int_distribution<int>(0, map.dim.x - 1);
    auto dist_h = std::uniform_int_distribution<int>(0, map.dim.y - 1);

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