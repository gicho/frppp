/**
 * @file main.cpp
 * @brief Demonstration of the FRP library for embedded real-time systems
 * 
 * This file shows how to use the FRP library in practice with the example
 * classes defined in example.hpp.
 */

#include "frp.hpp"
#include "example.hpp"
#include <iostream>
#include <iomanip>

// Function to print a section header
void print_section(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(50, '=') << "\n";
}

// Function to print a subsection header
void print_subsection(const std::string& title) {
    std::cout << "\n" << std::string(40, '-') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(40, '-') << "\n";
}

int main() {
    std::cout << "FRP Library Demonstration for Embedded Systems\n";
    
    // Demonstrate the temperature sensor system
    print_section("Temperature Sensor System");
    {
        example::TemperatureSensorSystem temp_system;
        
        // Initial state
        print_subsection("Initial State");
        std::cout << "Sensor 1 Temperature: " << temp_system.get_sensor1_temperature() << " °C\n";
        std::cout << "Sensor 2 Temperature: " << temp_system.get_sensor2_temperature() << " °C\n";
        std::cout << "Average Temperature: " << temp_system.get_average_temperature() << " °C\n";
        std::cout << "Alert Active: " << (temp_system.is_alert_active() ? "YES" : "NO") << "\n";
        
        // Update sensor 1
        print_subsection("After Updating Sensor 1");
        temp_system.update_sensor1(450.0f); // Raw value that converts to 25°C
        std::cout << "Sensor 1 Temperature: " << temp_system.get_sensor1_temperature() << " °C\n";
        std::cout << "Sensor 2 Temperature: " << temp_system.get_sensor2_temperature() << " °C\n";
        std::cout << "Average Temperature: " << temp_system.get_average_temperature() << " °C\n";
        std::cout << "Alert Active: " << (temp_system.is_alert_active() ? "YES" : "NO") << "\n";
        
        // Update sensor 2
        print_subsection("After Updating Sensor 2");
        temp_system.update_sensor2(550.0f); // Raw value that converts to 35°C
        std::cout << "Sensor 1 Temperature: " << temp_system.get_sensor1_temperature() << " °C\n";
        std::cout << "Sensor 2 Temperature: " << temp_system.get_sensor2_temperature() << " °C\n";
        std::cout << "Average Temperature: " << temp_system.get_average_temperature() << " °C\n";
        std::cout << "Alert Active: " << (temp_system.is_alert_active() ? "YES" : "NO") << "\n";
        
        // Trigger an alert
        print_subsection("Triggering High Temperature Alert");
        temp_system.update_sensor1(800.0f); // Raw value that converts to 60°C
        temp_system.update_sensor2(750.0f); // Raw value that converts to 55°C
        std::cout << "Sensor 1 Temperature: " << temp_system.get_sensor1_temperature() << " °C\n";
        std::cout << "Sensor 2 Temperature: " << temp_system.get_sensor2_temperature() << " °C\n";
        std::cout << "Average Temperature: " << temp_system.get_average_temperature() << " °C\n";
        std::cout << "Alert Active: " << (temp_system.is_alert_active() ? "YES" : "NO") << "\n";
    }
    
    // Demonstrate the signal processing system
    print_section("Signal Processing System");
    {
        example::SignalProcessingSystem signal_system;
        
        print_subsection("Processing Various Signals");
        
        // Process a value below threshold (should not be processed)
        std::cout << "Processing value 5 (below threshold):\n";
        signal_system.process_input(5);
        std::cout << "Processed count: " << signal_system.get_processed_count() << "\n\n";
        
        // Process a value above threshold
        std::cout << "Processing value 15 (above threshold):\n";
        signal_system.process_input(15);
        std::cout << "Processed count: " << signal_system.get_processed_count() << "\n\n";
        
        // Process a value that triggers an alert
        std::cout << "Processing value 150 (critical threshold):\n";
        signal_system.process_input(150);
        std::cout << "Processed count: " << signal_system.get_processed_count() << "\n";
    }
    
    // Demonstrate the motor control system
    print_section("Motor Control System");
    {
        example::MotorControlSystem motor_system;
        
        // Initial state
        print_subsection("Initial State");
        std::cout << "Motor Power: " << motor_system.get_motor_power() << "%\n";
        
        // Set throttle
        print_subsection("After Setting Throttle to 50%");
        motor_system.set_throttle(0.5f);
        std::cout << "Motor Power: " << motor_system.get_motor_power() << "%\n";
        
        // Increase temperature
        print_subsection("After Temperature Increase");
        motor_system.update_temperature(85.0f);
        std::cout << "Motor Power: " << motor_system.get_motor_power() << "%\n";
        
        // Emergency stop
        print_subsection("After Emergency Stop");
        motor_system.set_emergency_stop(true);
        std::cout << "Motor Power: " << motor_system.get_motor_power() << "%\n";
        
        // Release emergency stop
        print_subsection("After Releasing Emergency Stop");
        motor_system.set_emergency_stop(false);
        std::cout << "Motor Power: " << motor_system.get_motor_power() << "%\n";
    }
    
    // Demonstrate basic FRP concepts directly
    print_section("Basic FRP Concepts");
    {
        print_subsection("Cells and Behaviors");
        
        // Create cells
        frp::Cell<int> counter(0);
        frp::Cell<std::string> message("Hello, FRP!");
        
        // Create behaviors from cells
        auto counter_behavior = frp::behavior_from_cell(counter);
        auto message_behavior = frp::behavior_from_cell(message);
        
        // Create a derived behavior using lift
        auto combined_behavior = frp::lift(
            [](int count, const std::string& msg) {
                return msg + " Count: " + std::to_string(count);
            },
            counter_behavior,
            message_behavior
        );
        
        // Sample the behaviors
        std::cout << "Counter: " << counter_behavior.sample() << "\n";
        std::cout << "Message: " << message_behavior.sample() << "\n";
        std::cout << "Combined: " << combined_behavior.sample() << "\n\n";
        
        // Update a cell and sample again
        counter.set_value(42);
        std::cout << "After updating counter to 42:\n";
        std::cout << "Counter: " << counter_behavior.sample() << "\n";
        std::cout << "Combined: " << combined_behavior.sample() << "\n";
    }
    
    {
        print_subsection("Signals and Sinks");
        
        // Create a signal
        frp::Signal<double> temperature_signal(25.5);
        
        // Create a sink
        frp::Sink<double> temperature_sink([](const double& temp) {
            std::cout << "Temperature: " << temp << " °C\n";
        });
        
        // Process the signal
        std::cout << "Processing temperature signal:\n";
        temperature_sink.process(temperature_signal);
        
        // Create a transformed signal
        auto fahrenheit_signal = temperature_signal.map([](double celsius) {
            return (celsius * 9.0 / 5.0) + 32.0;
        });
        
        // Create a sink for Fahrenheit
        frp::Sink<double> fahrenheit_sink([](const double& temp) {
            std::cout << "Temperature: " << temp << " °F\n";
        });
        
        // Process the transformed signal
        std::cout << "Processing Fahrenheit signal:\n";
        fahrenheit_sink.process(fahrenheit_signal);
    }
    
    return 0;
}
