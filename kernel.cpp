extern "C"
{
  asm(".section .multiboot;"
      ".align 4;"
      "multiboot_header:;"
      ".long 0x1BADB002;"
      ".long 0;"
      ".long -(0x1BADB002+0);");

  void _start()
  {
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    const char* msg = "Hello, world!";
    for (int i = 0; msg[i]; ++i)
      vga[i] = (0x0F << 8) | msg[i];
    for (;;)
      ;
  }
}