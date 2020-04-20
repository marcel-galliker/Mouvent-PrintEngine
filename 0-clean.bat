
rd ".\Workspace\RemoteSystemsTempFiles" /S /Q

rd ".\metadata"
rd  ".\Workspace\.metadata\.plugins\*.refactor" /S /Q
del ".\Workspace\\.metadata\\.plugins\*.pdom" /S /Q
rd  ".\Workspace\.metadata\.plugins\org.eclipse.core.resources\.history" /S /Q
rd  ".\Workspace\.metadata\.plugins\org.python.pydev" /S /Q

del *.zip /S /Q
del *.7z /S /Q
del *.sdf /S /Q
del *.bak /S /Q
del *.old /S /Q
del *.db /S /Q


rd .\ipch /S /Q
rd .vs /S /Q
rd .\CodeDB /S /Q
rd .\_out /S /Q
rd .\_temp /S /Q

rd .\rx_digiprint_app\rx_digiprint_app\rx_digiprint_app\bin /S /Q
rd .\rx_digiprint_app\rx_digiprint_app\rx_digiprint_app\obj /S /Q
rd .\rx_digiprint_app\rx_digiprint_app\rx_digiprint_app.Droid\bin /S /Q
rd .\rx_digiprint_app\rx_digiprint_app\rx_digiprint_app.Droid\obj /S /Q

echo off
for /F "delims=\" %%I in ('dir /ad /b .\') DO (
    cd ".\%%I"
    cd
    rd /S /Q "ipch"
    rd /S /Q "Debug"
    rd /S /Q "_out"
    rd /S /Q "_temp"
    rd /S /Q "CodeDB"
    rd /S /Q "Debug-soc"
    rd /S /Q "Release"
    rd /S /Q "Release-soc"
    rd /S /Q "x64"
    rd /S /Q "Obj"
    rd /S /Q "Bin"
    rd /S /Q "VisualGDBCache"
    rd /S /Q ".vs"
    cd..
)
rd /S /Q .\rx_head_nios\software\SAPE_65\obj

cd RX-LabelComposer
call 0-clean.bat
