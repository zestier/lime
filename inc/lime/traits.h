#pragma once

#include "traits.h"
#include <tuple>

template <typename Func>
struct func_traits;

template <typename TObj, typename R, typename... TArgs>
struct func_traits<R(TObj::* const)(TArgs...)> {
    using result = R;
    using parameters = std::tuple<TArgs...>;
};

template <typename TObj, typename R, typename... TArgs>
struct func_traits<R(TObj::* const)(TArgs...) const> {
    using result = R;
    using parameters = std::tuple<TArgs...>;
};

template <typename F>
struct func_traits {
private:
    constexpr static auto funcPtr = &F::operator();
    using Function = decltype(funcPtr);
    using Internal = func_traits<Function>;
public:
    using result = typename Internal::result;
    using parameters = typename Internal::parameters;
};