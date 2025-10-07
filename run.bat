md bin
md boot

clang -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c thirdparty\xvg\vga\vga.c -o bin\vga.o
clang -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c thirdparty\xvg\sgl\sgl.c -o bin\sgl.o

clang++ -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c kernel.cpp -o bin\kernel.o

ld.lld -m elf_i386 bin\kernel.o bin\vga.o bin\sgl.o -o boot\kernel.elf

qemu-system-i386 -kernel boot\kernel.elf -display sdl -vga std