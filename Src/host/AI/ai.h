// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#define AI_DEBUG_TRACEPATH
#define AI_DEBUG_BRAINIINFO
// #define AI_DEBUG_MOVETOPLAYER

#define toActor(mob) getVar(mob,owner)

// Brain action update states
#define UPDATE_STATE_CONTINUE 0
#define UPDATE_STATE_COMPLETED 1
#define UPDATE_STATE_FAILED -1

#ifdef EDITOR
	ai_reloadThis = {
		call compile preprocessfilelinenumbers "src\host\AI\ai_init.sqf";
	};
#else
	#undef AI_DEBUG_TRACEPATH
	#undef AI_DEBUG_BRAINIINFO
	#undef AI_ENABLE_DEBUG_LOG
	#undef AI_DEBUG_MOVETOPLAYER
#endif