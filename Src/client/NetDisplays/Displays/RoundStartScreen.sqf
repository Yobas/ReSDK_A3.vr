// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\..\host\lang.hpp>
#include <..\..\..\SETTINGS.h>

// Экран начала раунда (пробуждение персонажа)
// Данные: [_headerLine, _charInfoLine, _descriptionText]
//   _headerLine - верхняя строка (колено, смена, локация)
//   _charInfoLine - информация о персонаже (имя, роль, возраст)
//   _descriptionText - описание роли и задачи
struct(RoundStartScreen) base(NDBase)

	decl(override) def(process)
	{
		params ["_args","_isFirstCall"];

		private _disp = self getv(thisDisplay);

		if (_isFirstCall) then {
			private _ctg = [_disp,WIDGETGROUP,[0,0,100,100]] call createWidget;
			nd_internal_currentStructObj callp(addSavedWidget, _ctg);

			// Полноэкранный чёрный фон
			private _bg = [_disp,BACKGROUND,[0,0,100,100],_ctg] call createWidget;
			_bg setBackgroundColor [0,0,0,1];
		};

		private _ctg = (nd_internal_currentStructObj callv(getSavedWidgets)) select 0;

		call nd_cleanupData;

		_args params [
			["_headerLine",""],
			["_charInfoLine",""],
			["_descriptionText",""]
		];

		// === Верхняя строка: "Шестое колено. Смена 143. Грязноямск." ===
		private _wid = [TEXT,[0,10,100,10],_ctg] call nd_regWidget;
		[_wid, format[
			"<t align='center' size='2.7' font='Ringbear' color='#b3b3b3' shadow='2' shadowColor='#000000'>%1</t>",
			_headerLine
		]] call widgetSetText;

		// === Информация о персонаже ===
		_wid = [TEXT,[0,18,100,12],_ctg] call nd_regWidget;
		[_wid, format[
			"<t align='center' size='2.7' font='Ringbear' color='#b3b3b3' shadow='2' shadowColor='#000000'>%1</t>",
			_charInfoLine
		]] call widgetSetText;

		// === Описание роли (задачи, воспоминания и т.д.) ===
		if (_descriptionText != "") then {
			private _scrollCtg = [_disp,WIDGETGROUP_H,[15,36,70,40],_ctg] call createWidget;
			_wid = [TEXT,[0,0,100,200],_scrollCtg] call nd_regWidget;
			[_wid, format[
				"<t align='center' size='1.5' font='Ringbear' color='#b3b3b3' shadow='1' shadowColor='#000000'>%1</t>",
				_descriptionText
			]] call widgetSetText;
			private _realH = _wid call widgetGetTextHeight;
			_wid ctrlSetPositionH (_realH / 100 * ((ctrlPosition _scrollCtg) select 3));
			_wid ctrlCommit 0;
		};

		private _btnClose = [_disp,[35,82,30,6],_ctg,true] call nd_addClosingButton;
		_btnClose ctrlSetText "Проснуться";
		_btnClose ctrlSetFont "Ringbear";
		_btnClose ctrlSetFontHeight 0.1;
		if (nd_lobby_isOpen) then {
			ctrlSetFocus _btnClose;
		};

		private _bedIcon = [_disp,ACTIVEPICTURE,[47,88,6,6],_ctg] call createWidget;
		_bedIcon ctrlSetText PATH_PICTURE("bed_icon.paa");
		_bedIcon ctrlAddEventHandler ["MouseButtonUp",{
			nextFrame(ifcheck(nd_lobby_isOpen,nd_closeND_lobby,nd_onClose));
		}];
	};

endstruct
