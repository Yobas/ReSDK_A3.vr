#include "intercept.hpp"
#include "python_engine.hpp"
#include "sqf_bindings.hpp"
#include <Windows.h>
#include <filesystem>
#include <string>

using namespace intercept;
using namespace python_host;

// Path to Python scripts (relative to Arma 3 directory or absolute)
static std::filesystem::path g_scripts_path;

// Get scripts path based on DLL location
static std::filesystem::path get_scripts_path() {
    // Default: look for 'python' folder next to the DLL
    // Or in @YourMod/python/

    // Try to get DLL path
    HMODULE hModule = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&get_scripts_path),
        &hModule
    );

    if (hModule) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);
        std::filesystem::path dll_path(path);

        // Scripts should be in: @Mod/python/ (DLL is in @Mod/intercept/)
        auto mod_path = dll_path.parent_path().parent_path();
        auto python_path = mod_path / "src";

        if (std::filesystem::exists(python_path)) {
            return python_path;
        }
    }

    // Fallback to current directory
    return std::filesystem::current_path() / "src";
}

// ============================================================================
// Intercept Plugin Exports
// ============================================================================

extern "C" {

DLLEXPORT int CDECL api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

DLLEXPORT void CDECL pre_start() {
    // Early initialization - Python not ready yet for SQF calls
}

DLLEXPORT void CDECL post_start() {
    
}

DLLEXPORT void CDECL pre_pre_init() {
    // Called before pre_init
}

DLLEXPORT void CDECL pre_init() {
    // Called at CBA XEH_preInit

    PythonEngine::instance().call_pre_init();
}

DLLEXPORT void CDECL post_init() {
    // Called at CBA XEH_postInit
    PythonEngine::instance().call_post_init();
}

DLLEXPORT void CDECL mission_ended() {
    // Called when mission ends
    PythonEngine::instance().call_mission_ended();
}

DLLEXPORT void CDECL on_frame() {
    // Called each frame
    PythonEngine::instance().call_on_frame();
}

DLLEXPORT void CDECL on_signal(std::string& signal_name_, game_value& value1_) {
    // Handle signals from SQF
    // Can be used to call Python functions from SQF:
    // "python_call" callExtension ["function_name", args]

    if (signal_name_ == "python_exec") {
        // Execute Python code string
        std::string code = static_cast<std::string>(value1_);
        PythonEngine::instance().exec_string(code);
    }
    else if (signal_name_ == "python_file") {
        // Execute Python file
        std::string filename = static_cast<std::string>(value1_);
        auto filepath = g_scripts_path / filename;
        PythonEngine::instance().exec_file(filepath);
    }
    else if (signal_name_ == "python_reload") {
        // 1. Остановить потоки (on_shutdown)
        PythonEngine::instance().call_function("on_shutdown");
        // 2. Перезагрузить main.py
        auto main_path = g_scripts_path / "main.py";
        PythonEngine::instance().exec_file(main_path);
    }
}

DLLEXPORT void CDECL python_reload(game_value_parameter ctx)
{
    PythonEngine::instance().call_function("on_shutdown");
    // 2. Перезагрузить main.py
    auto main_path = g_scripts_path / "main.py";
    PythonEngine::instance().exec_file(main_path);
}

DLLEXPORT void CDECL python_exec(game_value_parameter ctx)
{
    auto& args = ctx.get_as<game_data_array>().getRef()->data;
    if (args.count() < 1) return;
    std::string code = static_cast<std::string>(args[0]);
    PythonEngine::instance().exec_string(code);
}

DLLEXPORT void CDECL on_interface_unload(r_string name_) {
    // Handle interface unload
}

DLLEXPORT void CDECL register_interfaces() {
    // Register custom SQF commands here if needed
        // Initialize Python interpreter
    g_scripts_path = get_scripts_path();
    PythonEngine::instance().initialize(g_scripts_path);
    // Example: register command to call Python from SQF
}

DLLEXPORT void CDECL handle_unload() {
    // Cleanup before unload
    PythonEngine::instance().shutdown();
}

DLLEXPORT bool CDECL is_signed() {
    return false; // Set to true if plugin is signed
}

} // extern "C"

// Windows DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
