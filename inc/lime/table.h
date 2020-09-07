#pragma once

#include "column.h"
#include <cstddef>

struct Table {
public: // Hot data. Used very frequently.
    ColumnList columns;
    void** instanceData;
    std::size_t rowCount;

public: // Cold data. Used less frequently. If/when it grows this could switch to a pointer to the cold data.
    std::size_t rowCapacity;
};