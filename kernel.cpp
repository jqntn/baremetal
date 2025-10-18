extern "C"
{
  __attribute__((section(".multiboot"), aligned(4), used))
  const int multiboot_header[] = { 0x1BADB002, 0, -(0x1BADB002 + 0) };

#include <stdarg.h>
#include <stdint.h>

#include "thirdparty/simpleboot/simpleboot.h"

  void memcpy(void* dst, const void* src, unsigned long n)
  {
    for (unsigned char *d = (unsigned char*)dst, *s = (unsigned char*)src; n--;)
      *d++ = *s++;
  }

  void memset(void* dst, unsigned char c, unsigned long n)
  {
    for (unsigned char* d = (unsigned char*)dst; n--;)
      *d++ = c;
  }

  long long __divdi3(long long a, long long b)
  {
    if (!b)
      return 0;
    long long q = 0;
    int s = (a < 0) != (b < 0) ? -1 : 1;
    a = a < 0 ? -a : a;
    b = b < 0 ? -b : b;
    for (; a >= b; a -= b)
      q++;
    return s * q;
  }

  long long __moddi3(long long a, long long b)
  {
    if (!b)
      return 0;
    int s = a < 0 ? -1 : 1;
    a = a < 0 ? -a : a;
    b = b < 0 ? -b : b;
    for (; a >= b; a -= b)
      ;
    return s * a;
  }

#define CONSOLE_SERIAL 0x3F8
#define CONSOLE_FB
#define CONSOLE_VGA

#define FB_COLOR(r, g, b)                                                      \
  (((vidmode.framebuffer_red_mask_size > 8                                     \
       ? (r) << (vidmode.framebuffer_red_mask_size - 8)                        \
       : (r) >> (8 - vidmode.framebuffer_red_mask_size))                       \
    << vidmode.framebuffer_red_field_position) |                               \
   ((vidmode.framebuffer_green_mask_size > 8                                   \
       ? (g) << (vidmode.framebuffer_green_mask_size - 8)                      \
       : (g) >> (8 - vidmode.framebuffer_green_mask_size))                     \
    << vidmode.framebuffer_green_field_position) |                             \
   ((vidmode.framebuffer_blue_mask_size > 8                                    \
       ? (b) << (vidmode.framebuffer_blue_mask_size - 8)                       \
       : (b) >> (8 - vidmode.framebuffer_blue_mask_size))                      \
    << vidmode.framebuffer_blue_field_position))

  multiboot_tag_framebuffer_t vidmode;
#ifdef CONSOLE_FB
#include "thirdparty/simpleboot/psf.h"
  uint32_t fb_x, fb_y;
  uint32_t fb_bg;
#endif
#ifdef CONSOLE_VGA
  uint16_t vga_x, vga_y;
  uint16_t vga_bg;
  uint16_t vga_fg;
#endif

  void console_init()
  {
#ifdef CONSOLE_FB
    fb_x = fb_y = 4;
    fb_bg = FB_COLOR(0, 0, 255);
#endif
#ifdef CONSOLE_VGA
    vga_x = vga_y = 0;
    vga_bg = 0x1;
    vga_fg = 0xF;
    if (!vidmode.framebuffer_addr)
      memset((void*)0xB8000, 0, 160 * 25);
#endif
  }

  void console_putc(const uint8_t c)
  {
#ifdef CONSOLE_SERIAL
    asm volatile("movb    %0, %%bl;"
                 "movl    $10000, %%ecx;"
                 "1:"
                 "inb     %%dx, %%al;"
                 "cmpb    $0xff, %%al;"
                 "je      2f;"
                 "dec     %%ecx;"
                 "jz      2f;"
                 "testb   $0x20, %%al;"
                 "jz      1b;"
                 "subb    $5, %%dl;"
                 "movb    %%bl, %%al;"
                 "outb    %%al, %%dx;"
                 "2:"
                 :
                 : "r"(c), "d"(CONSOLE_SERIAL + 5)
                 : "al", "bl", "ecx");
#endif
#ifdef CONSOLE_FB
    psf2_t* font = (psf2_t*)font_psf;
    uint32_t x, y, line, mask, offs, bpl = (font->width + 7) >> 3;
    uint8_t *glyph, *fb = (uint8_t*)vidmode.framebuffer_addr;
    if (fb) {
      switch (c) {
        case '\r': {
          fb_x = 4;
        } break;
        case '\n': {
          fb_x = 4;
          fb_y += font->height;
        } break;
        default: {
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
                line += vidmode.framebuffer_pitch) {
              memcpy(fb + offs, fb + line, vidmode.framebuffer_pitch);
            }
            for (y = fb_y, offs = fb_y * vidmode.framebuffer_pitch;
                 y < vidmode.framebuffer_height;
                 y++, offs += vidmode.framebuffer_pitch) {
              memset(fb + offs, 0, vidmode.framebuffer_pitch);
            }
          }
          glyph = (uint8_t*)font_psf + font->headersize +
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
                case 16: {
                  *((uint16_t*)(fb + line)) =
                    ((uint32_t)*glyph) & mask ? 0xFFFF : fb_bg;
                  line += 2;
                } break;
                case 24: {
                  *((uint32_t*)(fb + line)) =
                    ((uint32_t)*glyph) & mask ? 0xFFFFFF : fb_bg;
                  line += 3;
                } break;
                case 32: {
                  *((uint32_t*)(fb + line)) =
                    ((uint32_t)*glyph) & mask ? 0xFFFFFFFF : fb_bg;
                  line += 4;
                } break;
              }
            }
            *((uint32_t*)(fb + line)) = fb_bg;
          }
        } break;
      }
    }
#endif
#ifdef CONSOLE_VGA
    if (!vidmode.framebuffer_addr) {
      switch (c) {
        case '\r': {
          vga_x = 0;
        } break;
        case '\n': {
          vga_x = 0;
          vga_y++;
        } break;
        default: {
          if (vga_y >= 25) {
            memcpy((void*)0xB8000, (void*)(0xB8000 + 160), 160 * 24);
            vga_x = 0;
            vga_y = 24;
            memset((void*)(0xB8000 + 24 * 160), 0, 160);
          }
          *((uint16_t*)((uintptr_t)0xB8000 + vga_y * 160 + vga_x++ * 2)) =
            (((vga_bg << 4) | vga_fg) << 8) | (uint8_t)c;
        } break;
      }
    }
#endif
  }

  /*
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
    int64_t arg;
    uint8_t* byte_ptr;
    uint16_t* wchar_ptr;
    char *char_ptr, tmp_str[19], hex_digit;
    int len, sign, idx, long_flag;

    va_start(args, fmt);
    arg = 0;
    while (*fmt) {
      if (*fmt == '%') {
        fmt++;
        if (*fmt == '%') {
          goto put;
        }
        len = long_flag = 0;
        while (*fmt >= '0' && *fmt <= '9') {
          len *= 10;
          len += *fmt - '0';
          fmt++;
        }
        if (*fmt == 'l') {
          long_flag++;
          fmt++;
        }
        if (*fmt == 'c') {
          arg = va_arg(args, int);
          console_putc(arg);
          fmt++;
          continue;
        } else if (*fmt == 'd') {
          if (!long_flag) {
            arg = va_arg(args, int32_t);
          } else {
            arg = va_arg(args, int64_t);
          }
          sign = 0;
          if ((int)arg < 0) {
            arg = -arg;
            sign++;
          }
          idx = 18;
          tmp_str[idx] = 0;
          do {
            tmp_str[--idx] = '0' + (arg % 10);
            arg /= 10;
          } while (arg != 0 && idx > 0);
          if (sign) {
            tmp_str[--idx] = '-';
          }
          if (len > 0 && len < 18) {
            while (idx > 18 - len) {
              tmp_str[--idx] = ' ';
            }
          }
          char_ptr = &tmp_str[idx];
          goto putstring;
        } else if (*fmt == 'x' || *fmt == 'p') {
          if (*fmt == 'x' && !long_flag) {
            arg = va_arg(args, int32_t);
          } else {
            arg = va_arg(args, int64_t);
          }
          idx = 16;
          tmp_str[idx] = 0;
          if (*fmt == 'p') {
            len = 16;
          }
          do {
            hex_digit = arg & 0xF;
            tmp_str[--idx] = hex_digit + (hex_digit > 9 ? 0x37 : 0x30);
            arg >>= 4;
          } while (arg != 0 && idx > 0);
          if (len > 0 && len <= 16) {
            while (idx > 16 - len) {
              tmp_str[--idx] = '0';
            }
          }
          char_ptr = &tmp_str[idx];
          goto putstring;
        } else if (*fmt == 's') {
          char_ptr = va_arg(args, char*);
        putstring:
          if (char_ptr == (void*)0) {
            char_ptr = (char*)"(null)";
          }
          while (*char_ptr) {
            console_putc(*char_ptr++);
          }
        }
        if (*fmt == 'S') {
          wchar_ptr = va_arg(args, uint16_t*);
          if (wchar_ptr == (void*)0) {
            wchar_ptr = (uint16_t*)L"(null)";
          }
          while (*wchar_ptr) {
            console_putc(*wchar_ptr++);
          }
        } else if (*fmt == 'D') {
          arg = va_arg(args, int64_t);
          if (len < 1) {
            len = 1;
          }
          do {
            for (idx = 28; idx >= 0; idx -= 4) {
              hex_digit = (arg >> idx) & 15;
              hex_digit += (hex_digit > 9 ? 0x37 : 0x30);
              console_putc(hex_digit);
            }
            console_putc(':');
            console_putc(' ');
            byte_ptr = (uint8_t*)arg;
            for (idx = 0; idx < 16; idx++) {
              hex_digit = (byte_ptr[idx] >> 4) & 15;
              hex_digit += (hex_digit > 9 ? 0x37 : 0x30);
              console_putc(hex_digit);
              hex_digit = byte_ptr[idx] & 15;
              hex_digit += (hex_digit > 9 ? 0x37 : 0x30);
              console_putc(hex_digit);
              console_putc(' ');
            }
            console_putc(' ');
            for (idx = 0; idx < 16; idx++) {
              console_putc(byte_ptr[idx] < 32 || byte_ptr[idx] >= 127
                             ? '.'
                             : byte_ptr[idx]);
            }
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

  /*
   * Print a binary UUID in human readable form
   */
  void dumpuuid(const uint8_t* uuid)
  {
    printf(
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x%02x%02x\n",
      uuid[3],
      uuid[2],
      uuid[1],
      uuid[0],
      uuid[5],
      uuid[4],
      uuid[7],
      uuid[6],
      uuid[8],
      uuid[9],
      uuid[10],
      uuid[11],
      uuid[12],
      uuid[13],
      uuid[14],
      uuid[15]);
  }

  void test_fb()
  {
    uint32_t col = FB_COLOR(255, 0, 255);
    uint32_t len = vidmode.framebuffer_width * vidmode.framebuffer_height;
    uint32_t* fb = (uint32_t*)vidmode.framebuffer_addr;

    for (; len--;)
      *fb++ = col;
  }

  void _start(uint32_t magic, uintptr_t addr)
  {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
      console_init();
      printf("INVALID MB2 MAGIC NUMBER: 0x%x\n", magic);
      goto halt;
    }

    printf("\n\n");

    if (addr & 7) {
      printf("unaligned MBI: 0x%x\n", addr);
      goto halt;
    }

    uint32_t size;
    size = ((multiboot_info_t*)addr)->total_size;
    printf("announced MBI size 0x%x\n", size);

    multiboot_mmap_entry_t* mmap;
    multiboot_tag_framebuffer_t* fb;
    multiboot_tag_t *tag, *tag_last;
    for (tag = (multiboot_tag_t*)(addr + 8),
        tag_last = (multiboot_tag_t*)(addr + size);
         tag < tag_last && tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {
      switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_CMDLINE: {
          printf("  command line = %s\n",
                 ((multiboot_tag_cmdline_t*)tag)->string);
        } break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
          printf("  boot loader name = %s\n",
                 ((multiboot_tag_loader_t*)tag)->string);
        } break;
        case MULTIBOOT_TAG_TYPE_MODULE: {
          printf("  module at 0x%x-0x%x. command line %s\n",
                 ((multiboot_tag_module_t*)tag)->mod_start,
                 ((multiboot_tag_module_t*)tag)->mod_end,
                 ((multiboot_tag_module_t*)tag)->string);
        } break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
          printf("  mmap\n");
          for (mmap = ((multiboot_tag_mmap_t*)tag)->entries;
               (uint8_t*)mmap < (uint8_t*)tag + tag->size;
               mmap = (multiboot_mmap_entry_t*)((uintptr_t)mmap +
                                                ((multiboot_tag_mmap_t*)tag)
                                                  ->entry_size)) {
            printf("    base_addr = 0x%8x%8x, "
                   "length = 0x%8x%8x, type = 0x%x %s, res = 0x%x\n",
                   (uint32_t)(mmap->base_addr >> 32),
                   (uint32_t)(mmap->base_addr & 0xFFFFFFFF),
                   (uint32_t)(mmap->length >> 32),
                   (uint32_t)(mmap->length & 0xFFFFFFFF),
                   mmap->type,
                   mmap->type == MULTIBOOT_MEMORY_AVAILABLE
                     ? "free"
                     : (mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE
                          ? "ACPI"
                          : (mmap->type == MULTIBOOT_MEMORY_NVS ? "ACPI NVS"
                                                                : "used")),
                   mmap->reserved);
          }
        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
          fb = (multiboot_tag_framebuffer_t*)tag;
          printf("  framebuffer\n");
          printf("    address 0x%8x%8x pitch %d\n",
                 (uint32_t)(fb->framebuffer_addr >> 32),
                 (uint32_t)(fb->framebuffer_addr & 0xFFFFFFFF),
                 fb->framebuffer_pitch);
          printf("    width %d height %d depth %d bpp\n",
                 fb->framebuffer_width,
                 fb->framebuffer_height,
                 fb->framebuffer_bpp);
          printf("    red channel:   at %d, %d bits\n",
                 fb->framebuffer_red_field_position,
                 fb->framebuffer_red_mask_size);
          printf("    green channel: at %d, %d bits\n",
                 fb->framebuffer_green_field_position,
                 fb->framebuffer_green_mask_size);
          printf("    blue channel:  at %d, %d bits\n",
                 fb->framebuffer_blue_field_position,
                 fb->framebuffer_blue_mask_size);
        } break;
        case MULTIBOOT_TAG_TYPE_EFI64: {
          printf("  EFI system table 0x%x\n",
                 ((multiboot_tag_efi64_t*)tag)->pointer);
        } break;
        case MULTIBOOT_TAG_TYPE_EFI64_IH: {
          printf("  EFI image handle 0x%x\n",
                 ((multiboot_tag_efi64_t*)tag)->pointer);
        } break;
        case MULTIBOOT_TAG_TYPE_SMBIOS: {
          printf("  SMBIOS table major %d minor %d\n",
                 ((multiboot_tag_smbios_t*)tag)->major,
                 ((multiboot_tag_smbios_t*)tag)->minor);
        } break;
        case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
          printf("  ACPI table (1.0, old RSDP)\n");
        } break;
        case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
          printf("  ACPI table (2.0, new RSDP)\n");
        } break;
        case MULTIBOOT_TAG_TYPE_EDID: {
          printf("  EDID info\n");
          printf("    manufacturer ID %02x%02x\n",
                 ((multiboot_tag_edid_t*)tag)->edid[8],
                 ((multiboot_tag_edid_t*)tag)->edid[9]);
          printf("    EDID ID %02x%02x version %d rev %d\n",
                 ((multiboot_tag_edid_t*)tag)->edid[10],
                 ((multiboot_tag_edid_t*)tag)->edid[11],
                 ((multiboot_tag_edid_t*)tag)->edid[18],
                 ((multiboot_tag_edid_t*)tag)->edid[19]);
          printf("    monitor type %02x size %d cm x %d cm\n",
                 ((multiboot_tag_edid_t*)tag)->edid[20],
                 ((multiboot_tag_edid_t*)tag)->edid[21],
                 ((multiboot_tag_edid_t*)tag)->edid[22]);
        } break;
        case MULTIBOOT_TAG_TYPE_SMP: {
          printf("  SMP supported\n");
          printf("    %d core(s)\n", ((multiboot_tag_smp_t*)tag)->numcores);
          printf("    %d running\n", ((multiboot_tag_smp_t*)tag)->running);
          printf("    %02x bsp id\n", ((multiboot_tag_smp_t*)tag)->bspid);
        } break;
        case MULTIBOOT_TAG_TYPE_PARTUUID: {
          printf("  partition UUIDs\n");
          printf("    boot ");
          dumpuuid(((multiboot_tag_partuuid_t*)tag)->bootuuid);
          if (tag->size >= 40) {
            printf("    root ");
            dumpuuid(((multiboot_tag_partuuid_t*)tag)->rootuuid);
          }
        } break;
        default: {
          printf("  /!\\ UNKNOWN MBI TAG /!\\\n");
          goto halt;
        } break;
      }
    }

    tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    printf("total MBI size 0x%x %s\n",
           (uintptr_t)tag - addr,
           ((uintptr_t)tag - addr) == size ? "OK" : "ERR");

    printf("\n\n");

    vidmode = *fb;

    console_init();
    printf("Hello, world!\n");

    test_fb();

  halt:
    for (;;)
      ;
  }
}