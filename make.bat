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

@REM FOR /R %~dp0src %%f IN (*.h *.cpp) DO (clang-format.exe -i %%f)

IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DVCPKG_ROOT_DIR=%~dp0deps\vcpkg ^
    -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" ^
    -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" ^
    -DCMAKE_LINKER="C:/Program Files/LLVM/bin/lld-link.exe" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%~dp0%BINDIR%\ ^
    -DCMAKE_BUILD_TYPE=Debug ^
    ..
IF %ERRORLEVEL% == 0 (ninja %1)
POPD

:end
