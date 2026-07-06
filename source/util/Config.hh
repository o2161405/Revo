#pragma once

// todo: make this an cli argument instead of baked into the binary
#define INPUT_SECTION ".revo_text"

namespace Revo::Config {

inline constexpr std::string_view InputSection{INPUT_SECTION};
inline constexpr std::string_view RelaInputSection{".rela" INPUT_SECTION};

} // namespace Revo::Config
