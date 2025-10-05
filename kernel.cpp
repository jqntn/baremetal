asm(".section .multiboot;"
    ".align 4;"
    "multiboot_header:;"
    ".long 0x1BADB002;"
    ".long 0;"
    ".long -(0x1BADB002+0);");

constexpr unsigned char seq[] = { 0x03, 0x01, 0x0F, 0x00, 0x0E };
constexpr unsigned char crtc[] = { 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,
                                   0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
                                   0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28,
                                   0x40, 0x96, 0xB9, 0xA3, 0xFF };
constexpr unsigned char gctl[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x40, 0x05, 0x0F, 0xFF };
constexpr unsigned char actrl[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
                                    0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00 };

static inline void
outb(unsigned short port, unsigned char val)
{
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char
inb(unsigned short port)
{
  unsigned char ret = 0;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static void
set_vga_mode_13h()
{
  // misc output
  outb(0x3C2, 0x63);

  // sequencer
  for (int i = 0; i < 5; ++i) {
    outb(0x3C4, i);
    outb(0x3C5, seq[i]);
  }

  // unlock crt controller
  outb(0x3D4, 0x11);
  outb(0x3D5, inb(0x3D5) & 0x7F);

  // crt controller
  for (int i = 0; i < 25; ++i) {
    outb(0x3D4, i);
    outb(0x3D5, crtc[i]);
  }

  // graphics controller
  for (int i = 0; i < 9; ++i) {
    outb(0x3CE, i);
    outb(0x3CF, gctl[i]);
  }

  // attribute controller
  for (int i = 0; i < 21; ++i) {
    inb(0x3DA);
    outb(0x3C0, i);
    outb(0x3C0, actrl[i]);
  }
}

extern "C" void
_start()
{
  volatile unsigned short* vgat = (unsigned short*)0xB8000;

  for (int i = 0; i < 80 * 25; ++i) {
    vgat[i] = (0x0F << 8) | ' ';
  }

  const char* msg = "Hello, world!";
  for (int i = 0; msg[i]; ++i) {
    vgat[i] = (0x0F << 8) | msg[i];
  }

  for (;;)
    ;

  set_vga_mode_13h();

  volatile unsigned char* vgag = (unsigned char*)0xA0000;

  for (int i = 0; i < 320 * 200; ++i) {
    vgag[i] = 1;
  }

  for (;;)
    ;
}