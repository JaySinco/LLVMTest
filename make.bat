@ECHO OFF

SET BUILDTYPE=Debug
SET OUTDIR=out
SET BINDIR=bin
SET TOOLDIR=%~dp0deps\.tools\bin
SET GENCPP=%~dp0src\gen-cpp
SET ANTLR4=java -jar %~dp0deps\antlr4\bin\antlr-4.9.2-complete.jar

IF "%1" == "clean" (
    IF EXIST %OUTDIR% (RMDIR /S/Q %OUTDIR%)
    IF EXIST %BINDIR% (RMDIR /S/Q %BINDIR%)
    GOTO end
)

IF "%1" == "release" (SET BUILDTYPE=Release)
ECHO -- Build configuration: "%BUILDTYPE%"

IF EXIST %GENCPP% (RMDIR /S/Q %GENCPP%)
%ANTLR4% -encoding utf8 -Dlanguage=Cpp %~dp0grammar\T*.g4 -o %GENCPP% -no-listener -no-visitor -Werror
IF NOT %ERRORLEVEL% == 0 GOTO end

%TOOLDIR%\cloc.exe --quiet src
FOR /R %~dp0src %%f IN (*.h *.cpp) DO (%TOOLDIR%\clang-format.exe -i %%f)

PUSHD C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
CALL VC\Auxiliary\Build\vcvars64.bat
POPD

IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DCMAKE_C_COMPILER="clang-cl" ^
    -DCMAKE_CXX_COMPILER="clang-cl" ^
    -DCMAKE_LINKER="lld-link" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%~dp0%BINDIR%\ ^
    -DCMAKE_BUILD_TYPE=%BUILDTYPE% ^
    ..
IF %ERRORLEVEL% == 0 (ninja)
POPD

:end
