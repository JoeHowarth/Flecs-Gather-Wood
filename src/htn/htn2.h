#pragma once

#include <fmt/ranges.h>  // Include this for container formatting

#include <functional>
#include <unordered_map>

#include "../utils/util.h"

/**** Sanity Aliases ****/

template <typename T>
using F = std::function<T>;

template <typename T>
using Option = std::optional<T>;

template <typename T>
using Vec = std::vector<T>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

/**** Real Aliases ****/

using Expr = std::variant<int, std::string>;

int& _i(Expr& e) {
    return std::get<int>(e);
}

std::string& _s(Expr& e) {
    return std::get<std::string>(e);
}

// using AttrMap = std::unordered_map<std::string, Expr>;

struct AttrMap {
    std::unordered_map<std::string, Expr> attrs;

    AttrMap(std::initializer_list<std::pair<const std::string, Expr>> init)
        : attrs(init) {}

    Expr& operator[](const std::string& key) {
        return attrs[key];
    }

    const Expr& at(const std::string& key) const {
        return attrs.at(key);
    }

    int& i(const std::string& key) {
        return _i(attrs.at(key));
    }

    std::string& s(const std::string& key) {
        return _s(attrs.at(key));
    }
};

struct Task {
    std::string name;
    AttrMap     attrs;
};

template <>
struct fmt::formatter<Task> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Task& t, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "Task: {}", t.name);
    }
};

template <>
struct fmt::formatter<Vec<Task>> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Vec<Task>& t, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "[ ");
        for (const auto& task : t) {
            out = fmt::format_to(ctx.out(), "{} ", task);
        }
        out = fmt::format_to(ctx.out(), " ]");
        return out;
    }
};

using State    = Map<std::string, AttrMap>;
using Operator = F<Option<State>(State, AttrMap)>;
using Method   = F<Option<Vec<Task>>(State, AttrMap)>;

struct HTN {
    Map<std::string, Operator>    operators;
    Map<std::string, Vec<Method>> methods;

    Option<Vec<Task>> hop(State state, Vec<Task> tasks) {
        return seek_plan(state, tasks, {}, 0);
    }

    Option<Vec<Task>>
    seek_plan(State state, const Vec<Task> tasks, Vec<Task> plan, int depth) {
        // Note: important that State, Vec<Task>, and Vec<Task> are passed by
        // value here. This is because we want to be able to backtrack and try
        // different plans without affecting the original state or plan.

        std::string spaces(depth * 2, ' ');
        fmt::println("{}{} Tasks: {}", depth, spaces, tasks);
        if (tasks.empty()) {
            fmt::println("{}{} Plan: {}", depth, spaces, plan);
            return plan;
        }

        Task task = tasks.front();

        if (operators.contains(task.name)) {
            auto        op = operators[task.name];
            std::string spaces(depth * 2, ' ');
            fmt::println("{}{:s} Operator: {}", depth, spaces, task);

            auto newState = op(state, task.attrs);
            if (!newState) {
                fmt::println("{}{} Operator failed: {}", depth, spaces, task);
                return std::nullopt;
            }

            const Vec<Task> rest(tasks.begin() + 1, tasks.end());
            plan.push_back(task);
            return seek_plan(*newState, rest, plan, depth + 1);
        } else if (methods.contains(task.name)) {
            fmt::println("{}{} Composite Task: {}", depth, spaces, task);
            auto relevant = methods[task.name];
            int  i        = 0;
            for (const Method& method : relevant) {
                i++;
                auto subtasks = method(state, task.attrs);
                fmt::println("{}{} i-th Method {}", depth, spaces, i);
                if (!subtasks) {
                    continue;
                }
                fmt::println("{}{} New Tasks: {}", depth, spaces, *subtasks);
                subtasks->insert(
                    subtasks->end(), tasks.begin() + 1, tasks.end()
                );
                auto solution = seek_plan(state, *subtasks, plan, depth + 1);
                if (!solution) {
                    continue;
                }
                return solution;
            }
            return std::nullopt;
        }
        fmt::println(
            "[Error] Task is not an operator or componsite task: {}", task.name
        );
        return std::nullopt;
    }
};

/**** Example ****/

using R = Option<State>;

/**** Operators ****/

R walk(State state, AttrMap attrs) {
    auto who  = attrs.s("who");
    auto from = attrs.s("from");
    auto to   = attrs.s("to");
    if (state.at("loc").s(who) != from) {
        return std::nullopt;
    }
    state.at("loc").s(who) = to;
    return std::move(state);
}

R call_taxi(State state, AttrMap attrs) {
    auto who                = attrs.s("who");
    state.at("loc")["taxi"] = state.at("loc").at(who);
    return std::move(state);
}

int taxi_rate(const int& dist) {
    return (3 + dist);
}

R ride_taxi(State state, AttrMap attrs) {
    auto who  = attrs.s("who");
    auto to   = attrs.s("to");
    auto from = attrs.s("from");

    auto& loc = state.at("loc");

    if (loc.s("taxi") != loc.s(who) || loc.s(who) != from) {
        fmt::println("Ride Taxi failed");
        fmt::println("loc.taxi: {}", loc.s("taxi"));
        fmt::println("loc.who: {}", loc.s(who));
        fmt::println(
            "loc.s(\"taxi\") != loc.s(who): {}", loc.s("taxi") != loc.s(who)
        );
        fmt::println("loc.s(who) != from: {}", loc.s(who) != from);
        return std::nullopt;
    }
    loc.s("taxi")        = to;
    loc.s(who)           = to;
    state.at("owe")[who] = taxi_rate(state.at("dist-" + from).i(to));
    return std::move(state);
}

R pay_driver(State state, AttrMap attrs) {
    auto who  = attrs.s("who");
    auto amt  = state.at("owe").i(who);
    auto have = state.at("cash").i(who);

    if (have < amt) {
        return std::nullopt;
    }

    state.at("cash").i(who) = have - amt;
    state.at("owe").i(who)  = 0;
    return std::move(state);
}

/**** Methods ****/

Option<Vec<Task>> travel_by_foot(State state, AttrMap attrs) {
    auto who  = attrs.s("who");
    auto from = attrs.s("from");
    auto to   = attrs.s("to");

    auto& loc = state.at("loc");

    if (state.at("dist-" + from).i(to) > 2) {
        return std::nullopt;
    }
    if (loc.s(who) != from) {
        return std::nullopt;
    }
    return {{{"walk", attrs}}};
}

Option<Vec<Task>> travel_by_foot_last_resort(State state, AttrMap attrs) {
    auto who  = attrs.s("who");
    auto from = attrs.s("from");
    auto to   = attrs.s("to");

    auto& loc = state.at("loc");

    if (loc.s(who) != from) {
        return std::nullopt;
    }
    return {{{"walk", attrs}}};
}

Option<Vec<Task>> travel_by_taxi(State state, AttrMap attrs) {
    const auto who  = attrs.s("who");
    const auto from = attrs.s("from");
    const auto to   = attrs.s("to");

    auto loc = state.at("loc");

    if (loc.s(who) == from) {
        return {
            {{"call_taxi", attrs}, {"ride_taxi", attrs}, {"pay_driver", attrs}}
        };
    }
    return std::nullopt;
}

void htn_main2() {
    HTN htn = {
        .operators =
            {{"walk", walk},               //
             {"call_taxi", call_taxi},     //
             {"ride_taxi", ride_taxi},     //
             {"pay_driver", pay_driver}},  //
        .methods =
            {{"travel",
              {travel_by_foot,                //
               travel_by_taxi,                //
               travel_by_foot_last_resort}}}  //
    };

    State state1 = {
        {"loc", {{"me", "home"}}},     //
        {"cash", {{"me", 20}}},        //
        {"owe", {{"me", "none"}}},     //
        {"dist-home", {{"park", 8}}},  //
        {"dist-park", {{"home", 8}}}   //
    };

    AttrMap attrs1 = {{"who", "me"}, {"from", "home"}, {"to", "park"}};

    {
        fmt::println("Test 1");
        State state = state1;
        auto  plan  = htn.hop(state, {{"travel", attrs1}});
        if (plan) {
            fmt::println("Plan: {}", *plan);
        } else {
            fmt::println("No plan found");
        }
    }

    {
        fmt::println("\nTest 2");
        State state                     = state1;
        state.at("dist-home").i("park") = 1;
        state.at("dist-park").i("home") = 1;

        auto plan = htn.hop(state, {{"travel", attrs1}});
        fmt::println("Plan: {}", *plan);
    }

    {
        fmt::println("\nTest 3");
        State state              = state1;
        state.at("cash").i("me") = 1;

        auto plan = htn.hop(state, {{"travel", attrs1}});
        fmt::println("Plan: {}", *plan);
    }
}