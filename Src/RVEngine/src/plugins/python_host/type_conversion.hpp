#pragma once
#include "intercept.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>
#include <variant>
#include <optional>

namespace py = pybind11;
using namespace intercept::types;

namespace python_host {

// ============================================================================
// Type Conversion: game_value <-> Python
// ============================================================================

// Convert game_value to Python object
py::object game_value_to_python(const game_value& gv);

// Convert Python object to game_value
game_value python_to_game_value(const py::object& obj);

// Convert vector3 to Python tuple
py::tuple vector3_to_python(const vector3& v);

// Convert Python sequence to vector3
vector3 python_to_vector3(const py::object& obj);

// Convert object handle to Python
py::object object_to_python(const object& obj);

// Convert Python to object handle
object python_to_object(const py::object& obj);

// ============================================================================
// Helper functions for array conversion
// ============================================================================

// Convert game_value array to Python list
py::list game_array_to_python(const game_value& gv);

// Convert Python list/tuple to game_value array
game_value python_sequence_to_game_array(const py::object& obj);

// ============================================================================
// Type checking utilities
// ============================================================================

bool is_game_value_scalar(const game_value& gv);
bool is_game_value_string(const game_value& gv);
bool is_game_value_array(const game_value& gv);
bool is_game_value_bool(const game_value& gv);
bool is_game_value_object(const game_value& gv);
bool is_game_value_nil(const game_value& gv);

} // namespace python_host
