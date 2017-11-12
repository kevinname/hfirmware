/*
    audio player - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/mp3.c
 * @brief   source file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */

#include "lib.h"
#include "mp3common.h"

#define MP3_READ_BUFFER_SIZE    (6 * 1024)

#if HS_USE_MP3
static int32_t _adec_mp3FillBuffer(hs_adecmp3_t* decoder) 
{
  hs_adec_t *pstAdec = &decoder->stAdec;
  int32_t bytes_read;
  uint32_t bytes_to_read;

  if (decoder->bytes_left > 0) 
    memmove(decoder->read_buffer, decoder->read_ptr, decoder->bytes_left);

  bytes_to_read = (MP3_READ_BUFFER_SIZE - decoder->bytes_left) & ~(512 - 1);

  bytes_read = pstAdec->pfnFetchData(pstAdec,
                                    (uint8_t *) (decoder->read_buffer + decoder->bytes_left),
                                     bytes_to_read);

  if (bytes_read != 0) 
  {
    decoder->read_ptr = decoder->read_buffer;
    decoder->read_offset = 0;
    decoder->bytes_left = decoder->bytes_left + bytes_read;
    return ADEC_OK;
  } 

  return ADEC_ERR;
}

static int32_t _adec_mp3Init(hs_adecmp3_t* decoder) 
{
  if(!decoder)
    return ADEC_ERR;

  /* init read session */
  decoder->read_ptr = NULL;
  decoder->bytes_left_before_decoding = decoder->bytes_left = 0;
  decoder->frames = 0;
  decoder->ms = 0;
  decoder->read_offset = 0;

  decoder->read_buffer = hs_malloc(MP3_READ_BUFFER_SIZE, __MT_Z_DMA);
  if (decoder->read_buffer == NULL)
    return ADEC_ERR;

  decoder->decoder = MP3InitDecoder();

  return ADEC_OK;
}

static int32_t _adec_mp3Sync(hs_adecmp3_t* decoder) 
{
  if ((decoder->read_ptr == NULL) || decoder->bytes_left < 2 * MAINBUF_SIZE) 
  {
    if (_adec_mp3FillBuffer(decoder) != 0)
      return ADEC_ERR;
  }

  decoder->read_offset = MP3FindSyncWord(decoder->read_ptr, decoder->bytes_left);
  if (decoder->read_offset < 0) 
  {
    decoder->bytes_left = 0;
    return 0;
  }

  decoder->read_ptr += decoder->read_offset;
  decoder->bytes_left -= decoder->read_offset;
  if (decoder->bytes_left < 1024) 
  {
    /* fill more data */
    if (_adec_mp3FillBuffer(decoder) != 0)
      return ADEC_ERR;
  }

  return 1;
}



static int32_t _adec_mp3Decode(hs_adecmp3_t* decoder, short *outBuf) 
{
  int err;
  uint32_t tmp = (uint32_t)decoder->read_ptr;
  MP3DecInfo *pstDecInfo = (MP3DecInfo *)decoder->decoder;  

  decoder->bytes_left_before_decoding = decoder->bytes_left;
  err = MP3Decode(decoder->decoder, &decoder->read_ptr, 
                  (int*) &decoder->bytes_left, outBuf, 0);  

  if (err != ERR_MP3_NONE) 
  {
    switch (err) 
    {
    case ERR_MP3_INDATA_UNDERFLOW:
      decoder->bytes_left = 0;
      if (_adec_mp3FillBuffer(decoder) != 0)
        return ADEC_ERR;
      break;

    case ERR_MP3_MAINDATA_UNDERFLOW:
      /* do nothing - next call to decode will provide more mainData */
      break;

    default:
      // skip this frame
      if (decoder->bytes_left > 0) 
      {
        decoder->bytes_left--;
        decoder->read_ptr++;
      } 
      break;
    }

    return 0;
  }

  decoder->frames += (uint32_t)decoder->read_ptr - tmp;  
  decoder->ms += ((uint32_t)decoder->read_ptr - tmp) * 1000 / (pstDecInfo->bitrate / 8);
  return 1;
}

static int32_t _adec_mp3DataHandle(hs_adecmp3_t* decoder, short *buffer) 
{
  hs_adec_t * pstAdec = &decoder->stAdec;
  int outputSamps;
  uint32_t flag = 0;

  if(decoder->frames <= pstAdec->u32Frames)
    return ADEC_OK;
  
  /* no error */
  MP3GetLastFrameInfo(decoder->decoder, &decoder->frame_info);

  if(decoder->frame_info.samprate == 0)
    return ADEC_OK;

  if (decoder->frame_info.samprate != (int) pstAdec->pstAo->stI2sCfg.sample_rate) 
  {
    pstAdec->pstAo->stI2sCfg.sample_rate = (hs_i2s_sample_t) decoder->frame_info.samprate;
    hs_ao_setSample(pstAdec->pstAo->stI2sCfg.sample_rate);
  }

  if(decoder->frame_info.bitsPerSample != pstAdec->pu8Width[(int)pstAdec->pstAo->stI2sCfg.sample_width])
  {
    if(decoder->frame_info.bitsPerSample == 16)
      pstAdec->pstAo->stI2sCfg.sample_width = I2S_BITWIDTH_16BIT;
    else
      pstAdec->pstAo->stI2sCfg.sample_width = I2S_BITWIDTH_24BIT;

    pstAdec->pstAo->stI2sCfg.sample_rate = (hs_i2s_sample_t) decoder->frame_info.samprate;

    flag = 1;
  }  

  if (decoder->frame_info.nChans == 1
      && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_MONO) 
  {
    pstAdec->pstAo->stI2sCfg.i2s_mode = I2S_PCMMODE_MONO;
    flag = 1;
  } 
  else if (decoder->frame_info.nChans == 2
      && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_STEREO) 
  {
    pstAdec->pstAo->stI2sCfg.i2s_mode = I2S_PCMMODE_STEREO;
    flag = 1;
  } 
  else 
  {
    ;
  }

  if(flag == 1)
  {
    hs_ao_stop(pstAdec->pstAo);
    hs_ao_start(pstAdec->pstAo);
  }  

  /* write to sound device */
  outputSamps = decoder->frame_info.outputSamps;
  if (outputSamps > 0)
  {
    pstAdec->pfnCarryAway((uint8_t *) buffer, outputSamps * sizeof(uint16_t));
    pstAdec->u32Frames = decoder->frames;
  }

  return ADEC_OK;
}

static void _adec_mp3Reinit(hs_adec_t *pstAdec)
{
  hs_adecmp3_t* decoder = (hs_adecmp3_t *)pstAdec->decoder;

  if((!pstAdec) || (!decoder))
    return ;

  decoder->read_ptr = NULL;
  decoder->bytes_left_before_decoding = decoder->bytes_left = 0;
  decoder->frames = 0;
  decoder->ms = 0;
}

static int _adec_mp3SkipId3v2(hs_adec_t *pstAdec)
{
  hs_adecmp3_t* decoder = (hs_adecmp3_t *)pstAdec->decoder;
  uint32_t offset, tmp, usedlen = 0;
  uint8_t *ptr;

  if((!pstAdec) || (!decoder))
    return ADEC_ERR;

  if ((decoder->read_ptr == NULL) || decoder->bytes_left < 2 * MAINBUF_SIZE) 
  {
    if (_adec_mp3FillBuffer(decoder) != 0)
      return ADEC_ERR;

    usedlen += decoder->bytes_left;
  }

  ptr = decoder->read_ptr;
  if (strncmp("ID3", (char *) ptr, 3) == 0) 
  {
    offset = (ptr[9] & 0x7F) + ((ptr[8] & 0x7F) << 7) + ((ptr[7] & 0x7F) << 14)
        + ((ptr[6] & 0x7F) << 21) + 10;

    while (offset > MP3_READ_BUFFER_SIZE) 
    {
      decoder->bytes_left = 0;
      if (_adec_mp3FillBuffer(decoder) != 0)
        return ADEC_ERR;

      usedlen += decoder->bytes_left;
      offset -= MP3_READ_BUFFER_SIZE;
    }

    decoder->read_offset = offset;
    decoder->read_ptr += decoder->read_offset;
    decoder->bytes_left -= decoder->read_offset;
    if (decoder->bytes_left < 1024) 
    {
      tmp = decoder->bytes_left;
      /* fill more data */
      if (_adec_mp3FillBuffer(decoder) != 0)
        return ADEC_ERR;

      usedlen += decoder->bytes_left - tmp;
    }

    decoder->frames = pstAdec->u32Frames;
    if(decoder->bytes_left > pstAdec->u32Frames)
    {
      decoder->read_offset = pstAdec->u32Frames;
      decoder->read_ptr += decoder->read_offset;
      decoder->bytes_left -= decoder->read_offset;       
    }
    else
    {
      tmp = pstAdec->u32Frames - decoder->bytes_left;
      usedlen += tmp;

      if(FR_OK != f_lseek(pstAdec->pstSrcFile, usedlen))
        return ADEC_ERR;

      decoder->bytes_left = 0;
      decoder->bytes_left_before_decoding = 0;
    }

    if (decoder->bytes_left < 1024) 
    {
      if (_adec_mp3FillBuffer(decoder) != 0)
        return ADEC_ERR;
    }
  }
  else
  {
    decoder->frames = pstAdec->u32Frames;
    
    if(FR_OK != f_lseek(pstAdec->pstSrcFile, decoder->frames))
      return ADEC_ERR;

    decoder->bytes_left = 0;
    if (_adec_mp3FillBuffer(decoder) != 0)
      return ADEC_ERR;
  }

  return ADEC_OK;
}

static int _adec_mp3Run(hs_adec_t *pstAdec)
{
  hs_adecmp3_t* decoder = (hs_adecmp3_t *)pstAdec->decoder;
  int32_t ret;

  if((!pstAdec) || (!decoder))
    return ADEC_ERR;

  if ((ret = _adec_mp3Sync(decoder)) != 1)
    return ret;

  #if 1
  if (_adec_mp3Decode(decoder, decoder->outBuf) == 1)
    _adec_mp3DataHandle(decoder, decoder->outBuf);
  else
    hs_printf("\r\n* Decode error! *\r\n\r\n");
  #else
  uint32_t cnt;
  static uint32_t g_max;
  
  cnt = __nds32__mfsr(NDS32_SR_PFMC1);
  ret = _adec_mp3Decode(decoder, decoder->outBuf);
  cnt = __nds32__mfsr(NDS32_SR_PFMC1) - cnt;
  
  if(ret == 1)
    _adec_mp3DataHandle(decoder, decoder->outBuf);
  else
    g_max = 0;

  cnt = cnt / (CPU_DEFAULT_CLOCK / 1000);
  if(cnt > g_max)
  {
    g_max = cnt;
    hs_printf("time: %d\r\n", cnt);
  }
  #endif
  
  return ADEC_OK;
}

static void _adec_mp3Destroy(hs_adec_t* pstAdec) 
{
  hs_adecmp3_t * decoder = (hs_adecmp3_t *) pstAdec->decoder;

  if(!decoder)
    return ;

  if(decoder->read_buffer)
    hs_free(decoder->read_buffer);

  if(decoder->decoder)
  {
    FreeBuffers(decoder->decoder);
    decoder->decoder = NULL;
  }

  hs_free(decoder);
  pstAdec->decoder = NULL;
}

hs_adec_t* hs_adec_mp3Create(void) 
{
  hs_adecmp3_t * decoder;
  hs_adec_t* pstAdec;

  /* allocate object */
  decoder = (hs_adecmp3_t *) hs_malloc(sizeof(hs_adecmp3_t), __MT_GENERAL);
  if (!decoder) 
    return NULL;

  if(0 != hs_mem_loadCode(__MCT_MP3))
  {
    hs_free(decoder);
    return NULL;
  }
  
  _adec_mp3Init(decoder);

  pstAdec = &decoder->stAdec;
  memcpy(pstAdec->flag, "mp3", sizeof("mp3"));
  pstAdec->pfnReinit  = _adec_mp3Reinit;
  pstAdec->pfnSkip    = _adec_mp3SkipId3v2;
  pstAdec->pfnRun     = _adec_mp3Run;
  pstAdec->pfnDestroy = _adec_mp3Destroy;  
  pstAdec->decoder    = decoder;

  return pstAdec;
}

#endif

