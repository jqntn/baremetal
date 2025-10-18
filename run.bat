@echo off

setlocal

set CLANG_FLAGS=-c --target=x86_64-elf -Os -Wall -Wextra -Werror -ffreestanding -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -fno-strict-aliasing -fno-builtin -fno-common
set LD_FLAGS=-m elf_x86_64 --entry=_start --Ttext=0x100000 --image-base=0x100000 --omagic --nostdlib --static

if not exist bin md bin
if not exist obj md obj
if not exist boot md boot

clang++ %CLANG_FLAGS% "kernel.cpp" -o "obj\kernel.o"

ld.lld %LD_FLAGS% "obj\*.o" -o "boot\kernel.elf"

if errorlevel 1 exit /b 1

"thirdparty\simpleboot\simpleboot.exe" -vv -c -k "kernel.elf" "boot" "bin\disk.img"
qemu-system-x86_64 -drive file="bin\disk.img",format=raw -display sdl -vga std -serial stdio

endlocal