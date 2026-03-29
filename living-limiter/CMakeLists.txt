cmake_minimum_required(VERSION 3.10)
project(living_limiter_core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pybind11
find_package(pybind11 REQUIRED)

# Define the Python extension module
pybind11_add_module(living_limiter_core src/bindings.cpp)

# Target properties
target_include_directories(living_limiter_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
