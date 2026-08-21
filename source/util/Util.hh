#pragma once

#include <bit>
#include <concepts>
#include <meta>
#include <ranges>
#include <type_traits>

namespace Revo::Util {

template <typename TVariable>
constexpr void
byteswap(TVariable& variable) noexcept {
    if constexpr (std::integral<TVariable>) {
        if constexpr (sizeof(TVariable) > 1 && std::endian::native == std::endian::little) {
            variable = std::byteswap(variable);
        }
    }
    else if constexpr (std::ranges::range<TVariable>) {
        for (auto& subvariable : variable) {
            byteswap(subvariable);
        }
    }
    else if constexpr (std::is_class_v<TVariable>) {
        constexpr auto ACCESS_CONTEXT = std::meta::access_context::current();
        template for (constexpr auto member : std::define_static_array(
                          std::meta::nonstatic_data_members_of(^^TVariable, ACCESS_CONTEXT))) {
            byteswap(variable.[:member:]);
        }
    }
    else {
        static_assert(false, "Unsupported TVariable type in byteswap");
    }
}

template <typename TType, std::ranges::viewable_range TRange>
[[nodiscard]] constexpr auto
enumerate(TRange&& range) {
    return std::views::zip(std::views::iota(TType{0}), std::forward<TRange>(range));
}

} // namespace Revo::Util
