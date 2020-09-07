#pragma once

#include "../cell.h"
#include "traits.h"
#include "types.h"
#include <boost/mp11/algorithm.hpp>
#include <cstddef>
#include <type_traits>

struct RowOffset { std::size_t value; };
struct TableOffset { std::size_t value; };

template <typename T>
struct CallableParam;

template <typename D, typename T>
struct CallableParamBase {
    using indirection_type = T;
    T value;

    T& operator*() { return value; }
    const T& operator*() const { return value; }
    D operator+(RowOffset r) const { return static_cast<const D&>(*this); }
    D operator+(TableOffset r) const { return static_cast<const D&>(*this); }
};

template <>
struct CallableParam<IterationCount> : CallableParamBase<CallableParam<IterationCount>, IterationCount> {
};

template <>
struct CallableParam<IterationIndex> : CallableParamBase<CallableParam<IterationIndex>, IterationIndex> {
    using Self = CallableParam<IterationIndex>;

    auto operator+(RowOffset r) const { return Self{ this->value.value + r.value }; }
};

template <>
struct CallableParam<RowCount> : CallableParamBase<CallableParam<RowCount>, RowCount> {
    using Self = CallableParam<IterationIndex>;

    auto operator+(RowOffset r) const { return Self{ this->value.value + r.value }; }
};

template <>
struct CallableParam<TableCount> : CallableParamBase<CallableParam<TableCount>, TableCount> {
};

template <>
struct CallableParam<TableList> : CallableParamBase<CallableParam<TableList>, TableList> {
};

template <typename T>
struct CallableParam : CallableParamBase<CallableParam<T>, std::add_pointer_t<T>> {
    using Self = CallableParam<T>;
    using indirection_type = T;

    indirection_type& operator*() { return *this->value; }
    const indirection_type& operator*() const { return *this->value; }
    auto operator+(RowOffset r) const { return Self{ this->value + r.value }; }
};

template <typename T>
using DecayedCallableParam = CallableParam<std::decay_t<T>>;


template <typename F>
struct FunctionCaller {
    using Traits = func_traits<F>;

    using Params = boost::mp11::mp_unique<
        boost::mp11::mp_transform<DecayedCallableParam, typename Traits::parameters>
    >;

    template <size_t I>
    static inline auto getIter(const Params& params) {
        using ParamType = typename std::tuple_element_t<I, typename Traits::parameters>;
        using StateType = DecayedCallableParam<ParamType>;
        return std::get<StateType>(params);
    }

    template <size_t ... I>
    static inline auto call(F& f, const Params& params, std::index_sequence<I ...>) {
        return f(*getIter<I>(params) ...);
    }

    static inline auto call(F& f, const Params& params) {
        using CallIndexSequence = std::make_index_sequence<std::tuple_size<typename Traits::parameters>::value>;
        return call(f, params, CallIndexSequence{});
    }
};