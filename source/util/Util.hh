#pragma once

#include <bit>
#include <concepts>
#include <meta>
#include <ranges>

namespace Revo::Util {

template <typename TObject>
constexpr void
byteswap(TObject& object) noexcept {
    constexpr auto ACCESS_CONTEXT = std::meta::access_context::current();

    auto byteswap_impl = []<typename TVariable>(
                             this auto const& self, TVariable& variable) constexpr noexcept {
        if constexpr (std::integral<TVariable>) {
            if constexpr (sizeof(TVariable) > 1 && std::endian::native == std::endian::little) {
                variable = std::byteswap(variable);
            }
        }
        else if constexpr (std::ranges::range<TVariable>) {
            for (auto& subvariable : variable) {
                self(subvariable);
            }
        }
        else if constexpr (std::is_class_v<TVariable>) {
            template for (constexpr auto member : std::define_static_array(
                              std::meta::nonstatic_data_members_of(^^TVariable, ACCESS_CONTEXT))) {
                self(variable.[:member:]);
            }
        }
        else {
            static_assert(false, "Unsupported TVariable type in byteswap");
        }
    };

    byteswap_impl(object);
}

} // namespace Revo::Util
