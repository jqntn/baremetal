#include <stdarg.h>
#include <stdint.h>

void
_write(const char c);

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
void
printf(const char* fmt, ...)
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
        _write(arg);
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
        if (!char_ptr) {
          char_ptr = (char*)"(null)";
        }
        while (*char_ptr) {
          _write(*char_ptr++);
        }
      }
      if (*fmt == 'S') {
        wchar_ptr = va_arg(args, uint16_t*);
        if (!wchar_ptr) {
          wchar_ptr = (uint16_t*)L"(null)";
        }
        while (*wchar_ptr) {
          _write(*wchar_ptr++);
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
            _write(hex_digit);
          }
          _write(':');
          _write(' ');
          byte_ptr = (uint8_t*)arg;
          for (idx = 0; idx < 16; idx++) {
            hex_digit = (byte_ptr[idx] >> 4) & 15;
            hex_digit += (hex_digit > 9 ? 0x37 : 0x30);
            _write(hex_digit);
            hex_digit = byte_ptr[idx] & 15;
            hex_digit += (hex_digit > 9 ? 0x37 : 0x30);
            _write(hex_digit);
            _write(' ');
          }
          _write(' ');
          for (idx = 0; idx < 16; idx++) {
            _write(byte_ptr[idx] < 32 || byte_ptr[idx] >= 127 ? '.'
                                                              : byte_ptr[idx]);
          }
          _write('\r');
          _write('\n');
          arg += 16;
        } while (--len);
      }
    } else {
    put:
      _write(*fmt);
    }
    fmt++;
  }
  va_end(args);
}