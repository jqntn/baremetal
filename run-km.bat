@echo off

setlocal

set CLANG_FLAGS=-c --target=i386-elf -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-strict-aliasing -fno-builtin
set LD_FLAGS=-m elf_i386 --entry=_start --Ttext=0x100000 --image-base=0x100000 --omagic --nostdlib --static

if not exist bin md bin
if not exist obj md obj
if not exist boot md boot

clang++ %CLANG_FLAGS% "kernel.cpp" -o "obj\kernel.o"

ld.lld %LD_FLAGS% "obj\kernel.o" -o "boot\kernel.elf"

if errorlevel 1 exit /b 1

qemu-system-x86_64 -kernel "boot\kernel.elf" -display sdl -vga std -serial stdio

endlocal