
#pragma once

#include <deque>
#include <optional>

#include "pathfinding/astar.h"
#include "utils/util.h"

struct Pathfinder {
    std::vector<unsigned char> map;
    Vec2I                      mapDim;

    // Call operator that forwards to find method
    std::optional<std::deque<Position>>
    operator()(Position start, Position target) {
        return find(start, target);
    }

    std::optional<std::deque<Position>> find(Position start, Position target) {
        std::vector<int> path(map.size());
        if (!AStarFindPath(
                toPair(start.v), toPair(target.v), map, toPair(mapDim), path
            )) {
            return std::nullopt;
        }

        std::deque<Position> result;
        for (int pos : path) {
            result.push_back(Position(Vec2I{pos % mapDim.x, pos / mapDim.x}));
        }
        return result;
    }
};