#pragma once

#include "table.h"
#include <gsl/span>
#include <new>

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
        .cellMetas = ColumnList::metas.data(),
    };
}

class TableFactory {
public:
    Table Alloc(gsl::span<const CellMeta> metas) {
        const auto rowCapacity = 128;
        const auto columnCount = metas.size();

        auto columns = static_cast<ColumnId*>(malloc(columnCount * sizeof(ColumnId)));
        auto instances = new void*[columnCount];
        auto cellMetas = static_cast<CellMeta*>(malloc(columnCount * sizeof(CellMeta)));

        auto index = 0;
        for (const auto& meta : metas) {
            new(columns + index) ColumnId(meta.id);
            instances[index] = std::calloc(rowCapacity, meta.storageBytes);
            new(cellMetas + index) CellMeta(meta);
            ++index;
        }

        Table ret = {};
        ret.columns = ColumnList(columns, columnCount);
        ret.cellMetas = cellMetas;
        ret.instanceData = instances;
        ret.rowCapacity = rowCapacity;
        return ret;
    }

    void Free(const Table& /*table*/) {
    }
};
