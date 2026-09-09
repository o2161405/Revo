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

[[nodiscard]] std::expected<Graph, std::string>
build(std::span<const Decode::Function> functions);

namespace Impl {

// Building steps
[[nodiscard]] std::expected<std::flat_set<u32>, std::string>
mark_leaders(std::span<const Decode::Function> functions);

[[nodiscard]] std::expected<void, std::string>
construct_blocks(Graph& graph, std::span<const Decode::Function> functions, //
    const std::flat_set<u32>& leaders);

[[nodiscard]] std::expected<void, std::string>
construct_edges(Graph& graph);

[[nodiscard]] std::expected<void, std::string>
check_returns(const Graph& graph);

[[nodiscard]] std::expected<void, std::string>
check_unreachable(const Graph& graph);

// Utility functions
[[nodiscard]] std::flat_map<BlockId, LinkContext>
merge_contexts(const Graph& graph, const Function& function);

void
apply_link(LinkContext& context, const Decode::Instruction& instruction);

} // namespace Impl

} // namespace Revo::CFG
