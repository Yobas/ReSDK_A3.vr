#include "sqf_bindings.hpp"
#include "type_conversion.hpp"
#include "js_engine.hpp"
#include "intercept.hpp"
#include <string>
#include <sstream>

using namespace intercept;
using namespace intercept::types;

namespace js_host {

// ============================================================================
// rvengine.player() -> GameValue
// Returns the local player object.
// ============================================================================

static JSValue js_player(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    try {
        client::invoker_lock lock;
        game_value* gv = new game_value(sqf::player());
        return wrap_game_value(ctx, gv);
    } catch (const std::exception& e) {
        return JS_ThrowInternalError(ctx, "player: %s", e.what());
    }
}

// ============================================================================
// rvengine.getPos(obj) -> [x, y, z]
// Returns position of the given object as a JS array.
// ============================================================================

static JSValue js_get_pos(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1)
        return JS_ThrowTypeError(ctx, "getPos: expected 1 argument (GameValue)");

    game_value* obj = unwrap_game_value(ctx, argv[0]);
    if (!obj)
        return JS_ThrowTypeError(ctx, "getPos: argument is not a GameValue");

    try {
        client::invoker_lock lock;
        game_value result = sqf::get_pos(static_cast<object>(*obj));

        auto& arr = result.to_array();
        if (arr.size() >= 3) {
            JSValue js_arr = JS_NewArray(ctx);
            JS_SetPropertyUint32(ctx, js_arr, 0, JS_NewFloat64(ctx, static_cast<float>(arr[0])));
            JS_SetPropertyUint32(ctx, js_arr, 1, JS_NewFloat64(ctx, static_cast<float>(arr[1])));
            JS_SetPropertyUint32(ctx, js_arr, 2, JS_NewFloat64(ctx, static_cast<float>(arr[2])));
            return js_arr;
        }

        return JS_NewArray(ctx);
    } catch (const std::exception& e) {
        return JS_ThrowInternalError(ctx, "getPos: %s", e.what());
    }
}

// ============================================================================
// console.log(...args) -> void
// Prints a message to the Arma 3 RPT log.
// ============================================================================

static JSValue js_console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::ostringstream oss;
    for (int i = 0; i < argc; i++) {
        if (i > 0) oss << " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            oss << str;
            JS_FreeCString(ctx, str);
        } else {
            oss << "[unconvertible]";
        }
    }
    JsEngine::log_info(oss.str());
    return JS_UNDEFINED;
}

// ============================================================================
// Registration
// ============================================================================

void register_sqf_bindings(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);

    // rvengine namespace object
    JSValue rvengine = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, rvengine, "player",
        JS_NewCFunction(ctx, js_player, "player", 0));
    JS_SetPropertyStr(ctx, rvengine, "getPos",
        JS_NewCFunction(ctx, js_get_pos, "getPos", 1));
    JS_SetPropertyStr(ctx, global, "rvengine", rvengine);

    // console object
    JSValue console = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, console, "log",
        JS_NewCFunction(ctx, js_console_log, "log", 1));
    JS_SetPropertyStr(ctx, global, "console", console);

    JS_FreeValue(ctx, global);
}

} // namespace js_host
