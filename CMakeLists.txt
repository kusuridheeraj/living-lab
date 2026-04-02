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

# Robust Linking for Windows
if(WIN32)
    if(MSVC)
        # MSVC specific: Use static runtime to avoid DLL hell in CI
        set_property(TARGET living_limiter_core PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        # MinGW/GCC specific: Static link the GCC runtimes
        target_link_options(living_limiter_core PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

# Installation (used by scikit-build-core)
install(TARGETS living_limiter_core DESTINATION living_limiter)
