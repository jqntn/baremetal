extern "C"
{
#include <stdint.h>

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

  /**
   * Display (extremely minimal) formated message on serial
   * %c: an ASCII character
   * %d: a decimal number
   * %x: a hexadecimal number
   * %p: a pointer
   * %s: a zero terminated ASCII string (8 bit)
   * %S: a zero terminated WCHAR string (16 bit characters, truncated to 8 bit)
   * %D: dump 16 bytes from given address
   */
  void printf(const char* fmt, ...)
  {
    __builtin_va_list args;
    int arg, len, sign, i;
    unsigned int uarg;
    char *p, tmpstr[19], n;
    /* macro to put a character on serial console */
#ifdef __aarch64__
#define mmio_base 0x3F000000
#define UART0_DR ((volatile uint32_t*)(mmio_base + 0x00201000))
#define UART0_FR ((volatile uint32_t*)(mmio_base + 0x00201018))
#define PUTC(c)                                                                \
  do {                                                                         \
    do {                                                                       \
      __asm__ __volatile__("nop");                                             \
    } while (*UART0_FR & 0x20);                                                \
    *UART0_DR = c;                                                             \
  } while (0)
#else
#define PUTC(c)                                                                \
  __asm__ __volatile__(                                                        \
    "xorl %%ebx, %%ebx; movb %0, %%bl;"                                        \
    "movl $10000,%%ecx;"                                                       \
    "1:inb %%dx, %%al;pause;"                                                  \
    "cmpb $0xff,%%al;je 2f;"                                                   \
    "dec %%ecx;jz 2f;"                                                         \
    "andb $0x20,%%al;jz 1b;"                                                   \
    "subb $5,%%dl;movb %%bl, %%al;outb %%al, %%dx;2:" ::"a"(c),                \
    "d"(0x3fd)                                                                 \
    : "rbx", "rcx");
#endif
    /* parse format and print */
    __builtin_va_start(args, fmt);
    arg = 0;
    while (*fmt) {
      if (*fmt == '%') {
        fmt++;
        if (*fmt == '%')
          goto put;
        len = 0;
        while (*fmt >= '0' && *fmt <= '9') {
          len *= 10;
          len += *fmt - '0';
          fmt++;
        }
        if (*fmt == 'c') {
          arg = __builtin_va_arg(args, int);
          PUTC((uint8_t)arg);
          fmt++;
          continue;
        } else if (*fmt == 'd') {
          arg = __builtin_va_arg(args, int);
          sign = 0;
          if ((int)arg < 0) {
            arg = -arg;
            sign++;
          }
          i = 18;
          tmpstr[i] = 0;
          do {
            tmpstr[--i] = '0' + (arg % 10);
            arg /= 10;
          } while (arg != 0 && i > 0);
          if (sign)
            tmpstr[--i] = '-';
          if (len > 0 && len < 18) {
            while (i > 18 - len)
              tmpstr[--i] = ' ';
          }
          p = &tmpstr[i];
          goto putstring;
        } else if (*fmt == 'x') {
          uarg = __builtin_va_arg(args, unsigned int);
          i = 16;
          tmpstr[i] = 0;
          do {
            n = uarg & 0xf;
            tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
            uarg >>= 4;
          } while (uarg != 0 && i > 0);
          if (len > 0 && len <= 16) {
            while (i > 16 - len)
              tmpstr[--i] = '0';
          }
          p = &tmpstr[i];
          goto putstring;
        } else if (*fmt == 's') {
          p = __builtin_va_arg(args, char*);
        putstring:
          if (p == (void*)0)
            p = (char*)"(null)";
          while (*p)
            PUTC(*p++);
        }
      } else {
      put:
        PUTC(*fmt);
      }
      fmt++;
    }
    __builtin_va_end(args);
  }

  void _start()
  {
    printf("Hello, world!\n");

    xvInitGfxMode(MODE03H);

    volatile unsigned short* vgat = (unsigned short*)0xB8000;

    for (int i = 0; i < 80 * 25; ++i)
      vgat[i] = (0x0F << 8) | ' ';

    const char* msg = "Hello, world!";
    for (int i = 0; msg[i]; ++i)
      vgat[i] = (0x0F << 8) | msg[i];

    for (;;)
      ;
  }
}