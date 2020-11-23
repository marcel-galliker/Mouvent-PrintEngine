@Echo Off
	Title	Register rx_AlignFilter
	Echo	Filter werden registriert ...
	Echo.
	
:Reg_rx_AlignFilter
	cd %~dp0
	Echo.	rx_AlignFilter.dll ...
	C:\Windows\SysWOW64\regsvr32.exe "rx_AlignFilter.dll"
	If errorlevel 3 goto Error
	
	Goto Succes

:Error
	Echo.
	Echo	Filter konnte nicht registriert werden %errorlevel%
	Echo.
	Goto Ende_Batch


:Succes
	Echo.
	Echo	Filter registriert
	Echo.

:Ende_Batch
	pause
