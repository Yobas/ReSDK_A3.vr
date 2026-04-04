// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

function(systools_generateV2Prototypes) {
	[
		"Будет собран подробный JSON-реестр v1 классов для будущего v2 porter-а. Продолжить?",
		"Экспорт v2 прототипов",
		[
			"Запустить",
			{ call systools_internal_generateV2Prototypes }
		],
		[
			"Нет",
			{}
		],
		"\A3\ui_f\data\map\markers\handdrawn\warning_CA.paa",
		findDisplay 313
	] call createMessageBox;
}

function(systools_exportCurrentMap) {
	["Экспорт карты в v2 porter пока не реализован. Сначала используется class registry exporter."] call showInfo;
}

function(systools_internal_generateV2Prototypes)
{
	private _outputDir = "src\editor\bin";
	private _outputFile = _outputDir + "\v1_class_registry.json";
	private _mapVersion = -1;

	if (isNullVar(p_table_allclassnames) || {count p_table_allclassnames == 0}) exitWith {
		["Типы не скомпилированы. Сначала выполните сборку игровых объектов."] call showError;
	};

	if (isNullVar(goasm_attributes_hasAttributeClass) || {isNullVar(goasm_attributes_getClassValues)}) exitWith {
		["Editor reflection по class-attributes не инициализирован."] call showError;
	};

	if !([_outputDir] call systools_v2exp_ensureOutputDir) exitWith {
		["Не удалось подготовить директорию экспорта: " + _outputDir] call showError;
	};

	private _classes = + (["GameObject",true] call oop_getinhlist);
	_classes sort true;

	if (["version"] call golib_hasCommonStorageParam) then {
		_mapVersion = "version" call golib_getCommonStorageParam;
	};

	private _records = [];
	private _skipped = 0;
	private _startedAt = tickTime;
	{
		["Collect info: %1",_x] call printTrace;
		if ([_x] call systools_v2exp_shouldSkipClass) then {
			INC(_skipped);
			continue;
		};

		private _record = [_x] call systools_v2exp_buildClassRecord;
		if !isNullVar(_record) then {
			_records pushBack _record;
		};
	} foreach _classes;

	private _meta = createHashMapFromArray [
		["exporter","v1_v2exporter"],
		["generatedAt",systemTime],
		["editorVersion",ifcheck(isNullVar(Core_version_name),"unknown",Core_version_name)],
		["mapVersion",_mapVersion],
		["source","resdk_fork.vr"],
		["classCountSource",count _classes],
		["classCountExported",count _records],
		["classCountSkipped",_skipped],
		["outputFile",_outputFile]
	];

	private _payload = createHashMapFromArray [
		["meta",_meta],
		["classes",_records]
	];

	private _json = toJson(_payload);
	if !([_outputFile,_json] call file_write) exitWith {
		["Не удалось сохранить JSON файл: " + _outputFile] call showError;
	};

	["V2 exporter done in %1 sec. Exported %2 classes; skipped %3",tickTime - _startedAt,count _records,_skipped] call printLog;
	[format["Экспорт завершён. Записано %1 классов в %2",count _records,_outputFile]] call showInfo;
}

function(systools_v2exp_ensureOutputDir)
{
	params ["_relativePath"];
	private _absolutePath = getMissionPath _relativePath;
	([_absolutePath,false] call folder_exists)
}

function(systools_v2exp_shouldSkipClass)
{
	params ["_class"];
	([_class,"InterfaceClass"] call goasm_attributes_hasAttributeClass)
	|| {([_class,"HiddenClass"] call goasm_attributes_hasAttributeClass)}
	|| {([_class,"NodeClass"] call goasm_attributes_hasAttributeClass)}
	|| {([_class,"TemplatePrefab"] call goasm_attributes_hasAttributeClass)}
}

function(systools_v2exp_getTypeObject)
{
	params ["_class"];
	missionNamespace getVariable ["pt_" + _class,nullPtr]
}

function(systools_v2exp_safeGetterValue)
{
	params ["_class","_field",["_altMethodName",""]];
	private _value = [_class,_field,true,_altMethodName] call oop_getFieldBaseValue;
	if isNullVar(_value) exitWith {nil};

	if (_value isEqualType "") exitWith {
		if (_value == "") then {nil} else {_value}
	};

	_value
}

function(systools_v2exp_safeFieldValue)
{
	params ["_class","_field"];
	private _value = [_class,_field,true] call oop_getFieldBaseValue;
	if isNullVar(_value) exitWith {nil};

	if (_value isEqualType "") exitWith {
		if (_value == "") then {nil} else {_value}
	};

	_value
}

function(systools_v2exp_safeMethodValue)
{
	params ["_class","_methodName"];
	private _value = [_class,"",true,_methodName] call oop_getFieldBaseValue;
	if isNullVar(_value) exitWith {nil};

	if (_value isEqualType "") exitWith {
		if (_value == "") then {nil} else {_value}
	};

	_value
}

function(systools_v2exp_trySetValue)
{
	params ["_map","_key","_value"];
	if isNullVar(_value) exitWith {};
	_map set [_key,_value];
}

function(systools_v2exp_isTypeOfClass)
{
	params ["_class","_checkedType"];
	private _typeObj = [_class] call systools_v2exp_getTypeObject;
	if isNullReference(_typeObj) exitWith {false};

	(tolower _checkedType) in (_typeObj getVariable ["__inhlist_map",createHashMap])
}

function(systools_v2exp_getMethodSource)
{
	params ["_class","_methodName"];
	private _typeObj = [_class] call systools_v2exp_getTypeObject;
	if isNullReference(_typeObj) exitWith {nil};

	private _methodCode = _typeObj getVariable [_methodName,objNull];
	if isNull(_methodCode) exitWith {nil};
	if !(_methodCode isEqualType {}) exitWith {nil};

	private _source = toString _methodCode;
	if (_source == "") exitWith {nil};
	_source
}

function(systools_v2exp_collectClassAttributes)
{
	params ["_class"];
	private _typeObj = [_class] call systools_v2exp_getTypeObject;
	if isNullReference(_typeObj) exitWith {createHashMap};

	private _attrMap = _typeObj getVariable ["_redit_attribClass",createHashMap];
	private _result = createHashMap;
	{
		_result set [_x,_y];
	} foreach _attrMap;

	_result
}

function(systools_v2exp_collectDiagnostics)
{
	params ["_class"];
	private _warnings = [];
	private _maybeProblematic = false;

	private _animateSource = [_class,"animateData"] call systools_v2exp_getMethodSource;
	if !isNullVar(_animateSource) then {
		private _normalized = toLower _animateSource;
		private _isSuspicious = ("getvariable" in _normalized)
			|| {("getself" in _normalized)}
			|| {("callself" in _normalized)}
			|| {("this getvariable" in _normalized)}
			|| {("getv(" in _normalized)}
			|| {("getvar(" in _normalized)};

		if (_isSuspicious) then {
			_maybeProblematic = true;
			_warnings pushBackUnique "door_animateData_dynamic";
		};
	};

	private _diagnostics = createHashMap;
	if (_maybeProblematic) then {
		_diagnostics set ["maybeProblematic",true];
	};
	if (count _warnings > 0) then {
		_diagnostics set ["warnings",_warnings];
	};

	if (count _diagnostics == 0) exitWith {nil};
	_diagnostics
}

function(systools_v2exp_collectTags)
{
	params ["_class"];
	private _tags = [];

	if ([_class,"",true,"isItem"] call oop_getFieldBaseValue) then {_tags pushBackUnique "item"};
	if ([_class,"",true,"isMob"] call oop_getFieldBaseValue) then {_tags pushBackUnique "mob"};
	if ([_class,"",true,"isDecor"] call oop_getFieldBaseValue) then {_tags pushBackUnique "decor"};
	if ([_class,"",true,"isStruct"] call oop_getFieldBaseValue) then {_tags pushBackUnique "struct"};
	if ([_class,"",true,"isDoor"] call oop_getFieldBaseValue) then {_tags pushBackUnique "door"};
	if ([_class,"",true,"isContainer"] call oop_getFieldBaseValue) then {_tags pushBackUnique "container"};
	if ([_class,"",true,"canLight"] call oop_getFieldBaseValue) then {_tags pushBackUnique "light"};
	if ([_class,"",true,"isSeat"] call oop_getFieldBaseValue) then {_tags pushBackUnique "seat"};
	if ([_class,"EffectClass"] call goasm_attributes_hasAttributeClass) then {_tags pushBackUnique "effect"};

	if ([_class,"MeleeWeapon"] call systools_v2exp_isTypeOfClass) then {_tags pushBackUnique "weapon"};
	if ([_class,"RangedWeapon"] call systools_v2exp_isTypeOfClass) then {_tags pushBackUnique "weapon"};
	if ([_class,"Cloth"] call systools_v2exp_isTypeOfClass) then {_tags pushBackUnique "wearable"};
	if ([_class,"Backpack"] call systools_v2exp_isTypeOfClass) then {_tags pushBackUnique "wearable"};
	if ([_class,"ElectronicDevice"] call systools_v2exp_isTypeOfClass) then {_tags pushBackUnique "electronic"};

	_tags
}

function(systools_v2exp_deriveEquipmentKind)
{
	params ["_class","_examineType","_allowedSlots"];
	if !isNullVar(_examineType) exitWith {
		if (_examineType in ["cloth","armor","helmet","mask","backpack"]) then {
			_examineType
		} else {
			nil
		};
	};

	if isNullVar(_allowedSlots) exitWith {nil};

	private _allowedSet = createHashMap;
	{
		_allowedSet set [_x,true];
	} foreach _allowedSlots;

	if (INV_ARMOR in _allowedSet) exitWith {"armor"};
	if (INV_HEAD in _allowedSet) exitWith {"helmet"};
	if (INV_FACE in _allowedSet) exitWith {"mask"};
	if (INV_BACKPACK in _allowedSet) exitWith {"backpack"};
	if (INV_BACK in _allowedSet) exitWith {"backpack"};
	if (INV_CLOTH in _allowedSet) exitWith {"cloth"};
	nil
}

function(systools_v2exp_buildEquipmentVisual)
{
	params ["_class"];
	private _armaClass = [_class,"armaClass"] call systools_v2exp_safeFieldValue;
	if isNullVar(_armaClass) exitWith {nil};
	if (_armaClass isEqualType 0 && {_armaClass == -1}) exitWith {nil};

	private _allowedSlots = [_class,"allowedSlots"] call systools_v2exp_safeFieldValue;
	private _examineType = [_class,"",true,"getExamine3dItemType"] call oop_getFieldBaseValue;
	private _kind = [_class,_examineType,_allowedSlots] call systools_v2exp_deriveEquipmentKind;

	private _equipment = createHashMapFromArray [
		["armaClass",_armaClass]
	];
	[_equipment,"allowedSlots",_allowedSlots] call systools_v2exp_trySetValue;
	[_equipment,"examine3dType",_examineType] call systools_v2exp_trySetValue;
	[_equipment,"kind",_kind] call systools_v2exp_trySetValue;

	_equipment
}

function(systools_v2exp_buildSeatData)
{
	params ["_class"];
	if !([_class,"",true,"isSeat"] call oop_getFieldBaseValue) exitWith {nil};

	private _offsetPos = [_class,"getChairOffsetPos"] call systools_v2exp_safeMethodValue;
	private _offsetDir = [_class,"getChairOffsetDir"] call systools_v2exp_safeMethodValue;
	private _seat = createHashMap;

	[_seat,"offsetPos",_offsetPos] call systools_v2exp_trySetValue;
	[_seat,"offsetDir",_offsetDir] call systools_v2exp_trySetValue;

	if (count _seat == 0) exitWith {nil};
	_seat
}

function(systools_v2exp_buildDoorData)
{
	params ["_class"];
	if !([_class,"",true,"isDoor"] call oop_getFieldBaseValue) exitWith {nil};

	private _animateSource = [_class,"animateData"] call systools_v2exp_getMethodSource;
	private _normalized = ifcheck(isNullVar(_animateSource),"",toLower _animateSource);
	private _isSuspicious = _normalized != ""
		&& {
			("getvariable" in _normalized)
			|| {("getself" in _normalized)}
			|| {("callself" in _normalized)}
			|| {("this getvariable" in _normalized)}
			|| {("getv(" in _normalized)}
			|| {("getvar(" in _normalized)}
		};

	private _door = createHashMap;
	private _animateData = nil;
	if (!_isSuspicious) then {
		_animateData = [_class,"animateData"] call systools_v2exp_safeMethodValue;
	};

	[_door,"animateData",_animateData] call systools_v2exp_trySetValue;
	[_door,"animateDataSource",_animateSource] call systools_v2exp_trySetValue;
	[_door,"interpSpeed",[_class,"interpSpeed"] call systools_v2exp_safeMethodValue] call systools_v2exp_trySetValue;
	if (_isSuspicious) then {
		_door set ["animateDataMaybeProblematic",true];
	};

	if (count _door == 0) exitWith {nil};
	_door
}

function(systools_v2exp_buildClassRecord)
{
	params ["_class"];
	private _typeObj = [_class] call systools_v2exp_getTypeObject;
	if isNullReference(_typeObj) exitWith {nil};

	private _declInfo = [_class,"__decl_info__"] call oop_getTypeValue;
	private _baseClass = [_class,"__motherClass"] call oop_getTypeValue;
	private _inheritanceChain = [_class,"__inhlistCase"] call oop_getTypeValue;
	private _modelPath = [_class,"model","getModel"] call systools_v2exp_safeGetterValue;
	private _name = [_class,"name","getName"] call systools_v2exp_safeGetterValue;
	private _desc = [_class,"desc","getDesc"] call systools_v2exp_safeGetterValue;
	private _material = [_class,"material"] call systools_v2exp_safeFieldValue;
	private _weight = [_class,"weight"] call systools_v2exp_safeFieldValue;
	private _classAttributes = [_class] call systools_v2exp_collectClassAttributes;
	private _tags = [_class] call systools_v2exp_collectTags;
	private _equipmentVisual = [_class] call systools_v2exp_buildEquipmentVisual;
	private _seat = [_class] call systools_v2exp_buildSeatData;
	private _door = [_class] call systools_v2exp_buildDoorData;
	private _diagnostics = [_class] call systools_v2exp_collectDiagnostics;

	private _record = createHashMapFromArray [
		["className",_class]
	];
	[_record,"baseClass",_baseClass] call systools_v2exp_trySetValue;
	[_record,"inheritanceChain",_inheritanceChain] call systools_v2exp_trySetValue;
	[_record,"declInfo",_declInfo] call systools_v2exp_trySetValue;
	[_record,"modelPath",_modelPath] call systools_v2exp_trySetValue;
	[_record,"name",_name] call systools_v2exp_trySetValue;
	[_record,"desc",_desc] call systools_v2exp_trySetValue;
	[_record,"material",_material] call systools_v2exp_trySetValue;
	[_record,"weight",_weight] call systools_v2exp_trySetValue;

	if (count _classAttributes > 0) then {
		_record set ["classAttributes",_classAttributes];
	};
	if (count _tags > 0) then {
		_record set ["tags",_tags];
	};
	if !isNullVar(_equipmentVisual) then {
		_record set ["equipmentVisual",_equipmentVisual];
	};
	if !isNullVar(_seat) then {
		_record set ["seat",_seat];
	};
	if !isNullVar(_door) then {
		_record set ["door",_door];
	};
	if !isNullVar(_diagnostics) then {
		_record set ["diagnostics",_diagnostics];
	};

	_record
}
