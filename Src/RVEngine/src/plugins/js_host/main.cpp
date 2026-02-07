#include "intercept.hpp"
#include "js_engine.hpp"
#include <Windows.h>
#include <filesystem>
#include <string>

using namespace intercept;
using namespace js_host;

static std::filesystem::path g_scripts_path;

static std::filesystem::path get_scripts_path() {
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

        // Scripts should be in: @Mod/srcjs/ (DLL is in @Mod/intercept/)
        auto mod_path = dll_path.parent_path().parent_path();
        auto scripts_path = mod_path / "srcjs";

        if (std::filesystem::exists(scripts_path)) {
            return scripts_path;
        }
    }

    return std::filesystem::current_path() / "srcjs";
}

// ============================================================================
// Intercept Plugin Exports
// ============================================================================

extern "C" {

DLLEXPORT int CDECL api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

DLLEXPORT void CDECL pre_start() {
}

DLLEXPORT void CDECL post_start() {
}

DLLEXPORT void CDECL pre_pre_init() {
}

DLLEXPORT void CDECL pre_init() {
    JsEngine::instance().call_pre_init();
}

DLLEXPORT void CDECL post_init() {
    JsEngine::instance().call_post_init();
}

DLLEXPORT void CDECL mission_ended() {
    JsEngine::instance().call_mission_ended();
}

DLLEXPORT void CDECL on_frame() {
    JsEngine::instance().call_on_frame();
}

DLLEXPORT void CDECL on_signal(std::string& signal_name_, game_value& value1_) {
    if (signal_name_ == "js_exec") {
        std::string code = static_cast<std::string>(value1_);
        JsEngine::instance().exec_string(code);
    }
    else if (signal_name_ == "js_file") {
        std::string filename = static_cast<std::string>(value1_);
        auto filepath = g_scripts_path / filename;
        JsEngine::instance().exec_file(filepath);
    }
    else if (signal_name_ == "js_reload") {
        JsEngine::instance().call_mission_ended();
        auto main_path = g_scripts_path / "main.js";
        if (JsEngine::instance().exec_file(main_path)) {
            JsEngine::instance().cache_lifecycle_functions();
        }
    }
}

DLLEXPORT void CDECL on_interface_unload(r_string name_) {
}

DLLEXPORT void CDECL register_interfaces() {
    g_scripts_path = get_scripts_path();
    JsEngine::instance().initialize(g_scripts_path);
}

DLLEXPORT void CDECL handle_unload() {
    JsEngine::instance().shutdown();
}

DLLEXPORT bool CDECL is_signed() {
    return false;
}

} // extern "C"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
