#include "Object.hh"

#include "elf/Section.hh"
#include "elf/Symbol.hh"

#include <algorithm>
#include <functional>
#include <optional>
#include <string_view>

namespace Revo::ELF {

std::optional<const Section&>
Object::get_section(std::string_view name) const {
    if (const auto it = std::ranges::find(sections, name, &Section::name); it != sections.end()) {
        return *it;
    }

    return std::nullopt;
}

std::optional<const Symbol&>
Object::get_symbol(std::string_view name) const {
    if (const auto it = std::ranges::find(symbols, name, &Symbol::name); it != symbols.end()) {
        return *it;
    }

    return std::nullopt;
}

} // namespace Revo::ELF
