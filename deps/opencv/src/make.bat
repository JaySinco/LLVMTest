@ECHO OFF

PUSHD C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
CALL VC\Auxiliary\Build\vcvars64.bat
POPD

SET OUTDIR=out
IF NOT EXIST %OUTDIR% (MKDIR %OUTDIR%)
PUSHD %OUTDIR%
cmake -G Ninja ^
    -DCMAKE_INSTALL_PREFIX=%~dp0..\ ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_WITH_STATIC_CRT=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DBUILD_TESTS=OFF ^
    -DBUILD_PERF_TESTS=OFF ^
    -DBUILD_opencv_apps=OFF ^
    -DBUILD_DOCS=OFF ^
    -DBUILD_JAVA=OFF ^
    -DBUILD_opencv_python2=OFF ^
    -DBUILD_opencv_python3=OFF ^
    -DWITH_CUDA=OFF ^
    -DOPENCV_EXTRA_MODULES_PATH=%~dp0opencv_contrib-4.5.3\modules ^
    ..\opencv-4.5.3
IF %ERRORLEVEL% == 0 (ninja && ninja install) 
POPD

DEL /Q ..\*.cmake
DEL /Q ..\*.cmd
DEL /Q ..\LICENSE
