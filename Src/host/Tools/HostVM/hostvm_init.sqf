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

call compile preprocessFileLineNumbers "src\host\Tools\HostVM\cba_hostvm_preinit.sqf";
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