#include "type_conversion.hpp"
#include <string>

namespace js_host {

static JSClassID js_game_value_class_id = 0;

// ============================================================================
// Finalizer — releases the heap-allocated game_value
// ============================================================================

static void js_game_value_finalizer(JSRuntime* rt, JSValueConst val) {
    game_value* gv = static_cast<game_value*>(JS_GetOpaque(val, js_game_value_class_id));
    delete gv;
}

static JSClassDef js_game_value_class_def = {
    "GameValue",
    js_game_value_finalizer,
    nullptr, // gc_mark
    nullptr, // call
    nullptr, // exotic
};

// ============================================================================
// Prototype methods
// ============================================================================

static JSValue js_game_value_is_nil(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    game_value* gv = static_cast<game_value*>(JS_GetOpaque2(ctx, this_val, js_game_value_class_id));
    if (!gv)
        return JS_ThrowTypeError(ctx, "GameValue: invalid object");
    return JS_NewBool(ctx, gv->is_nil());
}

static JSValue js_game_value_to_string(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    game_value* gv = static_cast<game_value*>(JS_GetOpaque2(ctx, this_val, js_game_value_class_id));
    if (!gv)
        return JS_ThrowTypeError(ctx, "GameValue: invalid object");

    if (gv->is_nil())
        return JS_NewString(ctx, "GameValue(nil)");

    try {
        std::string str = "GameValue(" + static_cast<std::string>(*gv) + ")";
        return JS_NewString(ctx, str.c_str());
    } catch (...) {
        return JS_NewString(ctx, "GameValue(<unconvertible>)");
    }
}

static JSValue js_game_value_to_number(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    game_value* gv = static_cast<game_value*>(JS_GetOpaque2(ctx, this_val, js_game_value_class_id));
    if (!gv)
        return JS_ThrowTypeError(ctx, "GameValue: invalid object");

    if (gv->is_nil())
        return JS_NewFloat64(ctx, 0.0);

    try {
        return JS_NewFloat64(ctx, static_cast<float>(*gv));
    } catch (...) {
        return JS_NewFloat64(ctx, 0.0);
    }
}

// ============================================================================
// Setup
// ============================================================================

void setup_game_value_class(JSRuntime* rt, JSContext* ctx) {
    JS_NewClassID(rt, &js_game_value_class_id);
    JS_NewClass(rt, js_game_value_class_id, &js_game_value_class_def);

    // Create prototype and register methods manually (C++-compatible)
    JSValue proto = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, proto, "isNil",
        JS_NewCFunction(ctx, js_game_value_is_nil, "isNil", 0));
    JS_SetPropertyStr(ctx, proto, "toString",
        JS_NewCFunction(ctx, js_game_value_to_string, "toString", 0));
    JS_SetPropertyStr(ctx, proto, "toNumber",
        JS_NewCFunction(ctx, js_game_value_to_number, "toNumber", 0));

    JS_SetClassProto(ctx, js_game_value_class_id, proto);
}

// ============================================================================
// Wrap / Unwrap
// ============================================================================

JSValue wrap_game_value(JSContext* ctx, game_value* gv) {
    JSValue obj = JS_NewObjectClass(ctx, js_game_value_class_id);
    if (JS_IsException(obj)) {
        delete gv;
        return obj;
    }
    JS_SetOpaque(obj, gv);
    return obj;
}

game_value* unwrap_game_value(JSContext* ctx, JSValueConst val) {
    return static_cast<game_value*>(JS_GetOpaque2(ctx, val, js_game_value_class_id));
}

} // namespace js_host
