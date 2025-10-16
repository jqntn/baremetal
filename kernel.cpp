// TODO: 32bit compat

extern "C"
{
#ifndef _MSC_VER
  asm(".section .multiboot;"
      ".align 4;"
      "multiboot_header:;"
      ".long 0x1BADB002;"
      ".long 0;"
      ".long -(0x1BADB002+0);");
#endif

#include <stdarg.h>
#include <stdint.h>

#include "thirdparty/simpleboot/loader.h"
#include "thirdparty/simpleboot/simpleboot.h"

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
    asm volatile("cld; repe cmpsb; xorl %%eax, %%eax; movb -1(%%edi), "
                 "%%al; subb -1(%%esi), %%al;"
                 : "=a"(ret)
                 : "D"(s1), "S"(s2), "c"(n)
                 :);
    return ret;
  }

  long long __divdi3(long long a, long long b)
  {
    return a / b;
  }

  long long __moddi3(long long a, long long b)
  {
    return a % b;
  }

  /**
   * Display (extremely minimal) formated message on serial
   */
  void printf_serial(const char* fmt, ...)
  {
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
  }

#define CONSOLE_SERIAL 0x3f8
#define CONSOLE_FB
#define CONSOLE_VGA
#define CONSOLE_UEFI
#define CONSOLE_BOCHS_E9

  fossbios_t* fossbios;
  efi_system_table_t* efi_system_table;
  multiboot_tag_framebuffer_t vidmode;

#ifdef CONSOLE_FB
#include "psf.h"
  uint32_t fb_x, fb_y;
  uint32_t fb_bg;
#endif
#ifdef CONSOLE_VGA
  uint16_t vga_x, vga_y;
#endif

  void console_init()
  {
#ifdef CONSOLE_SERIAL
    if (fossbios && fossbios->serial)
      fossbios->serial->setmode(0, 115200, 8, 0, 1);
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
    fb_bg = FB_COLOR(0, 0, 255);
#endif
#ifdef CONSOLE_VGA
    vga_x = vga_y = 0;
    if (!vidmode.framebuffer_addr && !efi_system_table)
      memset((void*)0xB8000, 0, 160 * 25);
#endif
#ifdef CONSOLE_UEFI
    if (efi_system_table && efi_system_table->ConOut)
      efi_system_table->ConOut->Reset(efi_system_table->ConOut, 0);
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
    if (fossbios && fossbios->serial)
      fossbios->serial->send(0, c);
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
    if (!fb && !efi_system_table)
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
    if (efi_system_table && efi_system_table->ConOut) {
      tmp[0] = c;
      tmp[1] = 0;
      efi_system_table->ConOut->OutputString(efi_system_table->ConOut, tmp);
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

  /**
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

  void _start(uint32_t magic, uintptr_t addr)
  {
    vgat_hello_world();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
      printf("Invalid magic number: 0x%x\n", (unsigned)magic);
      goto halt;
    }
    if (addr & 7) {
      printf("Unaligned MBI: 0x%x\n", addr);
      goto halt;
    }

    unsigned int mbi_size;
    mbi_size = ((multiboot_info_t*)addr)->total_size;
    printf("\nAnnounced MBI size 0x%x\n", mbi_size);

    multiboot_mmap_entry_t* p_mb_mmap;
    multiboot_tag_framebuffer_t* p_mb_tag_fb;
    multiboot_tag_t *mb_tag, *mb_tag_last;
    for (mb_tag = (multiboot_tag_t*)(addr + 8),
        mb_tag_last = (multiboot_tag_t*)(addr + mbi_size);
         mb_tag < mb_tag_last && mb_tag->type != MULTIBOOT_TAG_TYPE_END;
         mb_tag =
           (multiboot_tag_t*)((uint8_t*)mb_tag + ((mb_tag->size + 7) & ~7))) {
      printf("Tag 0x%x, Size 0x%x\n", mb_tag->type, mb_tag->size);
      switch (mb_tag->type) {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
          printf("Command line = %s\n",
                 ((multiboot_tag_cmdline_t*)mb_tag)->string);
          break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
          printf("Boot loader name = %s\n",
                 ((multiboot_tag_loader_t*)mb_tag)->string);
          break;
        case MULTIBOOT_TAG_TYPE_MODULE:
          printf("Module at 0x%x-0x%x. Command line %s\n",
                 ((multiboot_tag_module_t*)mb_tag)->mod_start,
                 ((multiboot_tag_module_t*)mb_tag)->mod_end,
                 ((multiboot_tag_module_t*)mb_tag)->string);
          break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
          printf("mmap\n");
          for (p_mb_mmap = ((multiboot_tag_mmap_t*)mb_tag)->entries;
               (uint8_t*)p_mb_mmap < (uint8_t*)mb_tag + mb_tag->size;
               p_mb_mmap =
                 (multiboot_mmap_entry_t*)((uintptr_t)p_mb_mmap +
                                           ((multiboot_tag_mmap_t*)mb_tag)
                                             ->entry_size))
            printf(
              " base_addr = 0x%8x%8x,"
              " length = 0x%8x%8x, type = 0x%x %s, res = 0x%x\n",
              (unsigned)(p_mb_mmap->base_addr >> 32),
              (unsigned)(p_mb_mmap->base_addr & 0xffffffff),
              (unsigned)(p_mb_mmap->length >> 32),
              (unsigned)(p_mb_mmap->length & 0xffffffff),
              (unsigned)p_mb_mmap->type,
              p_mb_mmap->type == MULTIBOOT_MEMORY_AVAILABLE
                ? "free"
                : (p_mb_mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE
                     ? "ACPI"
                     : (p_mb_mmap->type == MULTIBOOT_MEMORY_NVS ? "ACPI NVS"
                                                                : "used")),
              (unsigned)p_mb_mmap->reserved);
        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
          p_mb_tag_fb = (multiboot_tag_framebuffer_t*)mb_tag;
          printf("framebuffer\n");
          printf(" address 0x%8x%8x pitch %d\n",
                 (unsigned)(p_mb_tag_fb->framebuffer_addr >> 32),
                 (unsigned)(p_mb_tag_fb->framebuffer_addr & 0xffffffff),
                 p_mb_tag_fb->framebuffer_pitch);
          printf(" width %d height %d depth %d bpp\n",
                 p_mb_tag_fb->framebuffer_width,
                 p_mb_tag_fb->framebuffer_height,
                 p_mb_tag_fb->framebuffer_bpp);
          printf(" red channel:   at %d, %d bits\n",
                 p_mb_tag_fb->framebuffer_red_field_position,
                 p_mb_tag_fb->framebuffer_red_mask_size);
          printf(" green channel: at %d, %d bits\n",
                 p_mb_tag_fb->framebuffer_green_field_position,
                 p_mb_tag_fb->framebuffer_green_mask_size);
          printf(" blue channel:  at %d, %d bits\n",
                 p_mb_tag_fb->framebuffer_blue_field_position,
                 p_mb_tag_fb->framebuffer_blue_mask_size);
          break;
        }
        case MULTIBOOT_TAG_TYPE_EFI64:
          printf("EFI system table 0x%x\n",
                 ((multiboot_tag_efi64_t*)mb_tag)->pointer);
          break;
        case MULTIBOOT_TAG_TYPE_EFI64_IH:
          printf("EFI image handle 0x%x\n",
                 ((multiboot_tag_efi64_t*)mb_tag)->pointer);
          break;
        case MULTIBOOT_TAG_TYPE_SMBIOS:
          printf("SMBIOS table major %d minor %d\n",
                 ((multiboot_tag_smbios_t*)mb_tag)->major,
                 ((multiboot_tag_smbios_t*)mb_tag)->minor);
          break;
        case MULTIBOOT_TAG_TYPE_ACPI_OLD:
          printf("ACPI table (1.0, old RSDP)\n");
          break;
        case MULTIBOOT_TAG_TYPE_ACPI_NEW:
          printf("ACPI table (2.0, new RSDP)\n");
          break;
        case MULTIBOOT_TAG_TYPE_EDID:
          printf("EDID info\n");
          printf(" manufacturer ID %02x%02x\n",
                 ((multiboot_tag_edid_t*)mb_tag)->edid[8],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[9]);
          printf(" EDID ID %02x%02x Version %d Rev %d\n",
                 ((multiboot_tag_edid_t*)mb_tag)->edid[10],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[11],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[18],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[19]);
          printf(" monitor type %02x size %d cm x %d cm\n",
                 ((multiboot_tag_edid_t*)mb_tag)->edid[20],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[21],
                 ((multiboot_tag_edid_t*)mb_tag)->edid[22]);
          break;
        case MULTIBOOT_TAG_TYPE_SMP:
          printf("SMP supported\n");
          printf(" %d core(s)\n", ((multiboot_tag_smp_t*)mb_tag)->numcores);
          printf(" %d running\n", ((multiboot_tag_smp_t*)mb_tag)->running);
          printf(" %02x bsp id\n", ((multiboot_tag_smp_t*)mb_tag)->bspid);
          break;
        case MULTIBOOT_TAG_TYPE_PARTUUID:
          printf("Partition UUIDs\n");
          printf(" boot ");
          dumpuuid(((multiboot_tag_partuuid_t*)mb_tag)->bootuuid);
          if (mb_tag->size >= 40) {
            printf(" root ");
            dumpuuid(((multiboot_tag_partuuid_t*)mb_tag)->rootuuid);
          }
          break;
        default:
          printf("---unknown MBI tag, this shouldn't happen with "
                 "Simpleboot/Easyboot!---\n");
          goto halt;
      }
    }
    mb_tag = (multiboot_tag_t*)((uint8_t*)mb_tag + ((mb_tag->size + 7) & ~7));
    printf("Total MBI size 0x%x %s\n",
           (uintptr_t)mb_tag - addr,
           ((uintptr_t)mb_tag - addr) == mbi_size ? "OK" : "ERR");

    vidmode = *p_mb_tag_fb;
    console_init();
    printf("Hello, world!\n");

  halt:
    for (;;)
      ;
  }
}