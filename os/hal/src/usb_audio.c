/*
 ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
 2011,2012,2013 Giovanni Di Sirio.

 This file is part of ChibiOS/RT.

 ChibiOS/RT is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 ChibiOS/RT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ---

 A special exception to the GPL can be applied should you wish to distribute
 a combined work that includes ChibiOS/RT, without being obliged to provide
 the source code for any proprietary components. See the file exception.txt
 for full details of how and when the exception can be applied.
 */

/*
 *   pengjiang, 20140714
 */
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "bmem.h"
#include "cmsis_os.h"

#if HAL_USE_USB_AUDIO

USBAudioDriver USBAUD1;

static union type_16bit_data t_buf;
static uint8_t g_Sr[3];
static usbaudio_stream g_ua_stream_ply = {UA_CMD_NONE, 0, 0, 0};
static usbaudio_stream g_ua_stream_rec = {UA_CMD_NONE, 0, 0, 0};
static usbaudio_cmd g_ua_cmd = {UA_CMD_NONE, 0, 0, 0};
static uint8_t g_IsPly = FALSE;
static uint8_t g_IsRec = FALSE;
static hs_audio_config_t ply_cfg =
{
  I2S_SAMPLE_48K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_64BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,
  I2S_PLY_BLOCK_SIZE,
  I2S_PLY_BLOCK_NUM,
};

static hs_audio_config_t rec_cfg =
{
  I2S_SAMPLE_16K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_64BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,
  I2S_REC_BLOCK_SIZE,
  I2S_REC_BLOCK_NUM,
};

#define UA_MAILBOX_SIZE 2
//static mailbox_t g_ply_mbox;
static mailbox_t g_cmd_mbox;
static msg_t g_message[UA_MAILBOX_SIZE];

static THD_FUNCTION(UsbAudioCmdThread, arg);

thread_t* g_thread_ply = NULL;

static mutex_t g_mp_ply;
static mutex_t g_mp_rec;

static uint8_t g_IsPlyByOthers = FALSE;
static uint8_t g_IsRecByOthers = FALSE;

const SAMPLE_RATE_CONVERT Sr_Table[] =
{
  { 8000, SR_8K, I2S_SAMPLE_8K, 0},
  { 12000, SR_12K, I2S_SAMPLE_12K, 0},
  { 16000, SR_16K, I2S_SAMPLE_16K, 0},
  { 24000, SR_24K, I2S_SAMPLE_24K, 0},
  { 32000, SR_32K, I2S_SAMPLE_32K, 0},
  { 48000, SR_48K, I2S_SAMPLE_48K, 0},
  { 96000, SR_96K, I2S_SAMPLE_96K, 0},
  { 11025, SR_11K, I2S_SAMPLE_11K, 40},
  { 22050, SR_22K, I2S_SAMPLE_22K, 20},
  { 44100, SR_44K, I2S_SAMPLE_44P1K, 10}, // 10 means every 10ms add 1 sample
  { 0, 0, I2S_SAMPLE_16K, 0},
};

uint16_t get_srdrv_by_sr_usb(uint32_t sr_usb)
{
  size_t i = 0;
  while(Sr_Table[i].SampleRate_Usb)
  {
    if(sr_usb == Sr_Table[i].SampleRate_Usb)
    {
      return Sr_Table[i].SampleRate_Drv;
    }
    i++;
  }
  return (-1);
}

hs_i2s_sample_t get_sri2s_by_sr_drv(uint16_t sr_drv)
{
  size_t i = 0;
  while(Sr_Table[i].SampleRate_Usb)
  {
    if(sr_drv == Sr_Table[i].SampleRate_Drv)
    {
      return Sr_Table[i].SampleRate_I2S;
    }
    i++;
  }
  return I2S_SAMPLE_16K; //default return 16k
}

uint32_t get_srusb_by_sr_drv(uint16_t sr_drv)
{
  size_t i = 0;
  while(Sr_Table[i].SampleRate_Usb)
  {
    if(sr_drv == Sr_Table[i].SampleRate_Drv)
    {
      return Sr_Table[i].SampleRate_Usb;
    }
    i++;
  }
  return (-1);
}

uint16_t get_srfrac_by_sr_drv(uint16_t sr_drv)
{
  size_t i = 0;
  while(Sr_Table[i].SampleRate_Usb)
  {
    if(sr_drv == Sr_Table[i].SampleRate_Drv)
    {
      return Sr_Table[i].SampleRate_Frac;
    }
    i++;
  }
  return (-1);
}

void set_audio_volume_ply_db(int8_t vol_db)
{
  int32_t vol_max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_USB);
  int32_t vol_min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_USB);
  if(vol_db<vol_min)
  {
    vol_db = vol_min;
  }
  else if(vol_db>vol_max)
  {
    vol_db = vol_max;
  }
  audioPlaySetVolume(vol_db);
}

void set_audio_volume_rec_db(int8_t vol_db)
{
  int32_t vol_max = audioRecordGetVolumeMax(AUDIO_REC_SRC_USB);
  int32_t vol_min = audioRecordGetVolumeMin(AUDIO_REC_SRC_USB);
  if(vol_db<vol_min)
  {
    vol_db = vol_min;
  }
  else if(vol_db>vol_max)
  {
    vol_db = vol_max;
  }
  audioRecordSetVolume(vol_db);
}

void set_ua_cmd(usbaudio_cmd* ua_cmd,uint8_t cmd, uint16_t para1, uint16_t para2, uint32_t pVal)
{
  chDbgAssert(ua_cmd, "null pointer");
  ua_cmd->cmd = cmd;
  ua_cmd->para1 = para1;
  ua_cmd->para2 = para2;
  ua_cmd->pval = pVal;
}

void clear_ua_cmd(usbaudio_cmd* ua_cmd)
{
  chDbgAssert(ua_cmd, "null pointer");
  ua_cmd->cmd = 0;
  ua_cmd->para1 = 0;
  ua_cmd->para2 = 0;
  ua_cmd->pval = 0;
}

void senddata_from_usb_to_audio(USBAudioDriver *usbaudp, uint8_t *pUsbBuf, uint16_t Bytes)
{
  size_t actualLen = 0;
  size_t copypos = 0;
  uint8_t *pData;
  systime_t timeout = S2ST(1);
  uint16_t cnt = SEND_DATA_CNT;

  chDbgAssert(usbaudp&&pUsbBuf, "null pointer");

  while(g_IsPly&&(copypos<Bytes)&&(cnt--))
  {
    actualLen = audioPlyGetDataBuffer(&pData,
        Bytes - copypos,
        timeout);
    chSysLock();
    if(actualLen)
    {
      chDbgAssert(pData, "null pointer-2");

      audioPlyCpyData(pData, &pUsbBuf[copypos], min(actualLen,Bytes));
    }
    chSysUnlock();
    audioPlySendDataDone(pData, actualLen);
    copypos += actualLen;
  }

}

void getdata_from_audio_to_usb(USBAudioDriver *usbaudp, uint8_t *pUsbBuf, uint16_t Bytes)
{
  size_t actualLen = 0;
  size_t copypos = 0;
  uint8_t *pData;
  systime_t timeout = S2ST(1);
  uint16_t cnt = SEND_DATA_CNT;

  chDbgAssert(usbaudp&&pUsbBuf, "null pointer");

  while(g_IsRec&&(copypos<Bytes)&&(cnt--))
  {
    actualLen = audioRecGetDataBuffer(&pData,
        Bytes - copypos,
        timeout);
    chSysLock();
    if(actualLen)
    {
      chDbgAssert(pData, "null pointer-2");

      memcpy(&pUsbBuf[copypos], pData, min(actualLen,Bytes));
    }
    chSysUnlock();
    audioRecGetDataDone(pData, actualLen);
    copypos += actualLen;
  }

}

void usbaud_dma_outep_cb(USBDriver *usbp, usbep_t ep)
{
  chDbgAssert(usbp, "null pointer");

  (void)usbp;
  (void)ep;
  //chMBPostI(&g_ply_mbox, TRUE);
  if (g_thread_ply != NULL)
  {
    chSchReadyI(g_thread_ply);
    g_thread_ply= NULL;
  }
}

void usbaud_dma_inep_cb(USBDriver *usbp, usbep_t ep)
{
  (void)usbp;
  (void)ep;
  chDbgAssert(usbp, "null pointer");
}

void usbaudInit(void)
{
  USBAUD1.state = USB_AUDIO_STATE_STOP;
}

static void start_play(USBDriver *usbp)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];

  chDbgAssert(usbp&&usbaudp, "null pointer");

  //set ch, bit, res, samplerate
  if(usbaudp->if_alt_set[IDX_PLY])
  {
    set_ua_cmd(&g_ua_cmd,UA_CMD_START_PLY ,0, 0, (uint32_t)usbp);
    chMBPostI(&g_cmd_mbox, TRUE);
  }
}

static void stop_play(USBDriver *usbp)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];

  chDbgAssert(usbp&&usbaudp, "null pointer");

  usbaudp->fmt_info[IDX_PLY].Bits = 0;
  usbaudp->fmt_info[IDX_PLY].ChNum = 0;
  usbaudp->fmt_info[IDX_PLY].Sr = 0;

  //stop playing
  if(usbaudp->if_alt_set[IDX_PLY])
  {
    set_ua_cmd(&g_ua_cmd,UA_CMD_STOP_PLY, 0, 0, (uint32_t)usbp);
    chMBPostI(&g_cmd_mbox, TRUE);
  }
  else
  {
    usb_finish_ep0setup(usbp, 0);
  }
}

static void start_rec(USBDriver *usbp)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];

  chDbgAssert(usbp&&usbaudp, "null pointer");

  //set ch, bit, res, samplerate
  if(usbaudp->if_alt_set[IDX_REC])
  {
    set_ua_cmd(&g_ua_cmd, UA_CMD_START_REC, 0, 0, (uint32_t)usbp);
    chMBPostI(&g_cmd_mbox, TRUE);
  }
}

static void stop_rec(USBDriver *usbp)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];

  chDbgAssert(usbp&&usbaudp, "null pointer");

  usbaudp->fmt_info[IDX_REC].Bits = 0;
  usbaudp->fmt_info[IDX_REC].ChNum = 0;
  usbaudp->fmt_info[IDX_REC].Sr = 0;
  usbaudp->rec_buf_idx = 0;
  usbaudp->rec_frac_cnt = 1;
  //stop rec
  if(usbaudp->if_alt_set[IDX_PLY])
  {
    set_ua_cmd(&g_ua_cmd,UA_CMD_STOP_REC, 0, 0, (uint32_t)usbp);
    chMBPostI(&g_cmd_mbox, TRUE);
  }
  else
  {
    usb_finish_ep0setup(usbp, 0);
  }
}

static void set_mute(USBDriver *usbp)
{

  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];
  uint8_t unitId = usbp->setup[5];
  uint8_t idx;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  idx = (unitId == UNITID_FEATURE_DAC_VOLUME)?IDX_PLY:IDX_REC;
  usbaudp->vol_ctl[idx].bmute = t_buf.u8vals[0];

  set_ua_cmd(&g_ua_cmd,UA_CMD_SET_MUTE,unitId, usbaudp->vol_ctl[idx].bmute, (uint32_t)usbp);
  chMBPostI(&g_cmd_mbox, TRUE);
}

static void set_vol(USBDriver *usbp)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];
  uint8_t unitId = usbp->setup[5];
  uint8_t chNr = usbp->setup[2];
  uint8_t idx;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  idx = (unitId == UNITID_FEATURE_DAC_VOLUME)?IDX_PLY:IDX_REC;

  if(chNr == 0)
  {
    usbaudp->vol_ctl[idx].wvol[0] = t_buf.u16val;
    usbaudp->vol_ctl[idx].wvol[1] = t_buf.u16val;
  }
  else
  {
    usbaudp->vol_ctl[idx].wvol[chNr-1] = t_buf.u16val;
  }
  set_ua_cmd(&g_ua_cmd,UA_CMD_SET_VOL,unitId|(chNr<<8),
      (chNr==0)?usbaudp->vol_ctl[idx].wvol[0]:usbaudp->vol_ctl[idx].wvol[chNr-1],
      (uint32_t)usbp);
  chMBPostI(&g_cmd_mbox, TRUE);
}

//set sample rate
static void set_sr(USBDriver *usbp, uint8_t idx)
{
  uint32_t samplerate = g_Sr[0] + (g_Sr[1]<<8) + (g_Sr[2]<<16);
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];
  audio_fmt_info *pfmt = &usbaudp->fmt_info[idx];

  chDbgAssert(usbp&&usbaudp&&pfmt, "null pointer");

  pfmt->Sr = get_srdrv_by_sr_usb(samplerate);

  if(pfmt->Bits
      &&pfmt->ChNum
      &&pfmt->Sr)
  {
    if(idx == IDX_PLY)
    {
      start_play(usbp);
    }
    else
    {
      start_rec(usbp);
    }
  }
}

static void set_sr_rec(USBDriver *usbp)
{
  chDbgAssert(usbp, "null pointer");

  set_sr(usbp,IDX_REC);
}

static void set_sr_ply(USBDriver *usbp)
{

  chDbgAssert(usbp,"null pointer");

  set_sr(usbp,IDX_PLY);
}

bool_t usbaud_class_req_handler_if(USBDriver *usbp)
{
  uint8_t request = usbp->setup[1];
  uint8_t controlselector = usbp->setup[3];
  uint8_t unitId = usbp->setup[5];
  uint8_t chNr = usbp->setup[2];
  uint16_t xferlen = 0xffff;
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];
  usbcallback_t cb = NULL;
  uint8_t idx = 0;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  t_buf.u16val = 0;

  idx = (unitId == UNITID_FEATURE_DAC_VOLUME)?IDX_PLY:IDX_REC;

  if((usbp->setup[0] & USB_RTYPE_DIR_MASK) == USB_RTYPE_DIR_HOST2DEV)
  {
    //set commands
    xferlen = usbFetchWord(&usbp->setup[6]);
    if(controlselector == MUTE_CONTROL)
    {
      if(unitId == UNITID_FEATURE_DAC_VOLUME
          ||unitId == UNITID_FEATURE_ADC_VOLUME)
      {
        xferlen = min(1,xferlen);
        cb = set_mute;
      }
    }
    else if(controlselector == VOLUME_CONTROL)
    {
      if(unitId == UNITID_FEATURE_DAC_VOLUME
          ||unitId == UNITID_FEATURE_ADC_VOLUME)
      {
        xferlen = min(2,xferlen);
        cb = set_vol;
      }
    }

  }
  else
  {
    //get commands
    if(request == GET_CUR)
    {
      if(controlselector == MUTE_CONTROL)
      {
        xferlen = min(1,usbFetchWord(&usbp->setup[6]));
        t_buf.u8vals[0] = usbaudp->vol_ctl[idx].bmute;
      }
      else if(controlselector == VOLUME_CONTROL)
      {
        xferlen = min(2,usbFetchWord(&usbp->setup[6]));
        if(chNr == 1 || chNr == 2)
        {
          t_buf.u16val = usbaudp->vol_ctl[idx].wvol[chNr-1];
        }
        else if(chNr == 0)
        {
          t_buf.u16val = usbaudp->vol_ctl[idx].wvol[0];
        }
      }
    }
    else if((request == GET_MIN )&&(controlselector == VOLUME_CONTROL))
    {
      if(chNr<=2)
      {
        xferlen = min(2,usbFetchWord(&usbp->setup[6]));
        t_buf.u16val = usbaudp->iso_info[idx]->volminmax.vol_min;
      }
    }
    else if((request == GET_MAX)&&(controlselector == VOLUME_CONTROL))
    {
      if(chNr<=2)
      {
        xferlen = min(2,usbFetchWord(&usbp->setup[6]));
        t_buf.u16val = usbaudp->iso_info[idx]->volminmax.vol_max;
      }
    }
    else if((request == GET_RES) && (controlselector == VOLUME_CONTROL))
    {
      if(chNr<=2)
      {
        xferlen = min(2,usbFetchWord(&usbp->setup[6]));
        t_buf.u16val = usbaudp->iso_info[idx]->volminmax.vol_res;
      }
    }
  }
  if(xferlen == 0xffff)
  {
    return FALSE;
  }
  else
  {
    usbSetupTransfer(usbp, (uint8_t *)&t_buf.u8vals[0], xferlen, cb);
    usb_finish_ep0setup(usbp, 0);
    return TRUE;
  }
}

bool_t usbaud_class_req_handler_ep(USBDriver *usbp, bool_t IsPly)
{
  uint16_t xferlen;

  chDbgAssert(usbp, "null pointer");

  if((usbp->setup[3] != SAMPLING_FREQ_CONTROL)
      || (usbp->setup[1]!= SET_CUR))
  {
    return FALSE;
  }
  xferlen = usbFetchWord(&usbp->setup[6]);
  xferlen = min(3,xferlen);

  usbSetupTransfer(usbp, &g_Sr[0], xferlen, IsPly?set_sr_ply:set_sr_rec);

  usb_finish_ep0setup(usbp, 0);

  return TRUE;
}

bool_t usbaud_std_req_handler_set_interface(USBDriver *usbp, bool_t IsPly)
{
  USBAudioDriver *usbaudp = (USBAudioDriver *)usbp->out_params[0];
  uint8_t altset = usbp->setup[2];
  uint8_t idx = 0;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  idx = (IsPly)?IDX_PLY:IDX_REC;

  if(altset == 0)
  {
    usbSetupTransfer(usbp, NULL, 0, NULL);
    if(idx == IDX_PLY)
    {
      stop_play(usbp);

    }
    else
    {
      stop_rec(usbp);
    }
    usbaudp->if_alt_set[idx] = altset;
  }
  else
  {
    usbaudp->if_alt_set[idx] = altset;
    if(altset>(usbaudp->iso_info[idx]->AltSettingNum-1))
    {
      return FALSE;
    }
    usbaudp->fmt_info[idx].Bits = usbaudp->iso_info[idx]->Settings[altset].Bits;
    usbaudp->fmt_info[idx].ChNum = usbaudp->iso_info[idx]->Settings[altset].ChNum;
    if(usbaudp->iso_info[idx]->Settings[altset].SrNum == 1)
    {
      usbaudp->fmt_info[idx].Sr = usbaudp->iso_info[idx]->Settings[altset].SrBitMap;
    }
    usbSetupTransfer(usbp, NULL, 0, NULL);
    if(usbaudp->fmt_info[idx].Bits
        &&usbaudp->fmt_info[idx].ChNum
        &&usbaudp->fmt_info[idx].Sr)
    {
      if(idx == IDX_PLY)
      {
        start_play(usbp);
      }
      else
      {
        start_rec(usbp);
      }
    }
    usb_finish_ep0setup(usbp, 0);

  }

  /* Nothing to do, there are no control lines.*/

  return TRUE;

}

void usbaudDataTransmitted(USBDriver *usbp, usbep_t ep)
{
  USBAudioDriver *usbaudp = usbp->out_params[0];
  audio_fmt_info *pfmt;
  uint16_t pktSiz, pktSiz_request;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  //if (usbaudp == NULL)
  //    return;
  chMtxLockS(&g_mp_rec);
  pfmt = &usbaudp->fmt_info[IDX_REC];
  pktSiz = (pfmt->Bits/8)*pfmt->ChNum*(get_srusb_by_sr_drv(pfmt->Sr)/1000);
  pktSiz_request = pktSiz;
  if(pfmt->Sr_Frac&&usbaudp->rec_frac_cnt == pfmt->Sr_Frac)
  {
    pktSiz += (pfmt->Bits/8)*pfmt->ChNum;
    usbaudp->rec_frac_cnt = 1;
  }
  else
  {
    usbaudp->rec_frac_cnt++;
  }

  nds32_dcache_flush();
  usbPrepareTransmit(usbp, ep, &usbaudp->rec_buf[usbaudp->rec_buf_idx], pktSiz, usbp->epc[ep]->dma_info.dma_mode[IDX_REC]);
  usbStartTransmitI(usbp, ep);

  if(pfmt->Sr_Frac&&usbaudp->rec_frac_cnt == pfmt->Sr_Frac)
  {
    pktSiz_request += (pfmt->Bits/8)*pfmt->ChNum;
  }
  usbaudp->rec_buf_idx++;
  usbaudp->rec_buf_idx %= REC_BUF_NUM;
  chMtxUnlockS(&g_mp_rec);
  if(!g_IsRecByOthers)
  {
    chSysUnlock();
    getdata_from_audio_to_usb(usbaudp, &usbaudp->rec_buf[usbaudp->rec_buf_idx], pktSiz_request);
    //chMBPostS(&g_rec_mbox, TRUE, TIME_INFINITE);
    chSysLock();
  }
}

void usbaudDataReceived(USBDriver *usbp, usbep_t ep)
{
  USBAudioDriver *usbaudp = usbp->out_params[0];
  USBOutEndpointState *osp = usbp->epc[ep]->out_state;
  uint8_t *ply_buf = &usbaudp->ply_buf[usbaudp->ply_buf_idx];

  chDbgAssert(usbp&&usbaudp, "null pointer");

  if(usbp->epc[ep]->dma_info.dma_mode[IDX_PLY]== DMA_MODE_0)
  {
    chMtxLockS(&g_mp_ply);
    //msg_t msg;

    nds32_dcache_flush();
    usbPrepareReceive(usbp, ep, ply_buf, PLY_MAX_PKT_SIZ, DMA_MODE_0);
    usbStartReceiveI(usbp, ep);
    chMtxUnlockS(&g_mp_ply);
    //chSysLock();
    g_thread_ply = chThdGetSelfX();
    chSchGoSleepS(CH_STATE_SUSPENDED);
    //chSysUnlock();

    //chMBFetchS(&g_ply_mbox, &msg, TIME_INFINITE);
    if(!g_IsPlyByOthers)
    {
      chSysUnlock();
      senddata_from_usb_to_audio(usbaudp, &usbaudp->ply_buf[usbaudp->ply_buf_idx], osp->rxcnt);
      chSysLock();
    }
    usbaudp->ply_buf_idx++;
    usbaudp->ply_buf_idx %= PLY_BUF_NUM;
  }
}

void usbaudObjectInit(USBAudioDriver *usbaudp,USBDriver *usbp)
{
  chDbgAssert(usbp&&usbaudp, "null pointer");

  usbaudp->ply_buf = osBmemAlloc(PLY_BUF_NUM * PLY_MAX_PKT_SIZ);
  usbaudp->rec_buf = osBmemAlloc(REC_BUF_NUM * 2);

  chDbgAssert(usbaudp->ply_buf&&usbaudp->rec_buf, "memory malloc failed");

  usbaudp->usbp = usbp;
}

static void usbaudSetDefVolMute(USBAudioDriver *usbaudp,uint8_t bIdx)
{
  chDbgAssert(usbaudp, "null pointer");

  if(bIdx == IDX_PLY)
  {
    //audioPlaySetVolume((usbaudp->vol_ctl[bIdx].wvol[0]&0xff00)>>8);
    set_audio_volume_ply_db((usbaudp->vol_ctl[bIdx].wvol[0]&0xff00)>>8);
    if(usbaudp->vol_ctl[bIdx].bmute == 1)
    {
      audioPlayMute();
    }
    else
    {
      audioPlayUnmute();
    }
  }
  else if(bIdx == IDX_REC)
  {
    //audioRecordSetVolume((usbaudp->vol_ctl[bIdx].wvol[0]&0xff00)>>8);
    set_audio_volume_rec_db((usbaudp->vol_ctl[bIdx].wvol[0]&0xff00)>>8);
    if(usbaudp->vol_ctl[bIdx].bmute == 1)
    {
      audioRecordMute();
    }
    else
    {
      audioRecordUnmute();
    }
  }
}

void usb_audio_reset(USBDriver *usbp, const as_if_info* info)
{
  USBAudioDriver *usbaudp = usbp->out_params[0];
  (void)info;

  chDbgAssert(usbp&&usbaudp, "null pointer");

  usbaudp->ply_buf_idx = 0;
  usbaudp->rec_buf_idx = 0;
  usbaudp->rec_frac_cnt = 0;
  clear_ua_cmd(&g_ua_cmd);
  clear_ua_cmd(&g_ua_stream_ply);
  clear_ua_cmd(&g_ua_stream_rec);
  g_IsPly = FALSE;
  g_IsRec = FALSE;
  g_IsPlyByOthers = FALSE;
  g_IsRecByOthers = FALSE;
}

void usbaudSetAsIfInfo(USBAudioDriver *usbaudp, const as_if_info* info)
{
  usbaudp->info = info;
}

void usbaudStart(USBAudioDriver *usbaudp)
{
  USBDriver *usbp = usbaudp->usbp;
  uint8_t i = 0;
  const as_if_info* info = usbaudp->info;
  chDbgAssert(usbp&&usbaudp, "null pointer");

  if(usbaudp->state == USB_AUDIO_STATE_START)
    return ;

  usbp->out_params[0] = usbaudp;

  audioStart();
  //audioSetRecordSource(AUDIO_RECORD_LINEIN);
  //audioSetPlaySource(AUDIO_PLAY_RAM);

  audioPlaySetVolume(0);

  if(g_IsPly)
  {
    audioPlayStop();
  }
  if(g_IsRec)
  {
    audioRecordStop();
  }

  usb_audio_reset(usbp, info);

  for(i=IDX_PLY;i<=IDX_REC;i++)
  {
    usbaudp->iso_info[i] = &info[i];
    usbaudp->if_alt_set[i] = 0;
    usbaudp->vol_ctl[i].bmute = info[i].defVol.bmute;
    usbaudp->vol_ctl[i].wvol[0] = info[i].defVol.wvol[0];
    usbaudp->vol_ctl[i].wvol[1] = info[i].defVol.wvol[1];
    usbaudSetDefVolMute(usbaudp, i);
  }
  chMtxObjectInit(&g_mp_rec);
  chMtxObjectInit(&g_mp_ply);
  //chMBObjectInit(&g_ply_mbox, g_message[0], UA_MAILBOX_SIZE);
  chMBObjectInit(&g_cmd_mbox, g_message, UA_MAILBOX_SIZE);
  usbaudp->cmd_thread = chThdCreateFromHeap(NULL,
      1024,
      NORMALPRIO, //HIGHPRIO,
      UsbAudioCmdThread,
      usbaudp);

  usbaudp->state = USB_AUDIO_STATE_START;

}

void usbaudStop(void)
{
  USBAUD1.state = USB_AUDIO_STATE_STOP;

  if(USBAUD1.cmd_thread) {
    set_ua_cmd(&g_ua_cmd,UA_CMD_EXIT_THREAD, 0, 0, 0);
    chMBPost(&g_cmd_mbox, TRUE, TIME_INFINITE);
  
    osThreadTerminate(USBAUD1.cmd_thread);
    USBAUD1.cmd_thread = NULL;
  }

  if(g_IsPly)
    audioPlayStop();

  if(g_IsRec)
    audioRecordStop();

  g_IsPly = g_IsRec = FALSE;
  osBmemFree(USBAUD1.ply_buf);
  osBmemFree(USBAUD1.rec_buf);
}

static void _Ply_CallBack(hs_audio_event_t enEvent)
{
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    g_IsPlyByOthers = TRUE;
  }
  else if(enEvent == AUDIO_EVENT_RESUME)
  {
    audioPlayAcquire();
    g_IsPlyByOthers = FALSE;
    audioPlayStart(&ply_cfg, I2S_PLY_BUFFER_SIZE/8, _Ply_CallBack);
    audioSetPlaySource(AUDIO_PLAY_RAM);
    audioInvertI2sOutput(TRACK_LR);
    audioSetRecordSource(AUDIO_RECORD_LINEIN);    
    audioPlySetAutoRepair(AUDIO_AUTOREPAIR_ENABLE);
    
    if(USBAUD1.vol_ctl[IDX_PLY].bmute)
    {
      audioPlayMute();
    }
    else
    {
      audioPlayUnmute();
    }
    set_audio_volume_ply_db((USBAUD1.vol_ctl[IDX_PLY].wvol[0]&0xff00)>>8);
    audioPlayUnmute();
    audioPlayRelease();
  }
}

static void _Rec_CallBack(hs_audio_event_t enEvent)
{
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    g_IsRecByOthers = TRUE;
  }
  else if(enEvent == AUDIO_EVENT_RESUME)
  {
    g_IsRecByOthers = FALSE;    
    audioRecordStart(&rec_cfg, _Rec_CallBack);
    audioSetRecordSource(AUDIO_RECORD_LINEIN);
    audioSetPlaySource(AUDIO_PLAY_RAM);

    if(USBAUD1.vol_ctl[IDX_REC].bmute)
    {
      audioRecordMute();
    }
    else
    {
      audioRecordUnmute();
    }
    set_audio_volume_rec_db((USBAUD1.vol_ctl[IDX_REC].wvol[0]&0xff00)>>8);
  }
}

static THD_FUNCTION(UsbAudioCmdThread, arg)
{
  uint8_t cmd;
  msg_t msg;
  USBDriver *usbp;
  USBAudioDriver *usbaudp;

  chRegSetThreadName("UsbAudioCmdThread");

  while(1)
  {
    chMBFetch(&g_cmd_mbox, &msg, TIME_INFINITE);
    chSysLock();
    usbaudp = (USBAudioDriver *)arg;
    usbp = usbaudp->usbp;

    chDbgAssert(usbp&&usbaudp, "null pointer");

    cmd = g_ua_cmd.cmd;
    chSysUnlock();

    if(cmd == UA_CMD_START_PLY)
    {
      ply_cfg.sample_rate = get_sri2s_by_sr_drv(usbaudp->fmt_info[IDX_PLY].Sr); 

      audioPlayAcquire();
      if(0==audioPlayStart(&ply_cfg, I2S_PLY_BUFFER_SIZE/8, _Ply_CallBack))
      {
        g_IsPly = TRUE;
        g_IsPlyByOthers = FALSE;
        audioSetPlaySource(AUDIO_PLAY_RAM);
      }
      else
      {
        g_IsPly = 3;
      }
      audioInvertI2sOutput(TRACK_LR);
      audioPlySetAutoRepair(AUDIO_AUTOREPAIR_ENABLE);

      set_audio_volume_ply_db((usbaudp->vol_ctl[IDX_PLY].wvol[0]&0xff00)>>8);
      if(usbaudp->vol_ctl[IDX_PLY].bmute)
      {
        audioPlayMute();
      }
      else
      {
        audioPlayUnmute();
      }

      audioPlayUnmute();
      audioPlayRelease();

    }
    else if(cmd == UA_CMD_STOP_PLY)
    {
      if(!g_IsPlyByOthers)
      {
        audioPlayStop();
      }
      //chSysLock();
      g_IsPly = FALSE;
      usb_finish_ep0setup(usbp, 0);
      //chSysUnlock();
    }
    else if(cmd == UA_CMD_START_REC)
    {
      uint8_t epNum = usbaudp->iso_info[IDX_REC]->EpNum&0x7f;
      audio_fmt_info *pfmt = &usbaudp->fmt_info[IDX_REC];
      uint16_t pktSiz = (pfmt->Bits/8)*pfmt->ChNum*(get_srusb_by_sr_drv(pfmt->Sr)/1000);

      chDbgAssert(pfmt, "null pointer");

      rec_cfg.sample_rate = get_sri2s_by_sr_drv(usbaudp->fmt_info[IDX_REC].Sr);      
      if(0==audioRecordStart(&rec_cfg, _Rec_CallBack))
      {
        //chSysLock();
        g_IsRec = TRUE;
        g_IsRecByOthers = FALSE;
        audioSetRecordSource(AUDIO_RECORD_LINEIN);

        //audioSetPlaySource(AUDIO_PLAY_RAM);
        //chSysUnlock();
      }

      set_audio_volume_rec_db((usbaudp->vol_ctl[IDX_REC].wvol[0]&0xff00)>>8);
      if(usbaudp->vol_ctl[IDX_REC].bmute)
      {
        audioRecordMute();
      }
      else
      {
        audioRecordUnmute();
      }
      
      memset(&usbaudp->rec_buf[usbaudp->rec_buf_idx], 0, pktSiz);
      chSysLock();
      pfmt->Sr_Frac = get_srfrac_by_sr_drv(pfmt->Sr);
      if(pfmt->Sr_Frac)
      {
        usbaudp->rec_frac_cnt++;
      }
      usbPrepareTransmit(usbp, epNum, &usbaudp->rec_buf[usbaudp->rec_buf_idx], pktSiz, usbp->epc[epNum]->dma_info.dma_mode[IDX_REC]);
      usbStartTransmitI(usbp, epNum);
      usbaudp->rec_buf_idx++;
      usbaudp->rec_buf_idx %= REC_BUF_NUM;
      chSysUnlock();
    }
    else if(cmd == UA_CMD_STOP_REC)
    {
      if(!g_IsRecByOthers)
      {
        audioRecordStop();
      }
      chSysLock();
      g_IsRec = FALSE;
      usb_finish_ep0setup(usbp, 0);
      chSysUnlock();
    }
    else if(cmd == UA_CMD_SET_MUTE)
    {
      if(g_ua_cmd.para1 == UNITID_FEATURE_DAC_VOLUME)
      {

        if(g_ua_cmd.para2 == 1)
        {
          audioPlayMute();
        }
        else
        {
          audioPlayUnmute();
        }
      }
      else if(g_ua_cmd.para1 == UNITID_FEATURE_ADC_VOLUME)
      {
        if(g_ua_cmd.para2 == 1)
        {
          audioRecordMute();
        }
        else
        {
          audioRecordUnmute();
        }
      }
    }
    else if(cmd == UA_CMD_SET_VOL)
    {
      uint8_t id = 0;
      chSysLock();
      id = g_ua_cmd.para1&0x00ff;
      chSysUnlock();
      if(id == UNITID_FEATURE_DAC_VOLUME)
      {
        //audioPlaySetVolume((g_ua_cmd.para2&0xff00)>>8);
        set_audio_volume_ply_db((g_ua_cmd.para2&0xff00)>>8);

      }
      else if(id == UNITID_FEATURE_ADC_VOLUME)
      {
        //audioRecordSetVolume((g_ua_cmd.para2&0xff00)>>8);
        set_audio_volume_rec_db((g_ua_cmd.para2&0xff00)>>8);
      }

    }
    else if(cmd == UA_CMD_EXIT_THREAD)
    {
      break;
    }
    chSysLock();
    clear_ua_cmd(&g_ua_cmd);
    chSysUnlock();
  }
}

#endif

