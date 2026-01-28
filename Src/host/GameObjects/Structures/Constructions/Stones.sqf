// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\..\engine.hpp>
#include <..\..\..\oop.hpp>
#include <..\..\..\text.hpp>
#include <..\..\GameConstants.hpp>

//камни
editor_attribute("InterfaceClass")
editor_attribute("TemplatePrefab")
class(BasicStone) extends(Constructions) 
	var(name,"Камень"); 
	editor_only(var(desc,"Камень");)
	var(material,"MatStone");
	var(dr,1);
endclass

editor_attribute("EditorGenerated")
class(SmallGrayStone) extends(BasicStone)
	var(model,"ca\rocks2\r2_stone.p3d");
	var(name,"Маленький камень");
endclass

editor_attribute("EditorGenerated")
class(MediumGrayStone) extends(SmallGrayStone)
	var(model,"a3\rocks_f\small_stone_01_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments) extends(BasicStone)
	var(model,"a3\rocks_f_argo\limestone\limestone_01_erosion_f.p3d");
	var(name,"Камни"); 
	var(desc,"Просто камни");
endclass