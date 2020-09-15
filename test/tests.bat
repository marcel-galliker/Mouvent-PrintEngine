cd %~dp0\..
start  "" /B "C:\Program Files (x86)\Windows Application Driver\WinAppDriver.exe"
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
msbuild /p:DefineConstants="RX_CTRL_SUBNET=\"127.168.200.\"" /m /property:Configuration=Debug /property:Platform=x64 /property:SolutionDir=. rx_main_ctrl\rx_main_ctrl.vcxproj
python -m pytest -c test\pytest.ini --tb=native --junit-xml=TEST-py.xml test
