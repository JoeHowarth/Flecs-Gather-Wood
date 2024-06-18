#pragma once

#include <flecs.h>

#include "components.h"
#include "pathfinder.h"
#include "tilemap.h"
#include "trees.h"

void handleIdle(
    flecs::world&                           ecs,
    const flecs::entity                     e,
    const flecs::filter<Position, TreeTag>& trees,
    GatherWoodBehavior&                     behavior,
    const Position&                         pos,
    Pathfinder&                             pathfinder,
    std::vector<flecs::entity>&             targetedTrees
) {
    fmt::println("[handleIdle] Worker {} is idle. pos: {}", e.id(), pos.v);

    // if have wood, return to base
    if (behavior.hasWood) {
        const Position base = Position(Vec2I(2, 2));

        if (pos == base) {
            fmt::println("[handleIdle] Worker {} returned to base", e.id());
            spawnWood(ecs, pos);
            behavior = GatherWoodBehavior{.state = Idle{}, .hasWood = false};
            return;
        }

        fmt::println("[handleIdle] Worker {} has wood", e.id());
        behavior = GatherWoodBehavior{
            .state   = MoveTo{.target = base, .path = *pathfinder(pos, base)},
            .hasWood = true
        };
        return;
    }

    // Look for a tree to chop
    using Tree = std::tuple<int, Position, flecs::entity>;
    auto less  = [](const Tree& a, const Tree& b) {
        return std::get<0>(a) < std::get<0>(b);
    };
    std::vector<Tree> closestTrees;
    int               thresholdDist = INT_MAX;
    closestTrees.reserve(100);

    std::optional<AiState> nextState = std::nullopt;
    trees.iter([&](flecs::iter& it) {
        auto treePosArr = it.field<Position>(1);
        for (auto i : it) {
            flecs::entity tree    = it.entity(i);
            Position      treePos = treePosArr[i];
            fmt::println("[handleIdle] Tree pos: {}", treePos.v);

            auto contains = [](auto& v, auto e) {
                return std::find(v.begin(), v.end(), e) != v.end();
            };
            if (contains(targetedTrees, tree)) {
                continue;
            }

            int dist = magnitude2(treePos.v - pos.v);
            if (dist == 0) {
                fmt::println(
                    "[assignTasks2] Worker {} is chopping tree "
                    "at {}",
                    e.id(), treePos.v
                );
                nextState = ChopingTree{.target = tree, .progress = 0};
                targetedTrees.push_back(tree);
                return;
            }

            closestTrees.push_back({dist, treePos, tree});
        }
    });
    if (nextState) {
        behavior.state = *nextState;
        return;
    }

    std::sort(closestTrees.begin(), closestTrees.end(), less);

    for (const auto& [dist, treePos, tree] : closestTrees) {
        fmt::println("[handleIdle] Tree pos: {}", treePos.v);
        auto path = pathfinder(pos, treePos);
        if (!path) {
            continue;
        }
        behavior.state = MoveTo{.target = treePos, .tree = tree, .path = *path};
        return;
    }
    behavior.state = Idle{};
    return;
}

void handleMoveTo(
    flecs::entity       e,
    Position&           pos,
    MoveTo&             moveTo,
    GatherWoodBehavior& behavior,
    const Tilemap&      map
) {
    fmt::println(
        "[moveTo] Worker {} is moving. pos: {} moveTo: {}", e, pos.v, moveTo
    );
    if (moveTo.target == pos) {
        behavior.state = Idle{};
        return;
    }

    if (moveTo.path.empty()) {
        fmt::println(
            "[moveTo] Worker {} path empty but not at target: {} from {}", e,
            moveTo.target.v, pos.v
        );
        return;
    }

    debugDrawer.lineStripMap(
        moveTo.path.begin(), moveTo.path.end(),
        [&map](Position pos) { return map.tileToWorld(pos); }
    );
    Position newPos = moveTo.path.front();
    moveTo.path.pop_front();

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
            "[moveTo] Worker {} moved to a non-grass tile: {} from {} ", e,
            newPos.v, pos.v
        );
        exit(1);
    }
    pos = newPos;

    if (moveTo.target == pos) {
        behavior.state = Idle{};
        return;
    }
    return;
}

void handleChopingTree(
    flecs::entity       e,
    ChopingTree&        chopping,
    GatherWoodBehavior& behavior,
    const Position&     pos
) {
    fmt::println(
        "[chopingTree] Worker {} is chopping tree at {}. progress: {}", e,
        pos.v, chopping.progress
    );

    if (!chopping.target.is_alive()) {
        fmt::println(
            "[chopingTree] Worker {} is dead while chopping tree at {}",
            e, pos.v
        );
        return;

    }

    chopping.progress += 1;
    if (chopping.progress >= 3) {
        fmt::println("[chopingTree] Worker {} finished chopping", e);
        chopping.target.destruct();

        behavior.hasWood = true;
        behavior.state   = Idle{};
        return;
    }
}

void gatherWoodBehaviorNaive(
    flecs::world& ecs, const Tilemap& map, Pathfinder& pathfinder
) {
    auto workers =
        ecs.filter_builder<Position, GatherWoodBehavior, WorkerTag>().build();
    auto trees =
        ecs.filter_builder<Position, TreeTag>().without<Targeted>().build();

    DeferGuard                 g(ecs);
    std::vector<flecs::entity> targetedTrees;
    workers.each([&](flecs::entity e, Position& pos,
                     GatherWoodBehavior& behavior, WorkerTag) {
        std::visit(
            match{
                [&](Idle) {
                    handleIdle(
                        ecs, e, trees, behavior, pos, pathfinder, targetedTrees
                    );
                },
                [&](MoveTo& moveTo) {
                    handleMoveTo(e, pos, moveTo, behavior, map);
                },
                [&](ChopingTree& chopping) {
                    handleChopingTree(e, chopping, behavior, pos);
                },
            },
            behavior.state
        );
    });
    for (auto tree : targetedTrees) {
        tree.add<Targeted>();
    }
}