
#pragma once

#include <deque>
#include <optional>

#include "pathfinding/astar.h"
#include "util.h"

struct Pathfinder {
    std::vector<unsigned char> map;
    Vec2I        mapDim;

    std::optional<std::deque<Position>> find(Position start, Position target) {
        std::vector<int> path(map.size());
        if (!AStarFindPath(toPair(start.v), toPair(target.v), map, toPair(mapDim), path)) {
            return std::nullopt;
        }

        std::deque<Position> result;
        for (int pos : path) {
            result.push_back(Position(Vec2I{pos % mapDim.x, pos / mapDim.x}));
        }
        return result;
    }
};