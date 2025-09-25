// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\WidgetSystem\widgets.hpp>
#include <..\..\InputSystem\inputKeyHandlers.hpp>

macro_const(vs_changer_border_size_x)
#define voice_changer_border_size_x 1
macro_const(vs_changer_border_size_y)
#define voice_changer_border_size_y 15

macro_const(vs_changer_size_h)
#define voice_changer_size_h 7
macro_const(vs_changer_size_w)
#define voice_changer_size_w 20

macro_const(vs_changer_bias_x)
#define voice_changer_bias_x 1
macro_const(vs_changer_bias_y)
#define voice_changer_bias_y 1

inline_macro
#define vec4(a,b,c,d) [a,b,c,d]

decl(widget[])
vs_voiceVolumeWidgets = [];

macro_func(vs_getWidgetText,widget())
#define vs_getWidgetText (vs_voiceVolumeWidgets select 0)
macro_func(vs_getWidgetProgress,widget())
#define vs_getWidgetProgress (vs_voiceVolumeWidgets select 1)
macro_func(vs_getWidgetGroup,widget())
#define vs_getWidgetGroup (vs_voiceVolumeWidgets select 3)

//время после которого тухнет диспл
macro_const(vs_voiceDelayFadein)
#define vs_voiceDelayFadein 4

//сколько за кадр будет тухнуть хуйня
macro_const(vs_voiceAmountFade)
#define vs_voiceAmountFade 0.025

decl(int)
vs_curVoiceMode = 0;

decl(bool)
vs_canFadeVoiceVolumeWidget = true;

decl(float)
vs_voiceVolume_lastUpdate = 0;

decl(any[])
vs_voiceVolumeList = [
	[vec4(0.212,0.659,0.878,0.3),"Шепот",4],
	[vec4(0.063,0.294,0.663,0.35),"Тихий",9],
	[vec4(0.271,0.027,0.941,0.5),"Обычно",18],
	[vec4(0.698,0.173,0.831,0.6),"Громко",35],
	[vec4(0.969,0,0.275,0.85),"ОРАТЬ",60]
];

decl(float())
vs_getMaxVolume = {
	array_selectlast(vs_voiceVolumeList) select 2
};

//Изменяет громкость разговора
decl(void(int))
vs_changeVoiceVolume = {
	params ["_mode"];
	
	private _countModes = count vs_voiceVolumeList;
	
	if (_mode > (_countModes - 1) || _mode < 0 || vs_curVoiceMode == _mode) exitWith {};
	
	(vs_voiceVolumeList select _mode) params ["_enumColor","_enumTxt","_enumVol"];
	
	_txt = vs_getWidgetText;
	[_txt,format["<t align='center'>%1</t>",_enumTxt]] call widgetSetText;
	_txt setBackgroundColor _enumColor;
	
	private _maxX = 100 - voice_changer_border_size_x * 2;
	
	[vs_getWidgetProgress,[
        voice_changer_border_size_x,
        voice_changer_border_size_y + 50,
        (_mode) / (_countModes-1) * _maxX,
        50 - voice_changer_border_size_y * 2
	]] call widgetSetPosition;
	
	vs_curVoiceMode = _mode;
	vs_voiceVolume_lastUpdate = tickTime + vs_voiceDelayFadein;
	
    [_enumVol] call vs_setLocalPlayerVoiceDistance;
};

decl(void())
vs_voiceVolumeOnUpdate = {
	if (!vs_canFadeVoiceVolumeWidget) exitWith {};
	
	_ctg = vs_getWidgetGroup;
	_ctgVision = _ctg getvariable "vision";
	
	if (tickTime > vs_voiceVolume_lastUpdate) then {

		if (_ctgVision < 1) then {
			MOD(_ctgVision, + vs_voiceAmountFade);
			_ctg setFade _ctgVision;
			_ctg commit 0;
			_ctg setVariable ["vision",_ctgVision]
		};	
	} else {
		if (_ctgVision != 0) then {
			_ctg setFade 0;
			_ctg commit 0;
			_ctg setVariable ["vision",0];
		};
	};
};

vs_internal_statusWidgets = [];

decl(void())
vs_initChangeVoiceCtrl = {
	_gui = getGUI;

	//init debug info
	_wgi = [_gui,WIDGETGROUP,[95,98,5,2]] call createWidget;
	_speak = [_gui,BACKGROUND,[0,0,50,100],_wgi] call createWidget;
	[_speak,"<t align='center' size='0.4'>mic</t>"] call widgetSetText;
	_online = [_gui,BACKGROUND,[50,0,50,100],_wgi] call createWidget;
	[_online,"<t align='center' size='0.4'>on</t>"] call widgetSetText;
	vs_internal_statusWidgets = [_wgi,_speak,_online];
	_upd = {
		_speak = vs_internal_statusWidgets select 1;
		_online = vs_internal_statusWidgets select 2;
		if (call vs_isSpeaking) then {
			_speak setBackgroundColor [1,0,0,1];
		} else {
			_speak setBackgroundColor [0.3,0,0,0.5];
		};
		if (call vs_isConnectedVoice) then {
			_online setBackgroundColor [0,1,0,1];
		} else {
			_online setBackgroundColor [0,0.3,0,0.5];
		};
		
	}; startUpdate(_upd,0);
	
	_wg = [_gui,WIDGETGROUP,[
		100 - voice_changer_size_w - voice_changer_bias_x,
		100 - voice_changer_size_h - voice_changer_bias_y,
		voice_changer_size_w,
		voice_changer_size_h
		]
	] call createWidget;
	_wg setVariable ["isVisible",false];
	_wg setvariable ["vision",0];
	//_wg setFade 1;
	//_wg commit 0;
	
	_txt = [_gui,TEXT,[
		voice_changer_border_size_x,
		voice_changer_border_size_y / 2,
		100 - voice_changer_border_size_x * 2,
		50
	],_wg] call createWidget;
	[_txt,"<t align='center'>Шепот</t>"] call widgetSetText;
	_txt setBackgroundColor [1,0,0,1];
	
	_bg = [_gui,BACKGROUND,WIDGET_FULLSIZE,_wg] call createWidget; 
	_bg setBackgroundColor [0.3,0.3,0.3,0.5];

	vs_voiceVolumeHandleUpdate = startUpdate(vs_voiceVolumeOnUpdate,0);
	
	_bg3 = [_gui,BACKGROUND,[
		voice_changer_border_size_x,
		voice_changer_border_size_y + 50,
		100 - voice_changer_border_size_x * 2,
		50 - voice_changer_border_size_y * 2
	],_wg] call createWidget;
	_bg3 setBackgroundColor [0.6,0.6,0.6,1];
	//_bg3 ctrlEnable true;
	
	vs_voiceVolumeWidgets = [_txt,_bg3,_bg,_wg];
	
	[2] call vs_changeVoiceVolume;
	
	(findDisplay 46) displayAddEventHandler ["MouseZChanged",{
		_value = _this select 1;
		
		//disable voice change on craft preview mode
		if (craft_isPreviewEnabled) exitWith {};

		if (_value > 0) then {
			[vs_curVoiceMode + 1] call vs_changeVoiceVolume;
		} else {
			[vs_curVoiceMode - 1] call vs_changeVoiceVolume;
		};
	}];
	
	// tranmith in game display
	[findDisplay 46] call vs_addDisplayInputHandlers;
	
};

vs_addDisplayInputHandlers = {
	params ["_d"];
	_d displayAddEventHandler ["KeyDown",{
		doPrepareKeyData(_this); 
		if isPressed(input_act_radio) then {[true] call vs_handleTransmith; null};
		if isPressed(input_act_voice) then {[true] call vs_handleSpeak; null};
		null
	}];
	_d displayAddEventHandler ["KeyUp",{
		doPrepareKeyData(_this); 
		if isPressed(input_act_radio) then {[false] call vs_handleTransmith; null};
		if isPressed(input_act_voice) then {[false] call vs_handleSpeak; null};
		null
	}];
};

#ifndef REDITOR_VOICE_DEBUG
	call vs_initChangeVoiceCtrl;
#endif