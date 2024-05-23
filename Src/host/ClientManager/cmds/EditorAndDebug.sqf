// ======================================================
// Copyright (c) 2017-2024 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#ifdef DEBUG
	
	addCommand("showthreads",PUBLIC_COMMAND)
	{
		_t = format["Active %1 threads: ",count cba_common_perFrameHandlerArray];
		{
			_delay = _x select 1;
			_startedAt = _x select 3;
			_threadLocation = _x select 6;
			// _threadStacktrace = +(_x select 7);
			// reverse _threadStacktrace;
			// _ist = _threadStacktrace findif {_x select 0 != ""};
			// if (_ist == -1) then {
			// 	_threadStacktrace = endl + "Unknown stacktrace region";
			// } else {
			// 	_tlist = (_threadStacktrace select [0,_ist+1]) apply {"		"+(_x call scriptError_internal_handleStack_short)};
			// 	_tlist = _tlist - [""];
			// 	_threadStacktrace = endl + ((_tlist) joinString endl);
			// };
			// _threadStacktrace = endl + ((_threadStacktrace apply {"	" + (_x call scriptError_internal_handleStack_short)}) joinString endl);
			_threadStacktrace = ""; // not implemented now...

			_code = (toString (_x select 0)) splitString ";";
			
			_threadStacktrace = [_threadStacktrace,endl,sbr] call stringReplace;

			if (count _code > 0 && {[_code select 0,"private ___fn___ = ",false] call stringStartWith}) then {
				_code = _code select 0;
				_code = _code select [count "private ___fn___ = ",count _code];

				modvar(_t) + sbr + (format["<t color='#00ff00'>%1</t> (per %2s, started %3s ago): %4%5",_code,_delay,tickTime - _startedAt,_threadLocation,_threadStacktrace]);
			} else {
				modvar(_t) + sbr + (format["<t color='#00ff00'>Unk thread %1</t> (per %4s, started %5s ago): %2%3",_x select 5,_threadLocation,_threadStacktrace,_delay,tickTime - _startedAt]);
			};
		} foreach cba_common_perFrameHandlerArray;

		callFuncParams(thisClient,ShowMessageBox,"Text" arg _t);
	};

	addCommand("jumpclass",PUBLIC_COMMAND)
	{
		private _item = ["target",""] call oop_getdata select 0;
		if isNullReference(_item) exitWith {
			callSelfParams(localSay,"Нет объекта" arg "log");
		};
		typeGetVar(typeGetFromObject(_item),__decl_info__) params ["_file","_line"];
		["WorkspaceHelper","gotoclass",[_file,_line],true] call rescript_callCommand;
	};

	addCommandWithDescription("spawncamp",ACCESS_PLAYER,"Спавнит костёр под персонажем") {
		checkIfMobExists();

		_posAtl = callSelf(getPos);
		["Campfire",_posAtl,null,false] call createStructure;
	};

	addCommand("alltome",ACCESS_ADMIN) {
		checkIfMobExists();
		_posAtl = callSelf(getPos);
		{
			_x setPosAtl _posAtl;
		} foreach cm_allInGameMobs;
	};

	addCommandWithDescription("fixpos",PUBLIC_COMMAND,"Возвращает персонажа в точку последнего подключения")
	{
		checkIfMobExists();

		callSelfParams(setPos,callSelf(getInitialPos));
	};

	addCommandWithDescription("giveup",PUBLIC_COMMAND,"Поднимает из бессознанки")
	{
		checkIfMobExists();
		setSelf(unconscious,1);
	};

	addCommandWithDescription("tptoadmin",PUBLIC_COMMAND,"Попытаться телепортироваться к администратору в сети") {
		checkIfMobExists();

		_cli = (["ACCESS_OWNERS"] call cm_accessTypeToNum) call cm_findClientByAccess;
		if equals(_cli,nullPtr) exitWith {
			rpcSendToClient(caller,"chatPrint",["Нет админа. Тп не будет((((" arg "error"]);
		};
		_act = getposatl getVar(_cli,actor);

		callSelfParams(setPos,_act);
	};

	addCommandWithDescription("givemelight",PUBLIC_COMMAND,"Создаёт факел под персонажем") {
		checkIfMobExists();
		_posAtl = callSelf(getPos);

		["Torch",_posAtl,null,false] call createItemInWorld;
	};

	addCommand("killme",PUBLIC_COMMAND)
	{
		checkIfMobExists();
		callSelfParams(die,"test");
	};

	addCommand("unctest",PUBLIC_COMMAND)
	{
		checkIfMobExists();
		callSelfParams(setUnconscious,10);
	};

	addCommandWithDescription("stamina",PUBLIC_COMMAND,"Восстанавливает запас выносливости")
	{
		checkIfMobExists();
		callSelfParams(addStaminaRegen,1000);
	};
	
	addCommandWithDescription("lobbyreturn",PUBLIC_COMMAND,"Возврат в лобби")
	{
		checkIfMobExists();
		
		[this,"Lobby"] call cm_switchToMob;
	};

	addCommandWithDescription("checkserverlight",PUBLIC_COMMAND,"Проверка освещенности")
	{
		checkIfMobExists();
		private _struct = "<t align='center'> Проверка освещения в радиусе 20 метров:</t>";
		
		modvar(_struct) + sbr + (format["srv_lt_value:: native: %1; api: %2",getLightingAt (getSelf(owner)), callSelf(getLighting)]);
		modvar(_struct) + sbr + (format["act_lt_logic: %1",getSelf(__lightSlots)]);
		
		_posAtl = callSelf(getPos);

		modvar(_struct) + sbr + "Items:";
		private _list = ["ILightible",_posAtl,20,true,true] call getAllItemsOnPosition;
		if array_isempty(_list) then {
			modvar(_struct) + sbr + "empty";
		} else {
			{
				modvar(_struct) + sbr + (format["%1*%2: logic: %3; light: %4",
					callFunc(_x,getName),
					getVar(_x,pointer),
					getVar(_x,lightIsEnabled),
					getVar(_x,loc) getvariable "srv_slt_obj"
				]);
			} foreach _list;
		};

		modvar(_struct) + sbr + "Structs:";
		_list = ["ILightibleStruct",_posAtl,20,true,true] call getGameObjectOnPosition;
		if array_isempty(_list) then {
			modvar(_struct) + sbr + "empty";
		} else {
			{
				if isTypeOf(_x,ILightibleStruct) then {
					modvar(_struct) + sbr + (format["%1*%2: logic: %3; light: %4",
						callFunc(_x,getName),
						getVar(_x,pointer),
						getVar(_x,lightIsEnabled),
						getVar(_x,loc) getvariable "srv_slt_obj"
					]);
				};

			} foreach _list;
		};

		callSelfParams(ShowMessageBox,"Text" arg _struct);
	};

#endif
// DEBUG

#ifdef EDITOR

addCommand("nvg",ACCESS_OWNERS)
{
	checkIfMobExists();
	if (parsenumber args > 0) then {
		callSelfParams(localEffectUpdate,"GhostNightVision")
	} else {
		callSelfParams(localEffectRemove,"GhostNightVision")
	};
};

addCommandWithDescription("newmob",ACCESS_OWNERS,"Опциональные аргументы: role=RHead type=Mob; role - роль с которой будет выдана экипировка. type - тип создаваемой сущности. Пример: newmob role=RCaretaker")
{
	checkIfMobExists();
	if (isMultiplayer) exitwith {};

	private _pos = (call interact_getIntersectData) select 1;
	if equals(_pos,vec3(0,0,0)) exitwith {};
	
	private _instance = "Mob";
	private _role = "";

	private _argv = args splitString "=;, ";
	for "_i" from 0 to (count _argv) - 1 step 2 do {
		_curit = _argv select _i;
		if (_i + 1 > ((count _argv) -1)) exitwith {};
		_curval = _argv select (_i + 1);
		if (_curit == "role") then {
			if isImplementClass(_curval) then {
				if isTypeNameOf(_curval,BasicRole) then {
					_role = _curval;
				};
			};
		};
		if (_curit == "type") then {
			if isImplementClass(_curval) then {
				if isTypeNameOf(_curval,BasicMob) then {
					_instance = _curval;
				};
			};
		};
	};

	private _gMob = _pos call gm_createMob;
	private _mob = instantiate(_instance);
	callFuncParams(_mob,initAsActor,_gMob);
	[_mob,8,10,8,12] call gurps_initSkills;
	setVar(_mob,name,"Существо");
	([0] call naming_getRandomName) params ["_f_","_s_"];
	[_mob,_f_,_s_] call naming_generateName;

	smd_allInGameMobs pushBackUnique _gMob;
	callFuncParams(_mob,setMobFace,pick faces_list_man);
	setVar(_mob,curTargZone,TARGET_ZONE_RANDOM);

	//fix initpos for new entity
	callFuncParams(_mob,setInitialPos,_pos);
	
	//setup previous entity initpos
	callFuncParams(this,setInitialPos,callFunc(this,getPos));

	if (_role != "") then {
		private _robj = _role call gm_getRoleObject;
		if !isNullReference(_robj) then {
			callFuncParams(_robj,getEquipment,_mob);
		};
	};
};

addCommandWithDescription("playtarget",ACCESS_OWNERS,"Перейти за другую сущность на которую вы нацелены")
{
	checkIfMobExists();
	private _data = (["target",""] call oop_getData) select 0;
	if !isReference(_data) exitwith {};
	if !isTypeOf(_data,BasicMob) exitwith {};

	[this,_data] call cm_switchToMob;
};


#endif

#ifdef EDITOR
addCommandWithDescription("rcsphere",PUBLIC_COMMAND,"Скрыть или показать желтую сферу при интеракциях")
{
	if (parseNumber args == 0) then {
		si_internal_rayObject hideObject true;
		["Курсор выключен","system"] call chatPrint;
	} else {
		si_internal_rayObject hideObject false;
		["Курсор включен","system"] call chatPrint;
	};
};

#include "..\..\..\client\WidgetSystem\widgets.hpp"

cmd_debug_voice_tester_handle = -1;
cmd_debug_voice_tester_pos = 0;
cmd_debug_voice_tester_widgets = [widgetNull];
addCommandWithDescription("debug_voice_tester",PUBLIC_COMMAND,"Тестирование гашения голоса")
{
	checkIfMobExists();
	_mode = parseNumber args;
	_updatePos = {
		cmd_debug_voice_tester_pos = eyePos player;
	};

	if (_mode == 0) exitWith {
		if (cmd_debug_voice_tester_handle==-1) then {
			if equals(cmd_debug_voice_tester_pos,0) then {
				{
					if isImplementFunc(_x,setDoorLock) then {
						callFuncParams(_x,setDoorLock,false arg false);
					};
				} foreach (["GameObject",true] call getAllObjectsInWorldTypeOf);
				//setDoorLock
			};
			
			call _updatePos;
			
			private _d = getGUI;
			_sx = 30;
			_sy = 30;
			private _ctg = [_d,WIDGETGROUP,[50-_sx/2,20,_sx,_sy]] call createWidget;
			([_d,BACKGROUND,WIDGET_FULLSIZE,_ctg] call createWidget) setBackgroundColor [0.3,0.3,0.3,0.5];
			cmd_debug_voice_tester_widgets = [_ctg];
			_txt = [_d,TEXT,WIDGET_FULLSIZE,_ctg] call createWidget;
			cmd_debug_voice_tester_widgets pushBack _txt;

			_method = {
				_txt = cmd_debug_voice_tester_widgets select 1;
				_src = cmd_debug_voice_tester_pos;
				_targ = eyePos player;
				_rdata = refcreate(0);
				_lvl = [objnull,1,_src,_rdata] call vs_calcVoiceIntersectionV2;
				[_txt,format["Debug voice: %1Frame: %2%1Distance: %3%1Voice: %4%1Debug info: %5"
					,sbr
					,diag_frameno
					,_src distance _targ
					,_lvl
					,refget(_rdata) splitString endl joinString sbr
				]] call widgetSetText;
			};
			cmd_debug_voice_tester_handle = startUpdate(_method,0);
		} else {
			[cmd_debug_voice_tester_widgets select 0] call deleteWidget;
			stopUpdate(cmd_debug_voice_tester_handle);
			cmd_debug_voice_tester_handle = -1;
		};
	};
	if (_mode == 1) exitWith {
		call _updatePos;
	};

};

#endif