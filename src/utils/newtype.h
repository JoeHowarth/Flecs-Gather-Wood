#pragma once

#include <fmt/core.h>

#include <concepts>
#include <iostream>
#include <type_traits>
#include <utility>

// Concept to check for the existence of an equality operator
template <typename T>
concept HasEqualityOperator = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
};

// Concept to check for the existence of a less-than operator
template <typename T>
concept HasLessThanOperator = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
};

// Concept to check for the existence of an addition operator
template <typename T>
concept HasAdditionOperator = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};

// Concept to check for the existence of a subtraction operator
template <typename T>
concept HasSubtractionOperator = requires(T a, T b) {
    { a - b } -> std::same_as<T>;
};

// Concept to check if the type can be formatted using fmt
template <typename T>
concept Formattable = requires(T a) {
    { fmt::format("{}", a) } -> std::same_as<std::string>;
};

template <typename T>
concept OstreamAble = requires(std::ostream& o, T a) {
    { o << a } -> std::same_as<std::ostream&>;
};

// Macro to define a new type with conditional forwarding
#define NEWTYPE(name, type)                                           \
    template <typename A = type>                                      \
    struct name##_ {                                                  \
        A v;                                                          \
        name##_() : v() {}                                            \
        explicit name##_(A v_) : v(v_) {}                             \
        name##_(const name##_&)                = default;             \
        name##_(name##_&&) noexcept            = default;             \
        name##_& operator=(const name##_&)     = default;             \
        name##_& operator=(name##_&&) noexcept = default;             \
        ~name##_()                             = default;             \
        template <typename T = A>                                     \
            requires HasEqualityOperator<T>                           \
        bool operator==(const name##_& other) const {                 \
            return v == other.v;                                      \
        }                                                             \
        template <typename T = A>                                     \
            requires(!HasEqualityOperator<T>)                         \
        bool operator==(const name##_& other) const = delete;         \
        template <typename T = A>                                     \
            requires HasLessThanOperator<T>                           \
        bool operator<(const name##_& other) const {                  \
            return v < other.v;                                       \
        }                                                             \
        template <typename T = A>                                     \
            requires(!HasLessThanOperator<T>)                         \
        bool operator<(const name##_& other) const = delete;          \
        template <typename T = A>                                     \
            requires HasAdditionOperator<T>                           \
        name##_ operator+(const name##_& other) const {               \
            return name##_(v + other.v);                              \
        }                                                             \
        template <typename T = A>                                     \
            requires(!HasAdditionOperator<T>)                         \
        name##_ operator+(const name##_& other) const = delete;       \
        template <typename T = A>                                     \
            requires HasSubtractionOperator<T>                        \
        name##_ operator-(const name##_& other) const {               \
            return name##_(v - other.v);                              \
        }                                                             \
        template <typename T = A>                                     \
            requires(!HasSubtractionOperator<T>)                      \
        name##_ operator-(const name##_& other) const = delete;       \
    };                                                                \
                                                                      \
    using name = name##_<type>;                                       \
                                                                      \
    template <typename A>                                             \
    std::ostream& operator<<(std::ostream& os, const name##_<A>& n) { \
        if constexpr (OstreamAble<A>) {                               \
            return os << #name << "(" << n.v << ")";                  \
        } else {                                                      \
            return os << #name;                                       \
        }                                                             \
    }                                                                 \
    template <>                                                       \
    struct fmt::formatter<name> : fmt::ostream_formatter {};

// template <>
// struct fmt::formatter<MoveTo> : fmt::ostream_formatter {};
// struct P {
//     int x;
// };

// // Use the macro to define a new type
// // Ex:
// NEWTYPE(Health, int)
// NEWTYPE(Position, P)

// void foo() {
//     Health hp = Health(100);
//     std::cout << hp << std::endl;
// }
