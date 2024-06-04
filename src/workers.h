#pragma once

#include <flecs.h>

#include <functional>

#include "components.h"
#include "pathfinder.h"
#include "tilemap.h"
#include "util.h"

void renderWorkers(flecs::world& ecs, const Tilemap& map) {
    ecs.each([&map](WorkerTag, Position& pos) {
        auto worldPos = map.tileToWorld(pos);

        textDrawer.draw(
            {.pos = worldPos, .size = 20, .color = sf::Color::Black}, "W"
        );
    });
}

void spawnWorkers(flecs::world& ecs, int count, Tilemap map) {
    auto dist_w = std::uniform_int_distribution<int>(0, map.dim.x);
    auto dist_h = std::uniform_int_distribution<int>(0, map.dim.y);

    for (int i = 0; i < count; i++) {
        flecs::entity e = ecs.entity();
        e.add<WorkerTag>();
        e.set<Position>(randomTile(Tilemap::Grass, map));
    }
}

struct Task {};
struct MoveTo {
    Position                            target;
    std::optional<std::deque<Position>> path;
};

// Target is a tree entity
struct ChopTree {};

// Target is a wood entity
struct PickUpItem {};

void registerTaskTypes(flecs::world& ecs) {
    ecs.component<Task>();
    ecs.component<MoveTo>().is_a<Task>();
    ecs.component<ChopTree>().is_a<Task>();
    ecs.component<PickUpItem>().is_a<Task>();
}

void assignTasks(flecs::world& ecs, const Tilemap& map) {
    // auto filter =
    //     ecs.rule_builder<WorkerTag, Position>().without<Task>().build();
    auto filter =
        ecs.filter_builder<WorkerTag, Position>().without<MoveTo>().build();

    // ecs.each([](flecs::entity e, WorkerTag) {
    //     fmt::println(
    //         "[assignTasks] Worker {} has MoveTo {}", e, e.has<MoveTo>()
    //     );
    // });

    ecs.defer_begin();
    filter.each([&ecs, &map](flecs::entity e, WorkerTag, Position& pos) {
        fmt::println("[assignTasks] Assigning task to worker {}", e.id());

        Position target = randomTile(Tilemap::Grass, map);
        MoveTo   moveTo{.target = target, .path = std::nullopt};
        e.set(moveTo);
        fmt::println("[assignTasks] Sanity check: {}", e.has<MoveTo>());
    });
    ecs.defer_end();

    // ecs.each([](flecs::entity e, WorkerTag) {
    //     fmt::println(
    //         "[assignTasks] Worker {} has MoveTo {}", e, e.has<MoveTo>()
    //     );
    // });
}

void updateMoveTo(
    flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
) {
    ecs.defer_begin();
    ecs.each([&](flecs::entity e, MoveTo& moveTo, Position& pos) {
        // If we don't have a path, find one
        if (!moveTo.path) {
            moveTo.path = pathfinder.find(pos, moveTo.target);
            // If we can't find a path, remove the MoveTo component
            if (!moveTo.path) {
                fmt::println(
                    "[moveTo] Failed to find path from {} to {}", pos.v,
                    moveTo.target.v
                );
                e.remove<MoveTo>();
                // ecs.remove<MoveTo>(e);
                return;
            }
        }

        debugDrawer.lineStripMap(
            moveTo.path->begin(), moveTo.path->end(),
            [&map](Position pos) { return map.tileToWorld(pos); }
        );

        // If we've reached the target, remove the MoveTo component
        if (moveTo.path->empty()) {
            if (pos != moveTo.target) {
                fmt::println(
                    "[moveTo] Path is empty but we're not at the target. pos: "
                    "{}, target: {}",
                    pos.v, moveTo.target.v
                );
            } else {
                fmt::println("[moveTo] Reached target: {}", pos.v);
            }
            e.remove<MoveTo>();
            // ecs.remove<MoveTo>(e);
            return;
        }
        // Move along the path
        pos = moveTo.path->front();
        moveTo.path->pop_front();
        fmt::println("[moveTo] {} moved to: {}", e, pos.v);
    });
    ecs.defer_end();
}
