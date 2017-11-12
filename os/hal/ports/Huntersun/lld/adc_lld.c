/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
    Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    templates/adc_lld.c
 * @brief   ADC Driver subsystem low level driver source template.
 *
 * @addtogroup ADC
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ADC1 driver identifier.
 */
#if PLATFORM_ADC_USE_ADC0 || defined(__DOXYGEN__)
ADCDriver ADCD0;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static void _adc_safety_timeout(void *p) {
  adc_channel_t chn = (adc_channel_t)p;

  chSysLockFromISR();
  if (ADCD0.thread[chn]) {
    thread_t *tp = ADCD0.thread[chn];
    ADCD0.thread[chn] = NULL;
    tp->p_u.rdymsg = MSG_TIMEOUT;
    chSchReadyI(tp);
  }
  chSysUnlockFromISR();
}

static void _adc_dma_timeout(void *p) {
  ADCDriver *adcp = (ADCDriver *)p;

  chSysLockFromISR();
  if (adcp->dmathrd != NULL) 
  {
    thread_t *tp = adcp->dmathrd;
    adcp->dmathrd = NULL;
    tp->p_u.rdymsg = MSG_TIMEOUT; 
    chSchReadyI(tp);  
  }
  chSysUnlockFromISR();
}

#define _adc_wakeup_dmareset(adcp) {                                   \
  if ((adcp)->dmathrd != NULL) {                                        \
    thread_t *tp = (adcp)->dmathrd;                                     \
    (adcp)->dmathrd = NULL;                                             \
    tp->p_u.rdymsg = MSG_RESET;                                                 \
    chSchReadyI(tp);                                                        \
  }                                                                         \
}

static void _adc_dma_complete(ADCDriver *adcp, hs_dma_cb_para_t *var)
{
  (void)var;
  if (adcp->dmathrd != NULL) 
  {
    thread_t *tp = adcp->dmathrd;
    adcp->dmathrd = NULL;
    tp->p_u.rdymsg = MSG_OK; 
    chSchReadyI(tp);  
  }
}

static void _adc_setChnPara(ADCDriver *adcp, adc_channel_t chn, const adc_chn_para_t *pPara)
{
  HS_ADC_Type *adc_r = adcp->adc;
  volatile uint32_t *reg = &adc_r->adc_ana_reg0 + chn;
  uint32_t u32Para = 0;
  
  __adc_set_en_r2r(u32Para, pPara->en_r2r);
  __adc_set_en_chop(u32Para, pPara->en_chop);
  __adc_set_gtune(u32Para, pPara->gtune);
  __adc_set_ldoctrl(u32Para, pPara->ldoctrl);
  __adc_set_fchop(u32Para, pPara->sel_fchop);
  __adc_set_sel_inp(u32Para, pPara->sel_inp);
  __adc_set_sel_vcm(u32Para, pPara->sel_vcm);
  __adc_set_en_count_sar(u32Para, pPara->en_count_sar);
  __adc_set_en_dem_sar(u32Para, pPara->en_dem_sar);
  __adc_set_set_sar_buf(u32Para, pPara->sar_buf);
  __adc_set_en_sar_ckdelay(u32Para, pPara->en_sar_ckdelay);

  *reg = u32Para;
}

static void _adc_setAttribute(ADCDriver *adcp, const adc_attr_t *pAttr)
{
  HS_ADC_Type *adc_r = adcp->adc;
  uint32_t attr = 0;
  
  if(pAttr == NULL)
	  return ;

  __adc_set_cont(attr, pAttr->cont);
  __adc_set_data_align(attr, pAttr->align);
  __adc_set_hwstart_en(attr, pAttr->hw_en);
  __adc_set_scan_dir(attr, pAttr->scandir);
  __adc_set_start_src(attr, pAttr->timer_sel);
  __adc_set_triger_edge(attr, pAttr->start_edge);
  __adc_set_discen(attr, pAttr->disen);
  __adc_set_sarq_bypass(attr, pAttr->sarq_bypass);
  __adc_set_swap_enable(attr, pAttr->swap_enable);
  __adc_set_test_mode(attr, pAttr->test_mode);

  adc_r->adc_cfg1 = attr;

  adc_r->adc_timing_cfg0 = 0x424166;
}

static void _adc_setTiming(ADCDriver *adcp, const adc_timing_t *pTiming)
{
  HS_ADC_Type *adc_r = adcp->adc;

  __adc_set_t1(adc_r, pTiming->t1);
  __adc_set_t2(adc_r, pTiming->t2);
  __adc_set_t3(adc_r, pTiming->t3);
  __adc_set_t4(adc_r, pTiming->t4);
  __adc_set_t5(adc_r, pTiming->t5);
  __adc_set_t6(adc_r, pTiming->t6);
  __adc_set_cic_cycle(adc_r, pTiming->cic_cycle);
  __adc_set_smp(adc_r, pTiming->smp_cycle);
  __adc_set_rst(adc_r, pTiming->rst_cycle);
}


#define _adc_wakeup_isr(adcp, msg, chn) {                                   \
  if ((adcp)->thread[chn] != NULL) {                                        \
    thread_t *tp = (adcp)->thread[chn];                                     \
    (adcp)->thread[chn] = NULL;                                             \
    tp->p_u.rdymsg = (msg);                                                 \
    chSchReadyI(tp);                                                        \
  }                                                                         \
}

static void _adc_serve_interrupt(ADCDriver *adcp)
{
  HS_ADC_Type *adc_r = adcp->adc;  
  uint16_t src, i;

  __adc_en_int(adc_r, 0);
  src = __adc_get_intr(adc_r);
  __adc_clr_intr(adc_r, src);

  if(src & ADC_ALL_DONE_MASK)
  {
    __adc_stop(adc_r, 1);
    cpm_delay_us(2);
    __adc_stop(adc_r, 0);

    if (adcp->dmathrd != NULL)
    {
      __adc_sw_start(adc_r, 1);
      cpm_delay_us(2);
      __adc_sw_start(adc_r, 0);
    }
  }

  for(i=0; i<ADC_CHANNEL_NUM; i++)
  {
    if (src & 1)
    {
      _adc_wakeup_isr(adcp, MSG_OK, i);      
    }
    
    src >>= 1;
  }

  __adc_en_int(adc_r, ADC_ALL_DONE_MASK);//chn_mask);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
#if PLATFORM_ADC_USE_ADC0 || defined(__DOXYGEN__)
CH_IRQ_HANDLER(ADC_IRQHandler) {

  CH_IRQ_PROLOGUE();
  osalSysLockFromISR();
  _adc_serve_interrupt(&ADCD0);
  osalSysUnlockFromISR();
  CH_IRQ_EPILOGUE();
  
}
#endif /* HS_I2C_USE_I2C0 */
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if PLATFORM_ADC_USE_ADC0
  /* Driver initialization.*/
  adcObjectInit(&ADCD0);

  ADCD0.adc = HS_ADC;
  ADCD0.adjust_ratio = 0.0;
#endif /* PLATFORM_ADC_USE_ADC1 */
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
int adc_lld_start(ADCDriver *adcp, const adc_attr_t *config) {

  if (adcp->state == ADC_STOP) {    
#if PLATFORM_ADC_USE_ADC0
    if (&ADCD0 == adcp) {
      cpmEnableADC0();   
      cpmResetADC0(); 

      adcp->channel_mask = 0;
      adcp->channel_num = 0;
      _adc_setAttribute(adcp, config);      
      nvicEnableVector(IRQ_ADC, ANDES_PRIORITY_MASK(HS_ADC_ADC0_IRQ_PRIORITY));

      cpmDisableADC0();
    }
#endif /* PLATFORM_ADC_USE_ADC0 */
  }

  return 0;
}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop(ADCDriver *adcp) {
  
  if (adcp->state == ADC_READY) {
    /* Resets the peripheral.*/
    
    /* Disables the peripheral.*/
#if PLATFORM_ADC_USE_ADC0
    if (&ADCD0 == adcp) {
      cpmEnableADC0();
      nvicDisableVector(IRQ_ADC);
      adc_lld_stop_conversion(adcp);
      cpmDisableADC0();
      
    }
#endif /* PLATFORM_ADC_USE_ADC0 */
  }
}

int adc_lld_set_timing(ADCDriver *adcp, const adc_timing_t *pTiming)
{
  if(pTiming == NULL)
    return -2;
  
  _adc_setTiming(adcp, pTiming);
  
  return 0;
}

int adc_lld_add_channel(ADCDriver *adcp, adc_channel_t chn, const adc_chn_para_t *pChnPara)
{
  if((adcp->channel_mask) & (1u << chn))
  {
    return -1;
  }
  
  if(adcp->channel_mask == 0)
  {
    cpmEnableADC0();
  }
  
  if(pChnPara != NULL)
  {
    _adc_setChnPara(adcp, chn, pChnPara);
  }

  adcp->channel_mask |= (1u << chn); 
  adcp->channel_num += 1;
  
  return 0;
}

int adc_lld_delete_channel(ADCDriver *adcp, adc_channel_t chn)
{
  if(adcp->channel_mask & (1u << chn))
  {
    adcp->channel_mask &= ~(1u << chn);
    adcp->channel_num -= 1;
  }

  if(adcp->channel_mask == 0)
  {
    cpmDisableADC0();
  }

  _adc_wakeup_isr(adcp, MSG_OK, chn);
  
  return 0;
}

uint32_t adc_lld_getChnData(ADCDriver *adcp, adc_channel_t chn, systime_t timeout)
{
  volatile uint32_t *u32Ptr;
  uint32_t i, data;
  virtual_timer_t vt;

  if((adcp->channel_mask & (1u << chn)) == 0)
    return 0;

  if (timeout != TIME_INFINITE)
  {
    chVTObjectInit(&vt);
    chVTSetI(&vt, timeout, _adc_safety_timeout, (void *)chn);
  }

  if((adcp->adc->adc_cfg1 & 4) == 0)
  {
    data = (adcp->adc->adc_cfg1 & (1u << 10)) == 0 ? 1 : adcp->channel_num;

    #if 1
    for(i=0; i<data; i++)
    {
      __adc_sw_start(adcp->adc, 1);
      cpm_delay_us(2);
      __adc_sw_start(adcp->adc, 0);
      cpm_delay_us(5);
    }
    #endif
  }
  
  adcp->thread[chn] = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
  {
    chVTResetI(&vt);
  }

  if(currp->p_u.rdymsg != MSG_OK)
    return 0;

  u32Ptr = &(adcp->adc->adc_data_reg0);
  u32Ptr += chn;
  data = *u32Ptr;

  return data;
}

/**
 * @brief   Starts an ADC conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
int adc_lld_start_conversion(ADCDriver *adcp) {
  HS_ADC_Type *adc_r = adcp->adc;
  uint16_t chn_mask = adcp->channel_mask;

  if(adcp->channel_mask == 0)
    return -2;

  __adc_set_chn_sel(adc_r, chn_mask);
  __adc_en_int(adc_r, ADC_ALL_DONE_MASK);//chn_mask);

  __adc_sw_start(adc_r, 1);
  __adc_sw_start(adc_r, 0);
  
  return 0;
}

int adc_lld_start_conversionWithDMA(ADCDriver *adcp, uint8_t *pDmaBuf, uint32_t DmaLen) {
  HS_ADC_Type *adc_r = adcp->adc;
  uint16_t chn_mask = adcp->channel_mask;
  hs_dma_config_t cfg;

  if(adcp->channel_mask == 0)
    return -2;

  if(adcp->pdma == NULL)
  {
    DmaLen = DmaLen - DmaLen % adcp->channel_num;
    adcp->pdma = dmaStreamAllocate(HS_ADC_DMA_PRIORITY, (hs_dmaisr_t)_adc_dma_complete, (void *)adcp);
    if (adcp->pdma == NULL) 
      return -2;
    cfg.slave_id = ADC_DMA_ID;
    cfg.direction = DMA_DEV_TO_MEM;
    cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
    cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
    cfg.src_burst = DMA_BURST_LEN_1UNITS;
    cfg.dst_burst = DMA_BURST_LEN_1UNITS;
    cfg.dev_flow_ctrl = FALSE;	
    cfg.lli_en = 0;
    dmaStreamSetMode(adcp->pdma, &cfg);
    
    __adc_set_fifo_en(adc_r, 1);
    __adc_set_fifo_rstn(adc_r, 0);
    __adc_set_fifo_rstn(adc_r, 1);
    __adc_set_dma_en(adc_r, 1);
    __adc_set_fifo_thres(adc_r, adcp->channel_num);

    dmaStreamStart(adcp->pdma, &adc_r->adc_rxfifo_data, pDmaBuf, DmaLen / 2);

  }
  
  __adc_set_chn_sel(adc_r, chn_mask); 
  __adc_en_int(adc_r, ADC_ALL_DONE_MASK);

  __adc_sw_start(adc_r, 1);
  cpm_delay_us(2);
  __adc_sw_start(adc_r, 0);

  return 0;
}

int adc_lld_dmaWaitForDone(ADCDriver *adcp, systime_t timeout) {
  HS_ADC_Type *adc_r = adcp->adc;
  virtual_timer_t vt;

  if(adcp->channel_mask == 0)
    return 0;
  
  nds32_dcache_flush();  
  if (timeout != TIME_INFINITE)
  {
    chVTObjectInit(&vt);
    chVTSetI(&vt, timeout, _adc_dma_timeout, (void *)adcp);
  }
  
  adcp->dmathrd = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
  {
    chVTResetI(&vt);
  }

  __adc_set_fifo_en(adc_r, 0);
  __adc_set_dma_en(adc_r, 0);
  if(adcp->pdma != NULL)
  {
    dmaStreamRelease(adcp->pdma);
    adcp->pdma = NULL;
  }

  return currp->p_u.rdymsg;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
int adc_lld_stop_conversion(ADCDriver *adcp) {
  HS_ADC_Type *adc_r = adcp->adc;
  uint32_t i;

  if(adcp->channel_mask == 0)
  {
    return -2;
  }

  __adc_stop(adc_r, 1);
  __adc_stop(adc_r, 0);

  __adc_en_int(adc_r, 0);
  __adc_set_chn_sel(adc_r, 0);

  for(i=0; i<ADC_CHANNEL_NUM; i++)
    _adc_wakeup_isr(adcp, MSG_RESET, i);

  _adc_wakeup_dmareset(adcp);
  adcp->channel_num = 0;
  return 0;
}

#endif /* HAL_USE_ADC */

/** @} */
