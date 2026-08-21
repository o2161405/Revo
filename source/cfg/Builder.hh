#pragma once

#include "cfg/Graph.hh"
#include "cfg/LinkContext.hh"
#include "decode/Types.hh"

#include <expected>
#include <flat_map>
#include <flat_set>
#include <span>
#include <string>

namespace Revo::CFG {

class Builder {
public:
    [[nodiscard]] static std::expected<Graph, std::string>
    build(std::span<const Decode::Function> functions);

private:
    Builder() = default;

    // Building steps
    [[nodiscard]] std::expected<void, std::string>
    mark_leaders(std::span<const Decode::Function> functions);

    [[nodiscard]] std::expected<void, std::string>
    construct_blocks(std::span<const Decode::Function> functions);

    [[nodiscard]] std::expected<void, std::string>
    construct_edges();

    [[nodiscard]] std::expected<void, std::string>
    check_returns();

    [[nodiscard]] std::expected<void, std::string>
    check_unreachable();

    // Utility functions
    [[nodiscard]] std::flat_map<BlockId, LinkContext>
    merge_contexts(const Function& function) const;

    static void
    apply_link(LinkContext& context, const Decode::Instruction& instruction);

    Graph mGraph;
    std::flat_set<u32> mLeaders;
};

} // namespace Revo::CFG
