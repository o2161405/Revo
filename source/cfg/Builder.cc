#include "Builder.hh"

#include "cfg/LinkImplementation.hh"
#include "ppc/Common.hh"
#include "ppc/Mnemonic.hh"
#include "util/Util.hh"

#include <algorithm>
#include <deque>
#include <functional>
#include <meta>
#include <ranges>

// todo: make this use symbol names instead of addresses

namespace Revo::CFG {

namespace {

[[nodiscard]] constexpr Terminator
instruction_terminator(const Decoder::Instruction& instruction) {
    if (instruction.is_call()) {
        return Terminator::Call;
    }
    if (instruction.branch_destination()) {
        return Terminator::Branch;
    }
    if (!instruction.indirect_branch_source) {
        return Terminator::Fallthrough;
    }
    if (*instruction.indirect_branch_source == PPC::Register::SPR::LR) {
        return Terminator::Return;
    }

    return Terminator::Indirect;
}

} // namespace

std::expected<Graph, std::string>
build(std::span<const Decoder::Function> functions) {
    Graph graph;

    return Impl::mark_leaders(functions)
        .and_then([&](const auto& leaders) { //
            return Impl::construct_blocks(graph, functions, leaders);
        })
        .and_then([&] { return Impl::construct_edges(graph); })
        .and_then([&] { return Impl::check_returns(graph); })
        .and_then([&] { return Impl::check_unreachable(graph); })
        .transform([&] {
            Console::success("Graphed {} blocks and {} edges", //
                graph.blocks.size(), graph.edges.size());
            return std::move(graph);
        });
}

namespace Impl {

std::expected<std::flat_set<u32>, std::string>
mark_leaders(std::span<const Decoder::Function> functions) {
    std::flat_set<u32> leaders;

    for (const auto& function : functions) {
        leaders.insert(function.offset);

        for (const auto& instruction : function.instructions) {
            const auto terminator = instruction_terminator(instruction);
            const auto destination = instruction.branch_destination();

            if (terminator == Terminator::Indirect) {
                return std::unexpected(std::format( //
                    "branch at {:#x} has a computed destination, which isn't supported.",
                    instruction.address));
            }

            if (terminator == Terminator::Call && !destination) {
                return std::unexpected(std::format( //
                    "call at {:#x} has a computed destination, which isn't supported.",
                    instruction.address));
            }

            if (destination) {
                leaders.insert(*destination);
            }

            if (terminator == Terminator::Branch || terminator == Terminator::Return) {
                const auto next_address = instruction.address + PPC::INSTRUCTION_SIZE;

                if (function.contains(next_address)) {
                    leaders.insert(next_address);
                }
            }
        }
    }

    return leaders;
}

std::expected<void, std::string>
construct_blocks(Graph& graph, std::span<const Decoder::Function> functions,
    const std::flat_set<u32>& leaders) //
{
    const auto same_block = [&](const auto&, const auto& next) {
        return !leaders.contains(next.address);
    };

    const auto make_block = [](auto instructions) {
        return Block{//
            .instructions = instructions,
            .terminator = instruction_terminator(instructions.back())};
    };

    for (const auto& function : functions) {
        graph.add_function(function.offset,
            function.instructions //
                | std::views::chunk_by(same_block) //
                | std::views::transform(make_block) //
                | std::ranges::to<std::vector>());
    }

    return {};
}

std::expected<void, std::string>
construct_edges(Graph& graph) {
    for (const auto& function : graph.functions) {
        for (const auto id : std::views::iota(function.first, function.last)) {
            const auto& block = graph.block(id);
            const auto& last = block.last();

            for (const auto& call : block.instructions //
                    | std::views::filter(&Decoder::Instruction::is_call)) {
                graph.add_edge(id, call, *call.branch_destination(), Edge::Type::Call);
            }

            if (instruction_terminator(last) == Terminator::Branch) {
                graph.add_edge(id, last, *last.branch_destination(), Edge::Type::Branch);
            }

            if (!block.falls_through()) {
                continue;
            }

            const auto next_address = last.address + PPC::INSTRUCTION_SIZE;
            const auto destination = graph.add_edge(
                id, last, next_address, Edge::Type::Fallthrough);

            if (!destination) {
                return std::unexpected(std::format( //
                    "block at {:#x} falls through to {:#x}, which isn't in an input function.", //
                    block.address(), next_address));
            }

            if (!function.contains(*destination)) {
                Console::warning("Block at {:#x} falls through into function {:#x}", //
                    block.address(), next_address);
            }
        }
    }

    for (const auto& edge : graph.edges | std::views::values //
            | std::views::filter(&Edge::is_external)) {
        Console::debug("Edge at {:#x} branches to external address {:#x}", //
            edge.source_address, edge.destination_address);
    }

    return {};
}

std::expected<void, std::string>
check_returns(const Graph& graph) {
    for (const auto& function : graph.functions) {
        for (auto [id, context] : merge_contexts(graph, function)) {
            const auto& block = graph.block(id);

            for (const auto& instruction : block.instructions) {
                apply_link(context, instruction);
            }

            if (instruction_terminator(block.last()) == Terminator::Return &&
                !context.return_in_lr) {
                return std::unexpected(std::format( //
                    "return at {:#x} has a computed destination, which isn't supported.",
                    block.last().address));
            }
        }
    }

    return {};
}

std::expected<void, std::string>
check_unreachable(const Graph& graph) {
    std::vector<bool> reachable(graph.blocks.size());
    std::deque<BlockId> queue;

    for (const auto& function : graph.functions) {
        reachable[function.first] = true;
        queue.push_back(function.first);
    }

    while (!queue.empty()) {
        const auto id = queue.front();
        queue.pop_front();

        for (const auto destination : graph.blocks_from(id)) {
            if (reachable[destination]) {
                continue;
            }

            reachable[destination] = true;
            queue.push_back(destination);
        }
    }

    for (const auto [id, block] : Util::enumerate<BlockId>(graph.blocks)) {
        if (!reachable[id]) {
            Console::warning("Block at {:#x} is unreachable", block.address());
        }
    }

    return {};
}

std::flat_map<BlockId, LinkContext>
merge_contexts(const Graph& graph, const Function& function) {
    std::flat_map<BlockId, LinkContext> contexts;
    std::deque<BlockId> queue{function.first};

    contexts.emplace(function.first, LinkContext{});

    while (!queue.empty()) {
        const auto id = queue.front();
        queue.pop_front();

        auto context = contexts.at(id);
        for (const auto& instruction : graph.block(id).instructions) {
            apply_link(context, instruction);
        }

        for (const auto destination : graph.blocks_from(id, function)) {
            const auto [it, inserted] = contexts.try_emplace(destination, context);
            auto merged = it->second.merge(context);

            if (!inserted && merged == it->second) {
                continue;
            }

            it->second = std::move(merged);

            if (!std::ranges::contains(queue, destination)) {
                queue.push_back(destination);
            }
        }
    }

    return contexts;
}

void
apply_link(LinkContext& context, const Decoder::Instruction& instruction) {
    template for (constexpr auto enumerator : Util::enumerators_array(^^PPC::Mnemonic)) {
        constexpr auto mnemonic = [:enumerator:];
        if (instruction.mnemonic == mnemonic) {
            LinkImplementation<mnemonic>::apply(context, instruction);
        }
    }

    if (instruction.is_call()) {
        context.call();
    }
}

} // namespace Impl

} // namespace Revo::CFG
