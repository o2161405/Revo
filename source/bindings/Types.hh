#pragma once

#include "bindings/Attributes.hh"
#include "util/Util.hh"

#include <algorithm>
#include <format>
#include <meta>
#include <stdexcept>
#include <vector>

#include <nanobind/nanobind.h>
#include <nanobind/stl/variant.h>

namespace Revo::Bindings {

template <typename TEnum>
struct Enumerator {
    TEnum value;
    const char* identifier;
    const char* repr;
};

template <typename TEnum>
class Enum {
public:
    template <typename... TAnnotations>
    static void
    make(nanobind::handle handle, TAnnotations... annotations) {
        auto enumeration = nanobind::enum_<TEnum>(
            handle, Util::identifier_string(^^TEnum), annotations...);

        for (const auto& enumerator : ENUMERATORS) {
            enumeration.value(enumerator.identifier, enumerator.value);
        }

        enumeration.def("__repr__", &repr);
        enumeration.def("__str__", &repr);
    }

    [[nodiscard]] static const char*
    repr(TEnum value) {
        if (const auto it = std::ranges::find(ENUMERATORS, value, &Enumerator<TEnum>::value);
            it != ENUMERATORS.end()) {
            return it->repr;
        }

        throw std::invalid_argument(std::format("{} has no enumerator with value {}",
            Util::identifier_string(^^TEnum), std::to_underlying(value)));
    }

private:
    static constexpr auto ENUMERATORS = []() consteval {
        std::vector<Enumerator<TEnum>> enumerators;

        for (const auto enumerator : Util::enumerators_array(^^TEnum)) {
            const auto annotations = std::meta::annotations_of_with_type(enumerator, ^^ReprText);
            if (annotations.size() > 1) {
                throw std::invalid_argument(std::format(
                    "{} has multiple ReprText annotations", std::meta::identifier_of(enumerator)));
            }

            auto repr = annotations.empty()
                ? Util::identifier_string(enumerator)
                : std::meta::extract<ReprText>(annotations.front()).text;

            enumerators.push_back({//
                .value = std::meta::extract<TEnum>(enumerator),
                .identifier = Util::identifier_string(enumerator),
                .repr = repr});
        }

        return std::define_static_array(enumerators);
    }();
};

} // namespace Revo::Bindings
