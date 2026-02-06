#pragma once
#include "intercept.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <string>
#include <memory>
#include <filesystem>

namespace py = pybind11;

namespace python_host {

// ============================================================================
// Python Engine - Manages embedded Python interpreter
// ============================================================================

class PythonEngine {
public:
    static PythonEngine& instance();

    // Initialize Python interpreter
    bool initialize(const std::filesystem::path& scripts_path);

    // Shutdown Python interpreter
    void shutdown();

    // Check if initialized
    bool is_initialized() const { return m_initialized; }

    // Execute Python file
    bool exec_file(const std::filesystem::path& file_path);

    // Execute Python code string
    bool exec_string(const std::string& code);

    // Call Python function by name (in __main__ module)
    py::object call_function(const std::string& func_name);

    // Call with arguments
    template<typename... Args>
    py::object call_function(const std::string& func_name, Args&&... args) {
        try {
            py::module_ main = py::module_::import("__main__");
            if (py::hasattr(main, func_name.c_str())) {
                return main.attr(func_name.c_str())(std::forward<Args>(args)...);
            }
        } catch (const py::error_already_set& e) {
            log_error("Python error calling " + func_name + ": " + std::string(e.what()));
        }
        return py::none();
    }

    // Call lifecycle hooks (if defined in Python)
    void call_on_frame();
    void call_pre_init();
    void call_post_init();
    void call_mission_ended();

    // Get scripts directory
    const std::filesystem::path& get_scripts_path() const { return m_scripts_path; }

    // Log to Arma RPT
    static void log_info(const std::string& msg);
    static void log_error(const std::string& msg);

private:
    PythonEngine() = default;
    ~PythonEngine();

    PythonEngine(const PythonEngine&) = delete;
    PythonEngine& operator=(const PythonEngine&) = delete;

    bool m_initialized = false;
    std::filesystem::path m_scripts_path;
    std::unique_ptr<py::scoped_interpreter> m_interpreter;
};

} // namespace python_host
