#pragma once

#include "decode/Types.hh"
#include "ppc/Common.hh"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <flat_map>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

namespace Revo::CFG {

using BlockId = std::size_t;

struct Block {
    std::span<const Decode::Instruction> instructions;

    [[nodiscard]] constexpr u32
    address() const {
        return instructions.front().address;
    }

    [[nodiscard]] constexpr const Decode::Instruction&
    last() const {
        return instructions.back();
    }
};

struct Function {
    BlockId first;
    BlockId last;
    u32 address;

    [[nodiscard]] constexpr bool
    contains(BlockId id) const {
        return id >= first && id < last;
    }

    [[nodiscard]] constexpr std::size_t
    size() const {
        return last - first;
    }

    [[nodiscard]] constexpr auto
    ids() const {
        return std::views::iota(first, last);
    }
};

struct Edge {
    enum class Type : u8 {
        Fallthrough,
        Branch,
        Call,
    };

    BlockId source_block;
    std::optional<BlockId> destination_block;
    u32 source_address;
    u32 destination_address;
    Type type;

    [[nodiscard]] constexpr bool
    is_call() const {
        return type == Type::Call;
    }

    [[nodiscard]] constexpr bool
    is_external() const {
        return !destination_block.has_value();
    }
};

struct Graph {
    std::vector<Block> blocks;
    std::vector<Function> functions;
    std::flat_multimap<BlockId, Edge> edges;

    [[nodiscard]] const Block&
    block(BlockId id) const pre(id < blocks.size()) {
        return blocks[id];
    }

    [[nodiscard]] std::optional<BlockId>
    find_block(u32 address) const {
        const auto it = std::ranges::lower_bound(blocks, address, {}, &Block::address);

        if (it == blocks.end() || it->address() != address) {
            return std::nullopt;
        }

        return static_cast<BlockId>(std::ranges::distance(blocks.begin(), it));
    }

    void
    add_function(u32 address, std::span<const Block> function_blocks) {
        const auto first = blocks.size();

        blocks.append_range(function_blocks);
        functions.push_back(Function{//
            .first = first,
            .last = blocks.size(),
            .address = address});
    }

    std::optional<BlockId>
    add_edge(BlockId source, const Decode::Instruction& from, u32 destination, Edge::Type type) {
        const auto destination_block = find_block(destination);

        /* clang-format off */
        edges.emplace(source, Edge{//
            .source_block = source,
            .destination_block = destination_block,
            .source_address = from.address,
            .destination_address = destination,
            .type = type
        });
        /* clang-format on */

        return destination_block;
    }

    [[nodiscard]] auto
    edges_from(BlockId id) const {
        const auto [first, last] = edges.equal_range(id);
        return std::ranges::subrange{first, last} | std::views::values;
    }

    [[nodiscard]] auto
    blocks_from(BlockId id) const {
        return edges_from(id) //
            | std::views::transform(&Edge::destination_block) //
            | std::views::join;
    }

    [[nodiscard]] auto
    blocks_from(BlockId id, const Function& function) const {
        return edges_from(id) //
            | std::views::filter(std::not_fn(&Edge::is_call)) //
            | std::views::transform(&Edge::destination_block) //
            | std::views::join //
            | std::views::filter(std::bind_front(&Function::contains, function));
    }
};

} // namespace Revo::CFG
