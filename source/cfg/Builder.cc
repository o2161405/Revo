#include "Builder.hh"

#include "cfg/LinkImplementation.hh"
#include "cfg/Terminator.hh"
#include "ppc/Mnemonic.hh"
#include "util/Util.hh"

#include <algorithm>
#include <deque>
#include <functional>
#include <meta>
#include <ranges>

// todo: make this use symbol names instead of addresses

namespace Revo::CFG {

std::expected<Graph, std::string>
Builder::build(std::span<const Decode::Function> functions) {
    Builder builder;

    return builder.mark_leaders(functions)
        .and_then(std::bind_front(&Builder::construct_blocks, &builder, functions))
        .and_then(std::bind_front(&Builder::construct_edges, &builder))
        .and_then(std::bind_front(&Builder::check_returns, &builder))
        .and_then(std::bind_front(&Builder::check_unreachable, &builder))
        .transform([&] {
            Console::success("Graphed {} blocks and {} edges", //
                builder.mGraph.blocks.size(), builder.mGraph.edges.size());
            return std::move(builder.mGraph);
        });
}

std::expected<void, std::string>
Builder::mark_leaders(std::span<const Decode::Function> functions) {
    for (const auto& function : functions) {
        mLeaders.insert(function.offset);

        for (const auto& instruction : function.instructions) {
            const auto instruction_terminator = terminator(instruction);
            const auto destination = instruction.branch_destination();

            if (instruction_terminator == Terminator::Indirect) {
                return std::unexpected(std::format( //
                    "branch {:#x} has a computed destination, which isn't supported",
                    instruction.address));
            }

            if (instruction_terminator == Terminator::Call && !destination) {
                return std::unexpected(std::format( //
                    "call {:#x} has a computed destination, which isn't supported",
                    instruction.address));
            }

            if (destination) {
                mLeaders.insert(*destination);
            }

            if (instruction_terminator == Terminator::Branch ||
                instruction_terminator == Terminator::Return) {
                const auto next_address = instruction.address + PPC::INSTRUCTION_SIZE;

                if (next_address < function.offset + function.size) {
                    mLeaders.insert(next_address);
                }
            }
        }
    }

    return {};
}

std::expected<void, std::string>
Builder::construct_blocks(std::span<const Decode::Function> functions) {
    const auto same_block = [&](const auto&, const Decode::Instruction& next) {
        return !mLeaders.contains(next.address);
    };

    const auto make_block = [](auto chunk) { //
        return Block{.instructions = chunk};
    };

    for (const auto& function : functions) {
        mGraph.add_function(function.offset,
            function.instructions //
                | std::views::chunk_by(same_block) //
                | std::views::transform(make_block) //
                | std::ranges::to<std::vector>());
    }

    return {};
}

std::expected<void, std::string>
Builder::construct_edges() {
    for (const auto& function : mGraph.functions) {
        for (const auto id : function.ids()) {
            const auto& block = mGraph.block(id);
            const auto& last = block.last();

            for (const auto& call : block.instructions //
                    | std::views::filter(&Decode::Instruction::is_call)) {
                mGraph.add_edge(id, call, *call.branch_destination(), Edge::Type::Call);
            }

            if (terminator(last) == Terminator::Branch) {
                mGraph.add_edge(id, last, *last.branch_destination(), Edge::Type::Branch);
            }

            if (!falls_through(last)) {
                continue;
            }

            const auto next_address = block.last().address + PPC::INSTRUCTION_SIZE;
            const auto destination = mGraph.add_edge(
                id, last, next_address, Edge::Type::Fallthrough);

            if (!destination) {
                return std::unexpected(std::format( //
                    "block {:#x} falls through to {:#x}, which isn't associated with a function", //
                    block.address(), next_address));
            }

            if (!function.contains(*destination)) {
                Console::warning("Block {:#x} falls through into function {:#x}", //
                    block.address(), next_address);
            }
        }
    }

    for (const auto& edge : mGraph.edges //
            | std::views::values | std::views::filter(&Edge::is_external)) {
        Console::debug("Edge {:#x} branches to external address {:#x}", //
            edge.source_address, edge.destination_address);
    }

    return {};
}

std::expected<void, std::string>
Builder::check_returns() {
    for (const auto& function : mGraph.functions) {
        for (auto [id, context] : merge_contexts(function)) {
            const auto& block = mGraph.block(id);

            for (const auto& instruction : block.instructions) {
                apply_link(context, instruction);
            }

            if (terminator(block.last()) == Terminator::Return && !context.return_in_lr) {
                return std::unexpected(std::format( //
                    "return {:#x} has a computed destination, which isn't supported",
                    block.last().address));
            }
        }
    }

    return {};
}

std::expected<void, std::string>
Builder::check_unreachable() {
    std::vector<bool> reachable(mGraph.blocks.size());
    std::deque<BlockId> queue;

    for (const auto& function : mGraph.functions) {
        reachable[function.first] = true;
        queue.push_back(function.first);
    }

    while (!queue.empty()) {
        const auto id = queue.front();
        queue.pop_front();

        for (const auto destination : mGraph.blocks_from(id)) {
            if (reachable[destination]) {
                continue;
            }

            reachable[destination] = true;
            queue.push_back(destination);
        }
    }

    for (const auto [id, block] : Util::enumerate<BlockId>(mGraph.blocks)) {
        if (!reachable[id]) {
            Console::warning("Block {:#x} is unreachable", block.address());
        }
    }

    return {};
}

std::flat_map<BlockId, LinkContext>
Builder::merge_contexts(const Function& function) const {
    std::flat_map<BlockId, LinkContext> contexts;
    std::deque<BlockId> queue{function.first};

    contexts.emplace(function.first, LinkContext{});

    while (!queue.empty()) {
        const auto id = queue.front();
        queue.pop_front();

        auto context = contexts.at(id);
        for (const auto& instruction : mGraph.block(id).instructions) {
            apply_link(context, instruction);
        }

        for (const auto destination : mGraph.blocks_from(id, function)) {
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
Builder::apply_link(LinkContext& context, const Decode::Instruction& instruction) {
    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^PPC::Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        if (instruction.mnemonic == mnemonic) {
            LinkImplementation<mnemonic>::apply(context, instruction);
        }
    }

    if (instruction.is_call()) {
        context.call();
    }
}

} // namespace Revo::CFG
