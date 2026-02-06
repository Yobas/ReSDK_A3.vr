cmake_minimum_required(VERSION 3.15)

# Plugin name
set(PLUGIN_NAME "python_host")
set(PLUGIN_TARGET "${PLUGIN_NAME}_x64")

# Create shared library (DLL)
add_library(${PLUGIN_TARGET} SHARED)

# Set output name - will create python_host_x64.dll
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    OUTPUT_NAME "${PLUGIN_NAME}_x64"
    PREFIX ""
)

# Set property for CMakeInclude.txt
set_property(GLOBAL PROPERTY INTERCEPT_CLIENT_TARGET ${PLUGIN_TARGET})

# Include Intercept SDK
include(${CMAKE_SOURCE_DIR}/CMakeInclude.txt)

# ============================================================================
# Python Configuration
# ============================================================================

# Python path - use your installed Python for building
# End users will only need python311.dll (distributed with mod)
set(PYTHON_ROOT "$ENV{LOCALAPPDATA}/Programs/Python/Python311" CACHE PATH "Path to Python installation")

# Set variables BEFORE pybind11 so it finds them
set(Python3_ROOT_DIR "${PYTHON_ROOT}")
set(Python3_EXECUTABLE "${PYTHON_ROOT}/python.exe")
set(PYTHON_EXECUTABLE "${PYTHON_ROOT}/python.exe")
set(Python3_INCLUDE_DIRS "${PYTHON_ROOT}/include")
set(Python3_LIBRARIES "${PYTHON_ROOT}/libs/python311.lib")
set(PYTHON_INCLUDE_DIRS "${PYTHON_ROOT}/include")
set(PYTHON_LIBRARIES "${PYTHON_ROOT}/libs/python311.lib")
set(PYTHONLIBS_FOUND TRUE)

message(STATUS "Using Python from: ${PYTHON_ROOT}")

# pybind11 - download via FetchContent (header-only, no Python search needed)
include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
)
# Use NOPYTHON to skip pybind11's Python search - we set it manually
set(PYBIND11_NOPYTHON ON)
FetchContent_MakeAvailable(pybind11)

# Plugin sources
target_sources(${PLUGIN_TARGET} PRIVATE
    main.cpp
    python_engine.hpp
    python_engine.cpp
    type_conversion.hpp
    type_conversion.cpp
    sqf_bindings.hpp
    sqf_bindings.cpp
)

# Link Python manually (not through pybind11)
target_include_directories(${PLUGIN_TARGET} PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(${PLUGIN_TARGET} PRIVATE ${Python3_LIBRARIES})

# pybind11 headers only
target_include_directories(${PLUGIN_TARGET} PRIVATE ${pybind11_SOURCE_DIR}/include)

# Compile definitions
target_compile_definitions(${PLUGIN_TARGET} PRIVATE
    PYTHON_HOST_EMBEDDED
)

# Set output directory to build/plugins/
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/plugins"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/build/Debug/plugins"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/build/Release/plugins"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/build/MinSizeRel/plugins"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/build/RelWithDebInfo/plugins"
)

# Set folder for IDE organization
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    FOLDER "Plugins"
)

message(STATUS "Configuring plugin: ${PLUGIN_TARGET} -> ${PLUGIN_NAME}_x64.dll")
message(STATUS "Python include: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python libs: ${Python3_LIBRARIES}")
