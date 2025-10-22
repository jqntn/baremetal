@echo off

setlocal

set CLANG_FLAGS=^
--target=x86_64-elf ^
-c ^
-Os ^
-Wall ^
-Wextra ^
-Werror ^
-ffreestanding ^
-fno-exceptions ^
-fno-unwind-tables ^
-fno-rtti ^
-fno-stack-protector ^
-fno-strict-aliasing ^
-fno-builtin ^
-fno-common ^
-nodefaultlibs ^
-I"thirdparty" ^
-I"thirdparty\printf" ^
-I"thirdparty\libc\include" ^
-I"thirdparty\libc\arch\x86_64\include" ^
-I"thirdparty\gdtoa\include" ^
-I"thirdparty\openlibm\include" ^
-I"thirdparty\openlibm\src" ^
-I"thirdparty\libmemory\include" ^
-DPRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD

set CLANG_FLAGS_KERNEL=^
-std=c99 ^
-Wstrict-prototypes ^
-Wunreachable-code ^
-Wshadow

set LD_FLAGS=^
-m elf_x86_64 ^
--entry=_start ^
--Ttext=0x100000 ^
--image-base=0x100000 ^
--omagic ^
--static ^
--nostdlib

if not exist bin md bin
if not exist obj md obj

clang %CLANG_FLAGS% %CLANG_FLAGS_KERNEL% "kernel.c" -o "obj\kernel.o" || exit /b 1

for /r "thirdparty\arith64" %%f in (*.c) do clang %CLANG_FLAGS% "%%f" -o "obj\%%~nf.o" || exit /b 1
for /r "thirdparty\printf" %%f in (*.c) do clang %CLANG_FLAGS% "%%f" -o "obj\%%~nf.o" || exit /b 1
for /r "thirdparty\libc\src" %%f in (*.c) do clang %CLANG_FLAGS% -DDISABLE_UNIMPLEMENTED_LIBC_APIS "%%f" -o "obj\%%~nf.o" || exit /b 1
for /r "thirdparty\gdtoa\src" %%f in (*.c) do clang %CLANG_FLAGS% -DNO_ERRNO -DIFNAN_CHECK -DGDTOA_NO_ASSERT -DNO_FENV_H -DNO_IEEE_Scale "%%f" -o "obj\%%~nf.o" || exit /b 1
for /r "thirdparty\libmemory\src" %%f in (*.c) do clang %CLANG_FLAGS% "%%f" -o "obj\%%~nf.o" || exit /b 1

ld.lld %LD_FLAGS% "obj\*.o" -o "boot\kernel.elf" || exit /b 1

"thirdparty\simpleboot\simpleboot.exe" -vv -c -k "kernel.elf" "boot" "bin\disk.img"
qemu-system-x86_64 -drive file="bin\disk.img",format=raw -display sdl -vga std -serial stdio

endlocal