extern "C"
{
#include <stdarg.h>
#include <stdint.h>

#include "thirdparty/simpleboot/loader.h"
#include "thirdparty/simpleboot/simpleboot.h"

#ifndef _MSC_VER
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

  void memcpy(void* dst, const void* src, uint32_t n)
  {
    asm volatile("repnz movsb" : : "D"(dst), "S"(src), "c"(n) :);
  }

  void memset(void* dst, uint8_t c, uint32_t n)
  {
    asm volatile("repnz stosb" : : "D"(dst), "a"(c), "c"(n) :);
  }

  int memcmp(const void* s1, const void* s2, uint32_t n)
  {
    int ret = 0;
    asm volatile("cld; repe cmpsb; xorl %%eax, %%eax; movb -1(%%rdi), "
                 "%%al; subb -1(%%rsi), %%al;"
                 : "=a"(ret)
                 : "D"(s1), "S"(s2), "c"(n)
                 :);
    return ret;
  }
#endif

  /**
   * Display (extremely minimal) formated message on serial
   */
  void printf_serial(const char* fmt, ...)
  {
#ifndef _MSC_VER
    va_list args;
    int arg, len, sign, i;
    unsigned int uarg;
    char *p, tmpstr[19]{}, n;
#define PUTC(c)                                                                \
  asm volatile("xorl %%ebx, %%ebx; movb %0, %%bl;"                             \
               "movl $10000, %%ecx;"                                           \
               "1: inb %%dx, %%al; pause;"                                     \
               "cmpb $0xff, %%al; je 2f;"                                      \
               "dec %%ecx; jz 2f;"                                             \
               "andb $0x20, %%al; jz 1b;"                                      \
               "subb $5, %%dl; movb %%bl, %%al; outb %%al, %%dx; 2:;"          \
               :                                                               \
               : "a"(c), "d"(0x3fd)                                            \
               : "rbx", "rcx");
    va_start(args, fmt);
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
          arg = va_arg(args, int);
          PUTC((unsigned char)arg);
          fmt++;
          continue;
        } else if (*fmt == 'd') {
          arg = va_arg(args, int);
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
          uarg = va_arg(args, unsigned int);
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
          p = va_arg(args, char*);
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
    va_end(args);
#endif
  }

#define CONSOLE_SERIAL 0x3f8
#define CONSOLE_FB

#ifdef CONSOLE_FB
  typedef struct
  {
    uint32_t magic, version, headersize, flags, numglyph, bytesperglyph, height,
      width;
  } __attribute__((packed)) psf2_t;
  uint8_t font_psf[2080] = {
    114, 181, 74,  134, 0,   0,   0,   0,   32,  0,   0,   0,   0,   0,   12,
    0,   128, 0,   0,   0,   16,  0,   0,   0,   16,  0,   0,   0,   8,   0,
    0,   0,   0,   0,   218, 2,   128, 130, 2,   128, 130, 2,   128, 182, 0,
    0,   0,   0,   0,   0,   126, 129, 165, 129, 129, 189, 153, 129, 129, 126,
    0,   0,   0,   0,   0,   0,   126, 255, 219, 255, 255, 195, 231, 255, 255,
    126, 0,   0,   0,   0,   0,   0,   0,   0,   108, 254, 254, 254, 254, 124,
    56,  16,  0,   0,   0,   0,   0,   0,   0,   0,   16,  56,  124, 254, 124,
    56,  16,  0,   0,   0,   0,   0,   0,   0,   0,   24,  60,  60,  231, 231,
    231, 24,  24,  60,  0,   0,   0,   0,   0,   0,   0,   24,  60,  126, 255,
    255, 126, 24,  24,  60,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    24,  60,  60,  24,  0,   0,   0,   0,   0,   0,   255, 255, 255, 255, 255,
    255, 231, 195, 195, 231, 255, 255, 255, 255, 255, 255, 0,   0,   0,   0,
    0,   60,  102, 66,  66,  102, 60,  0,   0,   0,   0,   0,   255, 255, 255,
    255, 255, 195, 153, 189, 189, 153, 195, 255, 255, 255, 255, 255, 0,   0,
    30,  14,  26,  50,  120, 204, 204, 204, 204, 120, 0,   0,   0,   0,   0,
    0,   60,  102, 102, 102, 102, 60,  24,  126, 24,  24,  0,   0,   0,   0,
    0,   0,   63,  51,  63,  48,  48,  48,  48,  112, 240, 224, 0,   0,   0,
    0,   0,   0,   127, 99,  127, 99,  99,  99,  99,  103, 231, 230, 192, 0,
    0,   0,   0,   0,   0,   24,  24,  219, 60,  231, 60,  219, 24,  24,  0,
    0,   0,   0,   0,   128, 192, 224, 240, 248, 254, 248, 240, 224, 192, 128,
    0,   0,   0,   0,   0,   2,   6,   14,  30,  62,  254, 62,  30,  14,  6,
    2,   0,   0,   0,   0,   0,   0,   24,  60,  126, 24,  24,  24,  126, 60,
    24,  0,   0,   0,   0,   0,   0,   0,   102, 102, 102, 102, 102, 102, 102,
    0,   102, 102, 0,   0,   0,   0,   0,   0,   127, 219, 219, 219, 123, 27,
    27,  27,  27,  27,  0,   0,   0,   0,   0,   124, 198, 96,  56,  108, 198,
    198, 108, 56,  12,  198, 124, 0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   254, 254, 254, 254, 0,   0,   0,   0,   0,   0,   24,  60,  126,
    24,  24,  24,  126, 60,  24,  126, 0,   0,   0,   0,   0,   0,   24,  60,
    126, 24,  24,  24,  24,  24,  24,  24,  0,   0,   0,   0,   0,   0,   24,
    24,  24,  24,  24,  24,  24,  126, 60,  24,  0,   0,   0,   0,   0,   0,
    0,   0,   0,   24,  12,  254, 12,  24,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   48,  96,  254, 96,  48,  0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   192, 192, 192, 254, 0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   40,  108, 254, 108, 40,  0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   16,  56,  56,  124, 124, 254, 254, 0,   0,
    0,   0,   0,   0,   0,   0,   0,   254, 254, 124, 124, 56,  56,  16,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   24,  60,  60,  60,  24,  24,  24,  0,
    24,  24,  0,   0,   0,   0,   0,   102, 102, 102, 36,  0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   108, 108, 254, 108, 108,
    108, 254, 108, 108, 0,   0,   0,   0,   24,  24,  124, 198, 194, 192, 124,
    6,   6,   134, 198, 124, 24,  24,  0,   0,   0,   0,   0,   0,   194, 198,
    12,  24,  48,  96,  198, 134, 0,   0,   0,   0,   0,   0,   56,  108, 108,
    56,  118, 220, 204, 204, 204, 118, 0,   0,   0,   0,   0,   48,  48,  48,
    32,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   12,
    24,  48,  48,  48,  48,  48,  48,  24,  12,  0,   0,   0,   0,   0,   0,
    48,  24,  12,  12,  12,  12,  12,  12,  24,  48,  0,   0,   0,   0,   0,
    0,   0,   0,   0,   102, 60,  255, 60,  102, 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   24,  24,  126, 24,  24,  0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   24,  24,  24,  48,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   254, 0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   24,  24,
    0,   0,   0,   0,   0,   0,   0,   0,   2,   6,   12,  24,  48,  96,  192,
    128, 0,   0,   0,   0,   0,   0,   56,  108, 198, 198, 214, 214, 198, 198,
    108, 56,  0,   0,   0,   0,   0,   0,   24,  56,  120, 24,  24,  24,  24,
    24,  24,  126, 0,   0,   0,   0,   0,   0,   124, 198, 6,   12,  24,  48,
    96,  192, 198, 254, 0,   0,   0,   0,   0,   0,   124, 198, 6,   6,   60,
    6,   6,   6,   198, 124, 0,   0,   0,   0,   0,   0,   12,  28,  60,  108,
    204, 254, 12,  12,  12,  30,  0,   0,   0,   0,   0,   0,   254, 192, 192,
    192, 252, 6,   6,   6,   198, 124, 0,   0,   0,   0,   0,   0,   56,  96,
    192, 192, 252, 198, 198, 198, 198, 124, 0,   0,   0,   0,   0,   0,   254,
    198, 6,   6,   12,  24,  48,  48,  48,  48,  0,   0,   0,   0,   0,   0,
    124, 198, 198, 198, 124, 198, 198, 198, 198, 124, 0,   0,   0,   0,   0,
    0,   124, 198, 198, 198, 126, 6,   6,   6,   12,  120, 0,   0,   0,   0,
    0,   0,   0,   0,   24,  24,  0,   0,   0,   24,  24,  0,   0,   0,   0,
    0,   0,   0,   0,   0,   24,  24,  0,   0,   0,   24,  24,  48,  0,   0,
    0,   0,   0,   0,   0,   6,   12,  24,  48,  96,  48,  24,  12,  6,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   126, 0,   0,   126, 0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   96,  48,  24,  12,  6,   12,  24,  48,
    96,  0,   0,   0,   0,   0,   0,   124, 198, 198, 12,  24,  24,  24,  0,
    24,  24,  0,   0,   0,   0,   0,   0,   0,   124, 198, 198, 222, 222, 222,
    220, 192, 124, 0,   0,   0,   0,   0,   0,   16,  56,  108, 198, 198, 254,
    198, 198, 198, 198, 0,   0,   0,   0,   0,   0,   252, 102, 102, 102, 124,
    102, 102, 102, 102, 252, 0,   0,   0,   0,   0,   0,   60,  102, 194, 192,
    192, 192, 192, 194, 102, 60,  0,   0,   0,   0,   0,   0,   248, 108, 102,
    102, 102, 102, 102, 102, 108, 248, 0,   0,   0,   0,   0,   0,   254, 102,
    98,  104, 120, 104, 96,  98,  102, 254, 0,   0,   0,   0,   0,   0,   254,
    102, 98,  104, 120, 104, 96,  96,  96,  240, 0,   0,   0,   0,   0,   0,
    60,  102, 194, 192, 192, 222, 198, 198, 102, 58,  0,   0,   0,   0,   0,
    0,   198, 198, 198, 198, 254, 198, 198, 198, 198, 198, 0,   0,   0,   0,
    0,   0,   60,  24,  24,  24,  24,  24,  24,  24,  24,  60,  0,   0,   0,
    0,   0,   0,   30,  12,  12,  12,  12,  12,  204, 204, 204, 120, 0,   0,
    0,   0,   0,   0,   230, 102, 102, 108, 120, 120, 108, 102, 102, 230, 0,
    0,   0,   0,   0,   0,   240, 96,  96,  96,  96,  96,  96,  98,  102, 254,
    0,   0,   0,   0,   0,   0,   198, 238, 254, 254, 214, 198, 198, 198, 198,
    198, 0,   0,   0,   0,   0,   0,   198, 230, 246, 254, 222, 206, 198, 198,
    198, 198, 0,   0,   0,   0,   0,   0,   124, 198, 198, 198, 198, 198, 198,
    198, 198, 124, 0,   0,   0,   0,   0,   0,   252, 102, 102, 102, 124, 96,
    96,  96,  96,  240, 0,   0,   0,   0,   0,   0,   124, 198, 198, 198, 198,
    198, 198, 214, 222, 124, 12,  14,  0,   0,   0,   0,   252, 102, 102, 102,
    124, 108, 102, 102, 102, 230, 0,   0,   0,   0,   0,   0,   124, 198, 198,
    96,  56,  12,  6,   198, 198, 124, 0,   0,   0,   0,   0,   0,   126, 126,
    90,  24,  24,  24,  24,  24,  24,  60,  0,   0,   0,   0,   0,   0,   198,
    198, 198, 198, 198, 198, 198, 198, 198, 124, 0,   0,   0,   0,   0,   0,
    198, 198, 198, 198, 198, 198, 198, 108, 56,  16,  0,   0,   0,   0,   0,
    0,   198, 198, 198, 198, 214, 214, 214, 254, 238, 108, 0,   0,   0,   0,
    0,   0,   198, 198, 108, 124, 56,  56,  124, 108, 198, 198, 0,   0,   0,
    0,   0,   0,   102, 102, 102, 102, 60,  24,  24,  24,  24,  60,  0,   0,
    0,   0,   0,   0,   254, 198, 134, 12,  24,  48,  96,  194, 198, 254, 0,
    0,   0,   0,   0,   0,   60,  48,  48,  48,  48,  48,  48,  48,  48,  60,
    0,   0,   0,   0,   0,   0,   0,   128, 192, 224, 112, 56,  28,  14,  6,
    2,   0,   0,   0,   0,   0,   0,   60,  12,  12,  12,  12,  12,  12,  12,
    12,  60,  0,   0,   0,   0,   16,  56,  108, 198, 0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   255, 0,   0,   48,  48,  24,  0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   120,
    12,  124, 204, 204, 204, 118, 0,   0,   0,   0,   0,   0,   224, 96,  96,
    120, 108, 102, 102, 102, 102, 124, 0,   0,   0,   0,   0,   0,   0,   0,
    0,   124, 198, 192, 192, 192, 198, 124, 0,   0,   0,   0,   0,   0,   28,
    12,  12,  60,  108, 204, 204, 204, 204, 118, 0,   0,   0,   0,   0,   0,
    0,   0,   0,   124, 198, 254, 192, 192, 198, 124, 0,   0,   0,   0,   0,
    0,   56,  108, 100, 96,  240, 96,  96,  96,  96,  240, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   118, 204, 204, 204, 204, 204, 124, 12,  204, 120,
    0,   0,   0,   224, 96,  96,  108, 118, 102, 102, 102, 102, 230, 0,   0,
    0,   0,   0,   0,   24,  24,  0,   56,  24,  24,  24,  24,  24,  60,  0,
    0,   0,   0,   0,   0,   6,   6,   0,   14,  6,   6,   6,   6,   6,   6,
    102, 102, 60,  0,   0,   0,   224, 96,  96,  102, 108, 120, 120, 108, 102,
    230, 0,   0,   0,   0,   0,   0,   56,  24,  24,  24,  24,  24,  24,  24,
    24,  60,  0,   0,   0,   0,   0,   0,   0,   0,   0,   236, 254, 214, 214,
    214, 214, 198, 0,   0,   0,   0,   0,   0,   0,   0,   0,   220, 102, 102,
    102, 102, 102, 102, 0,   0,   0,   0,   0,   0,   0,   0,   0,   124, 198,
    198, 198, 198, 198, 124, 0,   0,   0,   0,   0,   0,   0,   0,   0,   220,
    102, 102, 102, 102, 102, 124, 96,  96,  240, 0,   0,   0,   0,   0,   0,
    118, 204, 204, 204, 204, 204, 124, 12,  12,  30,  0,   0,   0,   0,   0,
    0,   220, 118, 102, 96,  96,  96,  240, 0,   0,   0,   0,   0,   0,   0,
    0,   0,   124, 198, 96,  56,  12,  198, 124, 0,   0,   0,   0,   0,   0,
    16,  48,  48,  252, 48,  48,  48,  48,  54,  28,  0,   0,   0,   0,   0,
    0,   0,   0,   0,   204, 204, 204, 204, 204, 204, 118, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   102, 102, 102, 102, 102, 60,  24,  0,   0,   0,
    0,   0,   0,   0,   0,   0,   198, 198, 214, 214, 214, 254, 108, 0,   0,
    0,   0,   0,   0,   0,   0,   0,   198, 108, 56,  56,  56,  108, 198, 0,
    0,   0,   0,   0,   0,   0,   0,   0,   198, 198, 198, 198, 198, 198, 126,
    6,   12,  248, 0,   0,   0,   0,   0,   0,   254, 204, 24,  48,  96,  198,
    254, 0,   0,   0,   0,   0,   0,   14,  24,  24,  24,  112, 24,  24,  24,
    24,  14,  0,   0,   0,   0,   0,   0,   24,  24,  24,  24,  24,  24,  24,
    24,  24,  24,  24,  24,  0,   0,   0,   0,   112, 24,  24,  24,  14,  24,
    24,  24,  24,  112, 0,   0,   0,   0,   0,   0,   118, 220, 0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   16,  56,
    108, 198, 198, 198, 254, 0,   0,   0,   0,   0
  };
  uint32_t fb_x, fb_y;
#endif
#ifdef CONSOLE_VGA
  uint16_t vga_x, vga_y;
#endif

  fossbios_t* FB;
  multiboot_tag_framebuffer_t vidmode;
  uint32_t fb_bg;

  void console_init()
  {
#ifdef CONSOLE_SERIAL
    if (FB && FB->serial)
      FB->serial->setmode(0, 115200, 8, 0, 1);
    else
      asm volatile("movl %0, %%edx;"
                   "xorb %%al, %%al; outb %%al, %%dx;"
                   "movb $0x80, %%al; addb $2, %%dl; outb %%al, %%dx;"
                   "movb $1, %%al; subb $3, %%dl; outb %%al, %%dx;"
                   "xorb %%al, %%al; incb %%dl; outb %%al, %%dx;"
                   "incb %%dl; outb %%al, %%dx;"
                   "movb $0x43, %%al; incb %%dl; outb %%al, %%dx;"
                   "movb $0x8, %%al; incb %%dl; outb %%al, %%dx;"
                   "xorb %%al, %%al; subb $4, %%dl; inb %%dx, %%al"
                   :
                   : "a"(CONSOLE_SERIAL + 1)
                   : "rdx");
#endif
#ifdef CONSOLE_FB
    fb_x = fb_y = 4;
#endif
#ifdef CONSOLE_VGA
    vga_x = vga_y = 0;
    if (!vidmode.framebuffer_addr && !ST)
      memset((void*)0xB8000, 0, 160 * 25);
#endif
#ifdef CONSOLE_UEFI
    if (ST && ST->ConOut)
      ST->ConOut->Reset(ST->ConOut, 0);
#endif
  }

  void console_putc(uint8_t c)
  {
#ifdef CONSOLE_UEFI
    uint16_t tmp[2];
#endif
#ifdef CONSOLE_FB
    psf2_t* font = (psf2_t*)font_psf;
    uint32_t x, y, line, mask, offs, bpl = (font->width + 7) >> 3;
    uint8_t *glyph, *fb = (uint8_t*)vidmode.framebuffer_addr;
#endif
    if (FB && FB->serial)
      FB->serial->send(0, c);
    else
      asm volatile("xorl %%ebx, %%ebx; movb %0, %%bl;"
#ifdef CONSOLE_SERIAL
                   "movl $10000, %%ecx;"
                   "1: inb %%dx, %%al; pause;"
                   "cmpb $0xff, %%al; je 2f;"
                   "dec %%ecx; jz 2f;"
                   "andb $0x20, %%al; jz 1b;"
                   "subb $5, %%dl; movb %%bl, %%al; outb %%al, %%dx; 2:;"
#endif
#ifdef CONSOLE_BOCHS_E9
                   "movb %%bl, %%al; outb %%al, $0xe9;"
#endif
                   :
                   : "a"(c), "d"(CONSOLE_SERIAL + 5)
                   : "rbx", "rcx");

#ifdef CONSOLE_FB
    if (fb)
      switch (c) {
        case '\r':
          fb_x = 4;
          break;
        case '\n':
          fb_x = 4;
          fb_y += font->height;
          break;
        default:
          if (fb_x + font->width + 5 >= vidmode.framebuffer_width) {
            fb_x = 4;
            fb_y += font->height;
          }
          if (fb_y + font->height + 5 > vidmode.framebuffer_height) {
            x = fb_y;
            fb_y = vidmode.framebuffer_height - font->height - 5;
            x -= fb_y;
            offs = 0;
            line = x * vidmode.framebuffer_pitch;
            for (y = x; y < vidmode.framebuffer_height; y++,
                offs += vidmode.framebuffer_pitch,
                line += vidmode.framebuffer_pitch)
              memcpy(fb + offs, fb + line, vidmode.framebuffer_pitch);
            for (y = fb_y, offs = fb_y * vidmode.framebuffer_pitch;
                 y < vidmode.framebuffer_height;
                 y++, offs += vidmode.framebuffer_pitch)
              memset(fb + offs, 0, vidmode.framebuffer_pitch);
          }
          glyph = font_psf + font->headersize +
                  (c > 0 && c < font->numglyph ? c : 0) * font->bytesperglyph;
          offs = fb_y * vidmode.framebuffer_pitch +
                 fb_x * ((vidmode.framebuffer_bpp + 7) >> 3);
          fb_x += (font->width + 1);
          for (y = 0; y < font->height;
               y++, glyph += bpl, offs += vidmode.framebuffer_pitch) {
            line = offs;
            mask = 1 << (font->width - 1);
            for (x = 0; x < font->width && mask; x++, mask >>= 1) {
              switch (vidmode.framebuffer_bpp) {
                case 15:
                case 16:
                  *((uint16_t*)(fb + line)) =
                    ((int)*glyph) & mask ? 0xFFFF : fb_bg;
                  line += 2;
                  break;
                case 24:
                  *((uint32_t*)(fb + line)) =
                    ((int)*glyph) & mask ? 0xFFFFFF : fb_bg;
                  line += 3;
                  break;
                case 32:
                  *((uint32_t*)(fb + line)) =
                    ((int)*glyph) & mask ? 0xFFFFFFFF : fb_bg;
                  line += 4;
                  break;
              }
            }
            *((uint32_t*)(fb + line)) = fb_bg;
          }
          break;
      }
#endif
#ifdef CONSOLE_VGA
    if (!fb && !ST)
      switch (c) {
        case '\r':
          vga_x = 0;
          break;
        case '\n':
          vga_x = 0;
          vga_y++;
          break;
        default:
          if (vga_y >= 25) {
            memcpy((void*)0xB8000, (void*)(0xB8000 + 160), 160 * 24);
            vga_x = 0;
            vga_y = 24;
            memset((void*)(0xB8000 + 24 * 160), 0, 160);
          }
          *((uint16_t*)((uintptr_t)0xB8000 + vga_y * 160 + vga_x++ * 2)) =
            0x0f00 | (c & 0xff);
          break;
      }
#endif
#ifdef CONSOLE_UEFI
    if (ST && ST->ConOut) {
      tmp[0] = c;
      tmp[1] = 0;
      ST->ConOut->OutputString(ST->ConOut, tmp);
    }
#endif
  }

  /**
   * Display (extremely minimal) formated message on console
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
    va_list args;
    uint8_t* ptr;
    int64_t arg;
    uint16_t* u;
    int len, sign, i, l;
    char *p, tmpstr[19]{}, n;

    va_start(args, fmt);
    arg = 0;
    while (*fmt) {
      if (*fmt == '%') {
        fmt++;
        if (*fmt == '%')
          goto put;
        len = l = 0;
        while (*fmt >= '0' && *fmt <= '9') {
          len *= 10;
          len += *fmt - '0';
          fmt++;
        }
        if (*fmt == 'l') {
          l++;
          fmt++;
        }
        if (*fmt == 'c') {
          arg = va_arg(args, int);
          console_putc((uint8_t)arg);
          fmt++;
          continue;
        } else if (*fmt == 'd') {
          if (!l)
            arg = (int32_t)va_arg(args, int32_t);
          else
            arg = va_arg(args, int64_t);
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
        } else if (*fmt == 'x' || *fmt == 'p') {
          if (*fmt == 'x' && !l)
            arg = (int32_t)va_arg(args, int32_t);
          else
            arg = va_arg(args, int64_t);
          i = 16;
          tmpstr[i] = 0;
          if (*fmt == 'p')
            len = 16;
          do {
            n = arg & 0xf;
            tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
            arg >>= 4;
          } while (arg != 0 && i > 0);
          if (len > 0 && len <= 16) {
            while (i > 16 - len)
              tmpstr[--i] = '0';
          }
          p = &tmpstr[i];
          goto putstring;
        } else if (*fmt == 's') {
          p = va_arg(args, char*);
        putstring:
          if (p == (void*)0)
            p = (char*)"(null)";
          while (*p)
            console_putc(*p++);
        }
        if (*fmt == 'S') {
          u = va_arg(args, uint16_t*);
          if (u == (void*)0)
            u = (uint16_t*)L"(null)";
          while (*u)
            console_putc(*u++);
        } else if (*fmt == 'D') {
          arg = va_arg(args, int64_t);
          if (len < 1)
            len = 1;
          do {
            for (i = 28; i >= 0; i -= 4) {
              n = (arg >> i) & 15;
              n += (n > 9 ? 0x37 : 0x30);
              console_putc(n);
            }
            console_putc(':');
            console_putc(' ');
            ptr = (uint8_t*)(uintptr_t)arg;
            for (i = 0; i < 16; i++) {
              n = (ptr[i] >> 4) & 15;
              n += (n > 9 ? 0x37 : 0x30);
              console_putc(n);
              n = ptr[i] & 15;
              n += (n > 9 ? 0x37 : 0x30);
              console_putc(n);
              console_putc(' ');
            }
            console_putc(' ');
            for (i = 0; i < 16; i++)
              console_putc(ptr[i] < 32 || ptr[i] >= 127 ? '.' : ptr[i]);
            console_putc('\r');
            console_putc('\n');
            arg += 16;
          } while (--len);
        }
      } else {
      put:
        console_putc(*fmt);
      }
      fmt++;
    }
    va_end(args);
  }

  void vgat_hello_world()
  {
    volatile unsigned short* vgat = (unsigned short*)0xB8000;

    for (int i = 0; i < 80 * 25; ++i)
      vgat[i] = (0x0F << 8) | ' ';

    const char* msg = "Hello, world!";
    for (int i = 0; msg[i]; ++i)
      vgat[i] = (0x0F << 8) | msg[i];
  }

  void _start()
  {
    printf_serial("Hello, world! (printf_serial)\n");

    console_init();
    printf("Hello, world! (printf)\n");

    vgat_hello_world();

    for (;;)
      ;
  }
}