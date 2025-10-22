void
memcpy(void* dst, const void* src, unsigned long n)
{
  for (unsigned char *d = (unsigned char*)dst, *s = (unsigned char*)src; n--;)
    *d++ = *s++;
}

void
memset(void* dst, unsigned char c, unsigned long n)
{
  for (unsigned char* d = (unsigned char*)dst; n--;)
    *d++ = c;
}