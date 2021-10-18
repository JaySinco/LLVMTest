@ECHO OFF

SET OUTDIR=out
SET BINDIR=bin

IF "%1" == "clean" (
    IF EXIST %OUTDIR% (RMDIR /S/Q %OUTDIR%)
    IF EXIST %BINDIR% (RMDIR /S/Q %BINDIR%)
    GOTO end
)

PUSHD C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
CALL VC\Auxiliary\Build\vcvars64.bat
POPD

IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DVCPKG_ROOT_DIR=%~dp0deps\vcpkg ^
    -DCMAKE_C_COMPILER="clang-cl" ^
    -DCMAKE_CXX_COMPILER="clang-cl" ^
    -DCMAKE_LINKER="lld-link" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%~dp0%BINDIR%\ ^
    -DCMAKE_BUILD_TYPE=Debug ^
    ..
IF %ERRORLEVEL% == 0 (ninja %1)
POPD

:end
