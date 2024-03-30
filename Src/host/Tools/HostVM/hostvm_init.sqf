// ======================================================
// Copyright (c) 2017-2024 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

/*
    Main hostvm loader.
    Hostvm can used for:
        - unit tests
        - parsing code
        - compiling, executing code
*/

#include "..\..\engine.hpp"


isHostVM = true;

//hostVM_password - use as serverpassword
server_password = hostVM_password;

diag_log ("Initialize cba module");
call compile preprocessFileLineNumbers "src\host\Tools\HostVM\cba_hostvm_functions.sqf";

diag_log ("Loading cba initializer");
call compile preprocessFileLineNumbers "src\host\Tools\HostVM\cba_hostvm_init.sqf";

//called after logger functions loaded
hostVM_postInit = {

    hvmPrint = {
        params ["_msg",["_fncaller","Unknown_function"]];
        "debug_console" CALLEXTENSION (format["hostvm(%2): %1",_msg,_fncaller]);
    };

    #define __strval__(v__) 'v__'
    #define definePrinter(__name) \
    __name = { \
        private _ftData = _this; \
        if equalTypes(_ftData,[]) then { \
            if (count _ftData > 0 && {equalTypes(_ftData select 0,"")}) then { \
                _ftData = format _ftData; \
            }; \
        }; \
        [_ftData, __strval__(__name) ] call hvmPrint; \
    };

    definePrinter(cprint)
    definePrinter(cprintErr)
    definePrinter(cprintWarn)
    definePrinter(discLog)
    definePrinter(discError)
    definePrinter(discWarning)


    definePrinter(logCritical)
    definePrinter(logError)
    definePrinter(logWarn)
    definePrinter(logInfo)
    definePrinter(logDebug)
    definePrinter(logTrace)

    definePrinter(systemLog)
    definePrinter(gameLog)
    definePrinter(rpLog)
    definePrinter(lifeLog)
    definePrinter(adminLog)
    definePrinter(combatLog)


};
call hostVM_postInit; //preinit logfile



//initialize rebgidge
["STAGE INITIALIZE REBRIDGE"] call cprint;

#include "..\..\..\ReBridge\ReBridge_init.sqf"

// активируем компонет
[] call ReBridge_start;
// Загрузим проект со скриптами (Путь должен быть полным)

private _scriptPath = ((call ReBridge_getWorkspace) + ("\Scripts\HostVM.reproj"));
["Script ReBridge path: %1",_scriptPath] call cprint;

private _buildResult =[_scriptPath] call rescript_build;
if (_buildResult != "ok") exitWith {
    ["ReBridge initialization error; Result: %1",_buildResult] call cprintErr;
    call hostVM_fatalShutdownServer;
};

["Initialize scripts..."] call cprint;

["Breakpoint"] call rescript_initScript;
["ScriptContext"] call rescript_initScript;
["WorkspaceHelper"] call rescript_initScript;
["FileManager"] call rescript_initScript;
["HostVM"] call rescript_initScript;

//todo remove this test critical exit
["HostVM","exit",[-100500]] call rescript_callCommandVoid;