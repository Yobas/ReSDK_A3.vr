#pragma once
#include "intercept.hpp"
#include "quickjs.h"

using namespace intercept::types;

namespace js_host {

// Initialize GameValue JS class (call once after creating runtime+context)
void setup_game_value_class(JSRuntime* rt, JSContext* ctx);

// Wrap a heap-allocated game_value* into a JS GameValue object (takes ownership)
JSValue wrap_game_value(JSContext* ctx, game_value* gv);

// Extract game_value* from a JS GameValue object (borrowed pointer, do not delete)
game_value* unwrap_game_value(JSContext* ctx, JSValueConst val);

} // namespace js_host
