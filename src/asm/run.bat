@echo off

if not exist %~dp0out mkdir %~dp0out
nasm -f bin -o %~dp0out\test.bin %~dp0test.asm
fvw -r %~dp0out\test.bin -w %~dp0vm\vm.vhd -a 0
bochsdbg.exe -q -f %~dp0bochsrc.bxrc
