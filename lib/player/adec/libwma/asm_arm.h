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

 function: arm7 and later wide math functions

 ********************************************************************/
#ifndef _ASM_ARM_H
#define _ASM_ARM_H

#include "nds32_intrinsic.h"
#include "coder.h"

__inline static int32_t MULT32(int32_t x, int32_t y) {
  return (int32_t) (((__int64 ) x * y) >> 32);
}

__inline static int32_t MULT31(int32_t x, int32_t y) {
  return MULT32(x, y) << 1;
}

__inline static int32_t MULT31_SHIFT15(int32_t x, int32_t y) {
  __int64 tmp = (__int64 ) x * y;
  return ((uint32_t) tmp >> 15) | ((tmp >> 32) << 17);
}

__inline static void XPROD32(int32_t a, int32_t b, int32_t t, int32_t v, int32_t *x,
    int32_t *y) {

  *x = MULT32(a, t) + MULT32(b, v);
  *y = MULT32(b, t) - MULT32(a, v);
}

__inline static void XPROD31(int32_t a, int32_t b, int32_t t, int32_t v, int32_t *x,
    int32_t *y) {

  *x = MULT31(a, t) - MULT31(b, v);
  *y = MULT31(b, t) + MULT31(a, v);
}

__inline static void XNPROD31(int32_t a, int32_t b, int32_t t, int32_t v, int32_t *x,
    int32_t *y) {

  *x = MULT31(a, t) - MULT31(b, v);
  *y = MULT31(b, t) + MULT31(a, v);
}

__inline static int32_t CLIP_TO_15(int32_t x) {
  int ret = x;
  ret -= ((x <= 32767) - 1) & (x - 32767);
  ret -= ((x >= -32768) - 1) & (x + 32768);
  return (ret);
}
#endif
