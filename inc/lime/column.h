#pragma once

#include <array>
#include <ctti/type_id.hpp>

using ColumnId = std::conditional_t<1, ctti::type_id_t, ctti::unnamed_type_id_t>;

template <typename T>
constexpr ColumnId GetColumnId() {
    if constexpr (std::is_same_v<ColumnId, ctti::type_id_t>)
        return ctti::type_id<T>();
    if constexpr (std::is_same_v<ColumnId, ctti::unnamed_type_id_t>)
        return ctti::unnamed_type_id<T>();
}

class ColumnList {
public:
    const ColumnId* typesBegin = nullptr;
    const ColumnId* typesEnd = nullptr;

    // An important property of column packs is they can be iterated in a stable way
    auto begin() const { return typesBegin; }
    auto end() const { return typesEnd; }
    auto size() const { return typesEnd - typesBegin; }
};

template <typename... Ts>
struct ColumnListT : ColumnList {
    static constexpr std::array<ColumnId, sizeof...(Ts)> IDs = { GetColumnId<Ts>()... };
    constexpr ColumnListT() {
        typesBegin = IDs.data();
        typesEnd = typesBegin + IDs.size();
    }
};