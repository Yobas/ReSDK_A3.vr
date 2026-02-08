#include "intercept.hpp"
#include "js_engine.hpp"
#include <Windows.h>
#include <filesystem>
#include <string>
#include <fstream>

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
        // Strip Windows extended-length path prefix (\\?\) if present.
        // Arma 3 may load DLLs with this prefix, and GetModuleFileNameW preserves it.
        std::wstring pathStr(path);
        if (pathStr.size() >= 4 && pathStr.substr(0, 4) == L"\\\\?\\")
            pathStr = pathStr.substr(4);
        std::filesystem::path dll_path(pathStr);

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

    // Check for debugger config file: @Mod/srcjs/debugger.cfg
    // Format: key=value per line. Supported keys:
    //   port=9229       (debugger TCP port, default 9229)
    //   wait=1          (block until debugger connects, default 1)
    // If file exists, debugger is enabled. Engine listens on the port.
    auto debugger_cfg = g_scripts_path / "debugger.cfg";
    if (std::filesystem::exists(debugger_cfg)) {
        int port = 9229;
        bool wait = true;
        std::ifstream cfg(debugger_cfg);
        if (cfg.is_open()) {
            std::string line;
            while (std::getline(cfg, line)) {
                // trim whitespace
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
                    line.pop_back();
                if (line.empty() || line[0] == '#') continue;

                auto eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string key = line.substr(0, eq);
                    std::string val = line.substr(eq + 1);
                    if (key == "port") {
                        int parsed = std::atoi(val.c_str());
                        if (parsed > 0 && parsed < 65536) port = parsed;
                    } else if (key == "wait") {
                        wait = (val == "1" || val == "true");
                    }
                } else {
                    // Legacy format: first line is just port number
                    int parsed = std::atoi(line.c_str());
                    if (parsed > 0 && parsed < 65536) port = parsed;
                }
            }
        }
        JsEngine::instance().set_debugger_enabled(true);
        JsEngine::instance().set_debugger_port(port);
        JsEngine::instance().set_debugger_wait(wait);
        JsEngine::log_info("Debugger enabled via debugger.cfg, port: " + std::to_string(port) + (wait ? " (waiting for connection)" : " (non-blocking)"));
    }

    JsEngine::instance().initialize(g_scripts_path);
}

DLLEXPORT void CDECL handle_unload() {
    JsEngine::instance().shutdown();
}

DLLEXPORT bool CDECL is_signed() {
    return false;
}

DLLEXPORT void CDECL js_exec(game_value_parameter ctx)
    {
        std::cout << "Signal received: " << std::string(ctx) << std::endl;
        std::cout << "Signal parameters: " << std::string(ctx) << std::endl;
        auto& first = ctx.get_as<game_data_array>().getRef()->data[0];
        std::string code = static_cast<std::string>(first);
        JsEngine::instance().exec_string(code);
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
