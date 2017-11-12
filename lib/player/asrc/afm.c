#include <lib.h>

#if HS_USE_FM && HS_USE_PLAYER

#define SIGNAL_SCAN  1
#define SIGNAL_ABORT 2
#define SIGNAL_NEXT  4
#define SIGNAL_PREV  8
#define SIGNAL_EXIT  0x100

static hs_fm_t *m_pstFm;

static uint32_t m_fm_chan_freq_idx;
static void _fm_emitChanInfo(int chan_idx, int freq)
{
  if (0 == freq)
    freq = m_pstFm ? m_pstFm->stPara.freq_min : 87500;
  if (-1 != chan_idx)
    hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, HS_CFG_EVENT_FM_CHAN_IND, (void *)chan_idx);
  hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, HS_CFG_EVENT_FM_FREQ_IND, (void *)freq);
  m_fm_chan_freq_idx = chan_idx == -1 ? 0xff : (chan_idx+1);
  m_fm_chan_freq_idx = (freq << 8u) | m_fm_chan_freq_idx;
  #if HS_USE_LEDDISP
  hs_led_disp(LED_DISP_FREQ);
  #endif
  //hs_printf("[%08u]fm %6d at P%02d: RSSI=%d SNR=%d\r\n", osKernelSysTick(), freq, chan_idx, fmGetRssi(&FMD0), fmGetSnr(&FMD0));
}

static void _fm_changeVol(hs_fm_t *pstFm)
{
  hs_ao_setVol(pstFm->pstAo, pstFm->stPara.volume);
}

static void _fm_changeFreq(hs_fm_t *pstFm)
{
  audioPlayMute();
  fmSetFrequency(&FMD0, pstFm->stPara.frequency);
  audioPlayUnmute();
  hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&pstFm->stPara.frequency, 4, 8);
  _fm_emitChanInfo(-1, pstFm->stPara.frequency);
}

static void _fm_sort_chan(hs_fm_t *pstFm)
{
  hs_fmpara_t *pstFmPara = &pstFm->stPara;
  int ii, jj;
  int new_chns, old_chns = pstFmPara->scan_chns_avail;
  int *tbl = pstFm->fm_tbl_freq;
  int *tbl_ctx = pstFm->fm_tbl_ctx;
  int freq_best = tbl[pstFm->chan_idx];
  int freq_vip = tbl[0];

  /* replace the redundant */
  for (ii = 0; ii < old_chns-1; ii++) {
    int freq = tbl[ii];
    for (jj = ii+1; jj < old_chns; jj++) {
      if (freq == tbl[jj])
        tbl[jj] = 0;
    }
  }

  new_chns = 0;
  for (ii = 0; ii < old_chns; ii++) {
    if (0 != tbl[ii])
      new_chns++;
  }

  /* strip the blank */
  for (ii = 0; ii < old_chns; ii++) {
    if (0 == tbl[ii]) {
      for (jj = ii+1; jj < old_chns; jj++) {
        if (0 != tbl[jj]) {
          tbl[ii] = tbl[jj];
          tbl_ctx[ii] = tbl_ctx[jj];
          tbl[jj] = 0;
          break;
        }
      }
    }
  }

  /* bubble short */
  for (ii = 0; ii < new_chns; ii++) {
    for (jj = 0; jj < new_chns-1; jj++) {
      if (tbl[jj] > tbl[jj+1]) {
        int temp = tbl[jj+1];
        tbl[jj+1] = tbl[jj];
        tbl[jj] = temp;
        temp = tbl_ctx[jj+1];
        tbl_ctx[jj+1] = tbl_ctx[jj];
        tbl_ctx[jj] = temp;
      }
    }
  }

  /* restore index to best & track vip */
  pstFm->chan_idx = 0;
  if (0xFF != pstFmPara->chan_vip)
    pstFmPara->chan_vip = 0;
  for (ii = 0; ii < new_chns; ii++) {
#if 1//1-1st 0-best
    (void)freq_best;
#else
    if (freq_best == tbl[ii])
      pstFm->chan_idx = ii;
#endif
    if ((freq_vip == tbl[ii]) && (0xFF != pstFmPara->chan_vip))
      pstFmPara->chan_vip = ii;
  }
  pstFmPara->scan_chns_avail = new_chns;
}

static int _fm_scan_alg(hs_fm_t *pstFm, bool next)
{
  hs_fmpara_t *fmp = &pstFm->stPara;
  int index, freq_end, step = fmp->freq_step;
  int snr[4] = {0}, rssi[4] = {0}, frequency[4] = {0}, fc_seeked = 0;
  int rssi_noise;
  osEvent evt;

  if (!next)
    step = -step;
  index = 0;
  /* return the current point if found nothing */
  fc_seeked = fmGetFrequency(&FMD0);
  /* start point */
  frequency[3] = fc_seeked;
  /* end point */
  freq_end = fc_seeked;

  while (!pstFm->abort_scan/*allow abort scan*/) {
    _fm_emitChanInfo(-1, frequency[3]);
    fmSetFrequency(&FMD0, frequency[3]);

    evt = osSignalWait(SIGNAL_ABORT, 60/*ms*/);
    if (evt.status == osEventSignal)
      break;

    rssi[3] = fmGetRssi(&FMD0);
    snr[3] = fmGetSnr(&FMD0);
    if (index<3)
      index++;

    if ((index == 3) &&
        (((0xFF != fmp->chan_vip) && (frequency[2] == pstFm->freq_vip)) || (
        (rssi[2] >= (rssi[0] + fmp->th.rssi_th)) &&
        (rssi[3] <= rssi[2]) &&
        (rssi[1] <= rssi[2]) &&
        (snr[2]  >= fmp->th.snr_th) &&
        /* unreliable at 96.0MHz due to interference by digital source */
        ((frequency[2] != 96000) && (frequency[2] != 95900))
                                                                            ))) {
      fc_seeked = frequency[2];

      /* fm stereo new alg by zhangzd on 2016.12.09 */
      rssi_noise = rssi[0];
      HS_BTPHY->FM_STEREO = (HS_BTPHY->FM_STEREO & 0xFFFF0000) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo)/*rssi_thl*/ << 8) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo+10)/*rssi_thh*/ << 0);

      if ((rssi[2] - rssi[0]) >= fmp->th.chhc_th) {
        HS_BTPHY->FM_CHHC_FILT &= ~(1 << 16); //fm_chhc_filt_en=0
      } else {
        HS_BTPHY->FM_CHHC_FILT |= (1 << 16); //fm_chhc_filt_en=1
      }
      if ((rssi[2] - rssi[0]) >= fmp->th.lrhc_th) {
        HS_BTPHY->FM_LRHC_FILT &= ~(1 << 16); //fm_lrhc_filt_en=0
      } else {
        HS_BTPHY->FM_LRHC_FILT |= (1 << 16); //fm_lrhc_filt_en=1
      }
      break;
    }
    else if ((index == 3) &&
             (rssi[1] >= (rssi[3] + fmp->th.rssi_th)) &&
             (rssi[0] <= rssi[1]) &&
             (rssi[3] <= rssi[2]) &&
             (snr[1]  >= fmp->th.snr_th) &&
             /* unreliable at 96.0MHz due to interference by digital source */
             ((frequency[1] != 96000) && (frequency[1] != 95900))) {
      fc_seeked = frequency[1];

      rssi_noise = rssi[3];
      HS_BTPHY->FM_STEREO = (HS_BTPHY->FM_STEREO & 0xFFFF0000) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo)/*rssi_thl*/ << 8) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo+10)/*rssi_thh*/ << 0);

      if ((rssi[1] - rssi[3]) >= fmp->th.chhc_th) {
        HS_BTPHY->FM_CHHC_FILT &= ~(1 << 16); //fm_chhc_filt_en=0
      } else {
        HS_BTPHY->FM_CHHC_FILT |= (1 << 16); //fm_chhc_filt_en=1
      }
      if ((rssi[1] - rssi[3]) >= fmp->th.lrhc_th) {
        HS_BTPHY->FM_LRHC_FILT &= ~(1 << 16); //fm_lrhc_filt_en=0
      } else {
        HS_BTPHY->FM_LRHC_FILT |= (1 << 16); //fm_lrhc_filt_en=1
      }
      break;
    }
    else{
      rssi[0] = rssi[1];
      rssi[1] = rssi[2];
      rssi[2] = rssi[3];
      snr[0] = snr[1];
      snr[1] = snr[2];
      snr[2] = snr[3];
      frequency[0] = frequency[1];
      frequency[1] = frequency[2];
      frequency[2] = frequency[3];
    }

    frequency[3] = frequency[2] + step;
    /* wrap back */
    if (frequency[3] > fmp->freq_max)
      frequency[3] = fmp->freq_min;
    else if (frequency[3] < fmp->freq_min)
      frequency[3] = fmp->freq_max;
    if (frequency[3] == freq_end)
      /* return the start point if found nothing */
      break;
  }

  fmSetFrequency(&FMD0, fc_seeked);

  return fc_seeked;
}

static void _fm_scan_full(hs_fm_t *pstFm)
{
  hs_fmpara_t *pstFmPara = &pstFm->stPara;
  int offset = sizeof(hs_fmpara_t);
  int freq_last, freq, chan_idx;
  int8_t snr, snr_max = -128;
  uint32_t hwctx;
  osEvent evt;

  chan_idx = 0;
#if 0
  /* track vip unless 0xFF */
  if (0xFF != pstFmPara->chan_vip) {
    pstFm->fm_tbl_freq[chan_idx] = pstFm->freq_vip;
    pstFm->fm_tbl_ctx[chan_idx] = 0;
    pstFm->chan_idx = chan_idx;
    _fm_emitChanInfo(chan_idx, pstFm->freq_vip);
    chan_idx++;
    pstFmPara->scan_chns_avail = 1;
  }
#endif

  fmSetFrequency(&FMD0, pstFmPara->freq_min);
  freq_last = pstFmPara->freq_min;
  while (!pstFm->abort_scan/*allow abort scan*/) {
    audioPlayMute();
    //freq = fmScanNext(&FMD0);
    freq = _fm_scan_alg(pstFm, true/*next*/);
    hwctx = fmGetContext(&FMD0);
    audioPlayUnmute();
    /* don't emit the frequency if wrap back */
    if (freq <= freq_last)
      break;
    freq_last = freq;
    /* hs_fm_stop() is called prio to emit SIGNAL_EXIT */
    if (pstFm->start_flag)
      _fm_emitChanInfo(chan_idx, freq);

    /* in case that SIGNAL_ABORT is received in _fm_scan_alg() */
    if (pstFm->abort_scan || !pstFm->start_flag) {
      evt.status = osEventSignal;
      hwctx = 0;
    } else {
      evt = osSignalWait(SIGNAL_ABORT,
                         pstFmPara->scan_found_delay * 1000);
      snr = fmGetSnr(&FMD0);
      if (snr_max < snr) {
        snr_max = snr;
        pstFm->chan_idx = chan_idx;
      }
    }
    pstFm->fm_tbl_freq[chan_idx] = freq;
    pstFm->fm_tbl_ctx[chan_idx] = hwctx;
    chan_idx++;
    if (chan_idx >= pstFmPara->scan_chns_max)
      break;
    if (pstFm->abort_scan)
      break;
    if (evt.status == osEventSignal)
      break;
  }
  pstFmPara->scan_chns_avail = chan_idx;

  _fm_sort_chan(pstFm);
  if (pstFmPara->scan_chns_avail > 0) {
    /* the best channel */
    chan_idx = pstFm->chan_idx;
    pstFm->stPara.frequency = pstFm->fm_tbl_freq[chan_idx];
    /* hs_fm_stop() is called prio to emit SIGNAL_EXIT */
    if (pstFm->start_flag) {
      fmSetFrequency(&FMD0, pstFm->stPara.frequency);
      fmSetContext(&FMD0, pstFm->fm_tbl_ctx[chan_idx]);
      _fm_emitChanInfo(chan_idx, pstFm->stPara.frequency);
    }

    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFmPara, sizeof(hs_fmpara_t), 0);
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_freq,
                          pstFmPara->scan_chns_avail * sizeof(int), offset);
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_ctx,
                          pstFmPara->scan_chns_avail * sizeof(int), offset + sizeof(int) * pstFmPara->scan_chns_max);
  }
  hs_printf("[%08u]FM table channels=%d; current idx=%d",osKernelSysTick(),  pstFmPara->scan_chns_avail, pstFm->chan_idx);
  for (chan_idx=0; chan_idx<pstFmPara->scan_chns_avail; chan_idx++) {
    if (chan_idx % 8 == 0)
      hs_printf("\r\n%d: ", chan_idx);
    hs_printf("%d:", pstFm->fm_tbl_freq[chan_idx]);
    hs_printf("%08lx ", pstFm->fm_tbl_ctx[chan_idx]);
  }
  hs_printf("\r\n");
  hs_cfg_flush(FLUSH_TYPE_ALL);

  pstFm->scan_flag = 0;
  pstFm->abort_scan = 0;
  /* stop prev/next one by one after fm scan */
  pstFm->scan_1by1 = 0;
}

static void _fm_thread(void *arg)
{
  hs_fm_t *pstFm = (hs_fm_t *)arg;
  hs_fmpara_t *pstFmPara = &pstFm->stPara;

  chRegSetThreadName("fmService");

  do {
    osEvent evt = osSignalWait(0/*ALL_EVENTS*/, osWaitForever);
    if (evt.status != osEventSignal)
      continue;
    if (evt.value.signals & SIGNAL_EXIT)
      break;

    if (evt.value.signals & SIGNAL_SCAN) {
      hs_cfg_sysSendMessage(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, HS_CFG_EVENT_FM_SEARCH_BEGIN);
      _fm_scan_full(pstFm);
      hs_cfg_sysSendMessage(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, HS_CFG_EVENT_FM_SEARCH_END);
      continue;
    }

    if (evt.value.signals & (SIGNAL_NEXT | SIGNAL_PREV)) {
      int offset = sizeof(hs_fmpara_t);
      int freq_last, freq, chan_idx = pstFm->chan_idx;
      uint32_t hwctx;
      bool next = evt.value.signals & SIGNAL_NEXT ? true : false;

      freq_last = fmGetFrequency(&FMD0);
      if (!pstFm->paused)
        audioPlayMute();
      //pstFmPara->frequency = fmScanNext(&FMD0);
      freq = _fm_scan_alg(pstFm, next);
      hwctx = fmGetContext(&FMD0);
      audioPlayUnmute();
      pstFm->scan_flag = 0;
      pstFm->abort_scan = 0;
      pstFm->scan_1by1 = 0;

      if (freq != freq_last) {
        /* xcx requirement:
         * current is P03, chan=99.9MHz;
         * if semi-scan to 102.0MHz, then replace P04 with chan=102.0
         */
        if (next) {
          chan_idx++;
          if (chan_idx >= pstFmPara->scan_chns_max)
            chan_idx = 0;
          if (0 == pstFm->fm_tbl_freq[chan_idx])
            pstFmPara->scan_chns_avail++;
        }
        else {
          if (chan_idx > 0)
            chan_idx--;
          else if (pstFmPara->scan_chns_avail > 0)
            chan_idx = pstFmPara->scan_chns_avail-1;
        }
        _fm_emitChanInfo(chan_idx, freq);
        hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&freq, 4, 8);

        pstFm->fm_tbl_freq[chan_idx] = freq;
        pstFm->fm_tbl_ctx[chan_idx] = hwctx;
        pstFm->chan_idx = chan_idx;

        hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFmPara, sizeof(hs_fmpara_t), 0);
        /* also store the updated fm table if semi-scan */
        hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_freq,
                              pstFmPara->scan_chns_avail * sizeof(int), offset);
        hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_ctx,
                              pstFmPara->scan_chns_avail * sizeof(int), offset + sizeof(int) * pstFmPara->scan_chns_max);
      }
    }
  } while (1);
}

hs_fm_t *_fm_create(void)
{
  hs_fm_t *pstFm;
  hs_fmpara_t *pstFmPara;
  FMConfig *pstFMDCfg;
  hs_cfg_res_t ret;

  pstFm = (hs_fm_t *)hs_malloc(sizeof(hs_fm_t), __MT_Z_GENERAL);
  if(!pstFm)
    return NULL;
  m_pstFm = pstFm;

  pstFmPara = &pstFm->stPara;
  ret = hs_cfg_getDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFmPara, sizeof(hs_fmpara_t));
  if ((HS_CFG_OK != ret) || (0 == pstFmPara->th.rssi_th_stereo) || (0 == pstFmPara->th.lrhc_th)) { /* back-compitable with old cfg data */
    pstFmPara->volume              = 0;
    pstFmPara->volume_step         = 6;

    pstFmPara->th.chhc_th          = 15;
    pstFmPara->th.lrhc_th          = 12;
    pstFmPara->th.rssi_th          = 8;
    pstFmPara->th.rssi_th_stereo   = 22;
    pstFmPara->th.snr_th           = 4;
    pstFmPara->th.snr_th_stereo    = -128;
    
    pstFmPara->frequency           = 0x195DC; //103.9MHz
    pstFmPara->freq_step           = 0x64;    //100KHz
    pstFmPara->freq_max            = 0x1a5e0; //108MHz
    pstFmPara->freq_min            = 0x155cc; //87.5MHz
    pstFmPara->chan_vip            = 0;
    pstFmPara->scan_found_delay    = 1; /* 1 second */
    pstFmPara->scan_chns_max       = 25;
  }
  pstFm->freq_vip = pstFmPara->frequency;

  pstFMDCfg = &pstFm->fmconfig;
  pstFMDCfg->freq_max = pstFmPara->freq_max;
  pstFMDCfg->freq_min = pstFmPara->freq_min;
  pstFMDCfg->step     = pstFmPara->freq_step;
  pstFMDCfg->th       = pstFmPara->th;
  hs_printf("[%08u]fm vip=%d, delay=%d; threshold rssi=%d snr=%d rssi_stereo=%d snr_stereo=%d chhc=%d lrhc=%d\r\n",
            osKernelSysTick(),
            pstFmPara->chan_vip, pstFmPara->scan_found_delay, pstFMDCfg->th.rssi_th, pstFMDCfg->th.snr_th, pstFMDCfg->th.rssi_th_stereo, pstFMDCfg->th.snr_th_stereo, pstFMDCfg->th.chhc_th, pstFMDCfg->th.lrhc_th, pstFmPara->scan_found_delay);

  /* available channels table after scan, max 50 channels */
  do {
    int offset = sizeof(hs_fmpara_t);
    int tbl_size;
    if (pstFmPara->scan_chns_max == 0)
      break;

    tbl_size = sizeof(int) * pstFmPara->scan_chns_max;
    pstFm->fm_tbl_freq = (int *)hs_malloc(tbl_size * 2, __MT_Z_GENERAL);
    if (pstFm->fm_tbl_freq == NULL)
      break;
    pstFm->fm_tbl_ctx = pstFm->fm_tbl_freq + pstFmPara->scan_chns_max;

    ret = hs_cfg_getPartDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_freq,
                                    tbl_size * 2, offset);
    if (HS_CFG_OK != ret) {
      pstFmPara->scan_chns_avail = 0;
    }
    /* restore index to FM table */
    pstFm->chan_idx = 0;
    for (offset=0; offset<pstFmPara->scan_chns_avail; offset++) {
      if (pstFm->fm_tbl_freq[offset] == pstFmPara->frequency)
        pstFm->chan_idx = offset;
    }
    /* track vip unless 0xFF */
    if ((pstFmPara->scan_chns_avail > 0) && (0xFF != pstFmPara->chan_vip))
      pstFm->freq_vip = pstFm->fm_tbl_freq[pstFmPara->chan_vip];
  } while(0);
  /* workaround for vip channel only */
  if ((0xFF != pstFmPara->chan_vip) && (pstFmPara->scan_chns_avail == 0))
    pstFmPara->scan_chns_avail = 1;

  pstFm->scan_1by1 = 0;

  return pstFm;
}

void hs_fm_stop(hs_fm_t *pstFm)
{
  if(!pstFm || !pstFm->start_flag)
    return ;

  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
  }

  pstFm->start_flag = 0;
  pstFm->paused = 0;
  //pstFm->scan_flag = 0;
  //pstFm->abort_scan = 0;
  
  fmStop(&FMD0);
  hs_ao_stop(pstFm->pstAo);  
}

void hs_fm_start(hs_fm_t *pstFm)
{
  int chan_idx;

  if(!pstFm)
    return ;

  if(pstFm->start_flag) 
  {
    /* press Play/Pause, Next/Prev can abort scan */
    if (pstFm->scan_flag) {
      pstFm->abort_scan = 1;
      osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
      return;
    }

    /* current config.ini uses D6=HS_CFG_EVENT_PLAYER_START to impl play/pause */
    //hs_fm_stop(pstFm);
    if (pstFm->paused)
      audioPlayUnmute();
    else
      audioPlayMute();
    pstFm->paused ^= 1;
    return;
  }

  chan_idx = pstFm->chan_idx;
  fmStart(&FMD0, &pstFm->fmconfig);
  audioPlayMute();
  fmSetFrequency(&FMD0, pstFm->stPara.frequency);
  if (pstFm->fm_tbl_ctx)
    fmSetContext(&FMD0, pstFm->fm_tbl_ctx[chan_idx]);
  audioPlayUnmute();
  _fm_emitChanInfo(chan_idx, pstFm->stPara.frequency);
  
  hs_ao_start(pstFm->pstAo);
  
  pstFm->start_flag = 1;
  pstFm->scan_flag = 0;
  pstFm->paused = 0;
  pstFm->abort_scan = 0;
}

/* semi-automatic scan function */
void hs_fm_scan_semi(hs_fm_t *pstFm, bool isNext)
{
  hs_fmpara_t *pstFmPara;

  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFmPara = &pstFm->stPara;
  /* start point is min or max if the 1st scan one by one */
  if ((pstFm->scan_1by1 == 0) && (pstFmPara->scan_chns_avail == 0)) {
    if (isNext)
      fmSetFrequency(&FMD0, pstFmPara->freq_min);
    else
      fmSetFrequency(&FMD0, pstFmPara->freq_max);
  }
  pstFm->scan_1by1 = 1;
  pstFm->scan_flag = 1;
  if (isNext)
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_NEXT);
  else
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_PREV);

  pstFm->paused = 0;
}

void hs_fm_next(hs_fm_t *pstFm)
{
  uint8_t avail = 0;
  hs_fmpara_t *pstFmPara;

  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFmPara = &pstFm->stPara;
#if 0
  if (0xFF != pstFmPara->chan_vip)
    avail = 1;
#endif
  if (!pstFm->scan_1by1 && (pstFmPara->scan_chns_avail > avail) && (NULL != pstFm->fm_tbl_freq)) {
    int chan_idx;

    pstFm->chan_idx++;
    if (pstFm->chan_idx >= pstFmPara->scan_chns_avail)
      pstFm->chan_idx = 0;
    chan_idx = pstFm->chan_idx;
    pstFmPara->frequency = pstFm->fm_tbl_freq[chan_idx];
    if (!pstFm->paused)
      audioPlayMute();
    fmSetFrequency(&FMD0, pstFmPara->frequency);
    if (pstFm->fm_tbl_ctx)
      fmSetContext(&FMD0, pstFm->fm_tbl_ctx[chan_idx]);
    audioPlayUnmute();
    _fm_emitChanInfo(chan_idx, pstFmPara->frequency);
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&pstFmPara->frequency, 4, 8);
  }
  else {
    hs_fm_scan_semi(pstFm, true/*isNext*/);
  }

  pstFm->paused = 0;
}

void hs_fm_prev(hs_fm_t *pstFm)
{
  uint8_t avail = 0;
  hs_fmpara_t *pstFmPara;

  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFmPara = &pstFm->stPara;
#if 0
  if (0xFF != pstFmPara->chan_vip)
    avail = 1;
#endif
  if ((pstFmPara->scan_chns_avail > avail) && (NULL != pstFm->fm_tbl_freq)) {
    int chan_idx;

    if (pstFm->chan_idx > 0)
      pstFm->chan_idx--;
    else if (pstFm->chan_idx == 0)
      pstFm->chan_idx = pstFmPara->scan_chns_avail - 1;
    chan_idx = pstFm->chan_idx;
    pstFmPara->frequency = pstFm->fm_tbl_freq[chan_idx]; 
    if (!pstFm->paused)
      audioPlayMute();
    fmSetFrequency(&FMD0, pstFmPara->frequency);
    if (pstFm->fm_tbl_ctx)
      fmSetContext(&FMD0, pstFm->fm_tbl_ctx[chan_idx]);
    audioPlayUnmute();
    _fm_emitChanInfo(chan_idx, pstFmPara->frequency);
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&pstFmPara->frequency, 4, 8);
  }
  else {
    hs_fm_scan_semi(pstFm, false/*isNext*/);
  }

  pstFm->paused = 0;
}

void hs_fm_volumeInc(hs_fm_t *pstFm)
{
  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFm->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) + pstFm->stPara.volume_step;
  _fm_changeVol(pstFm);

  pstFm->paused = 0;
}

void hs_fm_volumeDec(hs_fm_t *pstFm)
{
  if(!pstFm || !pstFm->start_flag)
    return ;
  
  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFm->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) - pstFm->stPara.volume_step;
  _fm_changeVol(pstFm);
  pstFm->paused = 0;
}

void hs_fm_freqInc(hs_fm_t *pstFm)
{
  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFm->stPara.frequency += FMD0.step;
  if(pstFm->stPara.frequency > FMD0.freq_max)
    pstFm->stPara.frequency = FMD0.freq_min;

  _fm_changeFreq(pstFm);
}

void hs_fm_freqDec(hs_fm_t *pstFm)
{
  if(!pstFm || !pstFm->start_flag)
    return ;

  /* press Play/Pause, Next/Prev can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  pstFm->stPara.frequency -= FMD0.step;
  if(pstFm->stPara.frequency < FMD0.freq_min)
    pstFm->stPara.frequency = FMD0.freq_max;

  _fm_changeFreq(pstFm);
}

void hs_fm_funcSet(hs_fm_t *pstFm, uint32_t u32Func)
{
  hs_fmpara_t *pstFmPara;

  if((!pstFm) || (!pstFm->start_flag) || (u32Func >= FM_FUNC_NUM))
    return ;

  pstFmPara = &pstFm->stPara;

  if (u32Func == FM_FUNC_INVALIDATE) {
    int offset = sizeof(hs_fmpara_t);
    int tbl_size;

    if (pstFm->scan_flag)
      return;

    pstFmPara->scan_chns_avail = 0;
#if 0
    /* track vip unless 0xFF */
    if (0xFF != pstFmPara->chan_vip) {
      pstFmPara->scan_chns_avail = 1;
      pstFmPara->chan_vip = 0;
      if (pstFm->fm_tbl_freq)
        pstFm->fm_tbl_freq[0] = pstFm->freq_vip;
    }
#endif
    tbl_size = sizeof(int) * pstFmPara->scan_chns_max;
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFmPara, sizeof(hs_fmpara_t), 0);
    if (pstFm->fm_tbl_freq) {
      hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_freq, tbl_size, offset);
      hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)pstFm->fm_tbl_ctx,  tbl_size, offset+tbl_size);
    }
    hs_cfg_flush(FLUSH_TYPE_ALL);
    return;
  }

  /* it is obsoleted, because any other press can abort scan */
  if (u32Func == FM_FUNC_ABORT_SEARCH) {
    pstFm->abort_scan = 0;
    return;
  }

  if (u32Func == FM_FUNC_SCAN_SEMI_NEXT) {
    hs_fm_scan_semi(pstFm, true);
    return;
  }

  if (u32Func == FM_FUNC_SCAN_SEMI_PREV) {
    hs_fm_scan_semi(pstFm, false);
    return;
  }

  /*
   * FM_FUNC_SEARCH
   * search all FM channels to find the available FM table;
   * store the FM table in cfg data.
   */
  if ((pstFmPara->scan_chns_max > 0) && (NULL != pstFm->fm_tbl_freq)) {
    if (pstFm->scan_flag)
      return;
    pstFm->scan_flag = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_SCAN);
  }
}

/*
 * @brief: LED engine obtains FM channel information to display.
 *
 * @return FM channel's [31:8]frequency in khz + [7:0]index
 */
__USED uint32_t hs_fm_getFreq(void)
{
  return m_fm_chan_freq_idx;
}

/*
 * @brief: obtain FM receiver internal status
 *
 * @return true-scanning  false-no scanning
 */
__USED bool hs_fm_isScan(void)
{
  if (m_pstFm)
    return FALSE;
  return m_pstFm->scan_flag ? TRUE : FALSE;
}

/*
 * @brief: obtain FM receiver internal status
 *
 * @return {FM_STATUS_SCANNING | PLAYING | PAUSE | TERMINATE}
 */
__USED hs_fmstatus_t hs_fm_getStatus(void)
{
  hs_fm_t *pstFm = m_pstFm;

  if ((!pstFm) || (!pstFm->start_flag))
    return FM_STATUS_TERMINATE;
  if (pstFm->scan_flag)
    return FM_STATUS_SCANNING;
  if (pstFm->paused)
    return FM_STATUS_PAUSE;
  else
    return FM_STATUS_PLAYING;
}

__USED hs_fm_t *hs_fm_getHandle(void)
{
  return m_pstFm;
}

/*
 * @brief: set the specfic FM channel
 * @param[in] u32Idx  the index to FM channel table
 *
 * @return none
 */
__USED void hs_fm_setIdx(uint32_t u32Idx)
{
  hs_fm_t *pstFm = m_pstFm;
  hs_fmpara_t *pstFmPara = &pstFm->stPara;
  
  if ((!pstFm) || (!pstFm->start_flag))
    return;

  /* UI's input can abort scan */
  if (pstFm->scan_flag) {
    pstFm->abort_scan = 1;
    osSignalSet(pstFm->FmScanThreadId, SIGNAL_ABORT);
    return;
  }

  if ((u32Idx > pstFmPara->scan_chns_avail) || (u32Idx == 0))
    return;

  u32Idx -= 1;

  pstFmPara->frequency = pstFm->fm_tbl_freq[u32Idx];
  if (!pstFm->paused)
    audioPlayMute();
  fmSetFrequency(&FMD0, pstFmPara->frequency);
  if (pstFm->fm_tbl_ctx)
    fmSetContext(&FMD0, pstFm->fm_tbl_ctx[u32Idx]);
  audioPlayUnmute();
  _fm_emitChanInfo(u32Idx, pstFmPara->frequency);
  hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&pstFmPara->frequency, 4, 8);
}

hs_fm_t *hs_fm_create(hs_ao_t *pstAo)
{
  hs_fm_t *pstFm;
  osThreadDef_t thdDef;

  pstFm = _fm_create();
  if(!pstFm)
    return NULL;

  pstFm->pstAo = pstAo;

  hs_pmu_enSleep(0);
  hs_pmu_flushDisable(1);

  thdDef.pthread   = (os_pthread)_fm_thread;
  thdDef.stacksize = 256;
  thdDef.tpriority = osPriorityNormal;
  pstFm->FmScanThreadId = osThreadCreate(&thdDef, pstFm);

  return pstFm;
}

void hs_fm_destroy(hs_fm_t *pstFm)
{
  /* led_engine will call hs_fm_getFreq() actively to display, so clear it */
  m_fm_chan_freq_idx = 0;

  if(pstFm)
  {
    m_pstFm = NULL;
    hs_fm_stop(pstFm);
    if (pstFm->FmScanThreadId) {
      osSignalSet(pstFm->FmScanThreadId, SIGNAL_EXIT);
      osThreadTerminate(pstFm->FmScanThreadId);
      pstFm->FmScanThreadId = NULL;
    }
    if (pstFm->fm_tbl_freq)
      hs_free(pstFm->fm_tbl_freq);
    hs_free(pstFm);
  }
  
  hs_cfg_flush(FLUSH_TYPE_ALL);
  hs_pmu_enSleep(1);
  hs_pmu_flushDisable(0);
}

#endif

