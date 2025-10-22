long long
__divdi3(long long a, long long b)
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

long long
__moddi3(long long a, long long b)
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