// ======================================================
// Copyright (c) 2017-2023 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\engine.hpp>
#include <..\client_compiled.hpp>

#ifdef _SQFVM
	#define importCommon(path) cmplog("common " + path); if (isNil {allClientContents}) then {allClientContents = [];}; \
	private _ctx = compile __pragma_prep_cli ("src\host\CommonComponents\" + path); \
	allClientContents pushback _ctx;
#endif

#ifdef __VM_PARSE_FILE
	#define importCommon(path) diag_log format["Start loading common module %1",path]; \
	private _ctx = compile preprocessFileLineNumberS ("src\host\CommonComponents\" + path); \
	diag_log format["   - Module %1 loaded",path];
#endif

importCommon("!PreInit.sqf");
importCommon("bitflags.sqf");
importCommon("Animator.sqf");
importCommon("Color.sqf");
importCommon("Gamemode.sqf");
importCommon("ModelsPath.sqf");
importCommon("Voice.sqf");
importCommon("CombatMode.sqf");
importCommon("SMD_shared.sqf");
importCommon("SoundEngine.sqf");
importCommon("Craft.sqf");
importCommon("DateTime.sqf");
importCommon("Replicator.sqf");
importCommon("AttackTypesAssoc.sqf");
importCommon("Pencfg.sqf");