// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\engine.hpp>
#include <..\..\oop.hpp>
#include <..\..\text.hpp>

//большие камни
editor_attribute("InterfaceClass")
editor_attribute("TemplatePrefab")
class(BigStoneDecor) extends(BigConstructions) 
	var(name,"Камень");
	editor_only(var(desc,"Груда камней. Не разрушаема");)
	var(material,"MatStone");
endclass

editor_attribute("EditorGenerated")
class(MediumBrownRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f\blunt\bluntrock_apart.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGrayRockGeneral) extends(BigStoneDecor)
	var(model,"a3\rocks_f_enoch\r_rock_general1.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigStonesharpWall) extends(BigGrayRockGeneral)
	var(model,"a3\rocks_f\stonesharp_wall.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGrayRockGeneral4) extends(BigGrayRockGeneral)
	var(model,"a3\rocks_f\water\stonesharp_big_w.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGrayRockGeneral3) extends(BigGrayRockGeneral)
	var(model,"a3\rocks_f\water\stonesharp_medium_w.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGrayRockGeneral2) extends(BigGrayRockGeneral)
	var(model,"a3\rocks_f\stonesharp_medium.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumPileGreenStones) extends(BigStoneDecor)
	var(model,"a3\rocks_f_exp\cliff\cliff_boulder_f.p3d");
	var(name,"Камень");
endclass

editor_attribute("EditorGenerated")
class(BigGreenRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f_exp\cliff\cliff_wall_long_f.p3d");
	var(name,"Камень");
endclass

editor_attribute("EditorGenerated")
class(BigDarkRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f\water\w_sharprock_monolith.p3d");
	var(name,"Большая тёмная скала");
endclass

editor_attribute("EditorGenerated")
class(LargeHighRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f_exp\cliff\cliff_wall_tall_f.p3d");
	var(name,"Большая скала");
endclass

editor_attribute("EditorGenerated")
class(LargeSmoothRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f\blunt\bluntrock_monolith.p3d");
	var(name,"Каменная скала");
endclass

editor_attribute("EditorGenerated")
class(BigSmoothStone) extends(BigStoneDecor)
	var(name,"Каменная скала");
	var(model,"a3\rocks_f\sharp\sharprock_monolith.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumGreenRock) extends(BigStoneDecor)
	var(model,"a3\rocks_f_exp\cliff\cliff_peak_f.p3d");
endclass