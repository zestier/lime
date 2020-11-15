#pragma once

#include <ctti/type_id.hpp>
#include <type_traits>

struct Cell { };

using CellId = std::conditional_t<1, ctti::type_id_t, ctti::unnamed_type_id_t>;

struct CellMeta {
    CellId id;
    std::size_t alignment;
    std::size_t storageBytes;
};

constexpr auto compare(const std::size_t l, const std::size_t r) {
    if (l < r)
        return -1;
    if (l > r)
        return 1;
    return 0;
}

constexpr auto compare(const CellMeta& lhs, const CellMeta& rhs) {
    if (auto cmp = compare(lhs.alignment, rhs.alignment); cmp != 0)
        return cmp;
    if (auto cmp = compare(lhs.storageBytes, rhs.storageBytes); cmp != 0)
        return cmp;
    return compare(lhs.id.hash(), rhs.id.hash());
}

constexpr bool operator==(const CellMeta& lhs, const CellMeta& rhs) { return compare(lhs, rhs) == 0; }
constexpr bool operator!=(const CellMeta& lhs, const CellMeta& rhs) { return compare(lhs, rhs) != 0; }
constexpr bool operator<(const CellMeta& lhs, const CellMeta& rhs)  { return compare(lhs, rhs) < 0;  }
constexpr bool operator<=(const CellMeta& lhs, const CellMeta& rhs) { return compare(lhs, rhs) <= 0; }
constexpr bool operator>(const CellMeta& lhs, const CellMeta& rhs)  { return compare(lhs, rhs) > 0;  }
constexpr bool operator>=(const CellMeta& lhs, const CellMeta& rhs) { return compare(lhs, rhs) >= 0; }

template <typename T>
struct CellMetaT : CellMeta {
private:
    static constexpr auto makeId() {
        if constexpr (std::is_same_v<CellId, ctti::type_id_t>)
            return ctti::type_id<T>();
        if constexpr (std::is_same_v<CellId, ctti::unnamed_type_id_t>)
            return ctti::unnamed_type_id<T>();
    }

public:
    constexpr CellMetaT() : CellMeta({
        .id = makeId(),
        .alignment = alignof(T),
        .storageBytes = std::is_empty_v<T> ? 0 : sizeof(T),
    }) {
    }
};
