// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

/*
	! Внимание
	У этой оптимизации есть импакт только в симуляции и то слабый.
	Если кто-то захочет повозиться с этой фичей - милости прошу
*/

#include <..\..\..\host\engine.hpp>
#include <..\..\..\host\NOEngine\NOEngine.hpp>

cul_init = {
	[true] call cul_setEnable;
};

#ifdef EDITOR
if !isNull(cul_culledObjects) then {
	{
		_x hideobject false;
	} foreach cul_culledObjects;
};
if !isNull(cul_handleUpdate) then {
	stopUpdate(cul_handleUpdate);
};
#endif

cul_culledObjects = [];
cul_lastPlayerDir = 0;
cul_handleUpdate = -1;

cul_setEnable = {
	params ["_mode"];
	if (_mode) then {
		if (cul_handleUpdate != -1) exitWith {};
		cul_handleUpdate = startUpdate(cul_processUpdate,0.01);
	} else {
		if (cul_handleUpdate == -1) exitWith {};
		stopUpdate(cul_handleUpdate);
		cul_handleUpdate = -1;
	};
};


cul_processUpdate = {
	_mob = player;
	_chunkType = CHUNK_TYPE_STRUCTURE;
	_chunk = [getPosATL _mob,_chunkType] call noe_posToChunk;
	if not_equals(getdir _mob,cul_lastPlayerDir) then {
		{
			_x hideobject false;
		} foreach cul_culledObjects;
		cul_culledObjects = [];
	};
	if (count cul_culledObjects > 0) exitWith {};
	cul_lastPlayerDir = getdir _mob;
	
	_around = [
		[_mob modelToWorldVisual [0,(-chunkSize_structure - 5) * -1,0],_chunkType] call noe_posToChunk,
		[_mob modelToWorldVisual [-chunkSize_structure-5,(-chunkSize_structure - 5) * -1,0],_chunkType] call noe_posToChunk,
		[_mob modelToWorldVisual [chunkSize_structure+5,(-chunkSize_structure - 5) * -1,0],_chunkType] call noe_posToChunk
	];
	
	//traceformat("ch: %1",_around);

	{
		_chunkObject = [_x,_chunkType] call noe_client_getPosChunkToData;
		_objData = _chunkObject select 2;
		
		{
			_o = (_y select 0);
			_o hideObject true;
			cul_culledObjects pushback _o;
		} foreach _objData;
	} foreach _around;

	traceformat("xCULL PROCESS: count %1",count cul_culledObjects);
};