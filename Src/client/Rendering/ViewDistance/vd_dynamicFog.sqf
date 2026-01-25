// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#define SPHERE_CLASS "Sphere_3DEN"
#define SPHERE_NOGROUND_CLASS "SphereNoGround_3DEN"
#define SPHERE_OUTER_CLASS "sign_sphere200cm_f"
#define SPHERE_USERTEX_CLASS "usertexture10m_f"

vd_dynFog_init = {

	private _s = [SPHERE_NOGROUND_CLASS] call vd_dynFog_createObj;
	//_s attachTo [player,[0,0,0]];
	_s setObjectScale 0.3;

	for "_i" from 1 to 10 do {

	_s = [SPHERE_USERTEX_CLASS] call vd_dynFog_createObj;
	_s attachto [player,[0,20+(_i * 0.5),0]];
	_s setObjectScale 10;

	[_s] call vd_dynFog_procRender;

	};
};

#ifdef EDITOR
if !isNull(vd_dynFog_objects) then {
	if equalTypes(vd_dynFog_objects,[]) then {

		deleteVehicle vd_dynFog_objects;
	};
};
#endif

vd_dynFog_objects = [];

vd_dynFog_createObj = {
	params ["_objclass"];
	private _obj = createVehicleLocal [_objclass,[0,0,0],[],0,"none"];
	vd_dynFog_objects pushBack _obj;
	_obj
};

vd_dynFog_procRender = {
	params ["_obj"];
	// _obj setobjecttexture [0,""];
	// _obj setobjectmaterial [0,""];

	_obj setObjectTexture [0,format ['#(argb,512,512,1)uiEx(bgColor:#00000000,display:RscDisplayEmpty,uniqueName:%1)', "testfog1"]];
	//_obj setobjectmaterial [0,""];
	//_obj setobjecttexture [0,"resources\ui\lobby\black512.paa"];
	//_obj setObjectMaterial [0,"a3\structures_f_bootcamp\vr\coverobjects\data\vr_coverobject_basic.rvmat"];
	
	//debug
	private _rc = ["testfog1"] call dec_getRenderContext;
	//if !isNullReference(_rc) then {
		{ctrldelete _x} foreach (allcontrols _rc);
		private _p = [_rc,PICTURE,[0,0,100,100]] call createWidget;
		_p ctrlsetposition [0,0,1,1];
		_p ctrlsetfade 0.5;
		_p ctrlcommit 0;
		private _color = 1;
		private _tex = "resources\ui\lobby\black512.paa";
		if (_i > 7) then {
			//_tex = "resources\ui\lobby\fog_render.paa";
			//_color = 0.2;
			_i = _i * 10;
		};
		
		_p ctrlsettextcolor [_color,_color,_color,0.03 + (_i * 0.05)  ];
		[_p,_tex] call widgetSetPicture;
		[_rc] call dec_applyContext;
	//};
	_obj
};