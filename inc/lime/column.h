#pragma once

#include "cell.h"
#include <boost/mp11/algorithm.hpp>
#include <array>
#include <gsl/span>
#include <type_traits>
#include <tuple>

using ColumnId = CellId;

template <typename T>
constexpr ColumnId GetColumnId() {
    return CellMetaT<T>().id;
}

template <typename T>
constexpr auto storage_required_v = std::is_empty_v<T> ? 0 : sizeof(T);

// Sorts by alignment, storage size, and then name.
template <typename L, typename R>
struct ColumnId_less {
    static constexpr bool value = CellMetaT<L>() > CellMetaT<R>();
};

// List is assumed to be unique and sorted by criteria listed above
class ColumnList : public gsl::span<const ColumnId> {
public:
    using span::span;

    bool Contains(const ColumnId& rhs) const {
        return std::find(std::cbegin(*this), std::cend(*this), rhs) != std::cend(*this);
    }

    bool ContainsAll(const ColumnList& rhs) const {
        auto curL = begin(), endL = end();
        auto curR = rhs.begin(), endR = rhs.end();
        for (; curR != endR; ++curR) {
            for (;; ++curL) {
                if (curL == endL)
                    return false;
                if (*curL == *curR)
                    break;
            }
        }
        return true;
    }
};

template <typename U>
struct extraction;

template <typename... Us>
struct extraction<std::tuple<Us...>> {
    static constexpr std::array<ColumnId, sizeof...(Us)> ids = { GetColumnId<Us>()... };
    static constexpr std::array<CellMeta, sizeof...(Us)> metas = { CellMetaT<Us>()... };
};

template <typename... Ts>
struct ColumnListT : ColumnList {
    using unique = boost::mp11::mp_unique<std::tuple<Ts...>>;
    using sorted = boost::mp11::mp_sort<unique, ColumnId_less>;
    using types = sorted;

    static constexpr auto ids = extraction<types>::ids;
    static constexpr auto metas = extraction<types>::metas;
    constexpr ColumnListT() : ColumnList(ids) { }
};