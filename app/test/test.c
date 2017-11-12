/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
                            2014..2015 pingping.wu@huntersun.com

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
 * @file    test.c
 * @brief   Tests support code.
 *
 * @addtogroup test
 * @{
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib.h"
#include "test.h"
#include "context.h"

#ifdef TEST_ENABLE

void nds32_dcache_invalidate_range(unsigned long start, unsigned long end);

void _test_eventHappened(uint16_t u16Msg, void *parg)
{
  (void)parg;

  hs_printf("\r\nEvent-0x%X have happened!\r\n", u16Msg);
}

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = hs_memInfo(&size, __MT_GENERAL);
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);

  n = hs_memInfo(&size, __MT_DMA);
  chprintf(chp, "DMA memory fragments   : %u\r\n", n);
  chprintf(chp, "DMA memory free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 1) {
    chprintf(chp, "Usage: thds [cls]\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time(ms)         name\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %9s %09lu   %9s\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state],
            tp->p_time,
            tp->p_name);

    if((argc == 1) && (strcmp(argv[0], "cls") == 0))
      tp->p_time = 0;  
    
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_sendEvent(BaseSequentialStream *chp, int argc, char *argv[])
{
  hs_cfg_mod_t module;
  uint16_t msg;
  uint32_t val=0;

  if ((argc != 2) && (argc != 3) && (argc != 0)) {
    chprintf(chp, "Usage: event [module_id message(hex)]. \r\n\tif no arg, will clear the events which have sent but not execute.\r\n");
    return;
  }

  if(argc == 0)
  {
    hs_cfg_sysClearMsg();
    chprintf(chp, "Clear the events which have sent but not execute!\r\n");
    return ;
  }
  
  module  = strtoul(argv[0], NULL, 16);
  msg     = strtoul(argv[1], NULL, 16);

  if(argc == 3)
    val = strtoul(argv[2], NULL, 16);

  if(HS_CFG_OK != hs_cfg_sysSendMsg(module, HS_CFG_SYS_EVENT, msg, (void *)val))
    chprintf(chp, "Send message of event error!\r\n");

  chprintf(chp, "Send message sucessfully!\r\n");
}

static void cmd_listenEvent(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint16_t msg, i;

  if (argc == 0) {
    chprintf(chp, "Usage: listen message1(hex) message2(hex) ...\r\n");
    return;
  }

  for(i=0; i<argc; i++){
    msg  = strtoul(argv[i], NULL, 16);
    hs_cfg_sysListenMsg(msg,  _test_eventHappened);
  }

  chprintf(chp, "\r\n");
}

void cmd_chipStatus(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  float batteryVolt, temperature;
  struct tm tim;
  
  if ((argc > 1)
    ||((argc == 1) && (strcmp(argv[0], "?") == 0))){
    chprintf(chp, "Usage: chip [date-string]\r\n");
    return;
  }

  if(argc == 1)
    hs_rtc_setTim((char *)argv[0]);

  batteryVolt = hs_adc_getBatteryVolt();
  temperature = hs_adc_getTemperature();
  hs_rtc_getTim(&tim);

  chprintf(chp, "Chip Battery voltage: %f mv\r\n", batteryVolt);
  chprintf(chp, "Chip Temperature:     %f oC\r\n", temperature);
  chprintf(chp, "Current Date:         %s\r\n", asctime(&tim));
}

static void cmd_mem8read(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t *pu8Ptr;
  uint32_t address, cnt, i;

  if (argc != 2) {
    chprintf(chp, "Usage: mem8r address count\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16);
  cnt     = strtoul(argv[1], NULL, 16);
  pu8Ptr = (uint8_t *)address;

  if(address & 0x80000000)
    nds32_dcache_invalidate_range(address, address + cnt);

  for(i=0; i<cnt; i++)
  {
    if((i%16) == 0)
      chprintf(chp, "\r\n%08X:\t", address+i);

    chprintf(chp, "%02X ", pu8Ptr[i]);
  }

  chprintf(chp, "\r\n\r\n");
}

static void cmd_mem8write(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t *pu8Ptr, val;
  uint32_t address;

  if (argc != 2) {
    chprintf(chp, "Usage: mem8w address value\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16);
  val     = strtoul(argv[1], NULL, 16);
  pu8Ptr = (uint8_t *)address;

  *pu8Ptr = val;

  if(*pu8Ptr == val)
    chprintf(chp, "[0x%08X]=0x%02X write success!\r\n\r\n", address, val);
  else
    chprintf(chp, "[0x%08X]=0x%02X write fail!\r\n\r\n", address, *pu8Ptr);
}

static void cmd_mem16read(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint16_t *pu16Ptr;
  uint32_t address, cnt, i;

  if (argc != 2) {
    chprintf(chp, "Usage: mem16r address count\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16) & 0xfffffffe;
  cnt     = strtoul(argv[1], NULL, 16);
  pu16Ptr = (uint16_t *)address;

  if(address & 0x80000000)
    nds32_dcache_invalidate_range(address, address + cnt*2);

  for(i=0; i<cnt; i++)
  {
    if((i%8) == 0)
      chprintf(chp, "\r\n%08X:\t", address+i*2);

    chprintf(chp, "%04X ", pu16Ptr[i]);
  }

  chprintf(chp, "\r\n\r\n");
}

static void cmd_mem16write(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint16_t *pu16Ptr, val;
  uint32_t address;

  if (argc != 2) {
    chprintf(chp, "Usage: mem16w address value\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16) & 0xfffffffe;
  val     = strtoul(argv[1], NULL, 16);
  pu16Ptr = (uint16_t *)address;

  *pu16Ptr = val;

  if(*pu16Ptr == val)
    chprintf(chp, "[0x%08X]=0x%04X write success!\r\n\r\n", address, val);
  else
    chprintf(chp, "[0x%08X]=0x%04X write fail!\r\n\r\n", address, *pu16Ptr);
}

static void cmd_mem32read(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t *pu32Ptr;
  uint32_t address, cnt, i;

  if (argc != 2) {
    chprintf(chp, "Usage: mem32r address count\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16) & 0xfffffffc;
  cnt     = strtoul(argv[1], NULL, 16);
  pu32Ptr = (uint32_t *)address;

  if(address & 0x80000000)
    nds32_dcache_invalidate_range(address, address + cnt*4);

  for(i=0; i<cnt; i++)
  {
    if((i%4) == 0)
      chprintf(chp, "\r\n%08X:\t", address+i*4);

    chprintf(chp, "%08X ", pu32Ptr[i]);
  }

  chprintf(chp, "\r\n\r\n");
}

static void cmd_mem32write(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t *pu32Ptr, val;
  uint32_t address;

  if (argc != 2) {
    chprintf(chp, "Usage: mem32w address value\r\n");
    return;
  }
  
  address = strtoul(argv[0], NULL, 16) & 0xfffffffc;
  val     = strtoul(argv[1], NULL, 16);
  pu32Ptr = (uint32_t *)address;

  *pu32Ptr = val;
  if(*pu32Ptr == val)
    chprintf(chp, "[0x%08X]=0x%08X write success!\r\n\r\n", address, val);
  else
    chprintf(chp, "[0x%08X]=0x%08X write fail!\r\n\r\n", address, *pu32Ptr);
}

void cmd_anaSetReg(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t start, end, val, temp;

  if (argc != 3) {
    chprintf(chp, "Usage: anaset startBit endBit val\r\n");
    return;
  }
  
  start = atoll(argv[0]);
  end   = atoll(argv[1]);
  val   = strtoul(argv[2], NULL, 16);

  pmu_ana_set(start, end, val);
  temp = pmu_ana_get(start, end);

  chprintf(chp, "[%d, %d] set value 0x%08x ", start, end, val);  
  if(temp != val)
    chprintf(chp, "error 0x%08x !\r\n\r\n", temp);

  chprintf(chp, "ok !\r\n\r\n");
}

void cmd_anaGetReg(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t start, end, val;

  if (argc != 2) {
    chprintf(chp, "Usage: anaset startBit endBit\r\n");
    return;
  }
  
  start = atoll(argv[0]);
  end   = atoll(argv[1]);

  val = pmu_ana_get(start, end);

  chprintf(chp, "[%d, %d]'s value is 0x%08x !\r\n\r\n", start, end, val);  
}

void cmd_flashErase(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t offset, length;

  if (argc != 2) {
    chprintf(chp, "Usage: sferase offset length\r\n");
    return;
  }

  offset = strtol(argv[0], NULL, 16);
  length  = strtol(argv[1], NULL, 16);

  sfAcquireBus(&SFD);
  sfReleasePD(&SFD, TIME_INFINITE);
  sfErase(&SFD, offset, length, TIME_INFINITE);
  sfReleaseBus(&SFD);

  chprintf(chp, "flash [0x%08X, 0x%08X] erase over!\r\n", offset, (offset+length));
}

void hs_adc_getCalAdjust(uint8_t en);
static void cmd_performance(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t val0, val1, val2;
  float mingz, ins_time;

  (void)chp;
  (void)argv;

  if (argc == 0) {
    val0 = __nds32__mfsr(NDS32_SR_PFMC0);
    val1 = __nds32__mfsr(NDS32_SR_PFMC1);
    val2 = __nds32__mfsr(NDS32_SR_PFMC2);

    __nds32__mtsr(0, NDS32_SR_PFMC0);
    __nds32__mtsr(0, NDS32_SR_PFMC1);
    __nds32__mtsr(0, NDS32_SR_PFMC2);

    mingz = val0 - val2;
    mingz = (mingz / val0) * 100;

    ins_time = val1;
    ins_time = ins_time / val0;
    chprintf(chp, "cnt0 = 0x%08x, cnt1 = 0x%08x, cnt2 = 0x%08x, hit-ratio=%f, ins_time=%f\r\n", 
                   val0, val1, val2, mingz, ins_time);
  }

  if (argc == 1)
  {
    val0 = atoll(argv[0]);
    hs_adc_getCalAdjust(val0);
  }
}

void cmd_audioperformance(BaseSequentialStream *chp, int argc, char *argv[])
{
  hs_audio_stream_t *stmp;
  uint8_t dir = AUDIO_STREAM_PLAYBACK;
  char *str = "ply";

  if(argc == 1)
  {
    if(strcmp(argv[0], "ply") == 0)
    {
      dir = AUDIO_STREAM_PLAYBACK;
      str = argv[0];
    }
    else if(strcmp(argv[0], "rec") == 0)
    {
      dir = AUDIO_STREAM_RECORD;
      str = argv[0];
    }
    else if(strcmp(argv[0], "?") == 0)
    {
      chprintf(chp, "Usage: apfm [ply/rec/?]\r\n");
      return;
    }
  }

  stmp = audioGetStream(dir);
  chprintf(chp, "\r\n[%s]cpu get buffer count: %d"
                "\r\n[%s]cpu return buffer count: %d"
                "\r\n[%s]cpu return buffer losed count: %d"
                "\r\n[%s]cpu have been slowed count: %d"
                "\r\n[%s]i2s have been slowed count: %d "
                "\r\n[%s]hardware run error count: %d"
                "\r\n[%s]hardware interrupt count: %d\r\n",
                  str, stmp->performance.get_buffer_cnt,
                  str, stmp->performance.return_buffer_cnt,
                  str, stmp->performance.return_lose_cnt,
                  str, stmp->performance.cpu_slow_cnt,
                  str, stmp->performance.i2s_slow_cnt,
                  str, stmp->performance.hw_error_cnt,
                  str, stmp->performance.hw_int_cnt);
}

static uint8_t read_gpio_val(uint8_t gpio_num){
  uint8_t val = 0;
  
  if (gpio_num < PAL_IOPORTS_WIDTH) {
    palSetPadMode(IOPORT0, gpio_num, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	val = (palReadPort(IOPORT0) >> gpio_num)&0x1;
  }else if(gpio_num < 2*PAL_IOPORTS_WIDTH) {
    palSetPadMode(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	val = (palReadPort(IOPORT1) >> (gpio_num-PAL_IOPORTS_WIDTH))&0x1;	
  }
  
  return val;
}


void cmd_gpioTest(BaseSequentialStream *chp, int argc, char *argv[]) {
	
  uint8_t gpio_num, direction, val;

  if((argc != 2) && (argc != 3))
  {
    if(strcmp(argv[0], "?") == 0)
      chprintf(chp, "gio pin dir [val]\r\n\tdir 0-output 1-input\r\n");
  }          
  
  gpio_num  = atoi(argv[0]) & 0xff;
  direction = atoi(argv[1]) & 0xff;
  
  if (gpio_num >= 2*PAL_IOPORTS_WIDTH)
     goto ERROR;
  
  if (direction == 1) {
      val = read_gpio_val(gpio_num); 
      chprintf(chp, "read gpio%d is %s\r\n", gpio_num, val?"high":"low");      
      
  } else if (direction == 0) {
	  
	  val = atoi(argv[2]) & 0xff;	  
	  if (gpio_num < PAL_IOPORTS_WIDTH) {
		  palSetPadMode(IOPORT0, gpio_num, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
		  if (val)
			  palSetPad(IOPORT0, gpio_num);
		  else
			  palClearPad(IOPORT0, gpio_num);
	  } else if (gpio_num < 2*PAL_IOPORTS_WIDTH) {
		  palSetPadMode(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
		  if (val)
			  palSetPad(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH);
		  else
			  palClearPad(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH);
	  } else
		  goto ERROR;	  
  } else 
	  goto ERROR;	  
  chThdSleepMilliseconds(100);
  return;
   
ERROR:
  
  chprintf(chp, "Usage: gio [number] [direction] [level]\r\n"); 

  chprintf(chp, "Example: gio 10 0 1 -- set gpio 10 output = 1\r\n" 
                "Example: gio 20 1 [0,1,2,3]  -- set gpio 10 input\r\n");
}



static void cmd_anainfo(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint32_t rdata[9];
  int idx;

  (void)argc;
  (void)argv;

  const char *pd_names[] = {
	"pd_bt_ldodiv      ",
	"pd_bt_ldommd      ",
	"pd_bt_synth_cp    ",
	"pd_bt_synth_ldocp ",
	"pd_bt_synth_vc_det",
	"pd_bt_ldopfd      ",
	"pd_ldo_v1p3_lobuf ",
	"pd_txdac          ",
	"pd_bt_tx_lobuf    ",
	"pd_txum           ",
	"pd_txpa           ",
	"unknown           ",
	"pd_bt_ldovco      ",
	"pd_bt_vco_pkdect  ",
	"unknown           ",
	"unknown           ",
	"pd_dac_bias       ",
	"pd_bt_rx_lobuf    ",
	"pd_rxadc_biasgen  ",
	"pd_rxadc_dem      ",
	"pd_rxadc_i        ",
	"pd_rxadc_q        ",
	"pd_rxadc_ovdect   ",
	"pd_rxagc          ",
	"pd_rxlna          ",
	"pd_rxgm           ",
	"pd_rxtia          ",
	"pd_rxmixer        ",
	"pd_rxfil          ",
	"pd_sars_core      ",
  };

  const char *pd_names_2[] = {
	"pd_sari           ",
	"rst_sari          ",
	"en_agc_amp        ",
	"pkd_if_rst        ",
	"rxagc_sel         ",
	"pd_pkdif          ",
	"pd_pkdect         ", //pd_pkdrf
	"pd_biasgen_mea    ",
	"pd_sarbias        ",
	"rxagc_os_cali     ",
	"pd_ldo_mea        ",
	"pd_ldo_mea_1p3    ",
  };

  for (idx = 0; idx <= 8; idx++) {
    HS_ANA->DBG_IDX = idx;
    rdata[idx] = HS_ANA->DBG_RDATA;
  }

  for (idx = 0; idx <= 29; idx++)
    chprintf(chp, "%s %s\r\n", pd_names[idx], rdata[0] & (1 << idx) ? "off" : "on");
  chprintf(chp, "sars_agc_en        %s\r\n", rdata[0] & (1 << 30) ? "1" : "0");
  chprintf(chp, "rxadc_rst          %s\r\n", rdata[0] & (1 << 31) ? "1" : "0");

  chprintf(chp, "pd_rf_pll          %s\r\n", rdata[1] & (1 <<  0) ? "off" : "on");
  chprintf(chp, "pd_tx_part0        %s\r\n", rdata[1] & (1 <<  1) ? "off" : "on");
  chprintf(chp, "pd_tx_part1        %s\r\n", rdata[1] & (1 <<  2) ? "off" : "on");
  chprintf(chp, "pd_tx_part2        %s\r\n", rdata[1] & (1 <<  3) ? "off" : "on");
  chprintf(chp, "pd_rx_part0        %s\r\n", rdata[1] & (1 <<  4) ? "off" : "on");
  chprintf(chp, "pd_rx_part1        %s\r\n", rdata[1] & (1 <<  5) ? "off" : "on");
  chprintf(chp, "main_st=0x%08lx\r\n", rdata[1]);

  for (idx = 0; idx <= 11; idx++)
    chprintf(chp, "%s %s\r\n", pd_names_2[idx], rdata[2] & (1 << idx) ? "1" : "0");
  chprintf(chp, "rx_gain: 0x%04x\r\n", rdata[2] >> 16);
  rdata[2] = HS_ANA->RX_AGC_CFG[4];
  chprintf(chp, "rx_gain regs get       : RX_AGC_CFG4=0x%08lx, rxagc %s, rx_gain=0x%04x\r\n",
           rdata[2],
           rdata[2] & (1 << 16) ? "enabled" : "disabled",
           rdata[2] & 0xffff);

  chprintf(chp, "fm gain                : fil_gain.tia_gain.gm_gain.lna_gm.lna_ro=%x.%x?.%x.%x.%x\r\n",
           rdata[2] & 0x3, //rx_gain[1:0] is fil_gain
           (rdata[3] >> 12) & 0x0f, (rdata[3] >> 8) & 0x0f, (rdata[3] >> 4) & 0x0f, (rdata[3] >> 0) & 0x0f);

  chprintf(chp, "rfpll lut              : track.ftune.ctune=%x.%x.%x\r\n", (rdata[4] >> 8) & 0x7, (rdata[4] >> 4) & 0x7, (rdata[4] >> 0) & 0xf);

  chprintf(chp, "rfpll bt_synth_int_freq: 0x%08lx\r\n", rdata[5]);
  chprintf(chp, "rfpll bt_synth_freq_sdm: 0x%08lx=%d.MHz\r\n", rdata[6], rdata[6] >> 20);

  chprintf(chp, "ldo adj                : en?.lobuf.xtal.btvco=%x?.%x.%x.%x\r\n",
           (rdata[7] >> 8) & 0x7,
           (rdata[7] >> 4) & 0x3,
           (rdata[7] >> 2) & 0x3,
           (rdata[7] >> 0) & 0x3);

  chprintf(chp, "dcoc offset            : ana.q.i=%02x.%02x.%02x\r\n",
           (rdata[8] >> 16) & 0x1f, (rdata[8] >>  8) & 0x1f, (rdata[8] >>  0) & 0x1f);
}

#ifdef TEST_RT_ENABLE
static void cmd_rt_test(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: rt_test\r\n");
    return;
  }

  TestThread(chp);
}
#endif

void cmd_cpu_sleep(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)chp;
	if(argc==1)
	{
		int slp_time = atoi(argv[0]);
		cpu_enter_sleep(slp_time);
	}

	else
	{
		hs_printf("CPU enter sleep, Default time 500\r\n");
		cpu_enter_sleep(500);
	}

}




#include "cmd.c"

void hs_test_init(void)
{
  hs_shell_init(commands);
}

void hs_test_chkExit(void)
{
  hs_shell_uninit();
}


#endif



