#pragma once

#include "table.h"
#include <gsl/span>

template <typename T>
T* CreateTypeBuffer(std::size_t rowCount) {
    return new T[rowCount]();
}

template <typename T>
struct CreateTypeBuffers { };

template <typename... Ts>
struct CreateTypeBuffers<std::tuple<Ts...>> {
    static void** exec(std::size_t rowCount) {
        return new void* [sizeof...(Ts)]{ CreateTypeBuffer<Ts>(rowCount)... };
    }
};

template <typename... Ts>
Table CreateTable(std::size_t rowCount) {
    using ColumnList = ColumnListT<Ts...>;
    return {
        .columns = ColumnList(),
        .instanceData = CreateTypeBuffers<typename ColumnList::types>::exec(rowCount),
        .rowCount = 0,
        .rowCapacity = rowCount,
        .cellMetas = typename ColumnList::metas.data(),
    };
}

class TableFactory {
public:
    Table Alloc(gsl::span<const CellMeta> metas) {
        const auto rowCapacity = 128;
        const auto columnCount = metas.size();

        auto columns = new ColumnId[columnCount];
        auto instances = new void*[columnCount];
        auto cellMetas = new CellMeta[columnCount];

        auto index = 0;
        for (const auto& meta : metas) {
            columns[index] = meta.id;
            instances[index] = calloc(rowCapacity, meta.storageBytes);
            cellMetas[index] = meta;
            ++index;
        }

        Table ret = {};
        ret.columns = ColumnList(columns, columnCount);
        ret.cellMetas = cellMetas;
        ret.instanceData = instances;
        ret.rowCapacity = rowCapacity;
        return ret;
    }

    void Free(const Table& table) {
    }
};