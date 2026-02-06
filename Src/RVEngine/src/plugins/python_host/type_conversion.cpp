#include "type_conversion.hpp"
#include <stdexcept>
#include <sstream>

namespace python_host {

// ============================================================================
// Type checking utilities
// ============================================================================

bool is_game_value_scalar(const game_value& gv) {
    if (gv.is_nil()) return false;
    auto type_str = gv.type_enum();
    return type_str == game_data_type::SCALAR;
}

bool is_game_value_string(const game_value& gv) {
    if (gv.is_nil()) return false;
    return gv.type_enum() == game_data_type::STRING;
}

bool is_game_value_array(const game_value& gv) {
    if (gv.is_nil()) return false;
    return gv.type_enum() == game_data_type::ARRAY;
}

bool is_game_value_bool(const game_value& gv) {
    if (gv.is_nil()) return false;
    return gv.type_enum() == game_data_type::BOOL;
}

bool is_game_value_object(const game_value& gv) {
    if (gv.is_nil()) return false;
    return gv.type_enum() == game_data_type::OBJECT;
}

bool is_game_value_nil(const game_value& gv) {
    return gv.is_nil();
}

// ============================================================================
// vector3 conversions
// ============================================================================

py::tuple vector3_to_python(const vector3& v) {
    return py::make_tuple(v.x, v.y, v.z);
}

vector3 python_to_vector3(const py::object& obj) {
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        py::sequence seq = obj.cast<py::sequence>();
        if (seq.size() >= 3) {
            return vector3(
                seq[0].cast<float>(),
                seq[1].cast<float>(),
                seq[2].cast<float>()
            );
        } else if (seq.size() == 2) {
            // 2D position, Z = 0
            return vector3(
                seq[0].cast<float>(),
                seq[1].cast<float>(),
                0.0f
            );
        }
    }
    throw std::runtime_error("Cannot convert Python object to vector3: expected tuple/list with 2-3 elements");
}

// ============================================================================
// game_value -> Python
// ============================================================================

py::object game_value_to_python(const game_value& gv) {
    if (gv.is_nil()) {
        return py::none();
    }

    auto type = gv.type_enum();

    switch (type) {
        case game_data_type::SCALAR:
            return py::cast(static_cast<float>(gv));

        case game_data_type::BOOL:
            return py::cast(static_cast<bool>(gv));

        case game_data_type::STRING:
            return py::cast(static_cast<std::string>(gv));

        case game_data_type::ARRAY: {
            py::list result;
            auto& arr = gv.to_array();
            for (size_t i = 0; i < arr.size(); ++i) {
                result.append(game_value_to_python(arr[i]));
            }
            return result;
        }

        case game_data_type::OBJECT: {
            // Return object as a special wrapper (handle)
            // We'll wrap it in a Python object that holds the game_value
            return py::cast(gv, py::return_value_policy::copy);
        }

        case game_data_type::NOTHING:
            return py::none();

        default:
            // For unsupported types, return as raw game_value
            return py::cast(gv, py::return_value_policy::copy);
    }
}

// ============================================================================
// Python -> game_value
// ============================================================================

game_value python_to_game_value(const py::object& obj) {
    // None -> nil
    if (obj.is_none()) {
        return game_value();
    }

    // Bool (must check before int, since bool is subclass of int in Python)
    if (py::isinstance<py::bool_>(obj)) {
        return game_value(obj.cast<bool>());
    }

    // Int -> scalar
    if (py::isinstance<py::int_>(obj)) {
        return game_value(static_cast<float>(obj.cast<int>()));
    }

    // Float -> scalar
    if (py::isinstance<py::float_>(obj)) {
        return game_value(obj.cast<float>());
    }

    // String
    if (py::isinstance<py::str>(obj)) {
        return game_value(obj.cast<std::string>());
    }

    // List/Tuple -> array
    if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
        return python_sequence_to_game_array(obj);
    }

    // Already a game_value (e.g., object wrapper)
    if (py::isinstance<game_value>(obj)) {
        return obj.cast<game_value>();
    }

    throw std::runtime_error("Cannot convert Python object to game_value: unsupported type");
}

// ============================================================================
// Array conversions
// ============================================================================

py::list game_array_to_python(const game_value& gv) {
    py::list result;
    if (!is_game_value_array(gv)) {
        return result;
    }

    auto& arr = gv.to_array();
    for (size_t i = 0; i < arr.size(); ++i) {
        result.append(game_value_to_python(arr[i]));
    }
    return result;
}

game_value python_sequence_to_game_array(const py::object& obj) {
    if (!py::isinstance<py::sequence>(obj)) {
        throw std::runtime_error("Expected Python sequence (list/tuple)");
    }

    py::sequence seq = obj.cast<py::sequence>();
    std::vector<game_value> arr;
    arr.reserve(seq.size());

    for (size_t i = 0; i < seq.size(); ++i) {
        arr.push_back(python_to_game_value(seq[i]));
    }

    return game_value(arr);
}

// ============================================================================
// Object conversions
// ============================================================================

py::object object_to_python(const object& obj) {
    // Wrap the object as game_value for Python
    return py::cast(game_value(obj), py::return_value_policy::copy);
}

object python_to_object(const py::object& obj) {
    if (py::isinstance<game_value>(obj)) {
        game_value gv = obj.cast<game_value>();
        if (is_game_value_object(gv)) {
            return static_cast<object>(gv);
        }
    }
    throw std::runtime_error("Cannot convert Python object to Arma object");
}

} // namespace python_host
