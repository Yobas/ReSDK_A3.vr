#include "js_engine.hpp"
#include "type_conversion.hpp"
#include "sqf_bindings.hpp"
#include "quickjs-debugger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <Windows.h>

namespace js_host {

// ============================================================================
// Singleton
// ============================================================================

JsEngine& JsEngine::instance() {
    static JsEngine inst;
    return inst;
}

JsEngine::~JsEngine() {
    shutdown();
}

// ============================================================================
// Logging
// ============================================================================

void JsEngine::log_info(const std::string& msg) {
    std::string full_msg = "[JSHost] " + msg;
    std::cout << full_msg << std::endl;
    try {
        intercept::client::invoker_lock lock;
        intercept::sqf::diag_log(full_msg);
    } catch (...) {}
}

void JsEngine::log_error(const std::string& msg) {
    std::string full_msg = "[JSHost ERROR] " + msg;
    std::cout << full_msg << std::endl;
    try {
        intercept::client::invoker_lock lock;
        intercept::sqf::diag_log(full_msg);
    } catch (...) {}
}

// ============================================================================
// Exception handling
// ============================================================================

void JsEngine::dump_exception() {
    JSValue exception = JS_GetException(m_ctx);
    const char* str = JS_ToCString(m_ctx, exception);
    if (str) {
        log_error(std::string("JS Exception: ") + str);
        JS_FreeCString(m_ctx, str);
    }

    // Try to get stack trace
    if (JS_IsError(exception)) {
        JSValue stack = JS_GetPropertyStr(m_ctx, exception, "stack");
        if (!JS_IsUndefined(stack)) {
            const char* stack_str = JS_ToCString(m_ctx, stack);
            if (stack_str) {
                log_error(std::string("Stack: ") + stack_str);
                JS_FreeCString(m_ctx, stack_str);
            }
        }
        JS_FreeValue(m_ctx, stack);
    }

    JS_FreeValue(m_ctx, exception);
}

// ============================================================================
// Initialize QuickJS Engine
// ============================================================================

bool JsEngine::initialize(const std::filesystem::path& scripts_path) {
    if (m_initialized) {
        log_info("QuickJS already initialized");
        return true;
    }

    m_scripts_path = scripts_path;

    log_info("Initializing QuickJS-NG engine...");
    log_info("Scripts path: " + scripts_path.string());

    // Create runtime
    m_rt = JS_NewRuntime();
    if (!m_rt) {
        log_error("Failed to create QuickJS runtime!");
        return false;
    }

    // Create context
    m_ctx = JS_NewContext(m_rt);
    if (!m_ctx) {
        log_error("Failed to create QuickJS context!");
        JS_FreeRuntime(m_rt);
        m_rt = nullptr;
        return false;
    }

    // Register GameValue opaque class
    setup_game_value_class(m_rt, m_ctx);

    // Register SQF bindings (rvengine.player, rvengine.getPos, console.log)
    register_sqf_bindings(m_ctx);

    m_initialized = true;
    log_info("QuickJS-NG engine initialized successfully");

    // Start debugger if enabled (before loading scripts so breakpoints work)
    start_debugger();

    // Auto-load main.js if it exists
    auto main_path = m_scripts_path / "main.js";
    if (std::filesystem::exists(main_path)) {
        log_info("Loading main.js...");
        if (exec_file(main_path)) {
            cache_lifecycle_functions();
        }
    } else {
        log_info("No main.js found at: " + main_path.string());
    }

    return true;
}

// ============================================================================
// Shutdown
// ============================================================================

void JsEngine::shutdown() {
    if (!m_initialized) return;

    log_info("Shutting down QuickJS-NG engine...");

    free_lifecycle_cache();

    if (m_ctx) {
        JS_FreeContext(m_ctx);
        m_ctx = nullptr;
    }

    if (m_rt) {
        JS_FreeRuntime(m_rt);
        m_rt = nullptr;
    }

    m_initialized = false;
    log_info("QuickJS-NG engine shut down");
}

// ============================================================================
// Execute File
// ============================================================================

bool JsEngine::exec_file(const std::filesystem::path& file_path) {
    if (!m_initialized) {
        log_error("Engine not initialized");
        return false;
    }

    // Read file contents
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        log_error("Failed to open file: " + file_path.string());
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string code = ss.str();
    file.close();

    // Use the full absolute path as filename so the debugger can match breakpoints.
    // The VS Code extension sends breakpoints with full paths.
    std::string filename = file_path.string();
    // Strip Windows extended-length path prefix (\\?\) if present
    if (filename.size() >= 4 && filename.substr(0, 4) == "\\\\?\\")
        filename = filename.substr(4);
    // Normalize backslashes to forward slashes for consistency
    std::replace(filename.begin(), filename.end(), '\\', '/');

    JSValue result = JS_Eval(m_ctx, code.c_str(), code.size(),
                             filename.c_str(), JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result)) {
        dump_exception();
        JS_FreeValue(m_ctx, result);
        return false;
    }

    JS_FreeValue(m_ctx, result);
    log_info("Executed: " + filename);
    return true;
}

// ============================================================================
// Execute String
// ============================================================================

bool JsEngine::exec_string(const std::string& code) {
    if (!m_initialized) {
        log_error("Engine not initialized");
        return false;
    }

    JSValue result = JS_Eval(m_ctx, code.c_str(), code.size(),
                             "<eval>", JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result)) {
        dump_exception();
        JS_FreeValue(m_ctx, result);
        return false;
    }

    JS_FreeValue(m_ctx, result);
    return true;
}

// ============================================================================
// Lifecycle function caching
// ============================================================================

void JsEngine::cache_lifecycle_functions() {
    free_lifecycle_cache();

    JSValue global = JS_GetGlobalObject(m_ctx);

    m_on_frame_func = JS_GetPropertyStr(m_ctx, global, "on_frame");
    if (!JS_IsFunction(m_ctx, m_on_frame_func)) {
        JS_FreeValue(m_ctx, m_on_frame_func);
        m_on_frame_func = JS_UNDEFINED;
    }

    m_pre_init_func = JS_GetPropertyStr(m_ctx, global, "pre_init");
    if (!JS_IsFunction(m_ctx, m_pre_init_func)) {
        JS_FreeValue(m_ctx, m_pre_init_func);
        m_pre_init_func = JS_UNDEFINED;
    }

    m_post_init_func = JS_GetPropertyStr(m_ctx, global, "post_init");
    if (!JS_IsFunction(m_ctx, m_post_init_func)) {
        JS_FreeValue(m_ctx, m_post_init_func);
        m_post_init_func = JS_UNDEFINED;
    }

    m_mission_ended_func = JS_GetPropertyStr(m_ctx, global, "mission_ended");
    if (!JS_IsFunction(m_ctx, m_mission_ended_func)) {
        JS_FreeValue(m_ctx, m_mission_ended_func);
        m_mission_ended_func = JS_UNDEFINED;
    }

    JS_FreeValue(m_ctx, global);
}

void JsEngine::free_lifecycle_cache() {
    if (!m_ctx) return;

    if (!JS_IsUndefined(m_on_frame_func)) {
        JS_FreeValue(m_ctx, m_on_frame_func);
        m_on_frame_func = JS_UNDEFINED;
    }
    if (!JS_IsUndefined(m_pre_init_func)) {
        JS_FreeValue(m_ctx, m_pre_init_func);
        m_pre_init_func = JS_UNDEFINED;
    }
    if (!JS_IsUndefined(m_post_init_func)) {
        JS_FreeValue(m_ctx, m_post_init_func);
        m_post_init_func = JS_UNDEFINED;
    }
    if (!JS_IsUndefined(m_mission_ended_func)) {
        JS_FreeValue(m_ctx, m_mission_ended_func);
        m_mission_ended_func = JS_UNDEFINED;
    }
}

// ============================================================================
// Call a cached JS function
// ============================================================================

bool JsEngine::call_js_function(JSValue func, const char* name) {
    if (JS_IsUndefined(func)) return false;

    JSValue global = JS_GetGlobalObject(m_ctx);
    JSValue result = JS_Call(m_ctx, func, global, 0, nullptr);
    JS_FreeValue(m_ctx, global);

    if (JS_IsException(result)) {
        log_error(std::string("Exception in ") + name + "():");
        dump_exception();
        JS_FreeValue(m_ctx, result);
        return false;
    }

    JS_FreeValue(m_ctx, result);
    return true;
}

// ============================================================================
// Lifecycle hooks
// ============================================================================

void JsEngine::call_on_frame() {
    if (!m_initialized) return;

    // Let debugger process pending messages (breakpoints, step, etc.)
    if (m_debugger_enabled && m_ctx) {
        js_debugger_cooperate(m_ctx);
    }

    call_js_function(m_on_frame_func, "on_frame");
}

void JsEngine::call_pre_init() {
    if (!m_initialized) return;
    call_js_function(m_pre_init_func, "pre_init");
}

void JsEngine::call_post_init() {
    if (!m_initialized) return;
    call_js_function(m_post_init_func, "post_init");
}

void JsEngine::call_mission_ended() {
    if (!m_initialized) return;
    call_js_function(m_mission_ended_func, "mission_ended");
}

// ============================================================================
// Debugger
// ============================================================================

void JsEngine::start_debugger() {
    if (!m_debugger_enabled || !m_ctx) return;

    std::string address = "0.0.0.0:" + std::to_string(m_debugger_port);

    if (m_debugger_wait) {
        log_info("Debugger: waiting for connection on port " + std::to_string(m_debugger_port) + "...");
        js_debugger_wait_connection(m_ctx, address.c_str());
        log_info("Debugger: connected!");
    } else {
        log_info("Debugger: connecting to localhost:" + std::to_string(m_debugger_port) + "...");
        js_debugger_connect(m_ctx, address.c_str());
        log_info("Debugger: connected!");
    }
}

bool JsEngine::is_debugger_connected() const {
    if (!m_rt) return false;
    return js_debugger_is_transport_connected(m_rt) != 0;
}

} // namespace js_host
