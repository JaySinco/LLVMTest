@ECHO OFF

PUSHD Python-3.8.10

@REM .\PCbuild\build.bat -p x64

IF NOT EXIST ..\..\include (
    XCOPY /E/Y/I/F .\Include ..\..\include
    XCOPY /E/Y/I/F .\PC\pyconfig.h ..\..\include
)
IF NOT EXIST ..\..\lib (XCOPY /E/Y/I/F .\PCbuild\amd64\python38.lib ..\..\lib\)
IF NOT EXIST ..\..\bin (
    XCOPY /E/Y/I/F .\PCbuild\amd64\*.pyd ..\..\bin\Lib\
    XCOPY /E/Y/I/F .\PCbuild\amd64\*.dll ..\..\bin\Lib\
    XCOPY /E/Y/I/F .\PCbuild\amd64\python.exe ..\..\bin\
    DEL /Q ..\..\bin\Lib\vcruntime140*.dll
    MOVE ..\..\bin\Lib\python3*.dll ..\..\bin\
    7z a ..\..\bin\Lib\python38.zip .\Lib\*
)

echo .\Lib> ..\..\bin\python38._pth
echo .\Lib\python38.zip>> ..\..\bin\python38._pth
echo .>> ..\..\bin\python38._pth
echo import site>> ..\..\bin\python38._pth
