#pragma once

#include "../database.h"
#include "function_caller.h"
#include "traits.h"
#include "types.h"
#include <boost/mp11/algorithm.hpp>
#include <numeric>
#include <tuple>

using CommonParams = boost::mp11::mp_transform<CallableParam, std::tuple<RowCount, TableCount, TableList>>;

class ExecutionContext {
public:
    Database db;

    /**
     * This function allows appending sequences of supported data which will be grouped
     * together for execution. The data may be a combination of functions, traits, scheduling
     * rules and more.
     */
    template <typename... Ts>
    void Append(Ts&&... group) {
        using Traits = ExecutionTraits<Ts...>;
        using Params = boost::mp11::mp_rename<boost::mp11::mp_set_intersection<CommonParams, typename Traits::parameters>, std::tuple>;
        Params params;

        const auto tableBits = db.FindMatches(
            boost::mp11::mp_rename<typename Traits::requires_types, ColumnListT>(),
            [](auto& a, const auto& b) { a &= b; },
            false
        );

        std::vector<Table> tables;
        const auto bits = tableBits;
        for (auto tableIndex = bits.find_first(); tableIndex != decltype(bits)::npos; tableIndex = bits.find_next(tableIndex))
            tables.push_back(db.tables[tableIndex]);

        trySet<RowCount>(params, [&tables](auto& v) {
            v.value = std::reduce(std::begin(tables), std::end(tables), std::size_t(0), [](const size_t l, const Table& r) { return l + r.rowCount; });
        });
        trySet<TableCount>(params, [&tables](auto& v) {
            v.value = tables.size();
        });
        trySet<TableList>(params, [&tables](auto& v) {
            v = tables;
        });

        (group.run(params), ...);
    }
};