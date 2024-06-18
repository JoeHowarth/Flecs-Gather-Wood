#pragma once

#include <flecs.h>

#include <deque>

#include "utils/util.h"

struct WorkerTag {};
struct TreeTag {};
struct WoodTag {};

struct Targeted {};

NEWTYPE(Position, Vec2I)
NEWTYPE(Tick, int)
NEWTYPE(Count, int)

struct Idle {};

struct MoveTo {
    Position             target;
    flecs::entity        tree;
    std::deque<Position> path;
};

struct ChopingTree {
    flecs::entity target;
    int           progress;
};

using AiState = std::variant<Idle, MoveTo, ChopingTree>;
struct GatherWoodBehavior {
    AiState state;
    bool    hasWood;
};

void registerComponents(flecs::world& ecs) {
    ecs.component<Tick>();
    ecs.set(Tick(0));
    ecs.component<Position>();
    ecs.component<WorkerTag>();
    ecs.component<TreeTag>();
    ecs.component<WoodTag>();
    ecs.component<Count>();
    ecs.component<Targeted>();
    ecs.component<GatherWoodBehavior>();
}

std::ostream& operator<<(std::ostream& os, const MoveTo& moveTo) {
    os << "MoveTo{target: " << moveTo.target.v;
    os << ", tree: " << moveTo.tree.id();
    os << ", path: [" << std::endl;
    for (const auto& pos : moveTo.path) {
        os << pos.v << ", " << std::endl;
    }
    os << "]";
    os << "}";

    return os;
}

template <>
struct fmt::formatter<MoveTo> : fmt::ostream_formatter {};