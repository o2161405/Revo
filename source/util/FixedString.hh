#pragma once

#include <cstddef>
#include <string_view>

namespace Revo::Util {
	template<std::size_t N>
	struct FixedString {
		char Data[N];

		consteval FixedString(const char (&str)[N]) {
			for (std::size_t index = 0; index < N; ++index) {
				Data[index] = str[index];
			}
		}

		constexpr operator std::string_view() const noexcept {
			return {Data, N - 1};
		}

		constexpr std::string_view view() const noexcept {
			return *this;
		}

		constexpr bool empty() const noexcept {
			return N == 1;
		}
	};
}