@ECHO OFF

PUSHD libtorch-1.9.0
IF NOT EXIST ..\..\include (MOVE include ..\..\include)
IF NOT EXIST ..\..\bin (MOVE bin ..\..\bin)
IF NOT EXIST ..\..\lib (MOVE lib ..\..\lib)
IF NOT EXIST ..\..\share (MOVE share ..\..\share)
