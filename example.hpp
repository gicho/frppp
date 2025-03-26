/**
 * @file example.hpp
 * @brief Example usage of the FRP library for embedded real-time systems
 * 
 * This file demonstrates how to use the FRP library in a practical embedded
 * system scenario without dynamic memory allocation.
 */

#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include "frp.hpp"
#include <cmath>
#include <iostream>

namespace example {

/**
 * @brief Example of a temperature sensor system using FRP
 * 
 * This example demonstrates a temperature monitoring system where:
 * - Raw sensor values are processed into temperature readings
 * - Multiple temperature readings are combined
 * - Alerts are generated when thresholds are exceeded
 * - All without dynamic memory allocation
 */
class TemperatureSensorSystem {
private:
    // Input cells (raw sensor values)
    frp::Cell<float> sensor1_raw_;
    frp::Cell<float> sensor2_raw_;
    
    // Processed temperature cells
    frp::Cell<float> sensor1_celsius_;
    frp::Cell<float> sensor2_celsius_;
    frp::Cell<float> average_temperature_;
    
    // Alert state
    frp::Cell<bool> high_temp_alert_;
    
    // Constants
    static constexpr float HIGH_TEMP_THRESHOLD = 50.0f;
    
    // The reactive graph
    using Graph = frp::ReactiveGraph<
        frp::Cell<float>, // sensor1_raw_
        frp::Cell<float>, // sensor2_raw_
        frp::Cell<float>, // sensor1_celsius_
        frp::Cell<float>, // sensor2_celsius_
        frp::Cell<float>, // average_temperature_
        frp::Cell<bool>   // high_temp_alert_
    >;
    
    Graph graph_;
    
    // Conversion function: raw ADC value to Celsius
    static constexpr float raw_to_celsius(float raw_value) {
        // Example conversion formula (would be calibrated for real sensors)
        return (raw_value * 0.1f) - 20.0f;
    }
    
    // Function to check if temperature exceeds threshold
    static constexpr bool is_high_temperature(float temp) {
        return temp > HIGH_TEMP_THRESHOLD;
    }
    
public:
    /**
     * @brief Constructor
     * 
     * Initializes the FRP graph with initial values
     */
    constexpr TemperatureSensorSystem()
        : sensor1_raw_(0.0f)
        , sensor2_raw_(0.0f)
        , sensor1_celsius_(0.0f)
        , sensor2_celsius_(0.0f)
        , average_temperature_(0.0f)
        , high_temp_alert_(false)
        , graph_(
            sensor1_raw_,
            sensor2_raw_,
            sensor1_celsius_,
            sensor2_celsius_,
            average_temperature_,
            high_temp_alert_
          )
    {
        // Initial update to establish the graph relationships
        update_graph();
    }
    
    /**
     * @brief Update sensor 1 with a new raw value
     */
    constexpr void update_sensor1(float raw_value) {
        graph_.get_cell<0>().set_value(raw_value);
        update_graph();
    }
    
    /**
     * @brief Update sensor 2 with a new raw value
     */
    constexpr void update_sensor2(float raw_value) {
        graph_.get_cell<1>().set_value(raw_value);
        update_graph();
    }
    
    /**
     * @brief Get the current temperature from sensor 1
     */
    constexpr float get_sensor1_temperature() const {
        return graph_.get_cell<2>().value();
    }
    
    /**
     * @brief Get the current temperature from sensor 2
     */
    constexpr float get_sensor2_temperature() const {
        return graph_.get_cell<3>().value();
    }
    
    /**
     * @brief Get the current average temperature
     */
    constexpr float get_average_temperature() const {
        return graph_.get_cell<4>().value();
    }
    
    /**
     * @brief Check if high temperature alert is active
     */
    constexpr bool is_alert_active() const {
        return graph_.get_cell<5>().value();
    }
    
private:
    /**
     * @brief Update the graph based on current values
     * 
     * This propagates changes through the reactive graph
     */
    constexpr void update_graph() {
        // Update sensor1_celsius from sensor1_raw
        graph_.update_cell<2>([](const auto& cells) {
            const auto& raw = std::get<0>(cells).value();
            return raw_to_celsius(raw);
        });
        
        // Update sensor2_celsius from sensor2_raw
        graph_.update_cell<3>([](const auto& cells) {
            const auto& raw = std::get<1>(cells).value();
            return raw_to_celsius(raw);
        });
        
        // Update average_temperature from both celsius readings
        graph_.update_cell<4>([](const auto& cells) {
            const auto& temp1 = std::get<2>(cells).value();
            const auto& temp2 = std::get<3>(cells).value();
            return (temp1 + temp2) / 2.0f;
        });
        
        // Update high_temp_alert from average_temperature
        graph_.update_cell<5>([](const auto& cells) {
            const auto& avg_temp = std::get<4>(cells).value();
            return is_high_temperature(avg_temp);
        });
    }
};

/**
 * @brief Example of a signal processing system using FRP
 * 
 * This example demonstrates processing discrete signals:
 * - Input signals are filtered and transformed
 * - Multiple signals are merged
 * - Actions are taken in response to signals
 * - All without dynamic memory allocation
 */
class SignalProcessingSystem {
private:
    // Signal handlers (using static storage)
    frp::Sink<int> value_processor_;
    frp::Sink<std::string> alert_handler_;
    
    // Counter for demonstration
    int processed_count_;
    
public:
    /**
     * @brief Constructor
     * 
     * Sets up signal handlers
     */
    constexpr SignalProcessingSystem()
        : value_processor_([this](const int& value) {
            // Process the value (in a real system, this might control hardware)
            processed_count_++;
            std::cout << "Processed value: " << value << " (count: " << processed_count_ << ")" << std::endl;
        })
        , alert_handler_([](const std::string& message) {
            // Handle alert (in a real system, this might trigger an alarm)
            std::cout << "ALERT: " << message << std::endl;
        })
        , processed_count_(0)
    {}
    
    /**
     * @brief Process a new input value
     * 
     * Demonstrates signal filtering, transformation, and handling
     */
    constexpr void process_input(int value) {
        // Create a signal with the input value
        frp::Signal<int> input_signal(value);
        
        // Filter the signal (only process values > 10)
        auto filtered_signal = frp::filter(input_signal, [](int v) { return v > 10; });
        
        // Process the filtered signal
        value_processor_.process(filtered_signal);
        
        // Transform the signal to create an alert if value is very high
        auto alert_signal = input_signal.map([](int v) -> std::string {
            if (v > 100) {
                return "Value exceeded critical threshold: " + std::to_string(v);
            }
            return "";
        });
        
        // Filter empty alerts
        auto filtered_alert = frp::filter(alert_signal, [](const std::string& s) { return !s.empty(); });
        
        // Process the alert
        alert_handler_.process(filtered_alert);
    }
    
    /**
     * @brief Get the count of processed values
     */
    constexpr int get_processed_count() const {
        return processed_count_;
    }
};

/**
 * @brief Example of a motor control system using FRP
 * 
 * This example demonstrates a motor control system where:
 * - Multiple input sources affect motor behavior
 * - Behaviors are combined using FRP principles
 * - Safety limits are enforced
 * - All without dynamic memory allocation
 */
class MotorControlSystem {
private:
    // Input cells
    frp::Cell<float> throttle_position_;
    frp::Cell<float> temperature_;
    frp::Cell<bool> emergency_stop_;
    
    // Output cells
    frp::Cell<float> motor_power_;
    
    // Constants
    static constexpr float MAX_POWER = 100.0f;
    static constexpr float OVERHEAT_THRESHOLD = 80.0f;
    
    // The reactive graph
    using Graph = frp::ReactiveGraph<
        frp::Cell<float>, // throttle_position_
        frp::Cell<float>, // temperature_
        frp::Cell<bool>,  // emergency_stop_
        frp::Cell<float>  // motor_power_
    >;
    
    Graph graph_;
    
    // Calculate motor power based on inputs
    static constexpr float calculate_power(float throttle, float temp, bool e_stop) {
        if (e_stop) {
            return 0.0f; // Emergency stop overrides everything
        }
        
        float power = throttle * MAX_POWER;
        
        // Reduce power if overheating
        if (temp > OVERHEAT_THRESHOLD) {
            float reduction_factor = 1.0f - ((temp - OVERHEAT_THRESHOLD) / 20.0f);
            reduction_factor = std::max(0.0f, reduction_factor);
            power *= reduction_factor;
        }
        
        return std::max(0.0f, std::min(power, MAX_POWER));
    }
    
public:
    /**
     * @brief Constructor
     * 
     * Initializes the FRP graph with initial values
     */
    constexpr MotorControlSystem()
        : throttle_position_(0.0f)
        , temperature_(25.0f)
        , emergency_stop_(false)
        , motor_power_(0.0f)
        , graph_(
            throttle_position_,
            temperature_,
            emergency_stop_,
            motor_power_
          )
    {
        // Initial update to establish the graph relationships
        update_graph();
    }
    
    /**
     * @brief Set the throttle position (0.0 to 1.0)
     */
    constexpr void set_throttle(float position) {
        float clamped = std::max(0.0f, std::min(position, 1.0f));
        graph_.get_cell<0>().set_value(clamped);
        update_graph();
    }
    
    /**
     * @brief Update the temperature reading
     */
    constexpr void update_temperature(float temp) {
        graph_.get_cell<1>().set_value(temp);
        update_graph();
    }
    
    /**
     * @brief Set the emergency stop state
     */
    constexpr void set_emergency_stop(bool active) {
        graph_.get_cell<2>().set_value(active);
        update_graph();
    }
    
    /**
     * @brief Get the current motor power level
     */
    constexpr float get_motor_power() const {
        return graph_.get_cell<3>().value();
    }
    
private:
    /**
     * @brief Update the graph based on current values
     * 
     * This propagates changes through the reactive graph
     */
    constexpr void update_graph() {
        // Update motor_power based on all inputs
        graph_.update_cell<3>([](const auto& cells) {
            const auto& throttle = std::get<0>(cells).value();
            const auto& temp = std::get<1>(cells).value();
            const auto& e_stop = std::get<2>(cells).value();
            return calculate_power(throttle, temp, e_stop);
        });
    }
};

} // namespace example

#endif // EXAMPLE_HPP
