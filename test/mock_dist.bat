cd %~dp0
python setup.py py2exe
xcopy /I /S /Y conf dist\test\conf
xcopy /I /S /Y ..\bin\gui dist\bin\gui
xcopy /I /S /Y ..\bin\win dist\bin\win

xcopy /I /S /Y mock\UI dist\UI

xcopy  /I /S /Y ..\rx_common_lib\*.h dist\rx_common_lib
mkdir dist\rx_ink_lib
copy ..\rx_ink_lib\rx_ink_common.h dist\rx_ink_lib

