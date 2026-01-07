// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

/*
	Рабочие наборы слоёв
	Функции:
		Автодобавление объекта в слой при создании и вставке

*/

init_function(LayersUtility_init)
{
	private _onchange = {
		params ["_entity"];
		if not_equalTypes(_entity,-1) exitWith {}; //skip models (only id's)
		if equals(get3DENEntity _entity,_entity) then {
			if array_exists(all3DENEntities select 6,_entity) then {
				call LayersUtility_syncExistsLayers;
			};
		};
	};
	["onEntityAdded",_onchange] call Core_addEventHandler;
	["onEntityRemoved",_onchange] call Core_addEventHandler;
	// ["onPaste",{
	// 	params ["_entities"];
	// 	private _selectedLayer = call LayersUtility_getSelectedLayer;
	// 	if (_selectedLayer == -1) exitWith {};
	// 	{
	// 		[_x,_selectedLayer] call LayersUtility_addObject;
	// 	} foreach _entities;
	// }] call Core_addEventHandler;
	["onObjectAdded",{
		params ["_obj"];
		private _selectedLayer = call LayersUtility_getSelectedLayer;
		if (_selectedLayer == -1) exitWith {};
		
		[_obj,_selectedLayer] call LayersUtility_addObject;
	}] call Core_addEventHandler;

	//! Только после полной загрузки редактора
	["onEditorLoaded",{
		//на данный момент грузимся с конфига
		call LayersUtility_loadStorage;
		call LayersUtility_syncWorkingSetText;
	}] call Core_addEventHandler;

	private _countItems = 10;
	layersUtility_countWorkingSetItems = _countItems;

	private _ctg = [getEdenDisplay,WIDGETGROUP,[40,10,20,15]] call createWidget;
	["layerutility_ctg_bind",_ctg] call widget_bind;

	_background = [getEdenDisplay,BACKGROUND,WIDGET_FULLSIZE,_ctg] call createWidget;
	_background setBackgroundColor [0.1,0.1,0.1,0.2];

	private _sizeItem = 13;
	private _titleSize = 15;

	private _title = [getEdenDisplay,TEXT,[0,0,90 + 10,_titleSize],_ctg] call createWidget;
	_title setBackgroundColor [0.1,0.1,0.1,0.8];
	[_title,"<t size='1'>Рабочие слои</t>"] call widgetSetText;

	// private _collapse = [getEdenDisplay,TEXT,[90,0,10,_titleSize],_ctg] call createWidget;
	// [_collapse,"<t size='1'>_</t>"] call widgetSetText;
	// _collapse setBackgroundColor [0.1,0.1,0.7,0.8];
	// _collapse ctrlAddEventHandler ["MouseButtonDown",{

	// }];
	
	// Drag & Drop для title
	if (!isNull(layersUtility_dragMouseMovingHandle)) then {
		getEdenDisplay displayRemoveEventHandler ["MouseMoving", layersUtility_dragMouseMovingHandle];
	};
	if !isNull(layersUtility_ctgPosition) then {
		[_ctg,layersUtility_ctgPosition call convertScreenCoords] call widgetSetPositionOnly;
	};

	layersUtility_isCollapsed = ifcheck(isNull(layersUtility_isCollapsed),true,layersUtility_isCollapsed);
	if (layersUtility_isCollapsed) then {
		_ctg ctrlshow false;
	};
	layersUtility_isDragging = false;
	layersUtility_dragStartMousePos = [0,0];
	layersUtility_dragStartCtgPos = [0,0];
	layersUtility_dragMouseMovingHandle = -1;
	layersUtility_ctgPosition = ctrlPosition _ctg select [0,2];
	

	layersUtility_selectedLayerIndex = ifcheck(isNull(layersUtility_selectedLayerIndex),0,layersUtility_selectedLayerIndex);
	private _oldvals = ifcheck(isNull(layersUtility_workingSet),[],layersUtility_workingSet);
	layersUtility_workingSet = [];
	layersUtility_workingSet resize layersUtility_countWorkingSetItems;
	layersUtility_workingSet = layersUtility_workingSet apply {-1};
	{
		if (_foreachIndex >= count _oldvals) exitWith {};
		private _curold = _oldvals select _foreachIndex;
		if (_curold != -1) then {
			layersUtility_workingSet set [_foreachIndex,_curold];
		};
	} foreach layersUtility_workingSet;
	
	_title ctrlAddEventHandler ["MouseButtonDown",{
		params ["_ctrl","_button","_x","_y","_shift","_ctrlKey","_altKey"];
		if not_equals(_button,0) exitWith {}; // только ЛКМ
		
		private _ctg = call LayersUtility_getWidgetGroup;
		if isNullReference(_ctg) exitWith {};
		
		layersUtility_isDragging = true;
		layersUtility_dragStartMousePos = getMousePosition;
		
		private _ctgPos = ctrlPosition _ctg;
		layersUtility_dragStartCtgPos = [_ctgPos select 0, _ctgPos select 1];
	}];

	// Добавляем обработчик движения мыши
	layersUtility_dragMouseMovingHandle = getEdenDisplay displayAddEventHandler ["MouseMoving",{
		if (!layersUtility_isDragging) exitWith {};
		
		private _ctg = call LayersUtility_getWidgetGroup;
		if isNullReference(_ctg) exitWith {};
		
		private _currentMousePos = getMousePosition;
		private _deltaX = (_currentMousePos select 0) - (layersUtility_dragStartMousePos select 0);
		private _deltaY = (_currentMousePos select 1) - (layersUtility_dragStartMousePos select 1);
		
		private _newX = (layersUtility_dragStartCtgPos select 0) + _deltaX;
		private _newY = (layersUtility_dragStartCtgPos select 1) + _deltaY;
		
		//clamp by screen size
		private _ctgSize = (ctrlPosition _ctg) params ["_x","_y","_w","_h"];
		_newX = clamp(_newX,safezoneX,safezoneW + safezoneX - _w);
		_newY = clamp(_newY,safezoneY,safezoneH + safezoneY - _h);

		layersUtility_ctgPosition = [_newX, _newY];

		_ctg ctrlSetPosition [_newX, _newY];
		_ctg ctrlCommit 0;
	}];
	
	_title ctrlAddEventHandler ["MouseButtonUp",{
		params ["_ctrl","_button","_x","_y","_shift","_ctrlKey","_altKey"];
		if not_equals(_button,0) exitWith {}; // только ЛКМ
		
		if (layersUtility_isDragging) then {
			layersUtility_isDragging = false;
		};
	}];

	layersUtility_layerButtonStyle_mouseEnter = [0.6,0.6,0.6,0.8];
	layersUtility_layerButtonStyle_mouseExit = [0.4,0.4,0.4,0.8];

	private _widgets = [];
	_ctg setvariable ["widgets",_widgets];

	private _ctgIn = [getEdenDisplay,WIDGETGROUP_H,[0,_titleSize,100,100-_titleSize],_ctg] call createWidget;

	for "_i" from 0 to (layersUtility_countWorkingSetItems-1) do {
		private _item = [getEdenDisplay,TEXT,[0,_sizeItem * _i,100,_sizeItem],_ctgIn] call createWidget;
		_item setBackgroundColor layersUtility_layerButtonStyle_mouseExit;
		_widgets pushBack _item;
		[_item,format["Слой %1",_i + 1]] call widgetSetText;
		_item ctrlAddEventHandler ["MouseEnter",{
			params ["_ctrl"];
			_ctrl setBackgroundColor layersUtility_layerButtonStyle_mouseEnter;
		}];
		_item ctrlAddEventHandler ["MouseExit",{
			params ["_ctrl"];
			_ctrl setBackgroundColor layersUtility_layerButtonStyle_mouseExit;
		}];
		_item setvariable ["layerindex",_i];
		_item ctrlAddEventHandler ["MouseButtonUp",{
			params ["_ctrl","_button","_x","_y","_shift","_ctrlKey","_altKey"];
			_i = _ctrl getvariable ["layerindex",-1];
			if (_i == -1) exitWith {};

			if (_button == MOUSE_LEFT) exitWith {
				//установить как рабочий
				layersUtility_selectedLayerIndex = _i;
				call LayersUtility_syncWorkingSetText;
				call LayersUtility_saveStorage;
			};

			_stackMenu = [];
			_stackMenu pushBack ["Установить как рабочий",{
				private _i = (call contextMenu_getContextParams) select 0;
				layersUtility_selectedLayerIndex = _i;
				call LayersUtility_syncWorkingSetText;
				call LayersUtility_saveStorage;
			}];
			_stackMenu pushBack ["Задать рабочий слой",{
				private _i = (call contextMenu_getContextParams) select 0;
				ContextMenu_internal_openedMousePos call mouseSetPosition;
				private _newLayerId = [
					"Выберите слой",
					format["Выберите слой для %1 рабочей области",_i + 1],
					null,
					false
				] call layer_openSelectLayer;
				if (_newLayerId != -1) then {
					layersUtility_workingSet set [_i,_newLayerId];
					call LayersUtility_syncWorkingSetText;
					call LayersUtility_saveStorage;
				};
			}];

			_stackMenu pushBack ["Задать раб.слои (несколько)",{
				private _i = (call contextMenu_getContextParams) select 0;
				ContextMenu_internal_openedMousePos call mouseSetPosition;
				private _newLayerIdList = [
					"Выберите слой",
					format["Выберите слои для %1 рабочей области",_i + 1],
					null,
					true
				] call layer_openSelectLayer;
				if (count _newLayerIdList > 0) then {
					{
						private _iOffset = _i + _foreachIndex;
						if (_iOffset >= layersUtility_countWorkingSetItems) exitWith {};
						layersUtility_workingSet set [_iOffset,_x];
					} foreach _newLayerIdList;
					call LayersUtility_syncWorkingSetText;
					call LayersUtility_saveStorage;
				};
			},null,"Задает рабочий слой для текущего рабочего набора и следующих по порядку"];

			if (!all_of(layersUtility_workingSet apply {equals(_x,-1)})) then {

				_stackMenu pushback ["Удалить рабочий набор",{
					private _i = (call contextMenu_getContextParams) select 0;
					layersUtility_workingSet set [_i,-1];
					call LayersUtility_syncWorkingSetText;
					call LayersUtility_saveStorage;
				},null,"Удалить слой из рабочего набора"];

				_stackMenu pushBack ["Удалить раб.наборы (несколько)",{
					private _list = [];
					private _listIndexes = [];
					{
						if (_x == -1) then {continue};
						_list pushBack ((_x call layer_internal_getLayerNameByPtr)+" (#"+(str _x)+")");
						_listIndexes pushBack _foreachIndex;
					} foreach layersUtility_workingSet;
					if (count _list == 0) exitWith {};
					
					private _del = refcreate(0);
					ContextMenu_internal_openedMousePos call mouseSetPosition;
					if ([
						_del,
						"Удалить слои",
						"Выберите слои для удаления из рабочего набора",
						_list apply {_x + ":ROOT"} joinString ";",
						null,
						true
					] call widget_winapi_openTreeView) then {
						if (refget(_del) == "") exitWith {};
						private _delList = (refget(_del) splitString (toString [9]));
						{
							private _id = _list find _x;
							if (_id == -1) then {continue};
							private _index = _listIndexes select _id;
							layersUtility_workingSet set [_index,-1];
						} foreach _delList;
						call LayersUtility_syncWorkingSetText;
						call LayersUtility_saveStorage;
					};
				}];
			};

			_stackMenu pushback ["Отмена",{}];

			[
				_stackMenu,
				call mouseGetPosition,
				[_i]
			] call contextMenu_create;

		}];
	};

	call LayersUtility_syncWorkingSetText;
}

function(LayersUtility_getWidgetGroup) { "layerutility_ctg_bind" call widget_getBind; }

function(LayersUtility_toggleMode) {
	layersUtility_isCollapsed = !(layersUtility_isCollapsed);
	if (layersUtility_isCollapsed) then {
		(call LayersUtility_getWidgetGroup) ctrlshow false;
	} else {
		(call LayersUtility_getWidgetGroup) ctrlshow true;
	};
}

function(LayersUtility_addObject)
{
	params ["_obj","_layer"];
	["Рабочий набор слоев", "Автоматическое добавление объекта в слой через рабочий набор", "a3\3den\data\cfg3den\history\changeAttributes_ca.paa"] collect3DENHistory {
		[_obj,_layer] call layer_addObject;
	};
}

function(LayersUtility_syncWorkingSetText) {
	{
		private _i = _x getvariable ["layerindex",-1];
		private _layerId = layersUtility_workingSet select _i;
		if (_layerId == -1) then {
			[_x,"- нет -"] call widgetSetText;
		} else {
			if (_i == layersUtility_selectedLayerIndex) then {
				[_x,format["<t color='#00FF00'>+%1+ (%2)</t>",_layerId call layer_internal_getLayerNameByPtr,_layerId]] call widgetSetText;
			} else {
				[_x,format["%1 (%2)",_layerId call layer_internal_getLayerNameByPtr,_layerId]] call widgetSetText;
			};
		};
	} foreach ((call LayersUtility_getWidgetGroup) getvariable "widgets");
}

function(LayersUtility_syncExistsLayers) {
	private _resyncText = false;
	{
		private _layerId = _x;
		if (_layerId != -1) then {
			if !(_layerId call layer_isExists) then {
				layersUtility_workingSet set [_foreachIndex,-1];
				_resyncText = true;
				if (_foreachIndex == layersUtility_selectedLayerIndex) then {
					layersUtility_selectedLayerIndex = -1;
				};
			};
		};
	} foreach layersUtility_workingSet;
	if (_resyncText) then {
		call LayersUtility_syncWorkingSetText;
		call LayersUtility_saveStorage;
	};
}

function(LayersUtility_getSelectedLayer) {
	if (layersUtility_selectedLayerIndex == -1) exitWith {-1};
	private _layerId = layersUtility_workingSet select layersUtility_selectedLayerIndex;
	if (_layerId == -1) exitWith {-1};
	if !(_layerId call layer_isExists) exitWith {-1};
	_layerId
}

function(LayersUtility_saveStorage)
{
	golib_com_objectHash set ["layersWorkingSet",layersUtility_workingSet];
	golib_com_objectHash set ["layersSelectedIndex",layersUtility_selectedLayerIndex];
	ignore3DENHistory {
		[""] call golib_saveCommonStorage;
	};
}

function(LayersUtility_loadStorage)
{
	if ("layersWorkingSet" call golib_hasCommonStorageParam) then {
		layersUtility_workingSet = golib_com_objectHash get "layersWorkingSet";
	};
	if ("layersSelectedIndex" call golib_hasCommonStorageParam) then {
		layersUtility_selectedLayerIndex = golib_com_objectHash get "layersSelectedIndex";
	};
}