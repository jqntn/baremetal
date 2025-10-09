@echo off

setlocal

set CLANG_FLAGS=-c --target=x86_64-elf -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-strict-aliasing -fno-builtin
set LD_FLAGS=-m elf_x86_64 --entry=_start --Ttext=0x100000 --image-base=0x100000 --omagic --nostdlib --static

if not exist bin md bin
if not exist obj md obj
if not exist boot md boot

clang++ %CLANG_FLAGS% "kernel.cpp" -o "obj\kernel.o"
clang %CLANG_FLAGS% "thirdparty\xvg\vga\vga.c" -o "obj\vga.o"
clang %CLANG_FLAGS% "thirdparty\xvg\sgl\sgl.c" -o "obj\sgl.o"

ld.lld %LD_FLAGS% "obj\kernel.o" "obj\vga.o" "obj\sgl.o" -o "boot\kernel.elf"

if errorlevel 1 exit /b 1

"thirdparty\simpleboot\simpleboot.exe" -vv -c -k "kernel.elf" "boot" "bin\disk.img"
qemu-system-x86_64 -drive file="bin\disk.img",format=raw -display sdl -vga std -serial stdio

endlocal