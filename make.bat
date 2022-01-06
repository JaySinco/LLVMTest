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
set dir=%~dp0
set outdir=out
set bindir=bin
set llvmbin=C:/Program Files/LLVM/bin
set cmakecache=%outdir%\CMakeCache.txt
set manifest=off

:flags
if "%~1"=="-h" goto usage
if "%~1"=="-d" if exist %cmakecache% del /q/s %cmakecache% & shift & goto flags
if "%~1"=="-m" (set manifest=on) & shift & goto flags

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

for /r %dir%src %%f in (*.h *.cpp) do "%llvmbin%/clang-format.exe" -i %%f

if not exist %outdir% mkdir %outdir%
pushd %outdir%
cmake -G Ninja ^
    -DVCPKG_ROOT_DIR=%dir%deps\vcpkg ^
    -DVCPKG_MANIFEST_MODE=%manifest% ^
    -DCMAKE_C_COMPILER="%llvmbin%/clang-cl.exe" ^
    -DCMAKE_CXX_COMPILER="%llvmbin%/clang-cl.exe" ^
    -DCMAKE_LINKER="%llvmbin%/lld-link.exe" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%dir%%bindir% ^
    -DCMAKE_BUILD_TYPE=Debug ^
    ..
if %errorlevel% == 0 (ninja %1)
popd

:end
