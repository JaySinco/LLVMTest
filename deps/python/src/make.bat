@ECHO OFF

PUSHD Python-3.8.10

@REM .\PCbuild\build.bat -p x64

IF NOT EXIST ..\..\include (
    XCOPY /E/Y/I/F .\Include ..\..\include
    XCOPY /E/Y/I/F .\PC\pyconfig.h ..\..\include
)
IF NOT EXIST ..\..\lib (XCOPY /E/Y/I/F .\PCbuild\amd64\python38.lib ..\..\lib\)
IF NOT EXIST ..\..\bin (
    XCOPY /E/Y/I/F .\PCbuild\amd64\*.pyd ..\..\bin\DLLs\
    XCOPY /E/Y/I/F .\PCbuild\amd64\*.dll ..\..\bin\DLLs\
    XCOPY /E/Y/I/F .\PCbuild\amd64\python.exe ..\..\bin\
    DEL /Q ..\..\bin\DLLs\vcruntime140*.dll
    MOVE ..\..\bin\DLLs\python3*.dll ..\..\bin\
)

IF NOT EXIST ..\..\bin\Lib (
    PUSHD .\Lib
    MKDIR ..\..\..\bin\Lib\
    zip -r ..\..\..\bin\Lib\python38.zip .\*
    POPD
)

echo .\DLLs> ..\..\bin\python38._pth
echo .\Lib\python38.zip>> ..\..\bin\python38._pth
echo .>> ..\..\bin\python38._pth
echo import site>> ..\..\bin\python38._pth

@REM download %~dp0..\bin\Lib\get-pip.py
