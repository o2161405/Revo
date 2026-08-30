#pragma once

#include "elf/Object.hh"

#include <expected>
#include <filesystem>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

namespace Revo::ELF {

[[nodiscard]] std::expected<Object, std::string>
parse(const std::filesystem::path& path);

[[nodiscard]] std::expected<Object, std::string>
parse(std::istream& stream);

namespace Impl {

[[nodiscard]] std::expected<void, std::string>
read_elf_header(Object&, std::istream&);

[[nodiscard]] std::expected<void, std::string>
read_sections(Object&, std::istream&);

[[nodiscard]] std::expected<void, std::string>
read_section_names(Object&);

[[nodiscard]] std::expected<void, std::string>
read_symbols(Object&);

[[nodiscard]] std::expected<void, std::string>
read_revo_relocations(Object&);

[[nodiscard]] std::expected<void, std::string>
read_revo_functions(Object&);

[[nodiscard]] std::expected<void, std::string>
check_functions(Object&);

[[nodiscard]] std::expected<void, std::string>
check_relocations(const Object&);

[[nodiscard]] std::expected<std::string_view, std::string>
read_string(const Section& section, u32 offset);

// see comment in .cc file about why the definition isn't here
template <typename TType>
[[nodiscard]] std::expected<std::vector<TType>, std::string>
read_table(const Section& section);

} // namespace Impl

} // namespace Revo::ELF
