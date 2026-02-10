#include "v8_engine.hpp"
#include "v8_inspector_client.hpp"
#include <v8.h>
#include <libplatform/libplatform.h>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace v8_host {

// ============================================================================
// Static helpers
// ============================================================================

static std::string v8_str_to_std(v8::Isolate* isolate, v8::Local<v8::Value> value) {
    if (value.IsEmpty() || !value->IsString()) return "";
    v8::String::Utf8Value utf8(isolate, value);
    return *utf8 ? std::string(*utf8) : "";
}

static v8::Local<v8::String> std_str_to_v8(v8::Isolate* isolate, const std::string& str) {
    return v8::String::NewFromUtf8(isolate, str.c_str(),
        v8::NewStringType::kNormal, static_cast<int>(str.size())).ToLocalChecked();
}

// ============================================================================
// JS Bindings: console.log / console.warn / console.error
// ============================================================================

static std::string args_to_string(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    std::string message;
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) message += " ";
        v8::String::Utf8Value str(isolate, args[i]);
        message += *str ? *str : "(null)";
    }
    return message;
}

static void js_console_log(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::HandleScope handle_scope(args.GetIsolate());
    std::string message = args_to_string(args);
    V8Engine::log_info(message);
    // Also send to DevTools Console
    auto& engine = V8Engine::instance();
    if (engine.inspector()) {
        auto ctx = args.GetIsolate()->GetCurrentContext();
        engine.inspector()->sendConsoleMessage(ctx, "log", message);
    }
}

static void js_console_warn(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::HandleScope handle_scope(args.GetIsolate());
    std::string message = args_to_string(args);
    V8Engine::log_info("[WARN] " + message);
    auto& engine = V8Engine::instance();
    if (engine.inspector()) {
        auto ctx = args.GetIsolate()->GetCurrentContext();
        engine.inspector()->sendConsoleMessage(ctx, "warning", message);
    }
}

static void js_console_error(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::HandleScope handle_scope(args.GetIsolate());
    std::string message = args_to_string(args);
    V8Engine::log_error(message);
    auto& engine = V8Engine::instance();
    if (engine.inspector()) {
        auto ctx = args.GetIsolate()->GetCurrentContext();
        engine.inspector()->sendConsoleMessage(ctx, "error", message);
    }
}

// ============================================================================
// JS Bindings: performance.now()
// ============================================================================

static void js_performance_now(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto& engine = V8Engine::instance();
    auto now = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(now - engine.start_time()).count();
    args.GetReturnValue().Set(ms);
}

// ============================================================================
// GameValue wrapper helpers
// ============================================================================

static v8::Global<v8::ObjectTemplate> s_game_value_template;

static v8::Local<v8::Object> wrap_game_value(v8::Isolate* isolate, intercept::types::game_value* gv) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::ObjectTemplate> templ = s_game_value_template.Get(isolate);
    v8::Local<v8::Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    obj->SetAlignedPointerInInternalField(0, gv);
    return handle_scope.Escape(obj);
}

static intercept::types::game_value* unwrap_game_value(v8::Isolate* isolate, v8::Local<v8::Value> val) {
    if (!val->IsObject()) return nullptr;
    v8::Local<v8::Object> obj = val.As<v8::Object>();
    if (obj->InternalFieldCount() < 1) return nullptr;
    return static_cast<intercept::types::game_value*>(obj->GetAlignedPointerFromInternalField(0));
}

// ============================================================================
// JS Bindings: rvengine.player() / rvengine.getPos(obj)
// ============================================================================

static void js_rvengine_player(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    try {
        intercept::client::invoker_lock lock;
        auto* gv = new intercept::types::game_value(intercept::sqf::player());
        args.GetReturnValue().Set(wrap_game_value(isolate, gv));
    } catch (const std::exception& e) {
        isolate->ThrowException(std_str_to_v8(isolate, std::string("player: ") + e.what()));
    }
}

static void js_rvengine_get_pos(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    if (args.Length() < 1) {
        isolate->ThrowException(std_str_to_v8(isolate, "getPos: expected 1 argument (GameValue)"));
        return;
    }

    intercept::types::game_value* obj = unwrap_game_value(isolate, args[0]);
    if (!obj) {
        isolate->ThrowException(std_str_to_v8(isolate, "getPos: argument is not a GameValue"));
        return;
    }

    try {
        intercept::client::invoker_lock lock;
        intercept::types::game_value result = intercept::sqf::get_pos(
            static_cast<intercept::types::object>(*obj));

        auto& arr = result.to_array();
        if (arr.size() >= 3) {
            v8::Local<v8::Context> context = isolate->GetCurrentContext();
            v8::Local<v8::Array> js_arr = v8::Array::New(isolate, 3);
            js_arr->Set(context, 0, v8::Number::New(isolate, static_cast<float>(arr[0]))).Check();
            js_arr->Set(context, 1, v8::Number::New(isolate, static_cast<float>(arr[1]))).Check();
            js_arr->Set(context, 2, v8::Number::New(isolate, static_cast<float>(arr[2]))).Check();
            args.GetReturnValue().Set(js_arr);
            return;
        }

        args.GetReturnValue().Set(v8::Array::New(isolate, 0));
    } catch (const std::exception& e) {
        isolate->ThrowException(std_str_to_v8(isolate, std::string("getPos: ") + e.what()));
    }
}

// ============================================================================
// V8Engine Singleton
// ============================================================================

V8Engine& V8Engine::instance() {
    static V8Engine s_instance;
    return s_instance;
}

V8Engine::~V8Engine() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

bool V8Engine::initialize(const std::filesystem::path& scripts_path) {
    if (m_initialized) return true;

    m_scripts_path = scripts_path;
    m_start_time = std::chrono::steady_clock::now();

    log_info("Initializing V8 engine...");

    // Initialize V8 platform
    m_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(m_platform.get());
    v8::V8::Initialize();

    // Create Isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    m_isolate = v8::Isolate::New(create_params);

    if (!m_isolate) {
        log_error("Failed to create V8 Isolate");
        return false;
    }

    {
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);

        // Create global object template with bindings
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_isolate);
        setup_global_bindings(global);

        // Create context
        v8::Local<v8::Context> context = v8::Context::New(m_isolate, nullptr, global);
        m_context.Reset(m_isolate, context);

        // Start inspector if debugger enabled
        if (m_debugger_enabled) {
            log_info("Starting V8 Inspector on port " + std::to_string(m_debugger_port) + "...");
            m_inspector = std::make_unique<V8InspectorClientImpl>(
                m_isolate, m_platform.get(), m_debugger_port, m_debugger_wait);
            m_inspector->contextCreated(context, "V8Host");
        }
    }

    m_initialized = true;
    log_info("V8 engine initialized successfully (version: " + std::string(v8::V8::GetVersion()) + ")");

    // Auto-load main.js
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
// Global Bindings Setup
// ============================================================================

void V8Engine::setup_global_bindings(v8::Local<v8::ObjectTemplate>& global) {
    v8::Isolate* isolate = m_isolate;

    // --- console object ---
    v8::Local<v8::ObjectTemplate> console = v8::ObjectTemplate::New(isolate);
    console->Set(isolate, "log",
        v8::FunctionTemplate::New(isolate, js_console_log));
    console->Set(isolate, "warn",
        v8::FunctionTemplate::New(isolate, js_console_warn));
    console->Set(isolate, "error",
        v8::FunctionTemplate::New(isolate, js_console_error));
    console->Set(isolate, "info",
        v8::FunctionTemplate::New(isolate, js_console_log));
    global->Set(isolate, "console", console);

    // --- performance object ---
    v8::Local<v8::ObjectTemplate> performance = v8::ObjectTemplate::New(isolate);
    performance->Set(isolate, "now",
        v8::FunctionTemplate::New(isolate, js_performance_now));
    global->Set(isolate, "performance", performance);

    // --- GameValue object template (1 internal field for game_value*) ---
    v8::Local<v8::ObjectTemplate> gv_templ = v8::ObjectTemplate::New(isolate);
    gv_templ->SetInternalFieldCount(1);
    s_game_value_template.Reset(isolate, gv_templ);

    // --- rvengine object ---
    v8::Local<v8::ObjectTemplate> rvengine = v8::ObjectTemplate::New(isolate);
    rvengine->Set(isolate, "player",
        v8::FunctionTemplate::New(isolate, js_rvengine_player));
    rvengine->Set(isolate, "getPos",
        v8::FunctionTemplate::New(isolate, js_rvengine_get_pos));
    global->Set(isolate, "rvengine", rvengine);
}

// ============================================================================
// Shutdown
// ============================================================================

void V8Engine::shutdown() {
    if (!m_initialized) return;

    log_info("Shutting down V8 engine...");

    {
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);

        // Free lifecycle functions
        m_on_frame_func.Reset();
        m_pre_init_func.Reset();
        m_post_init_func.Reset();
        m_mission_ended_func.Reset();

        // Free static V8 globals (must be done before isolate dispose,
        // otherwise their atexit destructors crash on dead isolate)
        s_game_value_template.Reset();

        // Destroy inspector before context
        if (m_inspector) {
            m_inspector->contextDestroyed();
            m_inspector.reset();
        }

        // Free context
        m_context.Reset();
    }

    // Dispose isolate
    m_isolate->Dispose();
    m_isolate = nullptr;

    // Shutdown V8
    v8::V8::Dispose();
    v8::V8::DisposePlatform();

    m_platform.reset();
    m_initialized = false;

    log_info("V8 engine shut down");
}

// ============================================================================
// Script Execution
// ============================================================================

bool V8Engine::exec_file(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        log_error("Cannot open file: " + file_path.string());
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    // Normalize path for V8 (forward slashes)
    std::string filename = file_path.string();
    std::replace(filename.begin(), filename.end(), '\\', '/');

    return exec_string(code, filename);
}

bool V8Engine::exec_string(const std::string& code, const std::string& filename) {
    if (!m_initialized || !m_isolate) return false;

    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context = m_context.Get(m_isolate);
    v8::Context::Scope context_scope(context);

    v8::TryCatch try_catch(m_isolate);

    v8::Local<v8::String> source = std_str_to_v8(m_isolate, code);

    // Set script origin (filename for stack traces and debugger)
    v8::ScriptOrigin origin(std_str_to_v8(m_isolate, filename));

    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
        report_exception(try_catch);
        return false;
    }

    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        report_exception(try_catch);
        return false;
    }

    return true;
}

// ============================================================================
// Lifecycle Functions
// ============================================================================

void V8Engine::cache_lifecycle_functions() {
    if (!m_initialized) return;

    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context = m_context.Get(m_isolate);
    v8::Context::Scope context_scope(context);

    v8::Local<v8::Object> global = context->Global();

    auto try_cache = [&](const char* name, v8::Global<v8::Function>& target) {
        v8::Local<v8::Value> val;
        if (global->Get(context, std_str_to_v8(m_isolate, name)).ToLocal(&val) && val->IsFunction()) {
            target.Reset(m_isolate, val.As<v8::Function>());
            log_info(std::string("Cached lifecycle function: ") + name);
        } else {
            target.Reset();
        }
    };

    try_cache("on_frame", m_on_frame_func);
    try_cache("pre_init", m_pre_init_func);
    try_cache("post_init", m_post_init_func);
    try_cache("mission_ended", m_mission_ended_func);
}

bool V8Engine::call_js_function(v8::Global<v8::Function>& func, const char* name) {
    if (func.IsEmpty() || !m_initialized) return false;

    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context = m_context.Get(m_isolate);
    v8::Context::Scope context_scope(context);

    // Pump inspector messages before JS execution
    if (m_inspector) {
        m_inspector->pumpMessages();
    }

    v8::TryCatch try_catch(m_isolate);
    v8::Local<v8::Function> fn = func.Get(m_isolate);
    v8::Local<v8::Value> recv = context->Global();

    v8::Local<v8::Value> result;
    if (!fn->Call(context, recv, 0, nullptr).ToLocal(&result)) {
        report_exception(try_catch);
        return false;
    }

    return true;
}

void V8Engine::call_on_frame() { call_js_function(m_on_frame_func, "on_frame"); }
void V8Engine::call_pre_init() { call_js_function(m_pre_init_func, "pre_init"); }
void V8Engine::call_post_init() { call_js_function(m_post_init_func, "post_init"); }
void V8Engine::call_mission_ended() { call_js_function(m_mission_ended_func, "mission_ended"); }

// ============================================================================
// Error Reporting
// ============================================================================

void V8Engine::report_exception(v8::TryCatch& try_catch) {
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context = m_context.Get(m_isolate);

    v8::Local<v8::Message> message = try_catch.Message();
    if (message.IsEmpty()) {
        v8::String::Utf8Value exception_str(m_isolate, try_catch.Exception());
        log_error(std::string("JS Exception: ") + (*exception_str ? *exception_str : "unknown"));
        return;
    }

    // Filename
    v8::String::Utf8Value filename(m_isolate, message->GetScriptResourceName());
    int linenum = message->GetLineNumber(context).FromMaybe(0);

    // Error message
    v8::String::Utf8Value msg(m_isolate, message->Get());

    std::string error = std::string(*filename ? *filename : "<unknown>") +
        ":" + std::to_string(linenum) + ": " +
        (*msg ? *msg : "unknown error");

    // Stack trace
    v8::Local<v8::Value> stack_trace_val;
    if (try_catch.StackTrace(context).ToLocal(&stack_trace_val) && stack_trace_val->IsString()) {
        v8::String::Utf8Value stack_trace(m_isolate, stack_trace_val);
        if (*stack_trace) {
            error += "\n" + std::string(*stack_trace);
        }
    }

    log_error(error);
}

// ============================================================================
// Logging
// ============================================================================

void V8Engine::log_info(const std::string& msg) {
    std::string full_msg = "[V8Host] " + msg;
    printf("%s\n", full_msg.c_str());
    fflush(stdout);
    try {
        intercept::client::invoker_lock lock;
        intercept::sqf::diag_log(full_msg);
    } catch (...) {}
}

void V8Engine::log_error(const std::string& msg) {
    std::string full_msg = "[V8Host] ERROR: " + msg;
    printf("%s\n", full_msg.c_str());
    fflush(stdout);
    try {
        intercept::client::invoker_lock lock;
        intercept::sqf::diag_log(full_msg);
    } catch (...) {}
}

} // namespace v8_host
