@echo off

setlocal

set CLANG_FLAGS=-c --target=x86_64-elf -Os -Wall -Wextra -Werror -ffreestanding -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -fno-strict-aliasing -fno-builtin -fno-common -nodefaultlibs -I"thirdparty" -I"thirdparty\printf" -I"thirdparty\libc\include" -I"thirdparty\libc\arch\x86_64\include" -I"thirdparty\gdtoa\include" -I"thirdparty\openlibm\include" -I"thirdparty\openlibm\src" -I"thirdparty\libmemory\include"
set LD_FLAGS=-m elf_x86_64 --entry=_start --Ttext=0x100000 --image-base=0x100000 --omagic --nostdlib --static

if not exist bin md bin
if not exist obj md obj
if not exist boot md boot

clang %CLANG_FLAGS% "kernel.c" -o "obj\kernel.o"

if errorlevel 1 exit /b 1

for /r "thirdparty\libc\src" %%f in (*.c) do clang %CLANG_FLAGS% -Wno-implicit-function-declaration -DDISABLE_UNIMPLEMENTED_LIBC_APIS "%%f" -o "obj\%%~nf.o"
for /r "thirdparty\gdtoa\src" %%f in (*.c) do clang %CLANG_FLAGS% -Wno-implicit-function-declaration -DNO_ERRNO -DIFNAN_CHECK -DGDTOA_NO_ASSERT -DNO_FENV_H -DNO_IEEE_Scale -c "%%f" -o "obj\%%~nf.o"
for /r "thirdparty\libmemory\src" %%f in (*.c) do clang %CLANG_FLAGS% -Wno-implicit-function-declaration "%%f" -o "obj\%%~nf.o"

if errorlevel 1 exit /b 1

ld.lld %LD_FLAGS% "obj\*.o" -o "boot\kernel.elf"

if errorlevel 1 exit /b 1

"thirdparty\simpleboot\simpleboot.exe" -vv -c -k "kernel.elf" "boot" "bin\disk.img"
qemu-system-x86_64 -drive file="bin\disk.img",format=raw -display sdl -vga std -serial stdio

endlocal