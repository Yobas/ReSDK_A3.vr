diag_log "Start initialize engine";

hostVM_password = "123"; //check password in init.cfg
hostVM_requireLoad = true;
hostVM_fatalShutdownServer = {
	[] spawn {
		uisleep 10;
		hostVM_password serverCommand "#shutdown";
	}
};

private _fp = "src\fn_init.sqf";
private _ex = fileExists _fp;
diag_log (format["loader found - %1",_ex]);
if (_ex) then {
	ISNIL{call compile preprocessFileLineNumbers _fp};
} else {
	diag_log "Loader not found. Fatal exit";
	call hostVM_fatalShutdownServer;
};