// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\..\engine.hpp>
#include <..\..\..\oop.hpp>
#include <..\..\..\text.hpp>
#include <..\..\GameConstants.hpp>

editor_attribute("InterfaceClass")
class(InteractibleInterior) extends(SmallDecorations) 
	var(name,"Штуковина"); 
	var(desc,"С этим хочется что-то сделать" pcomma " но пока не знаю что...");
	var(material,"MatMetal");
	getterconst_func(getCoefAutoWeight,20);
	var(dr,2);
endclass

editor_attribute("EditorGenerated")
class(TruthMachine) extends(InteractibleInterior)
	var(model,"ml_exodusnew\priborchks_1.p3d");
	var(name,"Правдоруб");
endclass

editor_attribute("EditorGenerated")
class(MachineTool) extends(InteractibleInterior)
	var(model,"ml_shabut\stanok\stanok.p3d");
	var(name,"Станок");
endclass

editor_attribute("EditorGenerated")
class(MachineTool2) extends(MachineTool)
	var(model,"ml_shabut\stanok_2\stanok_2.p3d");
endclass

editor_attribute("EditorGenerated")
class(TechBlock) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_24\teh_shkaf.p3d");
endclass

editor_attribute("EditorGenerated")
class(MainframeConsole) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_14_10\d6table.p3d");
endclass

editor_attribute("EditorGenerated")
class(ATSBlock) extends(InteractibleInterior)
	var(model,"ml_exodusnew\ostankinoprops.p3d");
	var(name,"Связной узел");
endclass

editor_attribute("EditorGenerated")
class(MetalProcessor) extends(InteractibleInterior)
	var(model,"a3\structures_f_enoch\industrial\agriculture\drainage_01_f.p3d");
	var(name,"nilРаботчик");
endclass

editor_attribute("EditorGenerated")
class(AcientClock) extends(InteractibleInterior)
	var(model,"ml_shabut\chasiks\chasiks.p3d");
	var(name,"Тикалка");
endclass


class(PipeStove) extends(InteractibleInterior)
	var(model,"ca\buildings\furniture\dkamna_bila.p3d");
endclass

class(Samovar) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_24\samovar.p3d");
	getter_func(isMovable,true);
endclass

class(HoochMachine) extends(InteractibleInterior)
	var(model,"ml_shabut\exoduss\samogonapparat.p3d");
endclass

class(Grill) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_05\grill.p3d");
endclass

class(StationTea) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_14_10\chai.p3d");
	getter_func(isMovable,true);
endclass

class(Scales) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_14_10\wesi.p3d");
	getter_func(isMovable,true);
endclass

class(OldEngine) extends(InteractibleInterior)
	var(model,"metro_ob\model\engine_sm_01.p3d");
endclass

class(ElectricPump) extends(InteractibleInterior)
	var(model,"metro_ob\model\engine_turbo_01.p3d");
endclass

class(DrumGenerator) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_24\generator.p3d");
endclass

class(SmallGreenGenerator) extends(InteractibleInterior)
	var(model,"metro_ob\model\genagenagenerator.p3d");
endclass

class(BigElectricPumpFan) extends(InteractibleInterior)
	var(model,"ml_shabut\exodus\turbonasos.p3d");
endclass

class(BigPipePump) extends(InteractibleInterior)
	var(model,"ml_shabut\exoduss\turbosos.p3d");
	getterconst_func(getName,"Водяной насос");
endclass

class(Forge) extends(InteractibleInterior)
	var(model,"ml_shabut\exoduss\forge.p3d");
endclass

class(Anvil) extends(InteractibleInterior)
	var(model,"ml\ml_object_new\model_14_10\nakowal.p3d");
	getter_func(isMovable,true);
endclass

class(Grindstone) extends(InteractibleInterior)
	var(name,"Точилка");
	var(model,"ml\ml_object_new\model_05\napilnik.p3d");
	var(material,"MatMetal");
endclass