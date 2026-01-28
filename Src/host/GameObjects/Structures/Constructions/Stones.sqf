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
	var(dr,10);
endclass

editor_attribute("EditorGenerated")
class(MediumStoneFragments) extends(BasicStone)
	var(model,"a3\rocks_f_exp\lavastones\lavastonecluster_small_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumStoneFragments4) extends(MediumStoneFragments)
	var(model,"a3\rocks_f\water\stonesharp_small_w.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumStoneFragments3) extends(MediumStoneFragments)
	var(model,"a3\rocks_f_exp\lavastones\lavastonecluster_large_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumStoneFragments2) extends(MediumStoneFragments)
	var(model,"a3\rocks_f\stonesharp_small.p3d");
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
class(MediumGrayStone2) extends(MediumGrayStone)
	var(model,"a3\rocks_f\stone_small_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments) extends(BasicStone)
	var(model,"a3\rocks_f_argo\limestone\limestone_01_erosion_f.p3d");
	var(name,"Камни"); 
	var(desc,"Просто камни");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments5) extends(SmallStoneFragments)
	var(model,"a3\rocks_f_exp\cliff\cliff_stonecluster_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments2) extends(SmallStoneFragments)
	var(model,"a3\rocks_f\water\w_sharpstones_erosion.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments3) extends(SmallStoneFragments)
	var(model,"a3\rocks_f\blunt\bluntstones_erosion.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallStoneFragments4) extends(SmallStoneFragments)
	var(model,"a3\rocks_f\sharp\sharpstones_erosion.p3d");
endclass