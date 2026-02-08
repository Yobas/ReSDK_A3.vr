#pragma once
#include "intercept.hpp"
#include "quickjs.h"
#include <string>
#include <filesystem>

namespace js_host {

class JsEngine {
public:
    static JsEngine& instance();

    bool initialize(const std::filesystem::path& scripts_path);
    void shutdown();
    bool is_initialized() const { return m_initialized; }

    bool exec_file(const std::filesystem::path& file_path);
    bool exec_string(const std::string& code);

    void call_on_frame();
    void call_pre_init();
    void call_post_init();
    void call_mission_ended();

    // Re-cache lifecycle functions from global scope (after reload)
    void cache_lifecycle_functions();

    const std::filesystem::path& get_scripts_path() const { return m_scripts_path; }

    // Debugger
    void set_debugger_enabled(bool enabled) { m_debugger_enabled = enabled; }
    void set_debugger_port(int port) { m_debugger_port = port; }
    void set_debugger_wait(bool wait) { m_debugger_wait = wait; }
    bool is_debugger_connected() const;

    static void log_info(const std::string& msg);
    static void log_error(const std::string& msg);

private:
    JsEngine() = default;
    ~JsEngine();
    JsEngine(const JsEngine&) = delete;
    JsEngine& operator=(const JsEngine&) = delete;

    // Call a cached lifecycle JS function
    bool call_js_function(JSValue func, const char* name);

    // Dump the pending JS exception to log
    void dump_exception();

    // Free cached lifecycle JSValues
    void free_lifecycle_cache();

    // Start debugger transport
    void start_debugger();

    bool m_initialized = false;
    std::filesystem::path m_scripts_path;

    JSRuntime* m_rt = nullptr;
    JSContext* m_ctx = nullptr;

    // Debugger settings
    bool m_debugger_enabled = false;
    int m_debugger_port = 9229;
    bool m_debugger_wait = false;  // wait for debugger to attach before continuing

    // Cached lifecycle function references
    JSValue m_on_frame_func = JS_UNDEFINED;
    JSValue m_pre_init_func = JS_UNDEFINED;
    JSValue m_post_init_func = JS_UNDEFINED;
    JSValue m_mission_ended_func = JS_UNDEFINED;
};

} // namespace js_host
