@ECHO OFF

PUSHD C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
CALL VC\Auxiliary\Build\vcvars64.bat
POPD

SET SOURCE=libcef-92.0.27
SET OUTDIR=out
IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DCEF_RUNTIME_LIBRARY_FLAG=/MD ^
    -DCMAKE_INSTALL_PREFIX=%~dp0..\ ^
    -DCMAKE_BUILD_TYPE=Release ^
    ..\%SOURCE%
IF %ERRORLEVEL% == 0 (ninja libcef_dll_wrapper)
POPD

IF NOT EXIST ..\include (XCOPY /E/Y/I/F %SOURCE%\include ..\include)
IF NOT EXIST ..\lib (
    XCOPY /Y/F %SOURCE%\Release\libcef.lib ..\lib\
    XCOPY /Y/F %OUTDIR%\libcef_dll_wrapper\libcef_dll_wrapper.lib ..\lib\
)
IF NOT EXIST ..\bin (
    XCOPY /E/Y/I/F %SOURCE%\Resources\* ..\bin\
    XCOPY /E/Y/I/F %SOURCE%\Release\*.dll ..\bin\
    XCOPY /E/Y/I/F %SOURCE%\Release\*.bin ..\bin\
)
