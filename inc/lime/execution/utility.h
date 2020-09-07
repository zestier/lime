#pragma once

#include "../cell.h"
#include "function_caller.h"
#include <boost/mp11/algorithm.hpp>

template <typename T>
using is_cell = std::is_base_of<Cell, T>;

template <typename Tuple, typename T>
void tryGet(const Tuple& tuple, T& v) {
    if constexpr (boost::mp11::mp_contains<Tuple, T>::value)
        v = std::get<T>(tuple);
}

template <typename T, typename P, typename F>
void trySet(P& p, F&& f) {
    using Param = CallableParam<T>;
    if constexpr (boost::mp11::mp_contains<P, Param>::value)
        f(*std::get<Param>(p));
}