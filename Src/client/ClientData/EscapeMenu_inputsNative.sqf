// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include "..\..\host\engine.hpp"
#include "..\..\host\oop.hpp"
#include "..\..\host\text.hpp"
#include "..\..\host\keyboard.hpp"

_function = {
	(findDisplay 313) createDisplay  "RscDisplayInterrupt"; ctrlActivate (findDisplay 49 displayCtrl 303);

	_d= findDisplay 4; //configure action display 131

	(_d displayctrl 106) ctrlshow false; //gamepad
	(_d displayctrl 4302) ctrlshow false; //cbaconfigaddons


	private _cpage = (_d displayctrl 108);//controlpage
	private _inputlist = (_d displayCtrl 102);
	private _delCat = [];
	// for "_i" from 0 to (lbSize _cpage) - 1 do {
	// 	private _dataName = _cpage lbdata _i;
	// 	traceformat("data %1: %2",_i arg _dataName)
	// 	if (_dataName in [
	// 		"Common",
	// 		"Weapons",
	// 		"View",
	// 		"Command",
	// 		"Multiplayer",
	// 		"Vehicle Movement",
	// 		"Helicopter Movement",
	// 		"Plane Movement",
	// 		"Submarine Movement",
	// 		"Development",
	// 		"Zeus",
	// 		"Custom controls",
	// 		"Editor Camera",
	// 		"Camera"
	// 	]) then {
	// 		_cpage lbSettext [_i,"DISABLED"];
	// 		_cpage lbsetdata [_i,"DISABLED"];
	// 		_delCat pushBack _i;
	// 	};
	// };


	_cpage setvariable ["_inputlist",_inputlist];
	_cpage ctrlAddEventHandler ["LBSelChanged",{
		params ["_list","_idx"];
		(_list getvariable "_inputlist") ctrlshow ((_list lbdata _idx) != "DISABLED");
	}];


	{
		_x ctrlshow false;
		_x ctrlsetfade 1;
		_x ctrlcommit 0;
	} foreach [_cpage,(_d displayctrl 1002),(_d displayctrl 114)];

	[_inputlist,_cpage] spawn {
	params ["_inputlist","_cpage"];
	uiSleep 0.001;
	_cpage lbSetCurSel 5;
	//_inputlist lbSetCurSel 6; //change action
	_allow = [0,1,2,3,6,11,14,15,16,17,18,22,23,32];
	//reverse _allow;
	for "_i" from (lbsize _inputlist) - 1 to 0 step -1 do {
		if !(_i in _allow) then {
			//_inputlist lbdelete  _i;
		} else {
			traceformat("input: %1",_inputlist lbtext _i)
		}
	};
	traceformat("%1",_inputlist lbtext 0)
	};

	{
	_x ctrlSetTooltip (format["%1: %2",ctrlClassName _x,ctrlidc _x])
	} foreach (allControls _d)
};

if !isNull(_debug_call) then {
	call _function;
};