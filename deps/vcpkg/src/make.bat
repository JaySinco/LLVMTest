@ECHO OFF

IF NOT EXIST ..\bin (
    git clone https://github.com/microsoft/vcpkg ..\bin
    ..\bin\bootstrap-vcpkg.bat -verbose -disableMetrics
)
