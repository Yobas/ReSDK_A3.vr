#include "python_engine.hpp"
#include "sqf_bindings.hpp"
#include "intercept.hpp"
#include <Windows.h>
#include <Python.h>
#include <iostream>
#include <fstream>
#include <sstream>

// ============================================================================
// Python embedded module definition (arma module)
// This is the module that gets imported in Python as 'arma'
// Must be defined OUTSIDE namespace and only ONCE
// ============================================================================
PYBIND11_EMBEDDED_MODULE(rvengine, rvengineModule) {
    rvengineModule.doc() = "Arma 3 Python bindings via RVEngine/Intercept";

    // Register SQF bindings
    python_host::register_sqf_bindings(rvengineModule);

    // Version info
    rvengineModule.attr("__version__") = "1.0.0";
    rvengineModule.attr("__intercept_api_version__") = INTERCEPT_SDK_API_VERSION;
}

namespace python_host {

// Helper anchor to locate the current DLL module
static void python_host_module_anchor() {}

PythonEngine& PythonEngine::instance() {
    static PythonEngine engine;
    return engine;
}

PythonEngine::~PythonEngine() {
    shutdown();
}

bool PythonEngine::initialize(const std::filesystem::path& scripts_path) {
    if (m_initialized) {
        log_info("Python already initialized");
        return true;
    }

    try {
        // Store scripts path
        m_scripts_path = scripts_path;

        // Ensure embedded Python DLLs can be found without overriding default search paths
        {
            std::filesystem::path base_dir;
            HMODULE hModule = nullptr;
            GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&python_host_module_anchor),
                &hModule
            );
            if (hModule) {
                wchar_t path[MAX_PATH];
                GetModuleFileNameW(hModule, path, MAX_PATH);
                base_dir = std::filesystem::path(path).parent_path();
            } else {
                base_dir = std::filesystem::current_path();
            }

            auto python_src = base_dir / "python_src";
            auto python_dlls = python_src / "DLLs";

            SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
            AddDllDirectory(python_src.wstring().c_str());
            AddDllDirectory(python_dlls.wstring().c_str());

            // Force embedded Python to use python_src paths (avoid system Python)
            std::wstring python_home = python_src.wstring();
            std::wstring python_zip = python_home + L"\\python311.zip";
            std::wstring python_lib = python_home + L"\\Lib";
            std::wstring python_site = python_home + L"\\Lib\\site-packages";
            std::wstring python_scripts = python_home + L"\\scripts";
            std::wstring python_dllsNew = python_home + L"\\DLLs";
            std::wstring python_path = python_zip + L";" + python_lib + L";" + python_site + L";" + python_scripts + L";" + python_dllsNew;

            Py_SetPythonHome(python_home.c_str());
            Py_SetPath(python_path.c_str());
            
        }

        // Initialize Python interpreter
        m_interpreter = std::make_unique<py::scoped_interpreter>();
        
        // Add scripts path to Python's sys.path
        py::module_ sys = py::module_::import("sys");
        py::list path = sys.attr("path").cast<py::list>();
        path.insert(0, m_scripts_path.string());

        // Import our arma module to make it available
        py::module_::import("rvengine");

        m_initialized = true;
        log_info("Python interpreter initialized. Scripts path: " + m_scripts_path.string());

        // Try to load main.py if it exists
        auto main_script = m_scripts_path / "main.py";
        if (std::filesystem::exists(main_script)) {
            log_info("Loading main.py...");
            exec_file(main_script);
        }

        return true;

    } catch (const py::error_already_set& e) {
        log_error("Failed to initialize Python: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        log_error("Failed to initialize Python: " + std::string(e.what()));
        return false;
    }
}

void PythonEngine::shutdown() {
    if (!m_initialized) return;

    try {
        // Call cleanup hook if exists
        call_function("on_shutdown");
    } catch (...) {
        // Ignore errors during shutdown
    }

    m_interpreter.reset();
    m_initialized = false;
    log_info("Python interpreter shutdown");
}

bool PythonEngine::exec_file(const std::filesystem::path& file_path) {
    if (!m_initialized) {
        log_error("Python not initialized");
        return false;
    }

    try {
        // Read file content
        std::ifstream file(file_path);
        if (!file.is_open()) {
            log_error("Cannot open file: " + file_path.string());
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        // Используем py::eval вместо compile через builtins
        py::dict global = py::globals();
        
        // Устанавливаем __file__ чтобы debugpy знал путь
        global["__file__"] = file_path.string();

        // Execute
        py::exec(code, global);
        log_info("Executed: " + file_path.filename().string());
        return true;

    } catch (const py::error_already_set& e) {
        log_error("Python error in " + file_path.string() + ": " + std::string(e.what()));
        return false;
    }
}

bool PythonEngine::exec_string(const std::string& code) {
    if (!m_initialized) {
        log_error("Python not initialized");
        return false;
    }

    try {
        // Используем py::eval вместо compile через builtins
        py::dict global = py::globals();
        global["__file__"] = (m_scripts_path / "main.py").string();
        py::exec(code, global);
        return true;
    } catch (const py::error_already_set& e) {
        log_error("Python error: " + std::string(e.what()));
        return false;
    }
}

py::object PythonEngine::call_function(const std::string& func_name) {
    if (!m_initialized) return py::none();

    try {
        py::module_ main = py::module_::import("__main__");
        if (py::hasattr(main, func_name.c_str())) {
            return main.attr(func_name.c_str())();
        }
    } catch (const py::error_already_set& e) {
        log_error("Python error calling " + func_name + ": " + std::string(e.what()));
    }
    return py::none();
}

void PythonEngine::call_on_frame() {
    if (!m_initialized) return;

    try {
        py::module_ main = py::module_::import("__main__");
        if (py::hasattr(main, "on_frame")) {
            main.attr("on_frame")();
        }
    } catch (const py::error_already_set& e) {
        // Don't spam logs for on_frame errors
        static int error_count = 0;
        if (error_count++ < 5) {
            log_error("Python on_frame error: " + std::string(e.what()));
        }
    }
}

void PythonEngine::call_pre_init() {
    call_function("pre_init");
}

void PythonEngine::call_post_init() {
    call_function("post_init");
}

void PythonEngine::call_mission_ended() {
    call_function("mission_ended");
}

void PythonEngine::log_info(const std::string& msg) {
    std::string full_msg = "[PythonHost] " + msg;
    std::cout << full_msg << std::endl;

    // Also log to Arma RPT
    try {
        /*intercept::client::host::functions.invoker_lock();
        intercept::sqf::diag_log(full_msg);
        intercept::client::host::functions.invoker_unlock();*/
    } catch (...) {
        // Ignore if Arma not ready
    }
}

void PythonEngine::log_error(const std::string& msg) {
    std::string full_msg = "[PythonHost ERROR] " + msg;
    //std::cerr << full_msg << std::endl;
    std::cout << full_msg << std::endl;

    // Also log to Arma RPT
    try {
        /*intercept::client::host::functions.invoker_lock();
        intercept::sqf::diag_log(full_msg);
        intercept::client::host::functions.invoker_unlock();*/
    } catch (...) {
        // Ignore if Arma not ready
    }
}

} // namespace python_host
