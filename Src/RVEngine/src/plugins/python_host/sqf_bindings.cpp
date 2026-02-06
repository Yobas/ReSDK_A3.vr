#include "sqf_bindings.hpp"
#include <iostream>

namespace python_host {

// ============================================================================
// Position functions implementation
// ============================================================================

py::tuple py_get_pos(const game_value& obj) {
    intercept::client::host::functions.invoker_lock();
    vector3 pos = intercept::sqf::get_pos(static_cast<object>(obj));
    intercept::client::host::functions.invoker_unlock();
    return vector3_to_python(pos);
}

py::tuple py_get_pos_asl(const game_value& obj) {
    intercept::client::host::functions.invoker_lock();
    vector3 pos = intercept::sqf::get_pos_asl(static_cast<object>(obj));
    intercept::client::host::functions.invoker_unlock();
    return vector3_to_python(pos);
}

py::tuple py_get_pos_atl(const game_value& obj) {
    intercept::client::host::functions.invoker_lock();
    vector3 pos = intercept::sqf::get_pos_atl(static_cast<object>(obj));
    intercept::client::host::functions.invoker_unlock();
    return vector3_to_python(pos);
}

void py_set_pos(const game_value& obj, py::object pos) {
    vector3 v = python_to_vector3(pos);
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::set_pos(static_cast<object>(obj), v);
    intercept::client::host::functions.invoker_unlock();
}

void py_set_pos_asl(const game_value& obj, py::object pos) {
    vector3 v = python_to_vector3(pos);
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::set_pos_asl(static_cast<object>(obj), v);
    intercept::client::host::functions.invoker_unlock();
}

void py_set_pos_atl(const game_value& obj, py::object pos) {
    vector3 v = python_to_vector3(pos);
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::set_pos_atl(static_cast<object>(obj), v);
    intercept::client::host::functions.invoker_unlock();
}

// ============================================================================
// Object functions implementation
// ============================================================================

game_value py_player() {
    intercept::client::invoker_lock thread_lock;
    intercept::client::host::functions.invoker_lock();
    object p = intercept::sqf::player();
    intercept::client::host::functions.invoker_unlock();
    return game_value(p);
}

game_value py_vehicle(const game_value& unit) {
    intercept::client::host::functions.invoker_lock();
    object v = intercept::sqf::vehicle(static_cast<object>(unit));
    intercept::client::host::functions.invoker_unlock();
    return game_value(v);
}

// ============================================================================
// Utility functions implementation
// ============================================================================

void py_hint(const std::string& text) {
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::hint(text);
    intercept::client::host::functions.invoker_unlock();
}

void py_system_chat(const std::string& text) {
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::system_chat(text);
    intercept::client::host::functions.invoker_unlock();
}

void py_diag_log(py::object value) {
    game_value gv = python_to_game_value(value);
    intercept::client::host::functions.invoker_lock();
    intercept::sqf::diag_log(gv);
    intercept::client::host::functions.invoker_unlock();
}

// ============================================================================
// Generic SQF call interface
// ============================================================================

py::object py_call_nular(const std::string& cmd) {
    // Get function pointer
    auto func = intercept::client::host::functions.get_nular_function(cmd);
    if (!func) {
        throw std::runtime_error("Unknown nular command: " + cmd);
    }

    intercept::client::host::functions.invoker_lock();
    game_value result = intercept::client::host::functions.invoke_raw_nular(func);
    intercept::client::host::functions.invoker_unlock();

    return game_value_to_python(result);
}

py::object py_call_unary(const std::string& cmd, py::object arg) {
    // Get function pointer (we use generic types)
    auto func = intercept::client::host::functions.get_unary_function(cmd);
    if (!func) {
        throw std::runtime_error("Unknown unary command: " + cmd);
    }

    game_value gv_arg = python_to_game_value(arg);

    intercept::client::host::functions.invoker_lock();
    game_value result = intercept::client::host::functions.invoke_raw_unary(func, gv_arg);
    intercept::client::host::functions.invoker_unlock();

    return game_value_to_python(result);
}

py::object py_call_binary(const std::string& cmd, py::object left, py::object right) {
    // Get function pointer
    auto func = intercept::client::host::functions.get_binary_function(cmd);
    if (!func) {
        throw std::runtime_error("Unknown binary command: " + cmd);
    }

    game_value gv_left = python_to_game_value(left);
    game_value gv_right = python_to_game_value(right);

    intercept::client::host::functions.invoker_lock();
    game_value result = intercept::client::host::functions.invoke_raw_binary(func, gv_left, gv_right);
    intercept::client::host::functions.invoker_unlock();

    return game_value_to_python(result);
}

// ============================================================================
// Register all bindings to Python module
// ============================================================================

void register_sqf_bindings(py::module_& m) {
    // Create 'sqf' submodule
    py::module_ operators = m.def_submodule("operators", "Native function bindings");

    // =========== game_value type ===========
    py::class_<game_value>(m, "GameValue")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<bool>())
        .def(py::init<const std::string&>())
        .def("is_nil", &game_value::is_nil)
        .def("__repr__", [](const game_value& gv) {
            if (gv.is_nil()) return std::string("GameValue(nil)");
            // Use operator std::string() for conversion
            try {
                return std::string("GameValue(") + static_cast<std::string>(gv) + ")";
            } catch (...) {
                return std::string("GameValue(<unconvertible>)");
            }
        })
        .def("__bool__", [](const game_value& gv) {
            if (gv.is_nil()) return false;
            try {
                return static_cast<bool>(gv);
            } catch (...) {
                return true; // non-nil is truthy
            }
        });

    // =========== Position functions ===========
    operators.def("get_pos", &py_get_pos, py::arg("obj"),
        "Get position of object in format [x, y, z]");

    operators.def("get_pos_asl", &py_get_pos_asl, py::arg("obj"),
        "Get position Above Sea Level of object");

    operators.def("get_pos_atl", &py_get_pos_atl, py::arg("obj"),
        "Get position Above Terrain Level of object");

    operators.def("set_pos", &py_set_pos, py::arg("obj"), py::arg("pos"),
        "Set position of object. pos can be [x, y, z] or (x, y, z)");

    operators.def("set_pos_asl", &py_set_pos_asl, py::arg("obj"), py::arg("pos"),
        "Set position Above Sea Level of object");

    operators.def("set_pos_atl", &py_set_pos_atl, py::arg("obj"), py::arg("pos"),
        "Set position Above Terrain Level of object");

    // =========== Object functions ===========
    operators.def("player", &py_player,
        "Get player object");

    operators.def("vehicle", &py_vehicle, py::arg("unit"),
        "Get vehicle of unit (returns unit if not in vehicle)");

    // =========== Utility functions ===========
    operators.def("hint", &py_hint, py::arg("text"),
        "Display hint message");

    operators.def("system_chat", &py_system_chat, py::arg("text"),
        "Display system chat message");

    operators.def("diag_log", &py_diag_log, py::arg("value"),
        "Write value to RPT log file");

    // =========== Generic call interface ===========
    operators.def("call", py::overload_cast<const std::string&>(&py_call_nular),
        py::arg("cmd"),
        "Call nular SQF command (no arguments)");

    operators.def("call", py::overload_cast<const std::string&, py::object>(&py_call_unary),
        py::arg("cmd"), py::arg("arg"),
        "Call unary SQF command (one argument)");

    operators.def("call", py::overload_cast<const std::string&, py::object, py::object>(&py_call_binary),
        py::arg("cmd"), py::arg("left"), py::arg("right"),
        "Call binary SQF command (two arguments)");

    // =========== Convenience aliases at module level ===========
    //operators.def("player", &py_player, "Get player object");
    //operators.def("hint", &py_hint, py::arg("text"), "Display hint message");
}

} // namespace python_host
