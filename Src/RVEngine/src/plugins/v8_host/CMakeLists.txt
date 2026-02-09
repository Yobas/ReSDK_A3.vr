cmake_minimum_required(VERSION 3.15)

# Plugin name
set(PLUGIN_NAME "v8_host")
set(PLUGIN_TARGET "${PLUGIN_NAME}_x64")

# V8 paths (from extracted NuGet packages in Third-party/v8/)
set(V8_ROOT "${CMAKE_SOURCE_DIR}/../../Third-party/v8")
set(V8_INCLUDE_DIR "${V8_ROOT}/dev/include")
set(V8_LIB_DIR_RELEASE "${V8_ROOT}/dev/lib/Release")
set(V8_LIB_DIR_DEBUG "${V8_ROOT}/dev/lib/Debug")
set(V8_REDIST_DIR_RELEASE "${V8_ROOT}/redist/lib/Release")
set(V8_REDIST_DIR_DEBUG "${V8_ROOT}/redist/lib/Debug")

# Verify V8 headers exist
if(NOT EXISTS "${V8_INCLUDE_DIR}/v8.h")
    message(FATAL_ERROR "V8 headers not found at ${V8_INCLUDE_DIR}. "
        "Please extract pmed/v8-nuget packages into Third-party/v8/")
endif()

# Create shared library (DLL)
add_library(${PLUGIN_TARGET} SHARED)

# Set output name
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    OUTPUT_NAME "${PLUGIN_NAME}_x64"
    PREFIX ""
)

# Set property for CMakeInclude.txt (Intercept SDK)
set_property(GLOBAL PROPERTY INTERCEPT_CLIENT_TARGET ${PLUGIN_TARGET})

# Include Intercept SDK
include(${CMAKE_SOURCE_DIR}/CMakeInclude.txt)

# Plugin sources
target_sources(${PLUGIN_TARGET} PRIVATE
    main.cpp
    v8_engine.hpp
    v8_engine.cpp
    v8_inspector_client.hpp
    v8_inspector_client.cpp
)

# Include directories
target_include_directories(${PLUGIN_TARGET} PRIVATE
    ${V8_INCLUDE_DIR}
)

# Link V8 libraries (config-specific via link_directories + lib names)
set(V8_LIBS
    v8.dll.lib
    v8_libbase.dll.lib
    v8_libplatform.dll.lib
)

# Add per-config library search paths
target_link_directories(${PLUGIN_TARGET} PRIVATE
    $<$<CONFIG:Debug>:${V8_LIB_DIR_DEBUG}>
    $<$<NOT:$<CONFIG:Debug>>:${V8_LIB_DIR_RELEASE}>
)

target_link_libraries(${PLUGIN_TARGET} PRIVATE
    ${V8_LIBS}
    ws2_32
    winmm
)

# Compile definitions
target_compile_definitions(${PLUGIN_TARGET} PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    V8_COMPRESS_POINTERS
    V8_31BIT_SMIS_ON_64BIT_ARCH
    V8_ENABLE_SANDBOX
)

# V8 v13 requires C++20
set_target_properties(${PLUGIN_TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# MSVC-specific settings
if(MSVC)
    target_compile_options(${PLUGIN_TARGET} PRIVATE /W3 /wd4244 /wd4267 /Zc:__cplusplus)
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

# Copy V8 runtime DLLs to output directory after build
set(V8_REDIST_DIR "$<IF:$<CONFIG:Debug>,${V8_REDIST_DIR_DEBUG},${V8_REDIST_DIR_RELEASE}>")
set(V8_RUNTIME_FILES
    v8.dll
    v8_libbase.dll
    v8_libplatform.dll
    icuuc.dll
    icudtl.dat
    third_party_abseil-cpp_absl.dll
    third_party_icu_icui18n.dll
    third_party_zlib.dll
)

foreach(V8_FILE ${V8_RUNTIME_FILES})
    add_custom_command(TARGET ${PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${V8_REDIST_DIR}/${V8_FILE}"
            "$<TARGET_FILE_DIR:${PLUGIN_TARGET}>/../${V8_FILE}"
        COMMENT "Copying ${V8_FILE}"
    )
endforeach()

message(STATUS "Configuring plugin: ${PLUGIN_TARGET} -> ${PLUGIN_NAME}_x64.dll (V8 engine)")
