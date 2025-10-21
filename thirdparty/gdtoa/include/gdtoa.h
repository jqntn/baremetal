/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#ifndef GDTOA_H_INCLUDED
#define GDTOA_H_INCLUDED

#include "arith.h"
#include <stdint.h>

enum
{ /* return values from strtodg */
  STRTOG_Zero = 0,
  STRTOG_Normal = 1,
  STRTOG_Denormal = 2,
  STRTOG_Infinite = 3,
  STRTOG_NaN = 4,
  STRTOG_NaNbits = 5,
  STRTOG_NoNumber = 6,
  STRTOG_Retmask = 7,

  /* The following may be or-ed into one of the above values. */

  STRTOG_Neg = 0x08,
  STRTOG_Inexlo = 0x10,
  STRTOG_Inexhi = 0x20,
  STRTOG_Inexact = 0x30,
  STRTOG_Underflow = 0x40,
  STRTOG_Overflow = 0x80
};

typedef struct FPI
{
	int nbits;
	int emin;
	int emax;
	int rounding;
	int sudden_underflow;
} FPI;

enum
{ /* FPI.rounding values: same as FLT_ROUNDS */
  FPI_Round_zero = 0,
  FPI_Round_near = 1,
  FPI_Round_up = 2,
  FPI_Round_down = 3
};

#ifdef __cplusplus
extern "C" {
#endif

extern char* dtoa(double d, int mode, int ndigits, int* decpt, int* sign, char** rve);
extern char* gdtoa(FPI * fpi, int be, uint32_t* bits, int* kindp, int mode, int ndigits,
						 int* decpt, char** rve);
extern void freedtoa(char*);
extern int strtodg(const char*, char**, FPI*, int32_t*, uint32_t*);

extern char* g_ddfmt(char*, double*, int, unsigned);
extern char* g_dfmt(char*, double*, int, unsigned);
extern char* g_ffmt(char*, float*, int, unsigned);
extern char* g_Qfmt(char*, void*, int, unsigned);
extern char* g_xfmt(char*, void*, int, unsigned);
extern char* g_xLfmt(char*, void*, int, unsigned);

extern int strtoId(const char*, char**, double*, double*);
extern int strtoIdd(const char*, char**, double*, double*);
extern int strtoIf(const char*, char**, float*, float*);
extern int strtoIQ(const char*, char**, void*, void*);
extern int strtoIx(const char*, char**, void*, void*);
extern int strtoIxL(const char*, char**, void*, void*);
extern int strtord(const char*, char**, int, double*);
extern int strtordd(const char*, char**, int, double*);
extern int strtorf(const char*, char**, int, float*);
extern int strtorQ(const char*, char**, int, void*);
extern int strtorx(const char*, char**, int, void*);
extern int strtorxL(const char*, char**, int, void*);
#if 1
extern int strtodI(const char*, char**, double*);
extern int strtopd(const char*, char**, double*);
extern int strtopdd(const char*, char**, double*);
extern int strtopf(const char*, char**, float*);
extern int strtopQ(const char*, char**, void*);
extern int strtopx(const char*, char**, void*);
extern int strtopxL(const char*, char**, void*);
#else
#define strtopd(s, se, x) strtord(s, se, 1, x)
#define strtopdd(s, se, x) strtordd(s, se, 1, x)
#define strtopf(s, se, x) strtorf(s, se, 1, x)
#define strtopQ(s, se, x) strtorQ(s, se, 1, x)
#define strtopx(s, se, x) strtorx(s, se, 1, x)
#define strtopxL(s, se, x) strtorxL(s, se, 1, x)
#endif

#ifdef __cplusplus
}
#endif
#endif /* GDTOA_H_INCLUDED */
