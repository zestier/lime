#pragma once

#include "table.h"

template <typename T>
T* CreateTypeBuffer(std::size_t rowCount) {
    return new T[rowCount]();
}

template <typename... Ts>
Table CreateTable(std::size_t rowCount) {
    void** typeBuffers = new void* [sizeof...(Ts)]{ CreateTypeBuffer<Ts>(rowCount)... };
    return {
        .columns = ColumnListT<Ts...>(),
        .instanceData = typeBuffers,
        .rowCount = 0,
        .rowCapacity = rowCount,
    };
}