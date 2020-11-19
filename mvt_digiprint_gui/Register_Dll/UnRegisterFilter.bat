@Echo Off
	Title	Un-Register rx_AlignFilter
	Echo	Filter werden un-registriert ...
	Echo.
	
:Reg_rx_AlignFilter
	cd %~dp0
	Echo.	rx_AlignFilter.dll ...
	C:\Windows\SysWOW64\regsvr32.exe "rx_AlignFilter.dll" -u
	If errorlevel 3 goto Error
	
	Goto Succes

:Error
	Echo.
	Echo	Filter konnte nicht un-registriert werden %errorlevel%
	Echo.
	Goto Ende_Batch


:Succes
	Echo.
	Echo	Filter un-egistriert
	Echo.

:Ende_Batch
	pause
