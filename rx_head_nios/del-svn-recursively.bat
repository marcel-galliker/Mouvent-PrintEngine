@echo off
FOR /D /R %%X IN (.svn) DO (
    echo %%X
    REM RD /S /Q %%X
    DEL %%X
)
