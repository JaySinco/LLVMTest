@ECHO OFF

PUSHD wil-492c01b
IF NOT EXIST ..\..\include (XCOPY /E/Y/I/F .\include ..\..\include)
