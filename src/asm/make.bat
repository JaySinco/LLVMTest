@echo off

set src=test
set out=%~dp0out
if not exist %out% mkdir %out%
nasm -f bin -o %out%\%src%.bin %~dp0%src%.asm
if exist %~dp0vm fvw -r %out%\%src%.bin -w %~dp0vm\vm.vhd -a 0
