// ======================================================
// Copyright (c) 2017-2026 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include "..\engine.hpp"
#include "..\struct.hpp"
#include <..\ServerRpc\serverRpc.hpp>

#include "ClientManager.h"


struct(PreAwaitClientData)
	def(owner) -1
	def(cancelToken) false

	def(gameToken) ""
	def(discordId) ""
	def(discordToken) ""
	def(refreshToken) ""
	def(expiryDate) ""
	def(roleAccessCheckKey) ""
	def(roleAccessCheckPending) false
	def(roleAccessCheckCompleted) false

	def(init)
	{
		params ["_owner"];
		self setv(owner,_owner);
	}

	//called on timer timeout
	def(onConnectTimeout)
	{
		if (self getv(cancelToken)) exitWith {};

		private _owner = self getv(owner);
		if !(_owner call cm_isClientExist) then {
			warningformat("Client (%1) did not have time to pass authorization in the allotted time (%2 sec)",_owner arg TIME_TO_INIT_CLIENT);
			[_owner,"Истекло время ожидания инициализации клиента."] call cm_serverKickById;
		};
	}
	def(openAuthProcess)
	{
		if (self getv(cancelToken)) exitWith {};
		rpcSendToClient(self getv(owner),"authproc",null);
	}

	def(startRoleAccessCheck)
	{
		private _owner = self getv(owner);
		private _discordId = self getv(discordId);
		private _requestKey = format["preauth:%1:%2",_owner,floor(random 999999)];

		self setv(roleAccessCheckKey,_requestKey);
		self setv(roleAccessCheckPending,true);
		self setv(roleAccessCheckCompleted,false);

		private _timeoutHandler = {
			params ["_pwData","_requestKey"];
			_pwData callp(onRoleAccessCheckTimeout,_requestKey);
		};
		invokeAfterDelayParams(_timeoutHandler,TIME_TO_VALIDATE_CLIENT,[self arg _requestKey]);

		[_requestKey,_discordId] call dsm_accounts_requestUpdateRoles;
	}

	def(onRoleAccessCheckTimeout)
	{
		params ["_requestKey"];
		if (self getv(cancelToken)) exitWith {};
		if (!(self getv(roleAccessCheckPending))) exitWith {};
		if ((self getv(roleAccessCheckKey)) != _requestKey) exitWith {};

		self setv(roleAccessCheckPending,false);
		self setv(roleAccessCheckCompleted,true);

		private _owner = self getv(owner);
		self setv(cancelToken,true);
		cm_preAwaitClientData deleteAt _owner;

		warningformat("Client (%1) did not have time to validate access in the allotted time (%2 sec)",self getv(owner) arg TIME_TO_VALIDATE_CLIENT);
		[self getv(owner),"Временная ошибка проверки доступа. Попробуйте позже."] call cm_serverKickById;
	}

	def(handleRoleAccessCheckResult)
	{
		params ["_roles"];
		if (self getv(cancelToken)) exitWith {};
		if (!(self getv(roleAccessCheckPending))) exitWith {};

		self setv(roleAccessCheckPending,false);
		self setv(roleAccessCheckCompleted,true);

		private _owner = self getv(owner);
		if ("Approved" in _roles) exitWith {
			self callv(openAuthProcess);
			self setv(cancelToken,true);
			cm_preAwaitClientData deleteAt _owner;
		};

		self setv(cancelToken,true);
		cm_preAwaitClientData deleteAt _owner;
		[_owner,"Получите доступ к игре - discord.relicta.ru"] call cm_serverKickById;
	}
endstruct
