#pragma once

#include <cstdint>

struct RowId {
    std::uint32_t index = 0;
    std::uint32_t generation = 0;
};