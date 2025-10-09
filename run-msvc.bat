@echo off

setlocal

set CL_FLAGS=/nologo /c /Zl /EHs-c- /GR- /GS- /Oi /Fo"obj\\"
set LINK_FLAGS=/NOLOGO /MACHINE:X86 /ENTRY:_start /BASE:0x100000 /NODEFAULTLIB /SUBSYSTEM:NATIVE /DRIVER /MERGE:.rdata=.text

if not exist bin md bin
if not exist obj md obj
if not exist boot md boot

cl %CL_FLAGS% "kernel.cpp"

link %LINK_FLAGS% "obj\kernel.obj" /OUT:"boot\kernel.exe"

if errorlevel 1 exit /b 1

"thirdparty\simpleboot\simpleboot.exe" -vv -c -k "kernel.exe" "boot" "bin\disk.img"
qemu-system-x86_64 -drive file="bin\disk.img",format=raw -display sdl -vga std -serial stdio

endlocal