clang++ -target i386-elf -ffreestanding -fno-exceptions -fno-rtti -c kernel.cpp -o kernel.o
ld.lld kernel.o -o kernel.elf
qemu-system-i386 -kernel kernel.elf -display sdl
