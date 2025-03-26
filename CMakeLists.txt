cmake_minimum_required(VERSION 3.14)
project(frp_embedded LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    add_compile_options(/W4 /WX)
endif()

# Add executable
add_executable(frp_demo main.cpp)

# Header-only library, so we just need to include the directory
target_include_directories(frp_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Print status message
message(STATUS "FRP Embedded Library configured with C++20 support")
message(STATUS "Build the 'frp_demo' target to run the examples")
