#pragma once

#include <tuple>

template <typename... Ts>
struct All {
    using requires_types = std::tuple<Ts...>;

    template <typename... Us>
    void run(const std::tuple<Us...>& commonParams) { }
};