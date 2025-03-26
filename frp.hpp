/**
 * @file frp.hpp
 * @brief Header-only Functional Reactive Programming (FRP) library for embedded real-time systems
 * 
 * This library implements FRP principles for constrained embedded systems with:
 * - No dynamic memory allocation
 * - Compile-time graph definition
 * - Static allocation of all resources
 * - C++20 features for type safety and compile-time evaluation
 */

#ifndef FRP_HPP
#define FRP_HPP

#include <array>
#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace frp {

/**
 * @brief Namespace for implementation details
 */
namespace detail {
    /**
     * @brief Type-erased function wrapper with static storage
     * 
     * This class provides a way to store callable objects without dynamic allocation.
     * The callable is stored in a fixed-size buffer and invoked through type erasure.
     * 
     * @tparam Signature Function signature
     * @tparam BufferSize Size of the internal buffer for storing the callable
     */
    template<typename Signature, std::size_t BufferSize = 64>
    class static_function;

    /**
     * @brief Specialization for functions with return type and arguments
     */
    template<typename R, typename... Args, std::size_t BufferSize>
    class static_function<R(Args...), BufferSize> {
    private:
        // Aligned storage for the callable object
        alignas(std::max_align_t) std::array<std::byte, BufferSize> buffer_;
        
        // Function pointer for invoking the stored callable
        R (*invoker_)(const void*, Args...);
        
        // Function pointer for copying the stored callable
        void (*copier_)(void*, const void*);
        
        // Function pointer for destroying the stored callable
        void (*destroyer_)(void*);

    public:
        /**
         * @brief Default constructor
         */
        constexpr static_function() noexcept
            : invoker_(nullptr), copier_(nullptr), destroyer_(nullptr) {}
        
        /**
         * @brief Constructor from callable object
         * 
         * @tparam F Type of callable
         * @param f Callable object
         */
        template<typename F>
        constexpr static_function(F f)
            requires (sizeof(F) <= BufferSize && std::is_invocable_r_v<R, F, Args...>) {
            
            // Store the callable in the buffer
            new (buffer_.data()) F(std::move(f));
            
            // Set up function pointers
            invoker_ = [](const void* ptr, Args... args) -> R {
                return (*static_cast<const F*>(ptr))(std::forward<Args>(args)...);
            };
            
            copier_ = [](void* dst, const void* src) {
                new (dst) F(*static_cast<const F*>(src));
            };
            
            destroyer_ = [](void* ptr) {
                static_cast<F*>(ptr)->~F();
            };
        }
        
        /**
         * @brief Copy constructor
         */
        constexpr static_function(const static_function& other) {
            if (other.invoker_) {
                other.copier_(buffer_.data(), other.buffer_.data());
                invoker_ = other.invoker_;
                copier_ = other.copier_;
                destroyer_ = other.destroyer_;
            } else {
                invoker_ = nullptr;
                copier_ = nullptr;
                destroyer_ = nullptr;
            }
        }
        
        /**
         * @brief Destructor
         */
        constexpr ~static_function() {
            if (destroyer_) {
                destroyer_(buffer_.data());
            }
        }
        
        /**
         * @brief Copy assignment operator
         */
        constexpr static_function& operator=(const static_function& other) {
            if (this != &other) {
                if (destroyer_) {
                    destroyer_(buffer_.data());
                }
                
                if (other.invoker_) {
                    other.copier_(buffer_.data(), other.buffer_.data());
                    invoker_ = other.invoker_;
                    copier_ = other.copier_;
                    destroyer_ = other.destroyer_;
                } else {
                    invoker_ = nullptr;
                    copier_ = nullptr;
                    destroyer_ = nullptr;
                }
            }
            return *this;
        }
        
        /**
         * @brief Function call operator
         */
        constexpr R operator()(Args... args) const {
            if (invoker_) {
                return invoker_(buffer_.data(), std::forward<Args>(args)...);
            }
            // Handle empty function case
            if constexpr (std::is_same_v<R, void>) {
                return;
            } else {
                return R{};
            }
        }
        
        /**
         * @brief Check if the function is callable
         */
        constexpr explicit operator bool() const noexcept {
            return invoker_ != nullptr;
        }
    };

    /**
     * @brief Helper for compile-time index sequences
     */
    template<std::size_t... Is>
    struct index_sequence {
        using type = index_sequence;
    };

    /**
     * @brief Helper to generate index sequences
     */
    template<std::size_t N, std::size_t... Is>
    struct make_index_sequence_impl : make_index_sequence_impl<N-1, N-1, Is...> {};

    template<std::size_t... Is>
    struct make_index_sequence_impl<0, Is...> : index_sequence<Is...> {};

    template<std::size_t N>
    using make_index_sequence = typename make_index_sequence_impl<N>::type;

} // namespace detail

/**
 * @brief Concept for types that can be used as cell values
 */
template<typename T>
concept CellValue = std::is_copy_constructible_v<T> && std::is_move_constructible_v<T>;

/**
 * @brief A cell represents a value that can change over time
 * 
 * @tparam T Type of the value stored in the cell
 */
template<CellValue T>
class Cell {
private:
    T value_;
    
public:
    /**
     * @brief Type of the value stored in the cell
     */
    using value_type = T;
    
    /**
     * @brief Constructor with initial value
     */
    constexpr explicit Cell(T initial_value) : value_(std::move(initial_value)) {}
    
    /**
     * @brief Get the current value of the cell
     */
    constexpr const T& value() const noexcept {
        return value_;
    }
    
    /**
     * @brief Update the value of the cell
     */
    constexpr void set_value(T new_value) {
        value_ = std::move(new_value);
    }
    
    /**
     * @brief Map function to transform the cell
     * 
     * @tparam F Function type
     * @param f Function to apply to the cell value
     * @return A new cell with the transformed value
     */
    template<typename F>
    constexpr auto map(F&& f) const {
        using R = std::invoke_result_t<F, T>;
        return Cell<R>(f(value_));
    }
};

/**
 * @brief A behavior represents a function from time to values
 * 
 * @tparam T Type of the value produced by the behavior
 */
template<CellValue T>
class Behavior {
private:
    detail::static_function<T()> function_;
    
public:
    /**
     * @brief Type of the value produced by the behavior
     */
    using value_type = T;
    
    /**
     * @brief Constructor with function
     */
    template<typename F>
    constexpr explicit Behavior(F&& f) : function_(std::forward<F>(f)) {}
    
    /**
     * @brief Constructor from a constant value
     */
    constexpr explicit Behavior(const T& value) 
        : function_([value]() { return value; }) {}
    
    /**
     * @brief Sample the behavior to get the current value
     */
    constexpr T sample() const {
        return function_();
    }
    
    /**
     * @brief Map function to transform the behavior
     * 
     * @tparam F Function type
     * @param f Function to apply to the behavior value
     * @return A new behavior with the transformed value
     */
    template<typename F>
    constexpr auto map(F&& f) const {
        using R = std::invoke_result_t<F, T>;
        return Behavior<R>([f, *this]() { return f(this->sample()); });
    }
};

/**
 * @brief Create a behavior from a cell
 * 
 * @tparam T Type of the cell value
 * @param cell Source cell
 * @return Behavior that samples the cell's value
 */
template<CellValue T>
constexpr auto behavior_from_cell(const Cell<T>& cell) {
    return Behavior<T>([&cell]() { return cell.value(); });
}

/**
 * @brief Lift a function to operate on behaviors
 * 
 * @tparam F Function type
 * @tparam Bs Behavior types
 * @param f Function to lift
 * @param bs Behaviors to apply the function to
 * @return A new behavior that applies the function to the sampled values
 */
template<typename F, typename... Bs>
constexpr auto lift(F&& f, const Bs&... bs) {
    using R = std::invoke_result_t<F, typename Bs::value_type...>;
    return Behavior<R>([f, &bs...]() { return f(bs.sample()...); });
}

/**
 * @brief A reactive graph represents a network of cells and behaviors
 * 
 * @tparam Cells Types of cells in the graph
 */
template<typename... Cells>
class ReactiveGraph {
private:
    std::tuple<Cells...> cells_;
    
public:
    /**
     * @brief Constructor with cells
     */
    constexpr explicit ReactiveGraph(Cells... cells) 
        : cells_(std::move(cells)...) {}
    
    /**
     * @brief Get a cell from the graph
     * 
     * @tparam I Index of the cell
     * @return Reference to the cell
     */
    template<std::size_t I>
    constexpr auto& get_cell() {
        return std::get<I>(cells_);
    }
    
    /**
     * @brief Get a cell from the graph (const version)
     * 
     * @tparam I Index of the cell
     * @return Const reference to the cell
     */
    template<std::size_t I>
    constexpr const auto& get_cell() const {
        return std::get<I>(cells_);
    }
    
    /**
     * @brief Update the graph based on dependencies
     * 
     * @tparam Deps Dependency indices
     * @param update_functions Functions to update cells
     */
    template<std::size_t... Deps, typename... Fs>
    constexpr void update(detail::index_sequence<Deps...>, Fs&&... update_functions) {
        (update_cell<Deps>(std::forward<Fs>(update_functions)), ...);
    }
    
    /**
     * @brief Update a specific cell
     * 
     * @tparam I Index of the cell to update
     * @param f Function to compute the new value
     */
    template<std::size_t I, typename F>
    constexpr void update_cell(F&& f) {
        auto& cell = std::get<I>(cells_);
        cell.set_value(f(cells_));
    }
};

/**
 * @brief Create a reactive graph from cells
 * 
 * @tparam Cells Types of cells
 * @param cells Cells to include in the graph
 * @return A new reactive graph
 */
template<typename... Cells>
constexpr auto make_graph(Cells... cells) {
    return ReactiveGraph<Cells...>(std::move(cells)...);
}

/**
 * @brief A signal represents a discrete event with a value
 * 
 * @tparam T Type of the value carried by the signal
 */
template<CellValue T>
class Signal {
private:
    T value_;
    bool occurred_;
    
public:
    /**
     * @brief Type of the value carried by the signal
     */
    using value_type = T;
    
    /**
     * @brief Default constructor
     */
    constexpr Signal() : value_{}, occurred_(false) {}
    
    /**
     * @brief Constructor with value
     */
    constexpr explicit Signal(T value) : value_(std::move(value)), occurred_(true) {}
    
    /**
     * @brief Check if the signal occurred
     */
    constexpr bool occurred() const noexcept {
        return occurred_;
    }
    
    /**
     * @brief Get the value of the signal
     */
    constexpr const T& value() const noexcept {
        return value_;
    }
    
    /**
     * @brief Reset the signal
     */
    constexpr void reset() noexcept {
        occurred_ = false;
    }
    
    /**
     * @brief Fire the signal with a value
     */
    constexpr void fire(T new_value) {
        value_ = std::move(new_value);
        occurred_ = true;
    }
    
    /**
     * @brief Map function to transform the signal
     * 
     * @tparam F Function type
     * @param f Function to apply to the signal value
     * @return A new signal with the transformed value
     */
    template<typename F>
    constexpr auto map(F&& f) const {
        using R = std::invoke_result_t<F, T>;
        if (occurred_) {
            return Signal<R>(f(value_));
        } else {
            return Signal<R>();
        }
    }
};

/**
 * @brief Merge two signals
 * 
 * @tparam T Type of the first signal
 * @tparam U Type of the second signal
 * @param s1 First signal
 * @param s2 Second signal
 * @param combine Function to combine values when both signals occur
 * @return A new signal that occurs when either input signal occurs
 */
template<CellValue T, CellValue U, typename F>
constexpr auto merge(const Signal<T>& s1, const Signal<U>& s2, F&& combine) {
    using R = std::invoke_result_t<F, T, U>;
    
    if (s1.occurred() && s2.occurred()) {
        return Signal<R>(combine(s1.value(), s2.value()));
    } else if (s1.occurred()) {
        return Signal<R>(combine(s1.value(), U{}));
    } else if (s2.occurred()) {
        return Signal<R>(combine(T{}, s2.value()));
    } else {
        return Signal<R>();
    }
}

/**
 * @brief Filter a signal based on a predicate
 * 
 * @tparam T Type of the signal
 * @tparam F Predicate type
 * @param signal Input signal
 * @param predicate Function to test the signal value
 * @return A new signal that only occurs when the predicate is true
 */
template<CellValue T, typename F>
constexpr auto filter(const Signal<T>& signal, F&& predicate) {
    if (signal.occurred() && predicate(signal.value())) {
        return Signal<T>(signal.value());
    } else {
        return Signal<T>();
    }
}

/**
 * @brief A sink represents a consumer of signals
 * 
 * @tparam T Type of the signal value
 */
template<CellValue T>
class Sink {
private:
    detail::static_function<void(const T&)> function_;
    
public:
    /**
     * @brief Constructor with function
     */
    template<typename F>
    constexpr explicit Sink(F&& f) : function_(std::forward<F>(f)) {}
    
    /**
     * @brief Process a signal
     */
    constexpr void process(const Signal<T>& signal) {
        if (signal.occurred()) {
            function_(signal.value());
        }
    }
};

} // namespace frp

#endif // FRP_HPP
