i386-elf-g++ -ffreestanding -fno-exceptions -fno-rtti -m32 -c kernel.cpp -o kernel.o
i386-elf-ld -Ttext 0x100000 kernel.o -o kernel.elf
qemu-system-i386 -kernel kernel.elf -display sdl
