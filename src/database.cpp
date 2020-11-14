#include <lime/database.h>

std::size_t Database::InsertTable(Table&& table) {
    const auto tableIndex = tables.size();
    tables.emplace_back(std::forward<Table>(table));

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
    return tableIndex;
}