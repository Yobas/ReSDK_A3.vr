#pragma once
#include "intercept.hpp"
#include <v8.h>
#include <libplatform/libplatform.h>
#include <string>
#include <filesystem>
#include <chrono>

namespace v8_host {

class V8InspectorClientImpl;  // forward declaration

class V8Engine {
public:
    static V8Engine& instance();

    bool initialize(const std::filesystem::path& scripts_path);
    void shutdown();
    bool is_initialized() const { return m_initialized; }

    bool exec_file(const std::filesystem::path& file_path);
    bool exec_string(const std::string& code, const std::string& filename = "<eval>");

    void call_on_frame();
    void call_pre_init();
    void call_post_init();
    void call_mission_ended();

    void cache_lifecycle_functions();

    const std::filesystem::path& get_scripts_path() const { return m_scripts_path; }

    // Debugger
    void set_debugger_enabled(bool enabled) { m_debugger_enabled = enabled; }
    void set_debugger_port(int port) { m_debugger_port = port; }
    void set_debugger_wait(bool wait) { m_debugger_wait = wait; }

    // Accessors for inspector
    v8::Isolate* isolate() const { return m_isolate; }
    v8::Local<v8::Context> context() const {
        return m_context.Get(m_isolate);
    }
    V8InspectorClientImpl* inspector() const { return m_inspector.get(); }

    static void log_info(const std::string& msg);
    static void log_error(const std::string& msg);

    // Start time for performance.now()
    std::chrono::steady_clock::time_point start_time() const { return m_start_time; }

private:
    V8Engine() = default;
    ~V8Engine();
    V8Engine(const V8Engine&) = delete;
    V8Engine& operator=(const V8Engine&) = delete;

    void setup_global_bindings(v8::Local<v8::ObjectTemplate>& global);
    bool call_js_function(v8::Global<v8::Function>& func, const char* name);
    void report_exception(v8::TryCatch& try_catch);

    bool m_initialized = false;
    std::filesystem::path m_scripts_path;
    std::chrono::steady_clock::time_point m_start_time;

    // V8 state
    std::unique_ptr<v8::Platform> m_platform;
    v8::Isolate* m_isolate = nullptr;
    v8::Global<v8::Context> m_context;

    // Cached lifecycle function references
    v8::Global<v8::Function> m_on_frame_func;
    v8::Global<v8::Function> m_pre_init_func;
    v8::Global<v8::Function> m_post_init_func;
    v8::Global<v8::Function> m_mission_ended_func;

    // Debugger
    bool m_debugger_enabled = false;
    int m_debugger_port = 9229;
    bool m_debugger_wait = false;
    std::unique_ptr<V8InspectorClientImpl> m_inspector;
};

} // namespace v8_host
