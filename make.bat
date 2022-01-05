@echo off
goto run
:usage
echo.usage: %~nx0 [flags] [target]
echo.
echo.Available flags:
echo.  -h  display this help message
echo.  -d  delete cmake cache file
echo.  -m  enable vcpkg manifest mode
echo.
goto end

:run
set DIR=%~dp0
set OUTDIR=out
set BINDIR=bin
set LLVMBIN=C:/Program Files/LLVM/bin
set CMAKECACHE=%OUTDIR%\CMakeCache.txt
set MANIFEST=off

:flags
if "%~1"=="-h" goto usage
if "%~1"=="-d" if exist %CMAKECACHE% del /q/s %CMAKECACHE% & shift & goto flags
if "%~1"=="-m" (set MANIFEST=on) & shift & goto flags

pushd C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
call VC\Auxiliary\Build\vcvars64.bat
popd

for /r %DIR%src %%f in (*.h *.cpp) do "%LLVMBIN%/clang-format.exe" -i %%f

if not exist %OUTDIR% mkdir %OUTDIR%
pushd %OUTDIR%
cmake -G Ninja ^
    -DVCPKG_ROOT_DIR=%DIR%deps\vcpkg ^
    -DVCPKG_MANIFEST_MODE=%MANIFEST% ^
    -DCMAKE_C_COMPILER="%LLVMBIN%/clang-cl.exe" ^
    -DCMAKE_CXX_COMPILER="%LLVMBIN%/clang-cl.exe" ^
    -DCMAKE_LINKER="%LLVMBIN%/lld-link.exe" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%DIR%%BINDIR%\ ^
    -DCMAKE_BUILD_TYPE=Debug ^
    ..
if %ERRORLEVEL% == 0 (ninja %1)
popd

:end
