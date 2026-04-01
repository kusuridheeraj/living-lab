cmake_minimum_required(VERSION 3.15)
project(living_limiter_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pybind11
find_package(pybind11 CONFIG REQUIRED)

# Define the Python extension module
pybind11_add_module(living_limiter_core src/bindings.cpp)

# Target properties
target_include_directories(living_limiter_core PRIVATE src)

# Link statically on Windows to avoid DLL load issues
if(WIN32)
    target_link_options(living_limiter_core PRIVATE -static-libgcc -static-libstdc++)
endif()

# Installation (used by scikit-build-core)
install(TARGETS living_limiter_core DESTINATION living_limiter)
