extern "C"
{
#include "thirdparty/xvg/sgl/sgl.h"
#include "thirdparty/xvg/vga/vga.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_VGA_DEVICE_ID 0x1050
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

  asm(".section .multiboot;"
      ".align 4;"
      "multiboot_header:;"
      ".long 0x1BADB002;"
      ".long 0;"
      ".long -(0x1BADB002+0);");

  void outb(unsigned short port, unsigned char val)
  {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
  }

  unsigned char insb(unsigned short port)
  {
    unsigned char ret = 0;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
  }

  void outl(unsigned short port, unsigned int val)
  {
    asm volatile("outl %0,%1" : : "a"(val), "Nd"(port));
  }

  unsigned int insl(unsigned short port)
  {
    unsigned int ret = 0;
    asm volatile("inl %1,%0" : "=a"(ret) : "Nd"(port));
    return ret;
  }

  static unsigned int pci_config_read(int bus, int slot, int func, int offset)
  {
    outl(PCI_CONFIG_ADDRESS,
         (1U << 31) | (bus << 16) | (slot << 11) | (func << 8) |
           (offset & 0xFC));
    return insl(PCI_CONFIG_DATA);
  }

  static unsigned long long find_virtio_vga()
  {
    for (int bus = 0; bus < 256; ++bus)
      for (int slot = 0; slot < 32; ++slot)
        for (int func = 0; func < 8; ++func)
          if (pci_config_read(bus, slot, func, 0) ==
              (VIRTIO_VENDOR_ID | (VIRTIO_VGA_DEVICE_ID << 16)))
            return pci_config_read(bus, slot, func, 0x10) & 0xFFFFFFF0;
    return 0;
  }

  void _start()
  {
    volatile unsigned short* vgat = (unsigned short*)0xB8000;

    for (int i = 0; i < 80 * 25; ++i)
      vgat[i] = (0x0F << 8) | ' ';

    const char* msg = "Hello, world!";
    for (int i = 0; msg[i]; ++i)
      vgat[i] = (0x0F << 8) | msg[i];

    // xvInitGfxMode(MODE13H);

    volatile unsigned int* virtio_fb = (unsigned int*)find_virtio_vga();

    for (int y = 0; y < SCREEN_HEIGHT; ++y)
      for (int x = 0; x < SCREEN_WIDTH; ++x)
        virtio_fb[y * SCREEN_WIDTH + x] = 0xFF0000;

    for (;;)
      ;
  }
}