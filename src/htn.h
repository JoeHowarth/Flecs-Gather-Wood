#pragma once

#include <functional>
#include <iostream>

#include "utils/array_vec.h"
#include "utils/util.h"

template <typename State>
struct CompoundTask;

using Param  = std::variant<int, float, std::string>;
using Params = ArrayVec<Param, 5>;

template <typename InOut>
using Func = std::function<InOut>;

template <typename InOut>
using OptFunc = std::optional<Func<InOut>>;

template <typename T, typename Var>
bool is(const Var& v) {
    return std::holds_alternative<T>(v);
}

/// Get a numeric index for a particular type included in variant type at
/// compile time.
/// @tparam V Variant type to search for index.
/// @tparam T Type, index of which must be returned.
/// @return Index of type T in variant V; or std::nullopt if V does not include
/// an alternative for T.
template <typename V, typename T, size_t I = 0>
constexpr std::optional<size_t> variant_index() {
    if constexpr (I >= std::variant_size_v<V>) {
        return std::nullopt;
    } else {
        if constexpr (std::is_same_v<std::variant_alternative_t<I, V>, T>) {
            return I;
        } else {
            return variant_index<V, T, I + 1>();
        }
    }
}

template <typename T, size_t I = 0>
constexpr size_t ParamInd() {
    return variant_index<Param, T, I>().value();
}

struct TypedParams {
    ArrayVec<Param, 5>  _params;
    ArrayVec<size_t, 5> indices;

    template <typename T>
    T get(int ithParam) const {
        size_t varInd = indices[ithParam];
        if (ParamInd<T>() != varInd) {
            throw std::runtime_error("Invalid type for param");
        }
        return std::get<T>(_params[ithParam]);
    }
};

// ensure params match signature
bool paramsValid(const Params& params, const ArrayVec<size_t, 5>& indices) {
    if (indices.size() != params.size()) {
        return false;
    }
    for (size_t i = 0; i < indices.size(); i++) {
        const Param& param = params[i];
        if (param.index() != indices[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
T get(const TypedParams& params, int ithParam) {
    return params.template get<T>(ithParam);
}

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
    Action<State> action = *domain.operators.at("mul")({2});
    FinalPlan<State> out = {action};
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