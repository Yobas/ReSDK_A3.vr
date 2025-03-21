// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

sp_audio_sayPlayer = {
	params ["_pathPost"];
	if (sp_debug_skipAudio) exitWith {-1};

	["singleplayer\sp_guide\" + _pathPost] call soundUI_play;
};

sp_audio_sayPlayerList = {
	private _pathlist = _this;
	if (sp_debug_skipAudio) exitWith {scriptNull};
	
	[{
		params ["_pathlist"];
		{
			private _h = [_x] call sp_audio_sayPlayer;
			{count soundParams _h == 0} call sp_threadWait;
		} foreach _pathlist;
	},[_pathlist]] call sp_threadStart;
	
};