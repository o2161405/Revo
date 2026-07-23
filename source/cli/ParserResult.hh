#pragma once

#include "cli/Argument.hh"
#include "cli/Specification.hh"
#include "util/Console.hh"

#include <filesystem>
#include <flat_map>
#include <optional>
#include <variant>

namespace Revo::CLI {

struct ParserResult {
    std::flat_map<Argument::Type, Argument::Variant> arguments;

    [[nodiscard]] bool
    contains(Argument::Type type) const {
        return arguments.contains(type);
    }

    template <Argument::Type TType>
        requires HasType<ArgumentSpecification<TType>>
    [[nodiscard]] std::optional<typename ArgumentSpecification<TType>::Type>
    get() const {
        const auto it = arguments.find(TType);
        if (it == arguments.end()) {
            return std::nullopt;
        }

        return std::get<typename ArgumentSpecification<TType>::Type>(it->second);
    }
};

} // namespace Revo::CLI
