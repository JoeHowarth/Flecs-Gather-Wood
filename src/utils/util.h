#pragma once

#include <flecs.h>

#include <iostream>
#include <sstream>

#include "layered_drawer.h"
#include "text.h"
#include "vectors.h"
#include "newtype.h"

// Note: Utility globals defined at bottom of the file

/**** Printing ****/

template <typename T>
T dbg(const T s) {
    std::cout << s << std::endl;
    return s;
}

template <>
struct fmt::formatter<flecs::entity> : fmt::ostream_formatter {};

/**** Drawing ******/

auto now() {
    return std::chrono::high_resolution_clock::now();
}

/**** Flecs ****/

struct DeferGuard {
    flecs::world& ecs;
    DeferGuard(flecs::world& ecs) : ecs(ecs) {
        ecs.defer_begin();
    }
    ~DeferGuard() {
        ecs.defer_end();
    }
};

/*** Match ****/

template <typename... Fs>
struct match : Fs... {
    using Fs::operator()...;

    // constexpr match(Fs &&... fs) : Fs{fs}... {}
};
template <class... Ts>
match(Ts...) -> match<Ts...>;

template <typename... Ts, typename... Fs>
constexpr decltype(auto)
operator|(std::variant<Ts...> const& v, match<Fs...> const& match) {
    return std::visit(match, v);
}

/*********************/
/**** Bad Globals ****/
/*********************/

TextDrawer    textDrawer("./open-sans/OpenSans-Bold.ttf");
LayeredDrawer debugDrawer(1);
const int     SIM_DEBUG_LAYER = 0;

std::random_device rd;         // Seed
std::mt19937       gen(rd());  // Mersenne Twister engine