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

