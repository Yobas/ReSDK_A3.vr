// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\..\engine.hpp>
#include <..\..\..\oop.hpp>
#include <..\..\..\text.hpp>
#include <..\..\GameConstants.hpp>

//кучи грязи
editor_attribute("InterfaceClass")
editor_attribute("TemplatePrefab")
class(SmallDirtPile) extends(Constructions) 
	var(name,"Куча грязи"); 
	editor_only(var(desc,"Небольшая куча грязи");)
	var(material,"MatDirt");
	var(dr,1);
endclass

editor_attribute("EditorGenerated")
class(SmallLightDirtPile) extends(SmallDirtPile)
	var(model,"ca\structures_e\wall\wall_l\wall_l4_10m_ep1.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumLightDirtPile) extends(SmallLightDirtPile)
	var(model,"ca\structures_e\wall\wall_l\wall_l5_10m_ep1.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumStonePile) extends(SmallDirtPile)
	var(model,"ca\structures\ruins\rubble_rocks_01.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallDirtLight) extends(SmallDirtPile)
	var(model,"ml_shabut\nvprops\gryazyuka3.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumPileOfLightMud) extends(SmallDirtPile)
	var(model,"apalon\metro_a3\surfaces\gryazoookass2.p3d");
	var(name,"Куча грязи");
endclass

editor_attribute("EditorGenerated")
class(MediumPileOfDirtAndStones) extends(SmallDirtPile)
	var(model,"ca\structures\castle\a_castle_walls_5_d_ruins.p3d");
	var(name,"Куча грязи");
endclass

editor_attribute("EditorGenerated")
class(Grave) extends(SmallDirtPile)
	var(model,"a3\structures_f_enoch\cultural\cemeteries\grave_11_f.p3d");
	var(name,"Могила");
endclass

editor_attribute("EditorGenerated")
class(Grave4) extends(Grave)
	var(model,"ca\buildings\misc\hrobecek.p3d");
endclass

editor_attribute("EditorGenerated")
class(Grave3) extends(Grave)
	var(model,"a3\structures_f_epb\civ\dead\grave_rocks_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(Grave2) extends(Grave)
	var(model,"a3\structures_f_epb\civ\dead\grave_dirt_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(Grave1) extends(Grave)
	var(model,"a3\structures_f_epb\civ\dead\grave_forest_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(DirtCraterLong) extends(SmallDirtPile)
	var(model,"a3\structures_f_enoch\military\training\craterlong_02_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(DirtCraterBrownLong) extends(DirtCraterLong)
	var(model,"a3\data_f\particleeffects\craterlong\craterlong.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumDirt) extends(SmallDirtPile)
	var(model,"apalon\metro_a3\surfaces\gryazoookass.p3d");
endclass

editor_attribute("EditorGenerated")
class(SmallDirtBrown) extends(SmallDirtPile)
	var(model,"ml_shabut\nvprops\gryazyuka4.p3d");
endclass
editor_attribute("EditorGenerated")
class(SmallDirtGrey) extends(SmallDirtPile)
	var(model,"ml_shabut\nvprops\gryazyuka5.p3d");
endclass
