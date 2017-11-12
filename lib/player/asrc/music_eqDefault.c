#include "lib.h"

static const hs_codec_eqpara_t g_stMusicEq[] = 
{
  /* 
   * 0. Standard style eq data
   */
  {
    {80, 320, 640, 1280, 2560, 5120, 10240},
    {0,   0,   0,   0,    0,    0,    0},
  },

  /* 
   * 1. Rock style eq data
   */
  {
    {80, 320, 640, 1280, 2560, 5120, 10240},
    {4,  0,   0,   -2,   3,    3,    5},
  },

  /* 
   * 2. Popular style eq data
   */
  {
    {80, 320, 640, 1280, 2560, 5120, 10240},
    {1,  0,   4,   4,    0,    -1,   1},
  },

  /* 
   * 3. Classical style eq data
   */
  {
    {80, 320, 640, 1280, 2560, 5120, 10240},
    {3,  0,   -1,  -1,    0,    1,    2},
  },

  /* 
   * 4. Jazz style eq data
   */
  {
    {31, 62, 125, 250, 500, 1000, 2000},
    {-1, 0,  -2,  1,   3,   2,    2},
  },

  /* 
   * 5. Folk style eq data
   */
  {
    {80, 320, 640, 1280, 2560, 5120, 10240},
    {0,  0,   2,   -1,   2,    1,    4},
  },
};

uint8_t hs_music_getDefaultEqNum(void)
{
  return sizeof(g_stMusicEq) / sizeof(hs_codec_eqpara_t);
}

NOINLINE const hs_codec_eqpara_t * hs_music_getDefaultEqData(uint8_t u8Idx)
{
  uint8_t u8EqNum = hs_music_getDefaultEqNum();

  u8Idx = u8Idx >= u8EqNum ? 0 : u8Idx;
  return &g_stMusicEq[u8Idx];
}