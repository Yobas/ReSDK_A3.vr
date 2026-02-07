#pragma once
#include "quickjs.h"

namespace js_host {

// Register SQF bindings (rvengine.player, rvengine.getPos, console.log)
// on the global object of the given context.
void register_sqf_bindings(JSContext* ctx);

} // namespace js_host
