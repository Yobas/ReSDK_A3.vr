// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\engine.hpp>
#include <..\..\oop.hpp>
#include <..\..\text.hpp>

//большие стены
editor_attribute("InterfaceClass")
editor_attribute("TemplatePrefab")
class(BigWall) extends(BigConstructions) 
	var(name,"Большая стена"); 
	var(material,"MatStone");
	editor_only(var(desc,"Огромная стена выполняющая роль ограничения доступной зоны. Не разрушаема");)
endclass

editor_attribute("EditorGenerated")
class(LargeConcretePillars) extends(BigWall)
	var(model,"a3\structures_f_exp\infrastructure\bridges\bridgesea_01_pillar_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(CanalWallWithLadder) extends(BigWall)
	var(model,"a3\structures_f\walls\canal_wall_stairs_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(AcientStonebrickWall) extends(BigWall)
	var(model,"a3\structures_f\dominants\castle\castle_01_wall_16_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(AcientLightStoneWall) extends(BigWall)
	var(model,"a3\structures_f\dominants\castle\castle_01_wall_01_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(AcientLightStoneWall2) extends(AcientLightStoneWall)
	var(model,"a3\structures_f\dominants\castle\castle_01_wall_15_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(LargeConcreteAcientWall) extends(BigWall)
	var(model,"ca\structures\nav_pier\nav_pier_c_l10.p3d");
	var(name,"Бетон");
endclass

editor_attribute("EditorGenerated")
class(LargeConcreteAcientWall2) extends(LargeConcreteAcientWall)
	var(model,"ca\structures\nav_pier\nav_pier_c2.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigTransferConveyor) extends(BigWall)
	var(model,"a3\structures_f_enoch\industrial\mines\mine_01_conveyor_end_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigTransferConveyor2) extends(BigTransferConveyor)
	var(model,"a3\structures_f_enoch\industrial\coalplant_01\coalplant_01_conveyor_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(MediumStoneWall) extends(BigWall)
	var(name,"Каменная стена");
	var(material,"MatStone");
	var(model,"a3\structures_f_enoch\cultural\castleruins\castleruins_01_wall_10m_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGermoGate) extends(BigWall)
	var(model,"ml\ml_germogate\l_02_alex_vorota.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGermoGate2) extends(BigGermoGate)
	var(model,"ml_exodusnew\germozatvor_menu2.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigGermoGate1) extends(BigGermoGate)
	var(model,"ml\ml_germogate\l_02_alex_vorota_custom_1_2.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigLightStoneWall) extends(BigWall)
	var(model,"a3\structures_f\dominants\castle\castle_01_wall_07_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigLightStoneWall1) extends(BigLightStoneWall)
	var(model,"a3\structures_f\dominants\castle\castle_01_wall_02_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigBrickWall) extends(BigWall)
	var(model,"a3\structures_f_exp\cultural\fortress_01\fortress_01_10m_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigBrickWall2) extends(BigBrickWall)
	var(model,"a3\structures_f_exp\cultural\fortress_01\fortress_01_d_r_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(BigBrickWall1) extends(BigBrickWall)
	var(model,"a3\structures_f_exp\cultural\fortress_01\fortress_01_outtercorner_90_f.p3d");
endclass

editor_attribute("EditorGenerated")
class(StoneArch) extends(BigWall)
	var(model,"ca\structures\castle\a_castle_gate.p3d");
	var(name,"Большая стена");
endclass

editor_attribute("EditorGenerated")
class(ConcreteArch) extends(BigWall)
	var(model,"ml\ml_object\l04_catacombs\l04_catacombs_00.p3d");
endclass


editor_attribute("EditorGenerated")
class(BigStoneWall) extends(BigWall)
	var(model,"ca\structures\castle\a_castle_wall1_20.p3d");
	var(desc, "Огромная каменная стена" pcomma " покрытая мхом");
endclass

editor_attribute("EditorGenerated")
class(BigStoneWall2) extends(BigStoneWall)
	var(model,"ca\structures\castle\a_castle_wall1_corner_2.p3d");
endclass
editor_attribute("EditorGenerated")
class(BigStoneWall1) extends(BigStoneWall)
	var(model,"ca\structures\castle\a_castle_wall1_end_2.p3d");
endclass