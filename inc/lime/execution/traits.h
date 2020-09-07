#pragma once

#include "../traits.h"
#include <boost/preprocessor/cat.hpp>
#include <boost/mp11/algorithm.hpp>

template<typename T>
struct void_ { typedef void type; };

#define LIME_DETAIL_EXTRACT_TRAIT(name)                                                                         \
    template <typename T, typename D, typename = void>                                                          \
    struct BOOST_PP_CAT(extract_,name) {                                                                        \
        using type = D;                                                                                         \
    };                                                                                                          \
    template <typename T, typename D>                                                                           \
    struct BOOST_PP_CAT(extract_,name) <T, D, typename void_<typename T::name>::type> {                         \
        using type = typename T::name;                                                                          \
    };                                                                                                          \
    template <typename T, typename D>                                                                           \
    using BOOST_PP_CAT(extract_,BOOST_PP_CAT(name,_t)) = typename BOOST_PP_CAT(extract_,name)<T, D>::type;

LIME_DETAIL_EXTRACT_TRAIT(parameters);
LIME_DETAIL_EXTRACT_TRAIT(requires_types);
#undef LIME_DETAIL_EXTRACT_TRAIT

template <typename... Ts>
struct ExecutionTraits;

template <typename T>
struct ExecutionTraits<T> {
    using parameters = extract_parameters_t<T, std::tuple<>>;
    using requires_types = extract_requires_types_t<T, std::tuple<>>;
};

template <typename... Ts>
struct ExecutionTraits {
    using parameters = boost::mp11::mp_unique<boost::mp11::mp_append<typename ExecutionTraits<Ts>::parameters...>>;
    using requires_types = boost::mp11::mp_unique<boost::mp11::mp_append<typename ExecutionTraits<Ts>::requires_types...>>;
};