#pragma once

#include <flecs.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <functional>
#include <sstream>
#include <type_traits>

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

std::ostream& operator<<(std::ostream& os, const MoveTo& moveTo) {
    os << "MoveTo{target: " << moveTo.target.v;
    if (moveTo.path) {
        os << ", path: [";
        for (const auto& pos : *moveTo.path) {
            os << pos.v << ", ";
        }
        os << "]";
    } else {
        os << ", path: null";
    }
    os << "}";

    return os;
}

template <>
struct fmt::formatter<MoveTo> : fmt::ostream_formatter {};

// Target is a tree entity
struct ChopTree {};

// Target is a wood entity
struct PickUpItem {};

using Tasks = std::variant<MoveTo, ChopTree, PickUpItem>;

struct GatherWoodBehavior {
    std::deque<Tasks> tasks;
};

void registerTaskTypes(flecs::world& ecs) {
    ecs.component<Task>();
    ecs.component<MoveTo>().is_a<Task>();
    ecs.component<ChopTree>().is_a<Task>();
    ecs.component<PickUpItem>().is_a<Task>();
    // ecs.component<GatherWoodBehavior>();
}



void assignTasks(flecs::world& ecs, const Tilemap& map) {
    // auto filter =
    //     ecs.rule_builder<WorkerTag, Position>().without<Task>().build();
    auto filter = ecs.filter_builder<WorkerTag, Position>()
                      .without<MoveTo>()
                      .without<ChopTree>()
                      .without<PickUpItem>()
                      .build();

    // ecs.each([](flecs::entity e, WorkerTag) {
    //     fmt::println(
    //         "[assignTasks] Worker {} has MoveTo {}", e, e.has<MoveTo>()
    //     );
    // });

    DeferGuard g(ecs);
    filter.iter([&ecs, &map](flecs::iter& it) {
        for (auto i : it) {
            auto e = it.entity(i);
            // e.get

            Position target = randomTile(Tilemap::Grass, map);
            MoveTo   moveTo{.target = target, .path = std::nullopt};
            fmt::println(
                "[assignTasks] Assigning task to worker {}, {}", e.id(), moveTo
            );
            e.set(moveTo);
        }
    });

    // ecs.each([](flecs::entity e, WorkerTag) {
    //     fmt::println(
    //         "[assignTasks] Worker {} has MoveTo {}", e, e.has<MoveTo>()
    //     );
    // });
}

void updateMoveTo(
    flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
) {
    DeferGuard g(ecs);
    ecs.each([&](flecs::entity e, MoveTo& moveTo, Position& pos) {
        // If we're already at the target, remove the MoveTo component
        if (pos == moveTo.target) {
            fmt::println("[moveTo] Worker {} is already at target", e);
            e.remove<MoveTo>();
            return;
        }

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
            return;
        }
        // Move along the path
        Position newPos = moveTo.path->front();
        moveTo.path->pop_front();
        if (magnitude(newPos.v - pos.v) >= 2.f) {
            fmt::println(
                "[moveTo] Worker {} moved more than 1 tile: {} from {}", e,
                newPos.v, pos.v
            );
            fmt::println("[moveTo] MoveTo: {}", moveTo);
            exit(1);
        }
        if (map[newPos] != Tilemap::Grass) {
            fmt::println(
                "[moveTo] Worker {} moved to a non-grass tile: {} from {}", e, newPos.v, pos.v
            );
            exit(1);
        }
        pos = newPos;
        fmt::println("[moveTo] {} moved to: {}", e, pos.v);
    });
}
