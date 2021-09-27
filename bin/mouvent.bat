SET pathbin=%~dp0
taskkill /F /T /IM rx_spooler_ctrl.exe
taskkill /F /T /IM rx_main_ctrl.exe
start %PATHBIN%\win\rx_main_ctrl.exe
