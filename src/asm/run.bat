@echo off

set src=boot
set out=%~dp0out
set vhd=%out%\vm.vhd
set vhdsize=64
set mkvhd=%out%\create_vhd.txt
set bxrc=%~dp0vm.bxrc

if not exist %out% mkdir %out%
if not exist %vhd% (
    echo create vdisk file=%vhd% maximum=%vhdsize% > %mkvhd%
    diskpart /s %mkvhd%
)
nasm -f bin -o %out%\%src%.bin %~dp0%src%.asm && ^
%~dp0..\..\bin\fixed-vhd-writer.exe -r %out%\%src%.bin -w %vhd% -a 0 && ^
bochs.exe -q -f %bxrc%
