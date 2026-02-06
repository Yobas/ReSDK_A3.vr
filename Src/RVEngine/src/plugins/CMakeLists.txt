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

# To add more plugins, just add more lines:
# add_subdirectory(another_plugin)
# add_subdirectory(third_plugin)