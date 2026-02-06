#pragma once
#include "intercept.hpp"
#include "type_conversion.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace intercept;
using namespace intercept::types;

namespace python_host {

// ============================================================================
// SQF Function Bindings for Python
// ============================================================================

// Initialize all SQF bindings in the given Python module
void register_sqf_bindings(py::module_& m);

// ============================================================================
// Position functions
// ============================================================================

// getPos(object) -> [x, y, z]
py::tuple py_get_pos(const game_value& obj);

// getPosASL(object) -> [x, y, z]
py::tuple py_get_pos_asl(const game_value& obj);

// getPosATL(object) -> [x, y, z]
py::tuple py_get_pos_atl(const game_value& obj);

// setPos(object, [x, y, z])
void py_set_pos(const game_value& obj, py::object pos);

// setPosASL(object, [x, y, z])
void py_set_pos_asl(const game_value& obj, py::object pos);

// setPosATL(object, [x, y, z])
void py_set_pos_atl(const game_value& obj, py::object pos);

// ============================================================================
// Object functions
// ============================================================================

// player() -> object
game_value py_player();

// vehicle(unit) -> object
game_value py_vehicle(const game_value& unit);

// ============================================================================
// Utility functions
// ============================================================================

// hint(text)
void py_hint(const std::string& text);

// systemChat(text)
void py_system_chat(const std::string& text);

// diag_log(value)
void py_diag_log(py::object value);

// ============================================================================
// Generic SQF call interface
// ============================================================================

// Call any nular command: sqf.call("player") -> game_value
py::object py_call_nular(const std::string& cmd);

// Call any unary command: sqf.call("getPos", obj) -> game_value
py::object py_call_unary(const std::string& cmd, py::object arg);

// Call any binary command: sqf.call("setPos", obj, pos) -> game_value
py::object py_call_binary(const std::string& cmd, py::object left, py::object right);

} // namespace python_host
