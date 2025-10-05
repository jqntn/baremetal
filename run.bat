clang -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c thirdparty\xvg\vga\vga.c -o vga.o
clang -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c thirdparty\xvg\sgl\sgl.c -o sgl.o

clang++ -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c kernel.cpp -o kernel.o

ld.lld -m elf_i386 kernel.o vga.o sgl.o -o kernel.elf

qemu-system-i386 -kernel kernel.elf -display sdl -vga std