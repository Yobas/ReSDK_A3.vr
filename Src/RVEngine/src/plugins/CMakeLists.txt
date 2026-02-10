# Plugins directory CMakeLists.txt
message(STATUS "Configuring Intercept plugins...")

# Add plugin subdirectories here
add_subdirectory(rv_client)

# Python Host plugin - requires Python3 and pybind11
# Set BUILD_PYTHON_HOST=ON to build this plugin
option(BUILD_PYTHON_HOST "Build Python Host plugin" OFF)
if(BUILD_PYTHON_HOST)
    add_subdirectory(python_host)
endif()

# JavaScript Host plugin - embeds QuickJS-NG engine
# Set BUILD_JS_HOST=ON to build this plugin
option(BUILD_JS_HOST "Build JavaScript Host plugin" OFF)
if(BUILD_JS_HOST)
    add_subdirectory(js_host)
endif()

# V8 JavaScript Host plugin - embeds V8 engine
# Set BUILD_V8_HOST=ON to build this plugin
option(BUILD_V8_HOST "Build V8 JavaScript Host plugin" ON)
if(BUILD_V8_HOST)
    add_subdirectory(v8_host)
endif()