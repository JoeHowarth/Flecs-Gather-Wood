#pragma once

#include <flecs.h>

#include "newtype.h"
#include "util.h"

struct WorkerTag {};
struct TreeTag {};
struct WoodTag {};
NEWTYPE(Position, Vec2I)
NEWTYPE(Tick, int)

void registerComponents(flecs::world& ecs) {
    ecs.component<Tick>();
    ecs.set(Tick(0));
    ecs.component<Position>();
    ecs.component<WorkerTag>();
    ecs.component<TreeTag>();
    ecs.component<WoodTag>();
}
