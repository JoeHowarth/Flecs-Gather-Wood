#pragma once

#include <functional>
#include <iostream>

#include "utils/array_vec.h"
#include "utils/util.h"

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