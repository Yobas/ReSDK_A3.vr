// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include <..\..\..\host\lang.hpp>

// Экран лорных сообщений при пробуждении
// Данные: строка текста (structured text)
struct(UnsleepInfoScreen) base(NDBase)

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

		private _text = if (equalTypes(_args,[])) then {_args select 0} else {_args};

		// === Текст сообщения ===
		private _scrollCtg = [_disp,WIDGETGROUP_H,[10,15,80,65],_ctg] call createWidget;
		private _wid = [TEXT,[0,0,100,200],_scrollCtg] call nd_regWidget;
		[_wid, format[
			"<t align='center' size='1.8' font='Ringbear' color='#b3b3b3' shadow='2' shadowColor='#000000'>%1</t>",
			_text
		]] call widgetSetText;
		private _realH = _wid call widgetGetTextHeight;
		_wid ctrlSetPositionH (_realH / 100 * ((ctrlPosition _scrollCtg) select 3));
		_wid ctrlCommit 0;

		// === Кнопка "Далее" ===
		private _btnClose = [_disp,[35,84,30,6],_ctg,true] call nd_addClosingButton;
		_btnClose ctrlSetText "Далее";
		_btnClose ctrlSetFont "Ringbear";
		_btnClose ctrlSetFontHeight 0.1;
		if (nd_lobby_isOpen) then {
			ctrlSetFocus _btnClose;
		};
	};

endstruct
