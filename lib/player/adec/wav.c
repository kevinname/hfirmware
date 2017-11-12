/*
    audio player - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/wav.c
 * @brief   source file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */

#include "lib.h"

#if HS_USE_MP3

static int32_t _adec_wavInit(hs_adecwav_t * pstAdecWav) 
{
  if(!pstAdecWav)
    return ADEC_ERR;

  /* init read session */
  pstAdecWav->read_ptr = NULL;
  pstAdecWav->bytes_left = 0;
  pstAdecWav->frames = 0;

  pstAdecWav->read_buffer = hs_malloc(WAV_READ_BUFFER_SIZE, __MT_DMA);
  if (pstAdecWav->read_buffer == NULL)
    return ADEC_ERR;

  return ADEC_OK;
}

static int32_t _adec_wavFillBuf(hs_adecwav_t *decoder) 
{
  hs_adec_t *pstAdec = &decoder->stAdec;
  size_t bytes_read;
  size_t bytes_to_read;

  bytes_to_read = 2400 - decoder->bytes_left;
  memmove(decoder->read_buffer, decoder->read_buffer + bytes_to_read, decoder->bytes_left);
  bytes_read = pstAdec->pfnFetchData(pstAdec,
                                     (uint8_t *) (decoder->read_buffer + decoder->bytes_left),
                                     bytes_to_read);
  
  if (bytes_read != 0) 
  {
    decoder->read_ptr = decoder->read_buffer;
    decoder->bytes_left += bytes_read;

    decoder->frames += bytes_read;
    return ADEC_OK;
  } 

  return ADEC_ERR;
}

static int _adec_wavDataHandle(hs_adecwav_t *decoder) 
{
  hs_adec_t *pstAdec = &decoder->stAdec;

  if(decoder->frames >= pstAdec->u32Frames)
  {
    pstAdec->pfnCarryAway(decoder->read_ptr, decoder->bytes_left);
    pstAdec->u32Frames = decoder->frames;
  }
  
  decoder->read_ptr = decoder->read_buffer;
  decoder->bytes_left = 0;

  return ADEC_OK;
}

static int _adec_wavRun(hs_adec_t * pstAdec) 
{
  hs_adecwav_t * pstAdecWav = (hs_adecwav_t *)pstAdec->decoder;
  
  if((!pstAdec) || (!pstAdecWav))
    return ADEC_ERR;

  if ((pstAdecWav->read_ptr == NULL) || pstAdecWav->bytes_left < 2400) 
  {
    if (_adec_wavFillBuf(pstAdecWav) != 0) 
      return ADEC_ERR;
  }

  return _adec_wavDataHandle(pstAdecWav);
}

static int _adec_wavSkipHeader(hs_adec_t * pstAdec) 
{
  hs_adecwav_t * pstAdecWav = (hs_adecwav_t *)pstAdec->decoder;
  wave_header *header;
  uint32_t offset;
  uint32_t flag = 0;

  if((!pstAdec) || (!pstAdecWav))
    return ADEC_ERR;

  if ((pstAdecWav->read_ptr == NULL) || pstAdecWav->bytes_left < 2400) 
  {
    if (_adec_wavFillBuf(pstAdecWav) != 0) 
      return ADEC_ERR;
  }

  header = (wave_header *) pstAdecWav->read_ptr;

  if (strncmp("RIFF", header->riff_sig, sizeof(header->riff_sig)) != 0) 
    return ADEC_ERR;

  if (strncmp("WAVE", header->wave_sig, sizeof(header->wave_sig)) != 0)
    return ADEC_ERR;

  if (header->format_tag != WAVE_FORMAT_PCM
      || (header->bits_per_sample != 16 && header->bits_per_sample != 24))
    return ADEC_ERR;

  if (header->sample_rate != (int) pstAdec->pstAo->stI2sCfg.sample_rate) 
  {
    pstAdec->pstAo->stI2sCfg.sample_rate = (hs_i2s_sample_t) header->sample_rate;
    hs_ao_setSample(pstAdec->pstAo->stI2sCfg.sample_rate);
  }

  if(header->bits_per_sample != pstAdec->pu8Width[(int)pstAdec->pstAo->stI2sCfg.sample_width])
  {
    if(header->bits_per_sample == 16)
      pstAdec->pstAo->stI2sCfg.sample_width = I2S_BITWIDTH_16BIT;
    else
      pstAdec->pstAo->stI2sCfg.sample_width = I2S_BITWIDTH_24BIT;
    
    pstAdec->pstAo->stI2sCfg.sample_rate = (hs_i2s_sample_t) header->sample_rate;
    
    flag = 1;
  }  

  if (header->channels == 1 && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_MONO) 
  {
    pstAdec->pstAo->stI2sCfg.i2s_mode = I2S_PCMMODE_MONO;
    flag = 1;
  } 
  else if (header->channels == 2 && pstAdec->pstAo->stI2sCfg.i2s_mode != I2S_PCMMODE_STEREO) 
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

  if(pstAdec->u32Frames == 0)
  {
    offset = sizeof(wave_header);  
    while (offset > WAV_READ_BUFFER_SIZE) 
    {
      pstAdecWav->bytes_left = 0;
      if (_adec_wavFillBuf(pstAdecWav) != 0)
        return ADEC_ERR;

      offset -= WAV_READ_BUFFER_SIZE;
    }

    pstAdecWav->read_offset = offset;
    pstAdecWav->read_ptr += pstAdecWav->read_offset;
    pstAdecWav->bytes_left -= pstAdecWav->read_offset;
  }
  else 
  {
    if(pstAdec->u32Frames > pstAdecWav->frames)
    {
      if(FR_OK != f_lseek(pstAdec->pstSrcFile, pstAdec->u32Frames))
        return ADEC_ERR;

      pstAdecWav->frames = pstAdec->u32Frames;
    }
    else
    {
      pstAdecWav->read_offset = pstAdec->u32Frames;
      pstAdecWav->read_ptr += pstAdecWav->read_offset;
      pstAdecWav->bytes_left -= pstAdecWav->read_offset;
    }
  }

  return ADEC_OK;
}

static void _adec_wavDestroy(hs_adec_t * pstAdec)
{
  hs_adecwav_t * pstAdecWav = (hs_adecwav_t *)pstAdec->decoder;
  
  if((!pstAdec) || (!pstAdecWav))
    return ;

  if(pstAdecWav->read_buffer)
    hs_free(pstAdecWav->read_buffer);
  
  hs_free(pstAdecWav);
  pstAdec->decoder = NULL;
}

static void _adec_wavReinit(hs_adec_t * pstAdec)
{
  hs_adecwav_t * pstAdecWav = (hs_adecwav_t *)pstAdec->decoder;

  pstAdecWav->read_ptr = NULL;
  pstAdecWav->bytes_left_before_decoding = pstAdecWav->bytes_left = 0;
  pstAdecWav->frames = 0;

}

hs_adec_t* hs_adec_wavCreate(void) 
{
  hs_adecwav_t * pstAdecWav;
  hs_adec_t* pstAdec;

  /* allocate object */
  pstAdecWav = (hs_adecwav_t*) hs_malloc(sizeof(hs_adecwav_t), __MT_DMA);
  if (!pstAdecWav)
    return NULL;
  
  if(ADEC_OK != _adec_wavInit(pstAdecWav))
  {
    hs_free(pstAdecWav);
    return NULL;
  }

  pstAdec = &pstAdecWav->stAdec;
  memcpy(pstAdec->flag, "wav", sizeof("wav"));
  pstAdec->pfnReinit  = _adec_wavReinit;
  pstAdec->pfnSkip    = _adec_wavSkipHeader;
  pstAdec->pfnRun     = _adec_wavRun;
  pstAdec->pfnDestroy = _adec_wavDestroy;  
  pstAdec->decoder    = pstAdecWav;
  
  return pstAdec;
}

#endif
