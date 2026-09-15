#pragma once

#include <expected>
#include <flat_map>
#include <flat_set>
#include <inplace_vector>
#include <string>
#include <type_traits>

#include <nanobind/nanobind.h>
#include <nanobind/stl/detail/nb_list.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/set.h>

/* a lot of the implementations for this were *heavily inspired* by nanobind's
   casters in their source code. the style i've gone for here is mostly
   nanobind's, but the templates use this codebase's conventions, and the names
   for the templates are from the c++ specification (with some creative liberties
   taken whenever there was a template called just "T" + other cases like that).
   if you stumbled into this file by accident, do yourself a favor and close it */

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

template <typename TExpectedValue>
struct type_caster<std::expected<TExpectedValue, std::string>> {
    using Caster = make_caster< //
        std::conditional_t<std::is_void_v<TExpectedValue>, void_type, TExpectedValue>>;

    static constexpr auto Name = Caster::Name;

    static handle
    from_cpp(std::expected<TExpectedValue, std::string>&& value, rv_policy policy,
        cleanup_list* cleanup) //
    {
        if (!value) {
            throw std::runtime_error(value.error());
        }

        if constexpr (std::is_void_v<TExpectedValue>) {
            return none().release();
        }
        else {
            return Caster::from_cpp(std::move(*value), policy, cleanup);
        }
    }
};

template <typename TKey, typename TCompare, typename TKeyContainer>
struct type_caster<std::flat_set<TKey, TCompare, TKeyContainer>>
    : set_caster< //
          std::flat_set<TKey, TCompare, TKeyContainer>, //
          TKey> {};

template <typename TKey, typename TMapped, typename TCompare, //
    typename TKeyContainer, typename TMappedContainer>
struct type_caster<std::flat_map<TKey, TMapped, TCompare, TKeyContainer, TMappedContainer>> //
    : dict_caster< //
          std::flat_map<TKey, TMapped, TCompare, TKeyContainer, TMappedContainer>, //
          TKey, TMapped> {};

template <typename TElementType, std::size_t TCapacity>
struct type_caster<std::inplace_vector<TElementType, TCapacity>>
    : list_caster< //
          std::inplace_vector<TElementType, TCapacity>, //
          TElementType> {};

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)
