#pragma once

#include <array>
#include <span>
#include <string_view>

#include <gctypes.h>

extern "C" {
    u32 test_read_write(u32 *memory, u32 value);
}

struct SubResult {
    bool passed;
    u32  expected;
    u32  got;
};

struct [[nodiscard]] TestResult {
    static constexpr u32 MAX_SUBTESTS = 4;

    std::array<SubResult, MAX_SUBTESTS> subresults;
    u32 count = 0;

    static TestResult single(bool passed, u32 expected, u32 got) {
        TestResult result;
        result.subresults[0] = { passed, expected, got };
        result.count = 1;
        return result;
    }

    void add(bool passed, u32 expected, u32 got) {
        if (count < MAX_SUBTESTS)
            subresults[count++] = { passed, expected, got };
    }

    bool allPassed() const {
        for (u32 i = 0; i < count; ++i)
            if (!subresults[i].passed) return false;
        return true;
    }
};

enum class TestID {
    CommonOperations,
    SumArray,
    ConditionalAdd,
    ReadWrite,
    Countdown,
    Fallthrough,
    NestedLoop,
    ThreeWay
};

struct Test {
    std::string_view name;
    TestID id;
};

struct TestGroup {
    std::string_view label;
    std::span<const Test> tests;
};

inline TestResult execute_test(TestID id) {
    if (id == TestID::ReadWrite) {
        u32 memory = 0;
        u32 got = test_read_write(&memory, 12345678);
        return TestResult::single(got == 12345678, 12345678, got);
    } 
    
    return TestResult::single(false, 0, 0); 
}

static const Test basicTests[] = {
    { "Read/Write", TestID::ReadWrite },
};

static const TestGroup testGroups[] = {
    { "Basic tests", basicTests },
};