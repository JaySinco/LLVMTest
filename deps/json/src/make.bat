@ECHO OFF

PUSHD json-3.10.2
IF NOT EXIST ..\..\include (XCOPY /E/Y/I/F .\include ..\..\include)
