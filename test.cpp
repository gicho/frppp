/**
 * @file test.cpp
 * @brief Simple tests for the FRP library
 * 
 * This file contains basic tests for the FRP library to verify
 * that it works correctly in different scenarios.
 */

#include "frp.hpp"
#include <iostream>
#include <cassert>
#include <string>

// Simple test framework
#define TEST(name) \
    std::cout << "Running test: " << name << "... "; \
    try {

#define END_TEST \
        std::cout << "PASSED" << std::endl; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << std::endl; \
        return 1; \
    } catch (...) { \
        std::cout << "FAILED: Unknown exception" << std::endl; \
        return 1; \
    }

// Test Cell functionality
void test_cell() {
    TEST("Cell basic operations")
        // Create a cell with an initial value
        frp::Cell<int> cell(42);
        
        // Check initial value
        assert(cell.value() == 42);
        
        // Update the value
        cell.set_value(100);
        assert(cell.value() == 100);
        
        // Test map function
        auto mapped_cell = cell.map([](int x) { return x * 2; });
        assert(mapped_cell.value() == 200);
    END_TEST
}

// Test Behavior functionality
void test_behavior() {
    TEST("Behavior basic operations")
        // Create a behavior with a function
        frp::Behavior<int> behavior([]() { return 42; });
        
        // Sample the behavior
        assert(behavior.sample() == 42);
        
        // Test map function
        auto mapped_behavior = behavior.map([](int x) { return x * 2; });
        assert(mapped_behavior.sample() == 84);
        
        // Create a cell and a behavior from it
        frp::Cell<int> cell(10);
        auto cell_behavior = frp::behavior_from_cell(cell);
        assert(cell_behavior.sample() == 10);
        
        // Update the cell and check the behavior
        cell.set_value(20);
        assert(cell_behavior.sample() == 20);
    END_TEST
    
    TEST("Behavior lifting")
        // Create two behaviors
        frp::Behavior<int> b1([]() { return 10; });
        frp::Behavior<int> b2([]() { return 20; });
        
        // Lift a function to operate on behaviors
        auto sum = frp::lift([](int x, int y) { return x + y; }, b1, b2);
        assert(sum.sample() == 30);
        
        // Lift a more complex function
        auto complex = frp::lift(
            [](int x, int y) { return x * y + x - y; },
            b1, b2
        );
        assert(complex.sample() == 10 * 20 + 10 - 20); // 190
    END_TEST
}

// Test Signal functionality
void test_signal() {
    TEST("Signal basic operations")
        // Create a signal
        frp::Signal<int> signal(42);
        
        // Check if the signal occurred
        assert(signal.occurred());
        
        // Check the value
        assert(signal.value() == 42);
        
        // Reset the signal
        signal.reset();
        assert(!signal.occurred());
        
        // Fire the signal again
        signal.fire(100);
        assert(signal.occurred());
        assert(signal.value() == 100);
        
        // Test map function
        auto mapped_signal = signal.map([](int x) { return x * 2; });
        assert(mapped_signal.occurred());
        assert(mapped_signal.value() == 200);
        
        // Create an empty signal and map it
        frp::Signal<int> empty_signal;
        auto mapped_empty = empty_signal.map([](int x) { return x * 2; });
        assert(!mapped_empty.occurred());
    END_TEST
    
    TEST("Signal filtering and merging")
        // Create two signals
        frp::Signal<int> s1(10);
        frp::Signal<int> s2(20);
        
        // Filter a signal
        auto filtered = frp::filter(s1, [](int x) { return x > 5; });
        assert(filtered.occurred());
        assert(filtered.value() == 10);
        
        // Filter a signal that doesn't pass the predicate
        auto filtered_fail = frp::filter(s1, [](int x) { return x > 15; });
        assert(!filtered_fail.occurred());
        
        // Merge two signals
        auto merged = frp::merge(s1, s2, [](int x, int y) { return x + y; });
        assert(merged.occurred());
        assert(merged.value() == 30);
        
        // Merge with an empty signal
        frp::Signal<int> empty;
        auto merged_with_empty = frp::merge(s1, empty, [](int x, int y) { return x + y; });
        assert(merged_with_empty.occurred());
        assert(merged_with_empty.value() == 10);
    END_TEST
}

// Test Sink functionality
void test_sink() {
    TEST("Sink basic operations")
        // Create a variable to track sink calls
        int processed_value = 0;
        bool was_called = false;
        
        // Create a sink
        frp::Sink<int> sink([&](const int& value) {
            processed_value = value;
            was_called = true;
        });
        
        // Create a signal and process it
        frp::Signal<int> signal(42);
        sink.process(signal);
        
        // Check that the sink was called with the right value
        assert(was_called);
        assert(processed_value == 42);
        
        // Reset tracking variables
        was_called = false;
        processed_value = 0;
        
        // Create an empty signal and process it
        frp::Signal<int> empty_signal;
        sink.process(empty_signal);
        
        // Check that the sink was not called
        assert(!was_called);
        assert(processed_value == 0);
    END_TEST
}

// Test ReactiveGraph functionality
void test_reactive_graph() {
    TEST("ReactiveGraph basic operations")
        // Create cells
        frp::Cell<int> input(10);
        frp::Cell<int> processed(0);
        frp::Cell<std::string> output("");
        
        // Create a graph
        auto graph = frp::make_graph(input, processed, output);
        
        // Update processed based on input
        graph.update_cell<1>([](const auto& cells) {
            return std::get<0>(cells).value() * 2;
        });
        
        // Check that processed was updated
        assert(graph.get_cell<1>().value() == 20);
        
        // Update output based on processed
        graph.update_cell<2>([](const auto& cells) {
            return "Result: " + std::to_string(std::get<1>(cells).value());
        });
        
        // Check that output was updated
        assert(graph.get_cell<2>().value() == "Result: 20");
        
        // Update input and propagate changes
        graph.get_cell<0>().set_value(15);
        
        // Update processed based on new input
        graph.update_cell<1>([](const auto& cells) {
            return std::get<0>(cells).value() * 2;
        });
        
        // Update output based on new processed
        graph.update_cell<2>([](const auto& cells) {
            return "Result: " + std::to_string(std::get<1>(cells).value());
        });
        
        // Check that all values were updated correctly
        assert(graph.get_cell<0>().value() == 15);
        assert(graph.get_cell<1>().value() == 30);
        assert(graph.get_cell<2>().value() == "Result: 30");
    END_TEST
    
    TEST("ReactiveGraph with dependencies")
        // Create a more complex graph with dependencies
        frp::Cell<int> a(5);
        frp::Cell<int> b(10);
        frp::Cell<int> c(0);  // c = a + b
        frp::Cell<int> d(0);  // d = c * 2
        
        auto graph = frp::make_graph(a, b, c, d);
        
        // Update c based on a and b
        graph.update_cell<2>([](const auto& cells) {
            return std::get<0>(cells).value() + std::get<1>(cells).value();
        });
        
        // Update d based on c
        graph.update_cell<3>([](const auto& cells) {
            return std::get<2>(cells).value() * 2;
        });
        
        // Check initial values
        assert(graph.get_cell<2>().value() == 15);  // c = 5 + 10
        assert(graph.get_cell<3>().value() == 30);  // d = 15 * 2
        
        // Update a and propagate changes
        graph.get_cell<0>().set_value(7);
        
        // Update c based on new a and b
        graph.update_cell<2>([](const auto& cells) {
            return std::get<0>(cells).value() + std::get<1>(cells).value();
        });
        
        // Update d based on new c
        graph.update_cell<3>([](const auto& cells) {
            return std::get<2>(cells).value() * 2;
        });
        
        // Check updated values
        assert(graph.get_cell<2>().value() == 17);  // c = 7 + 10
        assert(graph.get_cell<3>().value() == 34);  // d = 17 * 2
    END_TEST
}

// Test constexpr functionality
void test_constexpr() {
    TEST("Constexpr functionality")
        // Verify that key operations can be performed at compile time
        constexpr frp::Cell<int> cell(42);
        constexpr int value = cell.value();
        static_assert(value == 42, "Cell value should be 42");
        
        // Test constexpr behavior
        constexpr frp::Behavior<int> behavior([]() constexpr { return 42; });
        constexpr int sampled = behavior.sample();
        static_assert(sampled == 42, "Behavior sample should be 42");
        
        // Test constexpr signal
        constexpr frp::Signal<int> signal(42);
        constexpr bool occurred = signal.occurred();
        constexpr int signal_value = signal.value();
        static_assert(occurred, "Signal should have occurred");
        static_assert(signal_value == 42, "Signal value should be 42");
    END_TEST
}

int main() {
    std::cout << "Running FRP library tests...\n";
    
    // Run all tests
    test_cell();
    test_behavior();
    test_signal();
    test_sink();
    test_reactive_graph();
    test_constexpr();
    
    std::cout << "All tests passed!\n";
    return 0;
}
