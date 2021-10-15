@ECHO OFF

SET OUTDIR=out
SET BINDIR=bin
SET TOOLDIR=%~dp0deps\.tools\bin

IF "%1" == "clean" (
    IF EXIST %OUTDIR% (RMDIR /S/Q %OUTDIR%)
    IF EXIST %BINDIR% (RMDIR /S/Q %BINDIR%)
    GOTO end
)

%TOOLDIR%\cloc.exe --quiet src
FOR /R %~dp0src %%f IN (*.h *.cpp) DO (%TOOLDIR%\clang-format.exe -i %%f)

PUSHD C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
CALL VC\Auxiliary\Build\vcvars64.bat
POPD

IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=%~dp0deps\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DCMAKE_C_COMPILER="clang-cl" ^
    -DCMAKE_CXX_COMPILER="clang-cl" ^
    -DCMAKE_LINKER="lld-link" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%~dp0%BINDIR%\ ^
    -DCMAKE_BUILD_TYPE=Debug ^
    ..
IF %ERRORLEVEL% == 0 (ninja %1)
POPD

:end
