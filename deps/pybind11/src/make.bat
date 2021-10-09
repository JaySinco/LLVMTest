@ECHO OFF

PUSHD pybind11-2.8.0
IF NOT EXIST ..\..\include (XCOPY /E/Y/I/F .\include ..\..\include)
