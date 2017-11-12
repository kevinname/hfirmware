/*
    ChibiOS/RT - Copyright (C) 2014 HunterSun Technologies
                 hongwei.li@huntersun.com.cn

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

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lib.h"
#include "string.h"

/*************************************************
  spi test:
*************************************************/
#define  CHIP_ID  "hs66010D"

#if HS_SPI_USE_SPI0
extern SPIDriver SPID0;
#endif

#if HS_SPI_USE_SPI1
extern SPIDriver SPID1;
#endif

unsigned char *master_txbuf, *master_rxbuf, *slave_data_buf;
spislavecallback_par_t pars;

void slave_cb(SPIDriver *spip, spislavecallback_par_t *par) {
	(void)spip;

    if (par->dev_status == PRO_CMD_STATUS) {
      if (par->cmd == SPI_CMD_READ_CHIP_ID)
        memcpy(par->data, CHIP_ID, sizeof(CHIP_ID));      
    }

    if (par->dev_status == PRO_WRITE_CMP_STATUS) {
      if (par->cmd == SPI_CMD_LOOP_TEST)
        memcpy(slave_data_buf, par->data, SPI_SLAVE_PROTOCOL_DATA_NUM);
      par->dev_status = PRO_INT_STATUS;
    }

    if (par->dev_status == PRO_CMD_STATUS) {
      if (par->cmd == SPI_CMD_LOOP_TEST)
        memcpy(par->data, slave_data_buf, SPI_SLAVE_PROTOCOL_DATA_NUM);
    }

    if (par->dev_status == PRO_READ_CMP_STATUS)
      par->dev_status = PRO_INT_STATUS;
}

SPIConfig slave_config = {
    NULL,
    slave_cb,
    &pars,
    3000000,
    0,
    -1,
    8,
    SPI_SLAVE_MODE,
};

SPIConfig master_config = {
    NULL,
    NULL,
    NULL,
    3000000,
    0,
    13,
    8,
    SPI_MASTER_MODE,
 };

static void spi0_padmux_set(void) {
   palSetPadMode(IOPORT0, 5, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI0_MOSI)|PAL_MODE_DRIVE_CAP(3));
   palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI0_MISO)|PAL_MODE_DRIVE_CAP(3));
   palSetPadMode(IOPORT0, 7, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI0_SCK)|PAL_MODE_DRIVE_CAP(3));
 //  palSetPadMode(IOPORT0, 8, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI0_CSN)|PAL_MODE_DRIVE_CAP(3));
  /* cs use gpio15 */
   palSetPadMode(IOPORT0, 13, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
}

static void spi1_padmux_set(void) {
   palSetPadMode(IOPORT1, 1, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI1_MOSI)|PAL_MODE_DRIVE_CAP(3));
   palSetPadMode(IOPORT1, 2, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI1_MISO)|PAL_MODE_DRIVE_CAP(3));
   palSetPadMode(IOPORT1, 3, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI1_SCK)|PAL_MODE_DRIVE_CAP(3));
   palSetPadMode(IOPORT1, 4, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SPI1_CSN)|PAL_MODE_DRIVE_CAP(3));        
}

static char* spi_master_read_chipID(SPIDriver *spi) {
    spiStart(spi, &master_config);
    master_txbuf[0] = SPI_SLAVE_READ_DIR;
    master_txbuf[1] = SPI_CMD_READ_CHIP_ID; 

    spiSelect(spi);
    spiExchange(spi, SPI_SLAVE_PROTOCOL_CMD_NUM, master_txbuf, master_rxbuf);
    spiUnselect(spi);

    msleep(1000);
    memset(master_rxbuf, 0x00, 256);
    
    spiSelect(spi);
    spiExchange(spi, SPI_SLAVE_PROTOCOL_DATA_NUM, master_txbuf, master_rxbuf);
    spiUnselect(spi);

    master_rxbuf[SPI_SLAVE_PROTOCOL_DATA_NUM] = 0;
    spiStop(spi);
    return (char*)master_rxbuf;
}

static void spidev_slave_start(SPIDriver *spi, spislavecallback_par_t *pars) {
        
    slave_data_buf = hs_malloc(SPI_SLAVE_PROTOCOL_DATA_NUM, __MT_DMA);
            
    /* master send and master and write read command */
    pars->data = hs_malloc(SPI_SLAVE_PROTOCOL_DATA_NUM, __MT_DMA);
    pars->dev_status = PRO_INT_STATUS;
    pars->cmd_num = 0;
    pars->data_num = 0;
    
   spiStart(spi, &slave_config);               
}

static void spidev_slave_stop(SPIDriver *spi) {
  hs_free(slave_data_buf);
  hs_free(spi->config->paras->data);
  spiStop(spi);        
}

void cmd_spi(BaseSequentialStream *chp, int argc, char *argv[]) {
  int cs = 0, i;
  SPIDriver *master_spi;
  SPIDriver *slave_spi;
  
  master_txbuf = hs_malloc(256, __MT_DMA);
  master_rxbuf = hs_malloc(256, __MT_DMA);
  
  for(i=0; i<256; i++)
    master_txbuf[i] = i;
  
  memset(master_rxbuf, 0x00, 256);
  
  if(argc == 1) {
    cs = strtol(argv[0], NULL, 10);   
    if(cs == 0) {
      master_spi = &SPID0;       
      spi0_padmux_set();      
    } else {
      master_spi = &SPID1;       
      spi1_padmux_set();
    }

    spiStart(master_spi, &master_config);
    spiExchange(master_spi, 256, master_txbuf, master_rxbuf);

    if(memcmp(master_txbuf, master_rxbuf, 256) != 0)
       chprintf(chp, "spi xfer failed\r\n");
    else
       chprintf(chp, "spi xfer sucess\r\n");
    
    spiStop(master_spi);

  }else if (argc == 2) {      
    if(!strtol(argv[0], NULL, 10)) {
      master_spi = &SPID0;
      slave_spi = &SPID1;             
    }else {
      master_spi = &SPID1;
      slave_spi = &SPID0;                   
    }

    HS_GPIO_Type *pGpio;
    uint16_t tmp;

     if(master_config.cs_gpio_index < 16){
       tmp = master_config.cs_gpio_index;
       pGpio = IOPORT0;
     }
     else{
       tmp = master_config.cs_gpio_index - 16;
       pGpio = IOPORT1;
     }

    spi0_padmux_set();
    spi1_padmux_set();

    palSetPad(pGpio, tmp);

    spidev_slave_start(slave_spi, &pars);
    char *s = spi_master_read_chipID(master_spi);
    chprintf(chp, "chipID: %s\r\n", s);
    spidev_slave_stop(slave_spi);

  }

  if(master_txbuf != NULL){
    hs_free(master_txbuf);
  }

  if(master_rxbuf != NULL){
    hs_free(master_rxbuf);
  }
}

