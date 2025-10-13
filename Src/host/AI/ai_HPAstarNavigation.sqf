// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

// ============================================================================
// HPA* NAVIGATION SYSTEM
// ============================================================================
// Иерархическая навигационная система для AI с поддержкой 3D навигации,
// многоуровневых структур и динамической генерации навигационной сетки.
// 
// Система использует региональный подход (10×10м регионы) для оптимизации
// производительности и поддержки карт любого размера.
// 
// ============================================================================
// БЫСТРЫЙ СТАРТ:
// ============================================================================
// 1. Инициализация навигации:
//    [getPosASL player, 50] call ai_nav_quickInit;
//
// 2. Поиск пути:
//    private _path = [getPosASL player, _targetPos] call ai_nav_findPath;
//
// 3. Тестирование с визуализацией:
//    [getPosASL player, _targetPos] call ai_nav_testPath;
//
// 4. Получить статистику:
//    call ai_nav_getStats;
//
// ============================================================================
// ВИЗУАЛИЗАЦИЯ:
// ============================================================================
// 🟢 Зеленые сферы    - узлы навигации
// 🟡 Желтые линии     - границы регионов
// 🔵 Голубые линии    - связи внутри региона
// 🔴 Красные линии    - переходные точки между регионами
// 🟣 Фиолетовые линии - найденный путь
// ============================================================================

// ============================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ И КОНФИГУРАЦИЯ
// ============================================================================
#define AI_NAV_DEBUG true
#define AI_NAV_DEBUG_DRAW true


// Навигационные данные
ai_nav_regions = createHashMap;      // regionKey -> region data
ai_nav_nodes = createHashMap;        // nodeId -> node data  
ai_nav_adjacency = createHashMap;    // nodeId -> [[neighborId, cost], ...]

// Конфигурация генерации сетки
ai_nav_regionSize = 10;              // Размер региона 10×10м
ai_nav_gridStep = 1;                 // Шаг сетки 1м
ai_nav_maxSlope = 45;                // Максимальный угол склона в градусах
ai_nav_raycastHeight = 300;          // Высота начала raycast
ai_nav_minDistZ = 0.7;				// Минимальное расстояние по Z построения узлов по z

ai_nav_nodeIdCounter = 0;

#ifdef AI_NAV_DEBUG
	#define ai_debug_decl(linedecl) linedecl
#else
	#define ai_debug_decl(linedecl) 
#endif

// Функция логирования для отладки
// Работает как в редакторе (3DEN), так и в игре
ai_debugLog = {
	if (is3den) then {
		_this call printTrace; // Внутренняя функция редактора
	} else {
		trace(format _this); // Макрос из engine.hpp
	};
};

// Генерация уникального ID для узла
ai_nav_generateNodeId = {
	ai_nav_nodeIdCounter = ai_nav_nodeIdCounter + 1;
	ai_nav_nodeIdCounter
};

// Очистка отладочных объектов
ai_nav_clearDebugObjects = {
	{
		if (!isNullReference(_x)) then {
			deleteVehicle _x;
		};
	} forEach ai_debug_objs;
	ai_debug_objs = [];
	["Debug objects cleared"] call ai_debugLog;
};


ai_nav_debug_createObj = {
	params ["_pos", ["_color",[1,1,1]],["_size",1],["_isSphere",false],["_list",ai_debug_objs]];
	private _cls = "Sign_Arrow_F";
	if (_isSphere) then {
		_cls = "Sign_Sphere10cm_F";
	};
	private _arrow = _cls createVehicle [0,0,0];
	_arrow setPosASL _pos;
	_arrow setObjectTexture [0, format["#(rgb,8,8,3)color(%1,%2,%3,1)",_color select 0,_color select 1,_color select 2]];
	_arrow setObjectScale _size;
	_list pushBack _arrow;
	_arrow
};

ai_nav_generateRegionNodes = {
	params ["_pos", ["_autoSave", true]];
	
	// Получаем ключ региона
	private _regionKey = _pos call ai_nav_getRegionKey;
	
	// Проверяем, не существует ли регион уже
	if (_regionKey in ai_nav_regions) exitWith {
		ai_debug_decl(["Region %1 already exists, skipping generation" arg _regionKey] call ai_debugLog);
		_regionKey
	};
	
	private _regionStartX = floor((_pos select 0) / ai_nav_regionSize) * ai_nav_regionSize;
	private _regionStartY = floor((_pos select 1) / ai_nav_regionSize) * ai_nav_regionSize;
	private _regionEndX = _regionStartX + ai_nav_regionSize - ai_nav_gridStep; // Исключаем правую границу
	private _regionEndY = _regionStartY + ai_nav_regionSize - ai_nav_gridStep; // Исключаем верхнюю границу
	private _queryPos = [];
	
	ai_debug_decl(private _t = tickTime;)
	for "_x" from _regionStartX to _regionEndX step ai_nav_gridStep do {
		for "_y" from _regionStartY to _regionEndY step ai_nav_gridStep do {
			_queryPos pushBack [
				[_x,_y,ai_nav_raycastHeight],
				[_x,_y,0],
				objNull,
				objNull,
				true,
				-1,
				"ROADWAY", //когда игроки смогут ходить по любым поверхностям и мобы смогут
				"ROADWAY",
				false //ret unique
			];
		};
	};

	private _nodes = [];

	//постройка сетки
	private _hits = lineIntersectsSurfaces [_queryPos];
	{
		private _prevpos = [0,0,99999];
		{
			_x params ["_pos","_norm","_obj"];
			//пропускаем самый верхний слой (это крыша потолка) 
			//! не всегда. пока отключено
			//if (_forEachIndex == 0) then {_prevpos = _pos;continue};
			
			//поверхность карты пропускаем
			if (isNullReference(_obj)) then {continue};

			//если с предыдущей точки расстояние меньше минимального, то пропускаем
			if (((_prevpos select 2)-(_pos select 2)) < ai_nav_minDistZ) then {
				continue
			};

			_nodes pushBack _pos;

			#ifdef AI_NAV_DEBUG_DRAW
			[_pos,[0,1,0],3,true] call ai_nav_debug_createObj;
			#endif

			_prevpos = _pos;
		} foreach _x;
	} foreach _hits;
	//ai_debug_decl(["generateRegionNodes time %1ms" arg ((tickTime - _t)*1000)toFixed 6] call ai_debugLog);

	#ifdef AI_NAV_DEBUG_DRAW
		ai_debug_loopDrawObjs = [];
		//debug draw region
		private _height =( _pos select 2)-3;
		private _corner1 = asltoatl[_regionStartX, _regionStartY, _height];
		private _corner2 = asltoatl[_regionEndX, _regionStartY, _height];
		private _corner3 = asltoatl[_regionEndX, _regionEndY, _height];
		private _corner4 = asltoatl[_regionStartX, _regionEndY, _height];
		
		// Рисуем 4 линии границ региона
		private _loopRegion1 = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [_corner1 arg _corner2 arg [1 arg 1 arg 0 arg 1] arg 20] arg null arg ai_debug_objs select 0);
		private _loopRegion2 = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [_corner2 arg _corner3 arg [1 arg 1 arg 0 arg 1] arg 20] arg null arg ai_debug_objs select 0);
		private _loopRegion3 = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [_corner3 arg _corner4 arg [1 arg 1 arg 0 arg 1] arg 20] arg null arg ai_debug_objs select 0);
		private _loopRegion4 = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [_corner4 arg _corner1 arg [1 arg 1 arg 0 arg 1] arg 20] arg null arg ai_debug_objs select 0);
		
		ai_debug_loopDrawObjs pushback _loopRegion1;
		ai_debug_loopDrawObjs pushback _loopRegion2;
		ai_debug_loopDrawObjs pushback _loopRegion3;
		ai_debug_loopDrawObjs pushback _loopRegion4;
		
		{
			//! отключено
			//private _loop = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [asltoatl _x arg asltoatl _x vectoradd vec3(0,0,10) arg null arg 15] arg null arg ai_debug_objs select 0);
			//ai_debug_loopDrawObjs pushback _loop;
		} foreach _nodes;
	#endif

	//постройка связей
	ai_debug_decl(private _tEdges = tickTime;)
	private _maxConnectionDist = ai_nav_gridStep * 2; // Максимальное расстояние связи (по диагонали)
	private _edgesList = [];
	_queryPos = [];

	
	private _connectionData = [];
	
	{
		private _currentNode = _x;
		private _currentIdx = _forEachIndex;
		
		// Ищем соседние узлы для связи
		for "_i" from (_currentIdx + 1) to (count _nodes - 1) do {
			private _neighborNode = _nodes select _i;
			private _dist = _currentNode distance _neighborNode;
			
			// Если узел в пределах дистанции связи
			if (
				_dist <= _maxConnectionDist
				//&& {([_currentNode, _neighborNode] call ai_nav_getSlopeAngleVec) <= ai_nav_maxSlope}
			) then {
				//Вроде здесь быстрее работает проверка
				if (([_currentNode, _neighborNode] call ai_nav_getSlopeAngleVec) > ai_nav_maxSlope) exitWith {};

				//по z расстояние не может быть больше 1м
				if (abs((_currentNode select 2) - (_neighborNode select 2)) > 1) exitWith {};

				// Проверяем, нет ли препятствий между узлами
				_queryPos pushBack [
					_currentNode vectoradd vec3(0,0,0.4),
					_neighborNode vectoradd vec3(0,0,0.4),
					objNull,
					objNull,
					true,
					1,
					"VIEW",
					"GEOM"
				];
				
				_connectionData pushBack [_currentNode, _neighborNode, _dist];
				
			};
		};
	} forEach _nodes;
	
	
	_hits = lineIntersectsSurfaces [_queryPos];
	
	{
		if (count _x == 0) then {
			(_connectionData select _forEachIndex) params ["_currentNode","_neighborNode","_dist"];
			_edgesList pushBack ([_currentNode, _neighborNode, _dist]);
			#ifdef AI_NAV_DEBUG_DRAW
				
				private _loopEdge = struct_newp(LoopedObjectFunction,ai_nav_debug_drawNode arg [
					asltoatl _currentNode vectoradd vec3(0,0,0.15) arg 
					asltoatl _neighborNode vectoradd vec3(0.1,0,0.15) arg 
					[0 arg 0.5 arg 1 arg 0.8] arg 
					15
				] arg null arg ai_debug_objs select 0);
				ai_debug_loopDrawObjs pushback _loopEdge;
			#endif
		};
	} foreach _hits;
	
	//ai_debug_decl(["generateRegionEdges time %1ms, edges count: %2" arg ((tickTime - _tEdges)*1000)toFixed 6 arg count _edgesList] call ai_debugLog);
	
	// Автоматически сохраняем регион, если включено
	if (_autoSave) then {
		[_regionKey, _nodes, _edgesList] call ai_nav_saveRegion;
	};
	
	// Возвращаем ключ региона
	_regionKey
};

ai_nav_getSlopeAngleVec = {
	params ["_pos1","_pos2"];
	private _dir = _pos2 vectorDiff _pos1;
	private _dirNorm = vectorNormalized _dir;

	// Горизонтальный вектор (проекция на XY плоскость)
	private _horDir = [_dirNorm select 0, _dirNorm select 1, 0];
	private _horDirNorm = vectorNormalized _horDir;

	// Угол между векторами через скалярное произведение
	private _dotProduct = _dirNorm vectorDotProduct _horDirNorm;
	private _slopeAngle = acos _dotProduct; // В градусах
	_slopeAngle
};

// ============================================================================
// ЭТАП 1: СОХРАНЕНИЕ ДАННЫХ РЕГИОНА
// ============================================================================

// Генерация ключа региона по позиции
ai_nav_getRegionKey = {
	params ["_x","_y"];
	private _rx = floor((_x ) / ai_nav_regionSize);
	private _ry = floor((_y) / ai_nav_regionSize);
	format ["%1_%2", _rx, _ry]
};

// Сохранение данных региона в глобальные структуры
ai_nav_saveRegion = {
	params ["_regionKey", "_nodes", "_edges"];
	
	// Создаем структуру данных региона
	private _regionData = createHashMap;
	_regionData set ["nodes", []];      // Массив ID узлов
	_regionData set ["edges", _edges];  // Связи остаются как есть
	_regionData set ["entrances", []];  // Переходные точки (пока пустые)
	
	private _nodeIds = [];
	private _posToIdMap = createHashMap; // Временная карта для связывания позиций с ID
	
	// Создаем ID для каждого узла и сохраняем в глобальный справочник
	{
		private _nodeId = call ai_nav_generateNodeId;
		private _nodeData = createHashMap;
		_nodeData set ["pos", _x];
		_nodeData set ["region", _regionKey];
		_nodeData set ["neighbors", []]; // Соседние узлы будут добавлены позже
		
		ai_nav_nodes set [_nodeId, _nodeData];
		_nodeIds pushBack _nodeId;
		
		// Сохраняем маппинг позиции к ID
		_posToIdMap set [str _x, _nodeId];
		
	} forEach _nodes;
	
	_regionData set ["nodes", _nodeIds];
	
	// Сохраняем связи внутри региона в ai_nav_adjacency
	{
		_x params ["_pos1", "_pos2", "_cost"];
		
		private _nodeId1 = _posToIdMap get (str _pos1);
		private _nodeId2 = _posToIdMap get (str _pos2);
		
		// Двунаправленные связи
		if (!isNil "_nodeId1" && !isNil "_nodeId2") then {
			// Связь 1 -> 2
			if (!(_nodeId1 in ai_nav_adjacency)) then {
				ai_nav_adjacency set [_nodeId1, []];
			};
			private _adjList1 = ai_nav_adjacency get _nodeId1;
			_adjList1 pushBackUnique [_nodeId2, _cost];
			
			// Связь 2 -> 1
			if (!(_nodeId2 in ai_nav_adjacency)) then {
				ai_nav_adjacency set [_nodeId2, []];
			};
			private _adjList2 = ai_nav_adjacency get _nodeId2;
			_adjList2 pushBackUnique [_nodeId1, _cost];
		};
	} forEach _edges;
	
	// Сохраняем регион
	ai_nav_regions set [_regionKey, _regionData];
	
	ai_debug_decl(["Region %1 saved: %2 nodes, %3 edges" arg _regionKey arg count _nodeIds arg count _edges] call ai_debugLog);
	
	_regionKey
};

// Получение данных региона
ai_nav_getRegion = {
	params ["_regionKey"];
	ai_nav_regions getOrDefault [_regionKey, nil]
};

// Проверка существования региона
ai_nav_hasRegion = {
	params ["_pos"];
	private _key = _pos call ai_nav_getRegionKey;
	_key in ai_nav_regions
};

// ============================================================================
// ЭТАП 2: ГЕНЕРАЦИЯ НЕСКОЛЬКИХ РЕГИОНОВ
// ============================================================================

// Генерация сетки регионов в радиусе от центральной точки
ai_nav_generateRegions = {
	params ["_centerPos", "_radius"];
	
	ai_debug_decl(private _tTotal = tickTime;)
	
	private _regionRadius = ceil(_radius / ai_nav_regionSize);
	private _centerRegionX = floor((_centerPos select 0) / ai_nav_regionSize);
	private _centerRegionY = floor((_centerPos select 1) / ai_nav_regionSize);
	
	private _generatedRegions = [];
	private _skippedRegions = 0;
	
	for "_rx" from (_centerRegionX - _regionRadius) to (_centerRegionX + _regionRadius) do {
		for "_ry" from (_centerRegionY - _regionRadius) to (_centerRegionY + _regionRadius) do {
			private _regionPos = [
				_rx * ai_nav_regionSize + ai_nav_regionSize/2,
				_ry * ai_nav_regionSize + ai_nav_regionSize/2,
				_centerPos select 2
			];
			
			// Проверяем расстояние от центра (генерируем только в радиусе)
			if ((_regionPos distance2D _centerPos) <= _radius) then {
				private _regionKey = [_regionPos] call ai_nav_generateRegionNodes;
				
				// Если регион успешно сгенерирован (не был пустым)
				if (!isNil "_regionKey" && {_regionKey in ai_nav_regions}) then {
					_generatedRegions pushBack _regionKey;
				} else {
					_skippedRegions = _skippedRegions + 1;
				};
			};
		};
	};
	
	// ai_debug_decl([
	// 	"Generated %1 regions in %2ms (skipped %3 empty regions)" arg 
	// 	count _generatedRegions arg 
	// 	((tickTime - _tTotal)*1000)toFixed 2 arg 
	// 	_skippedRegions
	// ] call ai_debugLog);
	
	_generatedRegions
};

// Очистка всех регионов
ai_nav_clearAllRegions = {
	ai_nav_regions = createHashMap;
	ai_nav_nodes = createHashMap;
	ai_nav_adjacency = createHashMap;
	ai_nav_nodeIdCounter = 0;
	
	// Очищаем визуализацию
	call ai_nav_clearDebugObjects;
	
	ai_debug_decl(["All regions cleared"] call ai_debugLog);
};

// ============================================================================
// ЭТАП 3: ПЕРЕХОДНЫЕ ТОЧКИ МЕЖДУ РЕГИОНАМИ (ENTRANCE POINTS)
// ============================================================================

// Найти переходные точки для всех регионов
ai_nav_buildEntrancePoints = {
	ai_debug_decl(private _tTotal = tickTime;)
	
	private _totalEntrances = 0;
	
	// Для каждого региона ищем переходные точки
	{
		private _regionKey = _x;
		private _entrances = [_regionKey] call ai_nav_findEntrancePoints;
		_totalEntrances = _totalEntrances + count _entrances;
	} forEach (keys ai_nav_regions);
	
	ai_debug_decl([
		"Built entrance points: %1 total entrances in %2ms" arg 
		_totalEntrances arg 
		((tickTime - _tTotal)*1000)toFixed 2
	] call ai_debugLog);
	
	_totalEntrances
};

// Найти переходные точки для конкретного региона
ai_nav_findEntrancePoints = {
	params ["_regionKey"];
	
	private _regionData = ai_nav_regions get _regionKey;
	if (isNil "_regionData") exitWith {
		ai_debug_decl(["Region %1 not found" arg _regionKey] call ai_debugLog);
		[]
	};
	
	private _nodeIds = _regionData get "nodes";
	private _entrances = createHashMap; // neighborRegionKey -> [nodeIds]
	
	// Получаем ключи соседних регионов
	_regionKey splitString "_" params ["_rx", "_ry"];
	_rx = parseNumber _rx;
	_ry = parseNumber _ry;
	
	// 8 направлений соседних регионов (N, S, E, W, NE, NW, SE, SW)
	private _neighborOffsets = [
		[0, 1],   // North
		[0, -1],  // South
		[1, 0],   // East
		[-1, 0],  // West
		[1, 1],   // NorthEast
		[-1, 1],  // NorthWest
		[1, -1],  // SouthEast
		[-1, -1]  // SouthWest
	];

	// Для каждого узла в регионе проверяем, является ли он граничным
	{
		private _nodeId = _x;
		private _nodeData = ai_nav_nodes get _nodeId;
		private _nodePos = _nodeData get "pos";
		
		// Проверяем каждое соседнее направление
		{
			_x params ["_dx", "_dy"];
			private _neighborKey = format ["%1_%2", _rx + _dx, _ry + _dy];
			
			// Если соседний регион существует
			if (_neighborKey in ai_nav_regions) then {
				private _neighborData = ai_nav_regions get _neighborKey;
				private _neighborNodeIds = _neighborData get "nodes";
				
				// Проверяем, есть ли связь с узлами соседнего региона
				{
					private _neighborNodeId = _x;
					private _neighborNodeData = ai_nav_nodes get _neighborNodeId;
					private _neighborPos = _neighborNodeData get "pos";
					
					// Если узлы достаточно близко (в пределах расстояния связи)
					private _dist = _nodePos distance _neighborPos;
					if (_dist <= (ai_nav_gridStep * 2)) then {
						// Проверяем препятствия						
						private _intersections = lineIntersectsSurfaces [
							_nodePos vectoradd vec3(0,0,0.4),
							_neighborPos vectoradd vec3(0,0,0.4),
							objNull,
							objNull,
							true,
							1,
							"VIEW",
							"GEOM"
						];
						
						if (count _intersections == 0) then {
							// Это переходная точка!
							if (!(_neighborKey in _entrances)) then {
								_entrances set [_neighborKey, []];
							};
							
							private _entranceList = _entrances get _neighborKey;
							
							// Добавляем только если еще нет
							if (!(_nodeId in _entranceList)) then {
								_entranceList pushBack _nodeId;
								
								// Сохраняем связь в adjacency
								if (!(_nodeId in ai_nav_adjacency)) then {
									ai_nav_adjacency set [_nodeId, []];
								};
								private _adjList = ai_nav_adjacency get _nodeId;
								_adjList pushBackUnique [_neighborNodeId, _dist];
								
								#ifdef AI_NAV_DEBUG_DRAW
								// Визуализация переходной точки (красная линия между регионами)
								private _loopEntrance = struct_newp(LoopedObjectFunction,
									ai_nav_debug_drawNode arg [
										asltoatl _nodePos vectoradd vec3(0,0,0.2) arg 
										asltoatl _neighborPos vectoradd vec3(0,0,0.2) arg 
										[1 arg 0 arg 0 arg 1] arg 
										25
									] arg 
									null arg 
									ai_debug_objs select 0
								);
								ai_debug_loopDrawObjs pushback _loopEntrance;
								#endif
							};
						};
						
					};
				} forEach _neighborNodeIds;
			};
		} forEach _neighborOffsets;
	} forEach _nodeIds;
	
	// Сохраняем переходные точки в регион
	_regionData set ["entrances", _entrances];
	
	ai_debug_decl([
		"Region %1: found %2 entrance connections" arg 
		_regionKey arg 
		count (keys _entrances)
	] call ai_debugLog);
	
	_entrances
};

// ============================================================================
// ЭТАП 4: A* АЛГОРИТМ ПОИСКА ПУТИ
// ============================================================================

// Найти ближайший узел к позиции
ai_nav_findNearestNode = {
	params ["_pos", ["_maxDistance", 50]];
	
	private _regionKey = _pos call ai_nav_getRegionKey;
	private _regionData = ai_nav_regions get _regionKey;
	
	// Если регион не существует, ищем в ближайших регионах
	if (isNil "_regionData") then {
		private _bestNode = -1;
		private _bestDist = 999999;
		
		{
			private _nodeData = _y;
			private _nodePos = _nodeData get "pos";
			private _dist = _pos distance _nodePos;
			
			if (_dist < _bestDist && _dist <= _maxDistance) then {
				_bestDist = _dist;
				_bestNode = _x;
			};
		} forEach ai_nav_nodes;
		
		if (_bestNode == -1) exitWith {-1};
		_bestNode
	};
	
	// Ищем в текущем регионе
	private _nodeIds = _regionData get "nodes";
	private _bestNode = -1;
	private _bestDist = 999999;
	
	{
		private _nodeId = _x;
		private _nodeData = ai_nav_nodes get _nodeId;
		private _nodePos = _nodeData get "pos";
		private _dist = _pos distance _nodePos;
		
		if (_dist < _bestDist) then {
			_bestDist = _dist;
			_bestNode = _nodeId;
		};
	} forEach _nodeIds;
	
	if (_bestDist > _maxDistance) then {
		-1
	} else {
		_bestNode
	}
};

// Восстановить путь из массива родителей
ai_nav_reconstructPath = {
	params ["_cameFrom", "_current"];
	
	private _path = [_current];
	
	while {_current in _cameFrom} do {
		_current = _cameFrom get _current;
		_path pushBack _current;
	};
	
	reverse _path;
	_path
};

// Эвристическая функция (расстояние по прямой)
ai_nav_heuristic = {
	params ["_nodeId1", "_nodeId2"];
	
	private _pos1 = (ai_nav_nodes get _nodeId1) get "pos";
	private _pos2 = (ai_nav_nodes get _nodeId2) get "pos";
	
	_pos1 distance _pos2
};

// Получить соседей узла
ai_nav_getNeighbors = {
	params ["_nodeId"];
	
	private _neighbors = ai_nav_adjacency getOrDefault [_nodeId, []];
	_neighbors
};

// A* алгоритм поиска пути между двумя узлами
ai_nav_findPathNodes = {
	params ["_startNodeId", "_goalNodeId"];
	FHEADER;
	
	if (_startNodeId == -1 || _goalNodeId == -1) exitWith {
		ai_debug_decl(["Invalid node IDs: start=%1, goal=%2" arg _startNodeId arg _goalNodeId] call ai_debugLog);
		[]
	};
	
	ai_debug_decl(private _tStart = tickTime;)
	
	// Инициализация
	private _openSet = [_startNodeId];
	private _cameFrom = createHashMap;
	
	private _gScore = createHashMap;
	_gScore set [_startNodeId, 0];
	
	private _fScore = createHashMap;
	_fScore set [_startNodeId, [_startNodeId, _goalNodeId] call ai_nav_heuristic];
	
	private _iterations = 0;
	private _maxIterations = 10000;
	
	while {count _openSet > 0 && _iterations < _maxIterations} do {
		_iterations = _iterations + 1;
		
		// Найти узел с минимальным fScore
		private _current = _openSet select 0;
		private _minF = _fScore getOrDefault [_current, 999999];
		
		{
			private _f = _fScore getOrDefault [_x, 999999];
			if (_f < _minF) then {
				_minF = _f;
				_current = _x;
			};
		} forEach _openSet;
		
		// Достигли цели
		if (_current == _goalNodeId) exitWith {
			private _path = [_cameFrom, _current] call ai_nav_reconstructPath;
			
			ai_debug_decl([
				"Path found: %1 nodes, %2 iterations, %3ms" arg 
				count _path arg 
				_iterations arg 
				((tickTime - _tStart)*1000)toFixed 2
			] call ai_debugLog);
			
			RETURN(_path);
		};
		
		// Удаляем текущий из openSet
		_openSet deleteAt (_openSet find _current);
		
		// Проверяем соседей
		private _neighbors = [_current] call ai_nav_getNeighbors;
		
		{
			_x params ["_neighborId", "_cost"];
			
			private _tentativeGScore = (_gScore getOrDefault [_current, 999999]) + _cost;
			
			if (_tentativeGScore < (_gScore getOrDefault [_neighborId, 999999])) then {
				_cameFrom set [_neighborId, _current];
				_gScore set [_neighborId, _tentativeGScore];
				_fScore set [_neighborId, _tentativeGScore + ([_neighborId, _goalNodeId] call ai_nav_heuristic)];
				
				if (!(_neighborId in _openSet)) then {
					_openSet pushBack _neighborId;
				};
			};
		} forEach _neighbors;
	};
	
	ai_debug_decl([
		"Path not found after %1 iterations (max: %2)" arg 
		_iterations arg 
		_maxIterations
	] call ai_debugLog);
	
	[]
};

// Найти путь между двумя позициями
ai_nav_findPath = {
	params ["_startPos", "_endPos"];
	
	ai_debug_decl(["Finding path from %1 to %2" arg _startPos arg _endPos] call ai_debugLog);
	
	// Находим ближайшие узлы
	private _startNode = [_startPos] call ai_nav_findNearestNode;
	private _endNode = [_endPos] call ai_nav_findNearestNode;
	
	if (_startNode == -1) exitWith {
		ai_debug_decl(["No start node found near %1" arg _startPos] call ai_debugLog);
		[]
	};
	
	if (_endNode == -1) exitWith {
		ai_debug_decl(["No end node found near %1" arg _endPos] call ai_debugLog);
		[]
	};
	
	// Запускаем A*
	private _pathNodes = [_startNode, _endNode] call ai_nav_findPathNodes;
	
	if (count _pathNodes == 0) exitWith {[]};
	
	// Конвертируем узлы в позиции
	private _pathPositions = [];
	{
		private _nodeData = ai_nav_nodes get _x;
		private _pos = _nodeData get "pos";
		_pathPositions pushBack _pos;
	} forEach _pathNodes;
	
	_pathPositions
};

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ТЕСТИРОВАНИЯ
// ============================================================================

// Быстрая инициализация навигации вокруг позиции
ai_nav_quickInit = {
	params ["_pos", ["_radius", 50]];
	
	// Очищаем предыдущие данные
	call ai_nav_clearAllRegions;
	
	// Генерируем регионы
	private _regions = [_pos, _radius] call ai_nav_generateRegions;
	
	["Navigation initialized: %1 regions generated around %2" arg count _regions arg _pos] call ai_debugLog;
	
	//Строим переходные точки между регионами
	private _entrances = call ai_nav_buildEntrancePoints;
	
	["Built %1 entrance connections between regions" arg _entrances] call ai_debugLog;
	
	_regions
};

// Получить информацию о системе навигации
ai_nav_getStats = {
	private _totalNodes = count (keys ai_nav_nodes);
	private _totalRegions = count (keys ai_nav_regions);
	private _totalEdges = 0;
	
	{
		private _regionData = _y;
		_totalEdges = _totalEdges + count (_regionData get "edges");
	} forEach ai_nav_regions;
	
	private _stats = createHashMap;
	_stats set ["regions", _totalRegions];
	_stats set ["nodes", _totalNodes];
	_stats set ["edges", _totalEdges];
	
	_stats
};

// ============================================================================
// ЭТАП 5: ВИЗУАЛИЗАЦИЯ И ТЕСТИРОВАНИЕ
// ============================================================================

// Визуализация найденного пути
ai_nav_debugDrawPath = {
	params ["_path", ["_color", [1,0,1,1]], ["_width", 30]];
	
	if (count _path < 2) exitWith {
		ai_debug_decl(["Path too short to draw: %1 nodes" arg count _path] call ai_debugLog);
	};
	
	private _pathLoops = [];
	
	for "_i" from 0 to (count _path - 2) do {
		private _p1 = _path select _i;
		private _p2 = _path select (_i + 1);
		
		private _loop = struct_newp(LoopedObjectFunction,
			ai_nav_debug_drawNode arg [
				asltoatl _p1 vectoradd vec3(0,0,0.3) arg 
				asltoatl _p2 vectoradd vec3(0,0,0.3) arg 
				_color arg 
				_width
			] arg 
			null arg 
			ai_debug_objsPath select 0
		);
		
		_pathLoops pushBack _loop;
	};
	
	ai_debug_decl(["Draw path with %1 segments" arg count _pathLoops] call ai_debugLog);
	
	_pathLoops
};

// Тест: найти и визуализировать путь между двумя позициями
ai_nav_testPath = {
	params ["_startPos", "_endPos"];
	
	// Находим путь
	private _path = [_startPos, _endPos] call ai_nav_findPath;
	
	if (count _path == 0) exitWith {
		["No path found between positions!"] call ai_debugLog;
		[]
	};
	
	// Визуализируем путь (фиолетовый цвет)
	deleteVehicle ai_debug_objsPath;
	ai_debug_objsPath = [];

	// Визуализируем старт и конец (зеленый и красный маркеры)
	[_startPos, [0,1,0], 5, true,ai_debug_objsPath] call ai_nav_debug_createObj; // Зеленый старт
	[_endPos, [1,0,0], 5, true,ai_debug_objsPath] call ai_nav_debug_createObj;   // Красный конец
	
	private _loops = [_path, [1,0,1,1], 30] call ai_nav_debugDrawPath;
	
	["Path found: %1 waypoints, distance: %2m" arg count _path arg (_startPos distance _endPos)toFixed 2] call ai_debugLog;
	
	_path
};

// ============================================================================
// DEBUG ФУНКЦИИ
// ============================================================================

ai_nav_debug_drawNode = {
	params ["_pos","_pos2",["_color",[1,0,0,1]],["_wdt",1]];
	drawLine3D [_pos, _pos2, _color,_wdt];
};