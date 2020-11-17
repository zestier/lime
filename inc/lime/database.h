#pragma once

#include "column.h"
#include "record.h"
#include "table.h"
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>

#include "placeholder.h"

inline ColumnId GetColumnId(const ColumnId& id) {
    return id;
}

inline ColumnId GetColumnId(const CellMeta& meta) {
    return meta.id;
}

class Database {
public:
    using TableBits = boost::dynamic_bitset<>;
    struct RecordLocation {
        std::uintptr_t tableIndex; // Table in database
        std::size_t rowIndex;      // Row in table
    };

    template <typename... Ts>
    using TableColumnList = ColumnListT<RecordId, Ts...>;

    RecordIdFactory recordFactory;
    std::vector<RecordLocation> recordLocations; // Index using an active record's index

    TableFactory tableFactory;
    std::vector<Table> tables;

    // The performance of this can likely be increased significantly. The use case
    // effectively wants a hash map where the values are all resizable arrays of the
    // same size. The "same size" constraint makes it possible to remove extra
    // indirections which would have performance penalities.
    std::unordered_map<ColumnId, TableBits> tablesWithColumn;
    TableBits tablesWithSpace;

    std::size_t InsertTable(Table&& table);

    template <typename Container, typename CombineOp>
    TableBits FindMatches(const Container& columns, CombineOp op, bool fallback) {
        auto iter = std::cbegin(columns);
        auto end = std::cend(columns);

        constexpr auto blockSize = std::numeric_limits<TableBits::block_type>::digits;
        const auto bitCount = tables.size() ? ((tables.size() - 1) & ~(blockSize - 1)) + blockSize : 0;
        if (iter == end) {
            TableBits ret;
            ret.resize(bitCount, fallback);
            return ret;
        }

        auto& first = tablesWithColumn[GetColumnId(*iter)];
        first.resize(bitCount);

        auto ret = first;
        for (++iter; iter != end; ++iter) {
            auto& next = tablesWithColumn[GetColumnId(*iter)];
            next.resize(bitCount);
            op(ret, next);
        }
        return ret;
    }

    template <typename Container>
    std::optional<std::size_t> FindInsertableTable(const Container& columns) {
        const auto bits = FindMatches(columns, [](auto& l, const auto& r) { l &= r; }, 0) & tablesWithSpace;
        for (auto tableIndex = bits.find_first(); tableIndex != TableBits::npos; tableIndex = bits.find_next(tableIndex)) {
            auto& testTable = tables[tableIndex];
            if (testTable.columns.size() != columns.size())
                continue;
            return tableIndex;
        }
        return std::nullopt;
    }

    template <typename T>
    void Set(Table& table, std::size_t rowIndex, const T& value) {
        const auto iter = std::find(std::cbegin(table.columns), std::cend(table.columns), GetColumnId<T>());
        const auto index = std::distance(std::cbegin(table.columns), iter);
        reinterpret_cast<T*>(table.instanceData[index])[rowIndex] = value;
    }

public:
    template <typename... Ts>
    RecordId Insert(Ts... args) {
        using TCL = TableColumnList<Ts...>;
        const auto ret = recordFactory.Alloc();

        // Find table
        auto tableIndex = FindInsertableTable(TCL());
        if (!tableIndex)
            tableIndex = InsertTable(tableFactory.Alloc(TCL::metas));
        auto& table = tables[tableIndex.value()];

        const auto rowIndex = table.rowCount++;

        // Store data
        Set(table, rowIndex, ret);
        (Set(table, rowIndex, args), ...);
        tablesWithSpace.set(tableIndex.value(), table.rowCount != table.rowCapacity);

        // Create a quick lookup table so we can later dtermine where a record is
        if (recordLocations.size() <= ret.index)
            recordLocations.resize(ret.index + 1);
        recordLocations[ret.index] = { .tableIndex = tableIndex.value(), .rowIndex = rowIndex };

        return ret;
    }

    template <typename... Ts>
    void Upsert(RecordId record, Ts... args) {
        using ColumnList = ColumnListT<Ts...>;
        if (!recordFactory.IsActive(record))
            return;
            
        // Very dumb! Make sure there's at least 1 space left in the tables list.
        // This is so initialTable's memory doesn't move if we insert a new table.
        tables.reserve(tables.size() + 1);

        // First need to establish if the record is presently in the correct table
        const auto location = recordLocations[record.index];
        auto& initialTable = tables[location.tableIndex];
        const auto sameTable = initialTable.columns.ContainsAll(ColumnList());

        // If already in the correct table updating the values is all that needs to be done.
        if (sameTable) {
            (Set(initialTable, location.rowIndex, args), ...);
            return;
        }

        // If not, move the record to an appropriate table and then update the values
        // Build the list of metadata to use.
        std::vector<CellMeta> cm;
        cm.insert(cm.end(), initialTable.cellMetas, initialTable.cellMetas + initialTable.columns.size());
        cm.insert(cm.end(), std::cbegin(ColumnList::metas), std::cend(ColumnList::metas));
        std::sort(std::begin(cm), std::end(cm), std::greater<CellMeta>());
        cm.erase(std::unique(std::begin(cm), std::end(cm)), std::end(cm));

        // Find new table
        auto tableIndex = FindInsertableTable(cm);
        if (!tableIndex)
            tableIndex = InsertTable(tableFactory.Alloc(cm));
        auto& table = tables[tableIndex.value()];
        const auto rowIndex = table.rowCount++;

        // Update the location record
        recordLocations[record.index] = { .tableIndex = tableIndex.value(), .rowIndex = rowIndex };

        // Copy over data from old table
      
        const auto initialColBeg = std::cbegin(initialTable.columns);
        const auto initialColEnd = std::cend(initialTable.columns);
        auto initialColIter = initialColBeg;
        for (std::ptrdiff_t i = 0; i < table.columns.size(); ++i) {
            const auto& meta = table.cellMetas[i];
            if (!meta.storageBytes)
                continue;
            
            for (; initialColIter < initialColEnd; ++initialColIter) {
                if (initialColIter->hash() < meta.id.hash())
                    continue;
                if (initialColIter->hash() == meta.id.hash()) {
                    const auto initialIndex = std::distance(initialColBeg, initialColIter);
                    std::memcpy(
                        static_cast<std::byte*>(table.instanceData[i]) + rowIndex * meta.storageBytes,
                        static_cast<std::byte*>(initialTable.instanceData[initialIndex]) + location.rowIndex * meta.storageBytes,
                        meta.storageBytes
                    );
                }
                break;
            }
        }

        // Remove data from old table
        if (location.rowIndex != --initialTable.rowCount) {
            for (std::ptrdiff_t i = 0; i < initialTable.columns.size(); ++i) {
                const auto& meta = initialTable.cellMetas[i];
                std::memcpy(
                    static_cast<std::byte*>(initialTable.instanceData[i]) + location.rowIndex * meta.storageBytes,
                    static_cast<std::byte*>(initialTable.instanceData[i]) + initialTable.rowCount * meta.storageBytes,
                    meta.storageBytes
                );
            }
        }

        // Insert new data
        (Set(table, rowIndex, args), ...);
    }
};
