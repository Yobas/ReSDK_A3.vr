// ======================================================
// Copyright (c) 2017-2024 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

class CfgPatches
{
    class RELICT
    {
        name = "TEST";
        author = "123aa";
        authorUrl = "";
		version = "1.0";
	};

};

class CfgFunctions
{
    class RELICT 
		{
		class Bootstrap 
			{
				file = "\src";
				class init {postInit  = 1;};
			};
		};
};

//own location for hostvm works
class CfgLocationTypes
{
	class CBA_NamespaceDummy
	{
		name="";
		drawStyle="area";
		texture="";
		color[]={0,0,0,0};
		size=0;
		textSize=0;
		shadow=0;
		font="PuristaMedium";
	};
};