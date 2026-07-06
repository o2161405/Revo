#pragma once

#include <array>
#include <span>
#include <string_view>

#include <gctypes.h>

extern "C" {
    // Basic tests
    u32 test_common_operations(u32 a, u32 b);
    u32 test_sum_array(const u32 *ptr, u32 count);
    u32 test_conditional_add(u32 a, u32 b);
    u32 test_read_write(u32 *memory, u32 value);

    // Control flow tests
    u32 test_countdown(u32 n);
    u32 test_fallthrough(u32 x);
    u32 test_nested_loop(u32 outer, u32 inner);
    u32 test_three_way(s32 selector);
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
    if (id == TestID::CommonOperations) {
        u32 got = test_common_operations(2, 3);
        return TestResult::single(got == 6, 6, got);
    } 
    else if (id == TestID::SumArray) {
        static const u32 data[] = { 1, 2, 3, 4, 5, 6, 7 };
        u32 got = test_sum_array(data, 7);
        return TestResult::single(got == 28, 28, got);
    } 
    else if (id == TestID::ConditionalAdd) {
        TestResult result;
        u32 got;
        got = test_conditional_add(7, 3); result.add(got == 107, 107, got);
        got = test_conditional_add(3, 7); result.add(got == 10, 10, got);
        return result;
    } 
    else if (id == TestID::ReadWrite) {
        u32 memory = 0;
        u32 got = test_read_write(&memory, 12345678);
        return TestResult::single(got == 12345678, 12345678, got);
    } 
    else if (id == TestID::Countdown) {
        u32 got = test_countdown(5);
        return TestResult::single(got == 0, 0, got);
    } 
    else if (id == TestID::Fallthrough) {
        TestResult result;
        u32 got;
        got = test_fallthrough(0); result.add(got == 10, 10, got);
        got = test_fallthrough(5); result.add(got == 16, 16, got);
        return result;
    } 
    else if (id == TestID::NestedLoop) {
        u32 got = test_nested_loop(3, 4);
        return TestResult::single(got == 12, 12, got);
    } 
    else if (id == TestID::ThreeWay) {
        TestResult result;
        u32 got;
        got = test_three_way(-5); result.add(got == 0xFFFFFFFF, 0xFFFFFFFF, got);
        got = test_three_way(0); result.add(got == 0, 0, got);
        got = test_three_way(7); result.add(got == 1, 1, got);
        return result;
    } 
    
    return TestResult::single(false, 0, 0); 
}

static const Test basicTests[] = {
    { "Common operations", TestID::CommonOperations },
    { "Sum array", TestID::SumArray },
    { "Conditional add", TestID::ConditionalAdd },
    { "Read/Write", TestID::ReadWrite },
};

static const Test controlFlowTests[] = {
    { "Countdown", TestID::Countdown },
    { "Fall-through", TestID::Fallthrough },
    { "Nested loop", TestID::NestedLoop },
    { "Three-way", TestID::ThreeWay }
};

static const TestGroup testGroups[] = {
    { "Basic tests", basicTests },
    { "Control flow tests", controlFlowTests },
};