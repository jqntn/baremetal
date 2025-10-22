:: LEGACY QEMU KERNEL MODE SUPPORT, USEFUL TO TEST VGA TEXT MODE AND I386 FOR NOW, WILL BE DROPPED

@echo off

setlocal

set CLANG_FLAGS=-c --target=i386-elf -ffreestanding -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -fno-strict-aliasing -fno-builtin -fno-common -nodefaultlibs -I"thirdparty"
set LD_FLAGS=-m elf_i386 -T "kernel.ld" --entry=_start --omagic --static --nostdlib

if not exist bin md bin
if not exist obj md obj

clang %CLANG_FLAGS% "kernel.c" -o "obj\kernel.o"

ld.lld %LD_FLAGS% "obj\kernel.o" -o "boot\kernel.elf"

if errorlevel 1 exit /b 1

qemu-system-x86_64 -kernel "boot\kernel.elf" -display sdl -vga std -serial stdio

endlocal