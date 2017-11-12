/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2002    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: normalized modified discrete cosine transform
 power of two length transform only [64 <= n ]


 Original algorithm adapted long ago from _The use of multirate filter
 banks for coding of high quality digital audio_, by T. Sporer,
 K. Brandenburg and B. Edler, collection of the European Signal
 Processing Conference (EUSIPCO), Amsterdam, June 1992, Vol.1, pp
 211-214

 The below code implements an algorithm that no longer looks much like
 that presented in the paper, but the basic structure remains if you
 dig deep enough to see it.

 This module DOES NOT INCLUDE code to generate/apply the window
 function.  Everybody has their own weird favorite including me... I
 happen to like the properties of y=sin(.5PI*sin^2(x)), but others may
 vehemently disagree.

 ********************************************************************/

/*Tremor IMDCT adapted for use with libwmai*/

#include "mdct2.h"
#include "mdct_lookup.h"
#include "asm_arm.h"

/*
* This should be used as a memory barrier, forcing all cached values in
* registers to wr writen back to memory.  Might or might not be beneficial
* depending on the architecture and compiler.
*/
#define MB()

/* 8 point butterfly (in place) */
inline void mdct_butterfly_8(int32_t *x) {

  register int32_t r0 = x[4] + x[0];
  register int32_t r1 = x[4] - x[0];
  register int32_t r2 = x[5] + x[1];
  register int32_t r3 = x[5] - x[1];
  register int32_t r4 = x[6] + x[2];
  register int32_t r5 = x[6] - x[2];
  register int32_t r6 = x[7] + x[3];
  register int32_t r7 = x[7] - x[3];

  x[0] = r5 + r3;
  x[1] = r7 - r1;
  x[2] = r5 - r3;
  x[3] = r7 + r1;
  x[4] = r4 - r0;
  x[5] = r6 - r2;
  x[6] = r4 + r0;
  x[7] = r6 + r2;
  MB();
}

/* 16 point butterfly (in place, 4 register) */
inline void mdct_butterfly_16(int32_t *x) {

  register int32_t r0, r1;

  r0 = x[0] - x[8];
  x[8] += x[0];
  r1 = x[1] - x[9];
  x[9] += x[1];
  x[0] = MULT31((r0 + r1), cPI2_8);
  x[1] = MULT31((r1 - r0), cPI2_8);
  MB();

  r0 = x[10] - x[2];
  x[10] += x[2];
  r1 = x[3] - x[11];
  x[11] += x[3];
  x[2] = r1;
  x[3] = r0;
  MB();

  r0 = x[12] - x[4];
  x[12] += x[4];
  r1 = x[13] - x[5];
  x[13] += x[5];
  x[4] = MULT31((r0 - r1), cPI2_8);
  x[5] = MULT31((r0 + r1), cPI2_8);
  MB();

  r0 = x[14] - x[6];
  x[14] += x[6];
  r1 = x[15] - x[7];
  x[15] += x[7];
  x[6] = r0;
  x[7] = r1;
  MB();

  mdct_butterfly_8(x);
  mdct_butterfly_8(x + 8);
}

/* 32 point butterfly (in place, 4 register) */
inline void mdct_butterfly_32(int32_t *x) {

  register int32_t r0, r1;

  r0 = x[30] - x[14];
  x[30] += x[14];
  r1 = x[31] - x[15];
  x[31] += x[15];
  x[14] = r0;
  x[15] = r1;
  MB();

  r0 = x[28] - x[12];
  x[28] += x[12];
  r1 = x[29] - x[13];
  x[29] += x[13];
  XNPROD31(r0, r1, cPI1_8, cPI3_8, &x[12], &x[13]);
  MB();

  r0 = x[26] - x[10];
  x[26] += x[10];
  r1 = x[27] - x[11];
  x[27] += x[11];
  x[10] = MULT31((r0 - r1), cPI2_8);
  x[11] = MULT31((r0 + r1), cPI2_8);
  MB();

  r0 = x[24] - x[8];
  x[24] += x[8];
  r1 = x[25] - x[9];
  x[25] += x[9];
  XNPROD31(r0, r1, cPI3_8, cPI1_8, &x[8], &x[9]);
  MB();

  r0 = x[22] - x[6];
  x[22] += x[6];
  r1 = x[7] - x[23];
  x[23] += x[7];
  x[6] = r1;
  x[7] = r0;
  MB();

  r0 = x[4] - x[20];
  x[20] += x[4];
  r1 = x[5] - x[21];
  x[21] += x[5];
  XPROD31(r0, r1, cPI3_8, cPI1_8, &x[4], &x[5]);
  MB();

  r0 = x[2] - x[18];
  x[18] += x[2];
  r1 = x[3] - x[19];
  x[19] += x[3];
  x[2] = MULT31((r1 + r0), cPI2_8);
  x[3] = MULT31((r1 - r0), cPI2_8);
  MB();

  r0 = x[0] - x[16];
  x[16] += x[0];
  r1 = x[1] - x[17];
  x[17] += x[1];
  XPROD31(r0, r1, cPI1_8, cPI3_8, &x[0], &x[1]);
  MB();

  mdct_butterfly_16(x);
  mdct_butterfly_16(x + 16);
}

/* N/stage point generic N stage butterfly (in place, 2 register) */
inline void mdct_butterfly_generic(int32_t *x, int points, int step) {

  const int32_t *T = sincos_lookup0;
  int32_t *x1 = x + points - 8;
  int32_t *x2 = x + (points >> 1) - 8;
  register int32_t r0;
  register int32_t r1;

  do {
    r0 = x1[6] - x2[6];
    x1[6] += x2[6];
    r1 = x2[7] - x1[7];
    x1[7] += x2[7];
    XPROD31(r1, r0, T[0], T[1], &x2[6], &x2[7]);
    T += step;

    r0 = x1[4] - x2[4];
    x1[4] += x2[4];
    r1 = x2[5] - x1[5];
    x1[5] += x2[5];
    XPROD31(r1, r0, T[0], T[1], &x2[4], &x2[5]);
    T += step;

    r0 = x1[2] - x2[2];
    x1[2] += x2[2];
    r1 = x2[3] - x1[3];
    x1[3] += x2[3];
    XPROD31(r1, r0, T[0], T[1], &x2[2], &x2[3]);
    T += step;

    r0 = x1[0] - x2[0];
    x1[0] += x2[0];
    r1 = x2[1] - x1[1];
    x1[1] += x2[1];
    XPROD31(r1, r0, T[0], T[1], &x2[0], &x2[1]);
    T += step;

    x1 -= 8;
    x2 -= 8;
  } while (T < sincos_lookup0 + 1024);
  do {
    r0 = x1[6] - x2[6];
    x1[6] += x2[6];
    r1 = x1[7] - x2[7];
    x1[7] += x2[7];
    XNPROD31(r0, r1, T[0], T[1], &x2[6], &x2[7]);
    T -= step;

    r0 = x1[4] - x2[4];
    x1[4] += x2[4];
    r1 = x1[5] - x2[5];
    x1[5] += x2[5];
    XNPROD31(r0, r1, T[0], T[1], &x2[4], &x2[5]);
    T -= step;

    r0 = x1[2] - x2[2];
    x1[2] += x2[2];
    r1 = x1[3] - x2[3];
    x1[3] += x2[3];
    XNPROD31(r0, r1, T[0], T[1], &x2[2], &x2[3]);
    T -= step;

    r0 = x1[0] - x2[0];
    x1[0] += x2[0];
    r1 = x1[1] - x2[1];
    x1[1] += x2[1];
    XNPROD31(r0, r1, T[0], T[1], &x2[0], &x2[1]);
    T -= step;

    x1 -= 8;
    x2 -= 8;
  } while (T > sincos_lookup0);
  do {
    r0 = x2[6] - x1[6];
    x1[6] += x2[6];
    r1 = x2[7] - x1[7];
    x1[7] += x2[7];
    XPROD31(r0, r1, T[0], T[1], &x2[6], &x2[7]);
    T += step;

    r0 = x2[4] - x1[4];
    x1[4] += x2[4];
    r1 = x2[5] - x1[5];
    x1[5] += x2[5];
    XPROD31(r0, r1, T[0], T[1], &x2[4], &x2[5]);
    T += step;

    r0 = x2[2] - x1[2];
    x1[2] += x2[2];
    r1 = x2[3] - x1[3];
    x1[3] += x2[3];
    XPROD31(r0, r1, T[0], T[1], &x2[2], &x2[3]);
    T += step;

    r0 = x2[0] - x1[0];
    x1[0] += x2[0];
    r1 = x2[1] - x1[1];
    x1[1] += x2[1];
    XPROD31(r0, r1, T[0], T[1], &x2[0], &x2[1]);
    T += step;

    x1 -= 8;
    x2 -= 8;
  } while (T < sincos_lookup0 + 1024);
  do {
    r0 = x1[6] - x2[6];
    x1[6] += x2[6];
    r1 = x2[7] - x1[7];
    x1[7] += x2[7];
    XNPROD31(r1, r0, T[0], T[1], &x2[6], &x2[7]);
    T -= step;

    r0 = x1[4] - x2[4];
    x1[4] += x2[4];
    r1 = x2[5] - x1[5];
    x1[5] += x2[5];
    XNPROD31(r1, r0, T[0], T[1], &x2[4], &x2[5]);
    T -= step;

    r0 = x1[2] - x2[2];
    x1[2] += x2[2];
    r1 = x2[3] - x1[3];
    x1[3] += x2[3];
    XNPROD31(r1, r0, T[0], T[1], &x2[2], &x2[3]);
    T -= step;

    r0 = x1[0] - x2[0];
    x1[0] += x2[0];
    r1 = x2[1] - x1[1];
    x1[1] += x2[1];
    XNPROD31(r1, r0, T[0], T[1], &x2[0], &x2[1]);
    T -= step;

    x1 -= 8;
    x2 -= 8;
  } while (T > sincos_lookup0);
}

void mdct_butterflies(int32_t *x, int points, int shift) {
  int stages = 8 - shift;
  int i, j;

  for (i = 0; --stages > 0; i++) {
    for (j = 0; j < (1 << i); j++)
      mdct_butterfly_generic(x + (points >> i) * j, points >> i,
          4 << (i + shift));
  }

  for (j = 0; j < points; j += 32) {
    mdct_butterfly_32(x + j);
  }
}
