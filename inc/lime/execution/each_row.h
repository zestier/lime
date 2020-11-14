#pragma once

#include "function_caller.h"
#include "traits.h"
#include "utility.h"
#include <boost/mp11/algorithm.hpp>

inline auto GetColumnOffset(const Table& table, ColumnId id) {
    const auto begin = std::begin(table.columns);
    const auto iter = std::find(begin, std::end(table.columns), id);
    return std::distance(begin, iter);
}

template <typename T>
inline T* GetColumnInstanceBuffer(const Table& table) {
    const auto offset = GetColumnOffset(table, GetColumnId<T>());
    return static_cast<T*>(table.instanceData[offset]);
}

template <typename T>
inline void GetParamIterator(const Table& table, CallableParam<T>& out) {
    if constexpr (is_cell<T>::value)
        out.value = GetColumnInstanceBuffer<T>(table);
}

template <typename Tuple>
inline auto PrepareParams(const Table& table) {
    Tuple output;
    std::apply([&](auto&& ... args) { (GetParamIterator(table, args), ...); }, output);
    return output;
}

template <typename T, typename U>
inline T Step(const T& v, U offset) {
    return v + offset;
}

template <typename... Ts, typename U>
inline std::tuple<Ts...> Step(std::tuple<Ts...> columns, U offset) {
    std::apply([offset](auto&& ... args) { ((args = Step(args, offset)), ...); }, columns);
    return columns;
}

template <typename T>
using indirection_type_t = typename T::indirection_type;

template <typename F>
class EachRow {
    using Caller = FunctionCaller<F>;

public: // Traits
    using parameters = boost::mp11::mp_append<
        typename Caller::Params,
        boost::mp11::mp_transform<CallableParam, std::tuple<TableList>>,
        std::conditional_t<boost::mp11::mp_contains<typename Caller::Params, CallableParam<IterationCount>>::value, std::tuple<CallableParam<RowCount>>, std::tuple<>>
    >;
    using requires_types = boost::mp11::mp_copy_if<
        boost::mp11::mp_transform<std::remove_cvref_t, boost::mp11::mp_transform<indirection_type_t, typename Caller::Params>>,
        is_cell
    >;

public:
    F func;
    constexpr EachRow(F&& f) : func(std::forward<F>(f)) {}

    template <typename... Ts>
    void run(const std::tuple<Ts...>& commonParams) {
        typename Caller::Params callParams;
        std::apply([&commonParams](auto&&... args) { ((tryGet(commonParams, args)), ...); }, callParams);
        trySet<IterationCount>(callParams, [&commonParams](auto& v) {
            CallableParam<RowCount> rc;
            tryGet(commonParams, rc);
            v.value = (*rc).value;
        });

        auto iterationOffset = std::size_t(0);
        const auto tables = *std::get<CallableParam<TableList>>(commonParams);
        std::for_each(std::begin(tables), std::end(tables), [this, callParams, &iterationOffset](const Table& table) {
            const auto rowCount = table.rowCount;
            if (!rowCount)
                return;

            auto params = callParams;
            std::apply([&](auto&& ... args) { (GetParamIterator(table, args), ...); }, params);
            trySet<IterationIndex>(params, [iterationOffset](auto& v) { v.value = iterationOffset; });

            iterationOffset += rowCount;

            for (std::size_t i = 0; i < rowCount; ++i)
                Caller::call(func, Step(params, RowOffset{ i }));
        });
    }
};