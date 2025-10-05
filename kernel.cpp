extern "C"
{
#include "thirdparty/xvg/sgl/sgl.h"
#include "thirdparty/xvg/vga/vga.h"

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

  void _start()
  {
    volatile unsigned short* vgat = (unsigned short*)0xB8000;

    for (int i = 0; i < 80 * 25; ++i) {
      vgat[i] = (0x0F << 8) | ' ';
    }

    const char* msg = "Hello, world!";
    for (int i = 0; msg[i]; ++i) {
      vgat[i] = (0x0F << 8) | msg[i];
    }

    xvInitGfxMode(MODE13H);

    sglClear(BLACK);

    sglDrawRect(150, 10, 100, 50, GREEN);

    sglDrawLine(120, 14, 24, 30, RED);

    sglDrawCircle(100, 100, 50, CYAN);

    sglDrawFilledTri(100, 100, 50, 150, 120, 120, MAGENTA);

    sglDrawTri(200, 100, 150, 170, 250, 190, YELLOW);

    sglSwapBuffers();
  }
}