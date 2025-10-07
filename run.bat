set CLANG_FLAGS=-c --target=i386-elf -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-strict-aliasing

md bin
md boot

clang %CLANG_FLAGS% thirdparty\xvg\vga\vga.c -o bin\vga.o
clang %CLANG_FLAGS% thirdparty\xvg\sgl\sgl.c -o bin\sgl.o

clang++ %CLANG_FLAGS% kernel.cpp -o bin\kernel.o

ld.lld -m elf_i386 --Ttext=0x100000 --image-base=0x100000 --omagic --nostdlib --static bin\kernel.o bin\vga.o bin\sgl.o -o boot\kernel.elf

qemu-system-i386 -kernel boot\kernel.elf -display sdl -vga std