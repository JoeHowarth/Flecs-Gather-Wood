#pragma once

#include <functional>
#include <iostream>

#include "params.h"
#include "utils/array_vec.h"
#include "utils/util.h"

template <typename State>
struct CompoundTask;

template <typename State>
struct Action;

template <typename State>
struct Operator {
    // name of operator
    const std::string name;

    // lambdas to implement preconditions and effects
    const Func<bool(const State& state, const TypedParams& params)>
        preconditions;
    const Func<State(const State& state, const TypedParams& params)> effects;

    static bool AlwaysValid(const State& state, const TypedParams& params) {
        return true;
    };

    /*** Handle Params ***/

    // array of indices of types in Params
    ArrayVec<size_t, 5> indices;

    template <typename... ParamTypes>
    static Operator<State> with_sig(
        const std::string name,
        const Func<bool(const State& state, const TypedParams& params)>
            preconditions,
        const Func<State(const State& state, const TypedParams& params)> effects
    ) {
        Operator<State> op{name, preconditions, effects};
        if (sizeof...(ParamTypes) > 5) {
            throw std::runtime_error(
                "Operator signature size exceeds maximum of 5"
            );
        }
        // (op.indices.push_back(variant_index<Param, ParamTypes>().value()),
        // ...);
        (op.indices.push_back(ParamInd<ParamTypes>()), ...);
        return std::move(op);
    }

    // template <typename... Args>
    // std::optional<Action<State>> operator()(Args&&... args) const {
    //     Params params = {Param(std::forward<Args>(args))...};
    //     return this->operator()(params);
    // }

    std::optional<Action<State>> operator()(const Params& params) const {
        if (!paramsValid(params, indices)) {
            return std::nullopt;
        }
        return Action<State>{*this, params};
    }
};

template <typename State>
struct Action {
    const Operator<State>& op;
    const Params           params;
};

template <typename State>
using Task = std::variant<
    std::reference_wrapper<const Operator<State>>,
    std::reference_wrapper<const CompoundTask<State>>,
    const std::string  // name of a compound task, needed for recursion
    >;

template <typename State>
struct CompoundTaskInstance;

template <typename State>
using TaskInstance = std::variant<Action<State>, CompoundTaskInstance<State>>;

template <typename State>
struct Method {
    const std::string name;
    const Func<bool(const State& state, const TypedParams& params)>
                                         preconditions;
    const std::vector<const Task<State>> tasks;

    /*** Handle Params ***/
};

template <typename State>
struct CompoundTask {
    const std::string                      name;
    const std::vector<const Method<State>> methods;

    template <typename... Args>
    std::optional<Action<State>> operator()(Args&&... args) const {
        Params params = {Param(std::forward<Args>(args))...};
        return this(params);
    }

    std::optional<Action<State>> operator()(const Params& params) const {
        // if (!paramsValid(params, indices)) {
        //     return std::nullopt;
        // }
        return Action<State>(*this, params);
    }
};

template <typename State>
struct CompoundTaskInstance {
    const CompoundTask<State>& task;
    const Params               params;
};

template <typename State>
struct Domain {
    const std::unordered_map<std::string, Operator<State>>     operators;
    const std::unordered_map<std::string, CompoundTask<State>> compoundTasks;
};

template <typename State>
using Plan = std::vector<Task<State>>;

template <typename State>
using FinalPlan = std::vector<Action<State>>;

template <typename State>
const Task<State> asTask(const auto& op) {
    return std::cref(op);
}

/*******************/
/**** Algorithm ****/
/*******************/

template <typename State>
std::optional<FinalPlan<State>> seek_plan(
    const Domain<State>& domain,
    const State&         state,
    Plan<State>&         plan,
    int                  depth
) {
    Action<State>    action = *domain.operators.at("mul")({2});
    FinalPlan<State> out    = {action};
    return out;
    // return std::nullopt;
}

template <typename State>
std::optional<FinalPlan<State>>
hop(const Domain<State>& domain,
    const State&         state,
    const Task<State>&   topLevelTask) {
    Plan<State> plan = {topLevelTask};
    return seek_plan(domain, state, plan, 0);
}

/**********************/
/**** Test Harness ****/
/**********************/

void htn_main() {
    fmt::println("htn_main");

    // struct State {
    //     Vec2I pos;
    // };
    using State = int;
    using Op    = Operator<State>;

    auto lessThanLimit = [](auto state, auto params) {
        const int limit = get<int>(params, 0);
        return state < limit;
    };

    std::unordered_map<std::string, Op> operators = {
        {"inc",
         Op::with_sig<int, int>(
             "inc", Op::AlwaysValid,
             [](auto state, auto params) { return state + get<int>(params, 1); }
         )},
        {"mul",
         Op::with_sig<int>(
             "mul", Op::AlwaysValid,
             [](auto state, auto params) { return state * get<int>(params, 0); }
         )}
    };

    std::unordered_map<std::string, CompoundTask<State>> compoundTasks = {
        {"ReachLimit",
         CompoundTask<State>{
             .name = "ReachLimit",
             .methods =
                 {Method<State>{
                      .name          = "Increment",
                      .preconditions = lessThanLimit,
                      .tasks =
                          {
                              asTask<State>(operators["inc"]),
                          }
                  },
                  Method<State>{
                      .name          = "Multiply",
                      .preconditions = lessThanLimit,
                      .tasks =
                          {
                              asTask<State>(operators["mul"]),
                          }
                  },
                  Method<State>{
                      .name          = "Done",
                      .preconditions = Op::AlwaysValid,
                      .tasks         = {}
                  }}
         }}
    };

    Domain<State> domain = {
        .operators = operators, .compoundTasks = compoundTasks
    };

    auto reachLimitTask = domain.compoundTasks.at("ReachLimit");
    auto plan           = hop<State>(domain, 5, reachLimitTask);
    if (plan) {
        for (auto& action : *plan) {
            fmt::println("Name: {}", action.op.name);
        }
    }
}