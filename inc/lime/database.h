#pragma once

#include "column.h"
#include "row.h"
#include "table.h"
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>

#include "placeholder.h"

class Database {
public:
    using TableBits = boost::dynamic_bitset<>;

    std::vector<Table> tables;
    std::vector<std::uint32_t> rowGenerations;

    // The performance of this can likely be increased significantly. The use case
    // effectively wants a hash map where the values are all resizable arrays of the
    // same size. The "same size" constraint makes it possible to remove extra
    // indirections which would have performance penalities.
    std::unordered_map<ColumnId, TableBits> tablesWithColumn;
    TableBits tablesWithSpace;

    template <typename... Ts>
    Table& InsertTable() {
        const auto tableIndex = tables.size();
        auto table = CreateTable<RowId, Ts...>(64);
        tables.push_back(table);

        const auto columns = table.columns;
        std::for_each(std::cbegin(columns), std::cend(columns), [this, tableIndex](ColumnId column) {
            constexpr auto blockSize = std::numeric_limits<TableBits::block_type>::digits;
            auto& tableBits = tablesWithColumn[column];
            if (tableIndex >= tableBits.size()) {
                tableBits.resize((tableIndex & ~(blockSize - 1)) + blockSize);
                tablesWithSpace.resize((tableIndex & ~(blockSize - 1)) + blockSize);
            }
            tableBits.set(tableIndex);
        });
        tablesWithSpace.set(tableIndex, true);
        return tables[tableIndex];
    }

    template <typename CombineOp>
    TableBits FindMatches(const ColumnList& columns, CombineOp op, bool fallback) {
        auto iter = std::cbegin(columns);
        auto end = std::cend(columns);

        constexpr auto blockSize = std::numeric_limits<TableBits::block_type>::digits;
        const auto bitCount = tables.size() ? ((tables.size() - 1) & ~(blockSize - 1)) + blockSize : 0;
        if (iter == end) {
            TableBits ret;
            ret.resize(bitCount, fallback);
            return ret;
        }

        do {
            auto entry = tablesWithColumn.find(*iter);
            ++iter;
            if (entry != tablesWithColumn.end()) {
                auto ret = entry->second;
                for (; iter != end; ++iter) {
                    entry = tablesWithColumn.find(*iter);
                    if (entry != tablesWithColumn.end())
                        op(ret, entry->second);
                }
                return ret;
            }
        } while (iter != end);
        return TableBits(bitCount);
    }

    template <typename... Ts>
    Table& FindOrInsertTable() {
        // Look for an existing table. An exact match would be a table that has all columns
        // and whose column count is the same as provided.
        constexpr auto columns = ColumnListT<RowId, Ts...>();
        const auto bits = FindMatches(columns, [](auto& l, const auto& r) { l &= r; }, 0) & tablesWithSpace;
        for (auto tableIndex = bits.find_first(); tableIndex != TableBits::npos; tableIndex = bits.find_next(tableIndex)) {
            auto& testTable = tables[tableIndex];
            if (testTable.columns.size() != columns.size())
                continue;
            if (testTable.rowCount == testTable.rowCapacity)
                continue;
            return testTable;
        }

        return InsertTable<Ts...>();
    }

    template <typename T>
    void Set(Table& table, std::size_t rowIndex, const T& value) {
        const auto iter = std::find(std::cbegin(table.columns), std::cend(table.columns), GetColumnId<T>());
        const auto index = std::distance(std::cbegin(table.columns), iter);
        reinterpret_cast<T*>(table.instanceData[index])[rowIndex] = value;
    }

public:
    template <typename... Ts>
    RowId Insert(Ts... args) {
        auto& table = FindOrInsertTable<Ts...>();
        (Set(table, table.rowCount, args), ...);
        table.rowCount += 1;
        tablesWithSpace.set(&table - tables.data(), table.rowCount != table.rowCapacity);
        return { 0, 0 };
    }

    template <typename... Ts>
    void Insert(RowId row, Ts... args) { }
};