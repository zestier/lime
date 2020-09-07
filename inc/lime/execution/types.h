#pragma once

#include "../table.h"
#include <cstddef>
#include <vector>

struct IterationIndex {
    std::size_t value;
};

struct RowCount {
    std::size_t value;
};

struct TableCount {
    std::size_t value;
};

struct IterationCount {
    std::size_t value;
};

using TableList = std::vector<Table>;