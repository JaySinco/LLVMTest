@ECHO OFF

PUSHD stb-3a11740
IF NOT EXIST ..\..\include (XCOPY /Y/I/F .\*.h ..\..\include)
