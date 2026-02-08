#include "quickjs-debugger.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* Debug logging macro - writes to a file since Arma may not have stderr visible */
static FILE* _dbg_get_log(void) {
    static FILE *f = NULL;
    if (!f) {
        f = fopen("P:\\armatools\\steamapps\\common\\Arma 3\\jshost_debug.log", "w");
        if (!f) f = stderr;
    }
    return f;
}
#define DBG_LOG(...) do { FILE *_f = _dbg_get_log(); fprintf(_f, "[DBG] "); fprintf(_f, __VA_ARGS__); fprintf(_f, "\n"); fflush(_f); } while(0)

typedef struct DebuggerSuspendedState {
    uint32_t variable_reference_count;
    JSValue variable_references;
    JSValue variable_pointers;
    const uint8_t *cur_pc;
} DebuggerSuspendedState;

static int js_transport_read_fully(JSDebuggerInfo *info, char *buffer, size_t length) {
    int offset = 0;
    while (offset < (int)length) {
        int received = (int)info->transport_read(info->transport_udata, buffer + offset, length - offset);
        if (received <= 0)
            return 0;
        offset += received;
    }
    return 1;
}

static int js_transport_write_fully(JSDebuggerInfo *info, const char *buffer, size_t length) {
    int offset = 0;
    while (offset < (int)length) {
        int sent = (int)info->transport_write(info->transport_udata, buffer + offset, length - offset);
        if (sent <= 0)
            return 0;
        offset += sent;
    }
    return 1;
}

static int js_transport_write_message_newline(JSDebuggerInfo *info, const char* value, size_t len) {
    char message_length[10];
    message_length[9] = '\0';
    sprintf(message_length, "%08x\n", (int)len + 1);
    if (!js_transport_write_fully(info, message_length, 9))
        return 0;
    int ret = js_transport_write_fully(info, value, len);
    if (!ret)
        return ret;
    char newline[2] = { '\n', '\0' };
    return js_transport_write_fully(info, newline, 1);
}

static int js_transport_write_value(JSDebuggerInfo *info, JSValue value) {
    /* Use debugging_ctx for JSON serialization — info->ctx may be NULL outside js_debugger_check */
    JSContext *ctx = info->debugging_ctx;
    JSValue stringified = JS_JSONStringify(ctx, value, JS_UNDEFINED, JS_UNDEFINED);
    size_t len;
    const char* str = JS_ToCStringLen(ctx, &len, stringified);
    int ret = 0;
    if (str && len) {
        DBG_LOG("send msg (%d bytes): %.200s", (int)len, str);
        ret = js_transport_write_message_newline(info, str, len);
    } else {
        DBG_LOG("send msg FAILED: str=%p len=%d", (void*)str, (int)len);
    }
    JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, stringified);
    JS_FreeValue(ctx, value);
    return ret;
}

static JSValue js_transport_new_envelope(JSDebuggerInfo *info, const char *type) {
    JSContext *ctx = info->debugging_ctx;
    JSValue ret = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret, "type", JS_NewString(ctx, type));
    return ret;
}

static int js_transport_send_event(JSDebuggerInfo *info, JSValue event) {
    JSContext *ctx = info->debugging_ctx;
    JSValue envelope = js_transport_new_envelope(info, "event");
    JS_SetPropertyStr(ctx, envelope, "event", event);
    return js_transport_write_value(info, envelope);
}

static int js_transport_send_response(JSDebuggerInfo *info, JSValue request, JSValue body) {
    JSContext *ctx = info->debugging_ctx;
    JSValue envelope = js_transport_new_envelope(info, "response");
    JS_SetPropertyStr(ctx, envelope, "body", body);
    JS_SetPropertyStr(ctx, envelope, "request_seq", JS_GetPropertyStr(ctx, request, "request_seq"));
    return js_transport_write_value(info, envelope);
}

static JSValue js_get_scopes(JSContext *ctx, int frame) {
    JSValue scopes = JS_NewArray(ctx);
    int scope_count = 0;

    JSValue local = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, local, "name", JS_NewString(ctx, "Local"));
    JS_SetPropertyStr(ctx, local, "reference", JS_NewInt32(ctx, (frame << 2) + 1));
    JS_SetPropertyStr(ctx, local, "expensive", JS_FALSE);
    JS_SetPropertyUint32(ctx, scopes, scope_count++, local);

    JSValue closure = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, closure, "name", JS_NewString(ctx, "Closure"));
    JS_SetPropertyStr(ctx, closure, "reference", JS_NewInt32(ctx, (frame << 2) + 2));
    JS_SetPropertyStr(ctx, closure, "expensive", JS_FALSE);
    JS_SetPropertyUint32(ctx, scopes, scope_count++, closure);

    JSValue global = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, global, "name", JS_NewString(ctx, "Global"));
    JS_SetPropertyStr(ctx, global, "reference", JS_NewInt32(ctx, (frame << 2) + 0));
    JS_SetPropertyStr(ctx, global, "expensive", JS_TRUE);
    JS_SetPropertyUint32(ctx, scopes, scope_count++, global);

    return scopes;
}

static inline int JS_IsInteger_Dbg(JSValueConst v)
{
    int tag = JS_VALUE_GET_TAG(v);
    return tag == JS_TAG_INT || tag == JS_TAG_BIG_INT;
}

static void js_debugger_get_variable_type(JSContext *ctx,
        struct DebuggerSuspendedState *state,
        JSValue var, JSValue var_val) {

    uint32_t reference = 0;
    if (JS_IsString(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "string"));
    else if (JS_IsInteger_Dbg(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "integer"));
    else if (JS_IsNumber(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "float"));
    else if (JS_IsBool(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "boolean"));
    else if (JS_IsNull(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "null"));
    else if (JS_IsUndefined(var_val))
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "undefined"));
    else if (JS_IsObject(var_val)) {
        JS_SetPropertyStr(ctx, var, "type", JS_NewString(ctx, "object"));

        void *p = JS_VALUE_GET_PTR(var_val);
        uint32_t pl = (uint32_t)(uintptr_t)p;
        JSValue found = JS_GetPropertyUint32(ctx, state->variable_pointers, pl);
        if (JS_IsUndefined(found)) {
            reference = state->variable_reference_count++;
            JS_SetPropertyUint32(ctx, state->variable_references, reference, JS_DupValue(ctx, var_val));
            JS_SetPropertyUint32(ctx, state->variable_pointers, pl, JS_NewInt32(ctx, reference));
        }
        else {
            JS_ToUint32(ctx, &reference, found);
        }
        JS_FreeValue(ctx, found);
    }
    JS_SetPropertyStr(ctx, var, "variablesReference", JS_NewInt32(ctx, reference));
}

static void js_debugger_get_value(JSContext *ctx, JSValue var_val, JSValue var, const char *value_property) {
    if (JS_IsArray(var_val)) {
        JSValue length = JS_GetPropertyStr(ctx, var_val, "length");
        uint32_t len;
        JS_ToUint32(ctx, &len, length);
        JS_FreeValue(ctx, length);
        char lenBuf[64];
        sprintf(lenBuf, "Array (%d)", len);
        JS_SetPropertyStr(ctx, var, value_property, JS_NewString(ctx, lenBuf));
        JS_SetPropertyStr(ctx, var, "indexedVariables", JS_NewInt32(ctx, len));
    }
    else {
        JS_SetPropertyStr(ctx, var, value_property, JS_ToString(ctx, var_val));
    }
}

static JSValue js_debugger_get_variable(JSContext *ctx,
    struct DebuggerSuspendedState *state,
    JSValue var_name, JSValue var_val) {
    JSValue var = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, var, "name", var_name);
    js_debugger_get_value(ctx, var_val, var, "value");
    js_debugger_get_variable_type(ctx, state, var, var_val);
    return var;
}

static int js_debugger_get_frame(JSContext *ctx, JSValue args) {
    JSValue reference_property = JS_GetPropertyStr(ctx, args, "frameId");
    int frame;
    JS_ToInt32(ctx, &frame, reference_property);
    JS_FreeValue(ctx, reference_property);
    return frame;
}

static void js_send_stopped_event(JSDebuggerInfo *info, const char *reason) {
    JSContext *ctx = info->debugging_ctx;

    JSValue event = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, event, "type", JS_NewString(ctx, "StoppedEvent"));
    JS_SetPropertyStr(ctx, event, "reason", JS_NewString(ctx, reason));
    int64_t id = (int64_t)info->ctx;
    JS_SetPropertyStr(ctx, event, "thread", JS_NewInt64(ctx, id));
    js_transport_send_event(info, event);
}

static void js_free_prop_enum(JSContext *ctx, JSPropertyEnum *tab, uint32_t len)
{
    uint32_t i;
    if (tab) {
        for(i = 0; i < len; i++)
            JS_FreeAtom(ctx, tab[i].atom);
        js_free(ctx, tab);
    }
}

static uint32_t js_get_property_as_uint32(JSContext *ctx, JSValue obj, const char* property) {
    JSValue prop = JS_GetPropertyStr(ctx, obj, property);
    uint32_t ret;
    JS_ToUint32(ctx, &ret, prop);
    JS_FreeValue(ctx, prop);
    return ret;
}

static void js_process_request(JSDebuggerInfo *info, struct DebuggerSuspendedState *state, JSValue request) {
    /* dctx = debugging context for JSON parsing of protocol messages.
       ctx  = debuggee context for stack inspection, eval, variables.
       ctx may be NULL during initial attach (before any JS code runs). */
    JSContext *dctx = info->debugging_ctx;
    JSContext *ctx = info->ctx ? info->ctx : dctx;
    JSValue command_property = JS_GetPropertyStr(dctx, request, "command");
    const char *command = JS_ToCString(dctx, command_property);
    if (strcmp("continue", command) == 0) {
        info->stepping = JS_DEBUGGER_STEP_CONTINUE;
        info->step_over = js_debugger_current_location(ctx, state->cur_pc);
        info->step_depth = js_debugger_stack_depth(ctx);
        js_transport_send_response(info, request, JS_UNDEFINED);
        info->is_paused = 0;
    }
    if (strcmp("pause", command) == 0) {
        js_transport_send_response(info, request, JS_UNDEFINED);
        js_send_stopped_event(info, "pause");
        info->is_paused = 1;
    }
    else if (strcmp("next", command) == 0) {
        info->stepping = JS_DEBUGGER_STEP;
        info->step_over = js_debugger_current_location(ctx, state->cur_pc);
        info->step_depth = js_debugger_stack_depth(ctx);
        js_transport_send_response(info, request, JS_UNDEFINED);
        info->is_paused = 0;
    }
    else if (strcmp("stepIn", command) == 0) {
        info->stepping = JS_DEBUGGER_STEP_IN;
        info->step_over = js_debugger_current_location(ctx, state->cur_pc);
        info->step_depth = js_debugger_stack_depth(ctx);
        js_transport_send_response(info, request, JS_UNDEFINED);
        info->is_paused = 0;
    }
    else if (strcmp("stepOut", command) == 0) {
        info->stepping = JS_DEBUGGER_STEP_OUT;
        info->step_over = js_debugger_current_location(ctx, state->cur_pc);
        info->step_depth = js_debugger_stack_depth(ctx);
        js_transport_send_response(info, request, JS_UNDEFINED);
        info->is_paused = 0;
    }
    else if (strcmp("evaluate", command) == 0) {
        JSValue args = JS_GetPropertyStr(dctx, request, "args");
        int frame = js_debugger_get_frame(dctx, args);
        JSValue expression = JS_GetPropertyStr(dctx, args, "expression");
        JS_FreeValue(dctx, args);
        JSValue result = js_debugger_evaluate(ctx, frame, expression);
        if (JS_IsException(result)) {
            JS_FreeValue(ctx, result);
            result = JS_GetException(ctx);
        }
        JS_FreeValue(dctx, expression);

        JSValue body = JS_NewObject(dctx);
        js_debugger_get_value(dctx, result, body, "result");
        js_debugger_get_variable_type(dctx, state, body, result);
        JS_FreeValue(ctx, result);
        js_transport_send_response(info, request, body);
    }
    else if (strcmp("stackTrace", command) == 0) {
        JSValue stack_trace = js_debugger_build_backtrace(ctx, state->cur_pc);
        js_transport_send_response(info, request, stack_trace);
    }
    else if (strcmp("scopes", command) == 0) {
        JSValue args = JS_GetPropertyStr(dctx, request, "args");
        int frame = js_debugger_get_frame(dctx, args);
        JS_FreeValue(dctx, args);
        JSValue scopes = js_get_scopes(dctx, frame);
        js_transport_send_response(info, request, scopes);
    }
    else if (strcmp("variables", command) == 0) {
        JSValue args = JS_GetPropertyStr(dctx, request, "args");
        JSValue reference_property = JS_GetPropertyStr(dctx, args, "variablesReference");
        uint32_t reference;
        JS_ToUint32(dctx, &reference, reference_property);
        JS_FreeValue(dctx, reference_property);

        JSValue properties = JS_NewArray(dctx);
        JSValue variable = JS_GetPropertyUint32(dctx, state->variable_references, reference);

        int skip_proto = 0;
        if (JS_IsUndefined(variable)) {
            skip_proto = 1;
            int frame = reference >> 2;
            int scope = reference % 4;

            assert(frame < (int)js_debugger_stack_depth(ctx));

            if (scope == 0)
                variable = JS_GetGlobalObject(ctx);
            else if (scope == 1)
                variable = js_debugger_local_variables(ctx, frame);
            else if (scope == 2)
                variable = js_debugger_closure_variables(ctx, frame);
            else
                assert(0);

            JS_SetPropertyUint32(dctx, state->variable_references, reference, JS_DupValue(ctx, variable));
        }

        JSPropertyEnum *tab_atom;
        uint32_t tab_atom_count;

        JSValue filter = JS_GetPropertyStr(dctx, args, "filter");
        if (!JS_IsUndefined(filter)) {
            const char *filter_str = JS_ToCString(dctx, filter);
            JS_FreeValue(dctx, filter);
            int indexed = strcmp(filter_str, "indexed") == 0;
            JS_FreeCString(dctx, filter_str);
            if (!indexed)
                goto unfiltered;

            uint32_t start = js_get_property_as_uint32(dctx, args, "start");
            uint32_t count = js_get_property_as_uint32(dctx, args, "count");

            char name_buf[64];
            for (uint32_t i = 0; i < count; i++) {
                JSValue value = JS_GetPropertyUint32(ctx, variable, start + i);
                sprintf(name_buf, "%d", i);
                JSValue variable_json = js_debugger_get_variable(dctx, state, JS_NewString(dctx, name_buf), value);
                JS_FreeValue(ctx, value);
                JS_SetPropertyUint32(dctx, properties, i, variable_json);
            }
            goto done;
        }

    unfiltered:

        if (!JS_GetOwnPropertyNames(ctx, &tab_atom, &tab_atom_count, variable,
            JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK)) {

            int offset = 0;

            if (!skip_proto) {
                const JSValue proto = JS_GetPrototype(ctx, variable);
                if (!JS_IsException(proto)) {
                    JSValue variable_json = js_debugger_get_variable(dctx, state, JS_NewString(dctx, "__proto__"), proto);
                    JS_FreeValue(ctx, proto);
                    JS_SetPropertyUint32(dctx, properties, offset++, variable_json);
                }
                else {
                    JS_FreeValue(ctx, proto);
                }
            }

            for(uint32_t i = 0; i < tab_atom_count; i++) {
                JSValue value = JS_GetProperty(ctx, variable, tab_atom[i].atom);
                JSValue variable_json = js_debugger_get_variable(dctx, state, JS_AtomToString(ctx, tab_atom[i].atom), value);
                JS_FreeValue(ctx, value);
                JS_SetPropertyUint32(dctx, properties, i + offset, variable_json);
            }

            js_free_prop_enum(ctx, tab_atom, tab_atom_count);
        }

    done:
        JS_FreeValue(ctx, variable);
        JS_FreeValue(dctx, args);
        js_transport_send_response(info, request, properties);
    }
    JS_FreeCString(dctx, command);
    JS_FreeValue(dctx, command_property);
    JS_FreeValue(dctx, request);
}

static void js_process_breakpoints(JSDebuggerInfo *info, JSValue message) {
    /* breakpoints are stored in debugging_ctx */
    JSContext *ctx = info->debugging_ctx;

    info->breakpoints_dirty_counter++;

    JSValue path_property = JS_GetPropertyStr(ctx, message, "path");
    const char *path = JS_ToCString(ctx, path_property);
    DBG_LOG("js_process_breakpoints: path='%s' dirty=%u", path ? path : "(null)", info->breakpoints_dirty_counter);
    JSValue path_data = JS_GetPropertyStr(ctx, info->breakpoints, path);

    if (!JS_IsUndefined(path_data))
        JS_FreeValue(ctx, path_data);
    path_data = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, info->breakpoints, path, path_data);
    JS_FreeCString(ctx, path);
    JS_FreeValue(ctx, path_property);

    JSValue breakpoints = JS_GetPropertyStr(ctx, message, "breakpoints");
    JS_SetPropertyStr(ctx, path_data, "breakpoints", breakpoints);
    JS_SetPropertyStr(ctx, path_data, "dirty", JS_NewInt32(ctx, info->breakpoints_dirty_counter));

    JS_FreeValue(ctx, message);
}

JSValue js_debugger_file_breakpoints(JSContext *ctx, const char* path) {
    JSDebuggerInfo *info = js_debugger_info(JS_GetRuntime(ctx));
    /* breakpoints are stored in debugging_ctx */
    JSValue path_data = JS_GetPropertyStr(info->debugging_ctx, info->breakpoints, path);
    return path_data;
}

static int js_process_debugger_messages(JSDebuggerInfo *info, const uint8_t *cur_pc) {
    /* Use debugging_ctx for JSON parsing/serialization of debugger protocol.
       info->ctx is the debuggee context (for stack inspection, eval, etc.),
       but may be NULL when called from js_debugger_attach before execution starts. */
    JSContext *ctx = info->debugging_ctx;
    JSRuntime *rt = JS_GetRuntime(ctx);
    struct DebuggerSuspendedState state;
    state.variable_reference_count = info->ctx ? js_debugger_stack_depth(info->ctx) << 2 : 0;
    state.variable_pointers = JS_NewObject(ctx);
    state.variable_references = JS_NewObject(ctx);
    state.cur_pc = cur_pc;
    int ret = 0;
    char message_length_buf[10];

    do {
        fflush(stdout);
        fflush(stderr);

        if (!js_transport_read_fully(info, message_length_buf, 9))
            goto done;

        message_length_buf[8] = '\0';
        int message_length = (int)strtol(message_length_buf, NULL, 16);
        assert(message_length > 0);
        if (message_length > info->message_buffer_length) {
            if (info->message_buffer) {
                js_free_rt(rt, info->message_buffer);
                info->message_buffer = NULL;
                info->message_buffer_length = 0;
            }
            info->message_buffer = (char *)js_malloc_rt(rt, message_length + 1);
            info->message_buffer_length = message_length;
        }

        if (!js_transport_read_fully(info, info->message_buffer, message_length))
            goto done;

        info->message_buffer[message_length] = '\0';

        DBG_LOG("recv msg (%d bytes): %.200s", message_length, info->message_buffer);

        JSValue message = JS_ParseJSON(ctx, info->message_buffer, message_length, "<debugger>");
        if (JS_IsException(message)) {
            DBG_LOG("ERROR: JS_ParseJSON failed!");
            JS_FreeValue(ctx, JS_GetException(ctx));
            goto done;
        }
        JSValue vtype = JS_GetPropertyStr(ctx, message, "type");
        const char *type = JS_ToCString(ctx, vtype);
        DBG_LOG("  type='%s' is_paused=%d", type ? type : "(null)", info->is_paused);
        if (type && strcmp("request", type) == 0) {
            js_process_request(info, &state, JS_GetPropertyStr(ctx, message, "request"));
        }
        else if (type && strcmp("continue", type) == 0) {
            DBG_LOG("  -> continue, unpausing");
            info->is_paused = 0;
        }
        else if (type && strcmp("breakpoints", type) == 0) {
            DBG_LOG("  -> breakpoints message");
            js_process_breakpoints(info, JS_GetPropertyStr(ctx, message, "breakpoints"));
        }
        else if (type && strcmp("stopOnException", type) == 0) {
            JSValue stop = JS_GetPropertyStr(ctx, message, "stopOnException");
            info->exception_breakpoint = JS_ToBool(ctx, stop);
            JS_FreeValue(ctx, stop);
        }
        else {
            DBG_LOG("  -> UNKNOWN message type!");
        }

        if (type) JS_FreeCString(ctx, type);
        JS_FreeValue(ctx, vtype);
        JS_FreeValue(ctx, message);
    }
    while (info->is_paused);

    ret = 1;

done:
    JS_FreeValue(ctx, state.variable_references);
    JS_FreeValue(ctx, state.variable_pointers);
    return ret;
}

void js_debugger_exception(JSContext *ctx) {
    JSDebuggerInfo *info = js_debugger_info(JS_GetRuntime(ctx));
    if (!info->exception_breakpoint)
        return;
    if (info->is_debugging)
        return;
    info->is_debugging = 1;
    info->ctx = ctx;
    js_send_stopped_event(info, "exception");
    info->is_paused = 1;
    js_process_debugger_messages(info, NULL);
    info->is_debugging = 0;
    info->ctx = NULL;
}

static void js_debugger_context_event(JSContext *caller_ctx, const char *reason) {
    if (!js_debugger_is_transport_connected(JS_GetRuntime(caller_ctx)))
        return;

    JSDebuggerInfo *info = js_debugger_info(JS_GetRuntime(caller_ctx));
    if (!info->debugging_ctx)
        return;
    if (info->debugging_ctx == caller_ctx)
        return;

    JSContext *ctx = info->debugging_ctx;

    JSValue event = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, event, "type", JS_NewString(ctx, "ThreadEvent"));
    JS_SetPropertyStr(ctx, event, "reason", JS_NewString(ctx, reason));
    JS_SetPropertyStr(ctx, event, "thread", JS_NewInt64(ctx, (int64_t)caller_ctx));
    js_transport_send_event(info, event);
}

void js_debugger_new_context(JSContext *ctx) {
    js_debugger_context_event(ctx, "new");
}

void js_debugger_free_context(JSContext *ctx) {
    js_debugger_context_event(ctx, "exited");
}

void js_debugger_check(JSContext* ctx, const uint8_t *cur_pc) {
    JSDebuggerInfo *info = js_debugger_info(JS_GetRuntime(ctx));
    if (info->is_debugging)
        return;
    if (info->debugging_ctx == ctx)
        return;
    info->is_debugging = 1;
    info->ctx = ctx;

    if (!info->attempted_connect) {
        info->attempted_connect = 1;
        char *address = getenv("QUICKJS_DEBUG_ADDRESS");
        if (address != NULL && !info->transport_close)
            js_debugger_connect(ctx, address);
    }
    else if (!info->attempted_wait) {
        info->attempted_wait = 1;
        char *address = getenv("QUICKJS_DEBUG_LISTEN_ADDRESS");
        if (address != NULL && !info->transport_close)
            js_debugger_wait_connection(ctx, address);
    }

    if (info->transport_close == NULL)
        goto done;

    struct JSDebuggerLocation location;
    int depth;
    (void)location;
    (void)depth;

    if (info->stepping) {
        location = js_debugger_current_location(ctx, cur_pc);
        depth = js_debugger_stack_depth(ctx);
        if (info->step_depth == depth
            && location.filename == info->step_over.filename
            && location.line == info->step_over.line
            && location.column == info->step_over.column)
            goto done;
    }

    int at_breakpoint = js_debugger_check_breakpoint(ctx, info->breakpoints_dirty_counter, cur_pc);
    if (at_breakpoint) {
        DBG_LOG("BREAKPOINT HIT! dirty=%u", info->breakpoints_dirty_counter);
        info->stepping = 0;
        info->is_paused = 1;
        js_send_stopped_event(info, "breakpoint");
    }
    else if (info->stepping) {
        if (info->stepping == JS_DEBUGGER_STEP_CONTINUE) {
            info->stepping = 0;
        }
        else if (info->stepping == JS_DEBUGGER_STEP_IN) {
            int depth = js_debugger_stack_depth(ctx);
            if (info->step_depth == depth) {
                struct JSDebuggerLocation location = js_debugger_current_location(ctx, cur_pc);
                if (location.filename == info->step_over.filename
                    && location.line == info->step_over.line
                    && location.column == info->step_over.column)
                    goto done;
            }
            info->stepping = 0;
            info->is_paused = 1;
            js_send_stopped_event(info, "stepIn");
        }
        else if (info->stepping == JS_DEBUGGER_STEP_OUT) {
            int depth = js_debugger_stack_depth(ctx);
            if (depth >= info->step_depth)
                goto done;
            info->stepping = 0;
            info->is_paused = 1;
            js_send_stopped_event(info, "stepOut");
        }
        else if (info->stepping == JS_DEBUGGER_STEP) {
            struct JSDebuggerLocation location = js_debugger_current_location(ctx, cur_pc);
            if ((location.filename == info->step_over.filename
                && location.line == info->step_over.line
                && location.column == info->step_over.column)
                || (int)js_debugger_stack_depth(ctx) > info->step_depth)
                goto done;
            info->stepping = 0;
            info->is_paused = 1;
            js_send_stopped_event(info, "step");
        }
        else {
            info->stepping = 0;
        }
    }

    if (!info->is_paused) {
        if (info->peek_ticks++ < 10000 && !info->should_peek)
            goto done;

        info->peek_ticks = 0;
        info->should_peek = 0;

        while (!info->is_paused) {
            int peek = (int)info->transport_peek(info->transport_udata);
            if (peek < 0)
                goto fail;
            if (peek == 0)
                goto done;
            if (!js_process_debugger_messages(info, cur_pc))
                goto fail;
        }
    }

    if (js_process_debugger_messages(info, cur_pc))
        goto done;

    fail:
        js_debugger_free(JS_GetRuntime(ctx), info);
    done:
        info->is_debugging = 0;
        info->ctx = NULL;
}

void js_debugger_free(JSRuntime *rt, JSDebuggerInfo *info) {
    if (!info->transport_close)
        return;

    const char* terminated = "{\"type\":\"event\",\"event\":{\"type\":\"terminated\"}}";
    js_transport_write_message_newline(info, terminated, strlen(terminated));

    info->transport_close(rt, info->transport_udata);

    info->transport_read = NULL;
    info->transport_write = NULL;
    info->transport_peek = NULL;
    info->transport_close = NULL;

    if (info->message_buffer) {
        js_free_rt(rt, info->message_buffer);
        info->message_buffer = NULL;
        info->message_buffer_length = 0;
    }

    JS_FreeValue(info->debugging_ctx, info->breakpoints);
    JS_FreeContext(info->debugging_ctx);
    info->debugging_ctx = NULL;
}

void js_debugger_attach(
    JSContext* ctx,
    size_t (*transport_read)(void *udata, char* buffer, size_t length),
    size_t (*transport_write)(void *udata, const char* buffer, size_t length),
    size_t (*transport_peek)(void *udata),
    void (*transport_close)(JSRuntime* rt, void *udata),
    void *udata
) {
    JSRuntime *rt = JS_GetRuntime(ctx);
    JSDebuggerInfo *info = js_debugger_info(rt);
    js_debugger_free(rt, info);

    info->debugging_ctx = JS_NewContext(rt);
    info->transport_read = transport_read;
    info->transport_write = transport_write;
    info->transport_peek = transport_peek;
    info->transport_close = transport_close;
    info->transport_udata = udata;

    JSContext *original_ctx = info->ctx;
    info->ctx = ctx;

    DBG_LOG("js_debugger_attach: sending entry event, ctx=%p, debugging_ctx=%p", (void*)ctx, (void*)info->debugging_ctx);
    js_send_stopped_event(info, "entry");

    info->breakpoints = JS_NewObject(info->debugging_ctx);
    info->is_paused = 1;

    DBG_LOG("js_debugger_attach: entering message loop (is_paused=%d)", info->is_paused);
    js_process_debugger_messages(info, NULL);
    DBG_LOG("js_debugger_attach: message loop exited (is_paused=%d)", info->is_paused);

    info->ctx = original_ctx;
}

int js_debugger_is_transport_connected(JSRuntime *rt) {
    return js_debugger_info(rt)->transport_close != NULL;
}

void js_debugger_cooperate(JSContext *ctx) {
    js_debugger_info(JS_GetRuntime(ctx))->should_peek = 1;
}
