# FRP Library for Embedded Real-Time Systems

A header-only Functional Reactive Programming (FRP) library designed specifically for constrained embedded real-time systems. This library implements FRP principles while avoiding dynamic memory allocation and ensuring that the reactive graph is defined at compile time.

## Features

- **Header-only**: Easy to integrate into any C++20 project
- **No dynamic memory allocation**: All resources are statically allocated
- **Compile-time graph definition**: The reactive graph is defined at compile time
- **Type-safe**: Leverages C++20 concepts and type traits for compile-time safety
- **Efficient**: Designed for constrained embedded systems with limited resources
- **Real-time friendly**: Predictable performance characteristics

## Design Principles

This library follows several key design principles:

1. **Zero heap allocation**: All memory is allocated statically or on the stack
2. **Compile-time graph construction**: The reactive graph is defined at compile time
3. **Type safety**: Extensive use of C++20 concepts and type traits
4. **Minimal dependencies**: Only requires the C++ standard library
5. **Embedded-first**: Designed with embedded constraints in mind

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 19.29+)
- CMake 3.14+ (for building examples)

## Building the Examples

```bash
# Create a build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run the demo
./frp_demo
```

## Core Concepts

### Cells

A `Cell<T>` represents a value that can change over time. It's the basic building block of the FRP system.

```cpp
// Create a cell with an initial value
frp::Cell<int> counter(0);

// Get the current value
int current = counter.value();

// Update the value
counter.set_value(42);
```

### Behaviors

A `Behavior<T>` represents a function from time to values. It can be created from a cell or a function.

```cpp
// Create a behavior from a cell
auto counter_behavior = frp::behavior_from_cell(counter);

// Sample the current value
int current = counter_behavior.sample();

// Create a behavior from a function
frp::Behavior<double> sine_wave([]() {
    return std::sin(get_current_time());
});
```

### Signals

A `Signal<T>` represents a discrete event with a value. It can be filtered, mapped, and merged.

```cpp
// Create a signal
frp::Signal<double> temperature_signal(25.5);

// Check if the signal occurred
bool occurred = temperature_signal.occurred();

// Get the value
double temp = temperature_signal.value();

// Transform the signal
auto fahrenheit_signal = temperature_signal.map([](double celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
});
```

### Sinks

A `Sink<T>` represents a consumer of signals. It processes signals when they occur.

```cpp
// Create a sink
frp::Sink<double> temperature_sink([](const double& temp) {
    std::cout << "Temperature: " << temp << " °C\n";
});

// Process a signal
temperature_sink.process(temperature_signal);
```

### Reactive Graph

A `ReactiveGraph<Cells...>` represents a network of cells and behaviors. It manages the dependencies between cells and updates them accordingly.

```cpp
// Create a graph with cells
auto graph = frp::make_graph(cell1, cell2, cell3);

// Get a cell from the graph
auto& cell = graph.get_cell<0>();

// Update a cell in the graph
graph.update_cell<0>([](const auto& cells) {
    return compute_new_value(cells);
});
```

## Example Use Cases

The library includes several example use cases:

1. **Temperature Sensor System**: Processes raw sensor values into temperature readings, combines multiple readings, and generates alerts when thresholds are exceeded.

2. **Signal Processing System**: Filters and transforms discrete signals, merges multiple signals, and takes actions in response to signals.

3. **Motor Control System**: Combines multiple input sources to control motor behavior, enforces safety limits, and responds to emergency conditions.

## Advanced Features

### Static Function Wrapper

The library includes a `static_function` wrapper that provides type erasure without dynamic allocation:

```cpp
// Create a static function with a specific signature
frp::detail::static_function<void(int)> callback([](int value) {
    std::cout << "Received: " << value << std::endl;
});

// Invoke the function
callback(42);
```

### Function Lifting

Functions can be "lifted" to operate on behaviors:

```cpp
// Create behaviors
auto b1 = frp::Behavior<int>([]() { return 10; });
auto b2 = frp::Behavior<int>([]() { return 20; });

// Lift a function to operate on behaviors
auto sum = frp::lift([](int x, int y) { return x + y; }, b1, b2);

// Sample the result
int result = sum.sample(); // 30
```

## License

This library is provided under the MIT License. See the LICENSE file for details.
