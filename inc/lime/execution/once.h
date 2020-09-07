#pragma once

#include "function_caller.h"

template <typename F>
struct Once {
    using Caller = FunctionCaller<F>;

public: // Traits
    using parameters = typename Caller::Params;

public:
    F func;
    constexpr Once(F f) : func(f) {}

    template <typename... Ts>
    void run(const std::tuple<Ts...>& commonParams) {
        parameters p;
        std::apply([&commonParams](auto&&... args) { ((tryGet(commonParams, args)), ...); }, p);
        Caller::call(func, p);
    }
};