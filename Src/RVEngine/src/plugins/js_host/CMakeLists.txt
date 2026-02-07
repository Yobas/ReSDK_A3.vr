cmake_minimum_required(VERSION 3.15)

# Plugin name
set(PLUGIN_NAME "js_host")
set(PLUGIN_TARGET "${PLUGIN_NAME}_x64")

# Create shared library (DLL)
add_library(${PLUGIN_TARGET} SHARED)

# Set output name - will create js_host_x64.dll
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    OUTPUT_NAME "${PLUGIN_NAME}_x64"
    PREFIX ""
)

# Set property for CMakeInclude.txt
set_property(GLOBAL PROPERTY INTERCEPT_CLIENT_TARGET ${PLUGIN_TARGET})

# Include Intercept SDK
include(${CMAKE_SOURCE_DIR}/CMakeInclude.txt)

# ============================================================================
# QuickJS-NG Configuration (local copy)
# ============================================================================

# Only build the qjs library target, skip executables and tests
add_subdirectory(quickjs-master EXCLUDE_FROM_ALL)

# Plugin sources
target_sources(${PLUGIN_TARGET} PRIVATE
    main.cpp
    js_engine.hpp
    js_engine.cpp
    type_conversion.hpp
    type_conversion.cpp
    sqf_bindings.hpp
    sqf_bindings.cpp
)

# Include directories (quickjs headers are exposed via qjs target's public includes)
target_link_libraries(${PLUGIN_TARGET} PRIVATE qjs)

# Compile definitions
target_compile_definitions(${PLUGIN_TARGET} PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# MSVC-specific: disable some warnings
if(MSVC)
    target_compile_options(${PLUGIN_TARGET} PRIVATE /W3 /wd4244 /wd4267)
endif()

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

message(STATUS "Configuring plugin: ${PLUGIN_TARGET} -> ${PLUGIN_NAME}_x64.dll (QuickJS-NG engine)")
