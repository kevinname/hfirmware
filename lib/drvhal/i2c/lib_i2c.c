/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/i2c.c
 * @brief   i2c file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */

#include "lib.h"

#if HAL_USE_I2C

#define I2C_DEV_TYPE_EEPROM 0x50
#define EEPROM_PAGE_SIZE    32

typedef struct {
  i2caddr_t addr;
  uint8_t alen;
  I2CDriver *i2cp;
  I2CConfig cfg;
} hs_i2c_master_t;

static void i2c_address_to_buffer(uint8_t *buf, uint32_t offset, uint8_t alen)
{
  /* 8/16/24-bit address is written MSB first */
  switch(alen) {
  default: /* case 3 */
    *buf++ = offset >> 16;
  case 2:
    *buf++ = offset >> 8;
  case 1:
  case 0: /* can't happen: for better codegen */
    *buf++ = offset >> 0;
  }
}

/*
 * @brief I2C master open the i2c slave device, 
 *        with the specifc i2c device address, i2c clock, and addressing mode
 *
 * @param[in] hz               the clock of i2c bus in Hz, Standard=100kHz, Fast=400kHz
 * @param[in] deviceAddr       the slave device's i2c address without R/W bit
 * @param[in] deviceAddrWidth  the i2c slave device addressing mode in 7-bit or 10-bit
                                 7:  7-bit i2c slave device address
                                 10: 10-bit i2c slave device address
 * @param[in] deviceWidth      the internal register offset addressing mode
 *                               0: no address cycle
 *                               1: 1-byte addressing
 *                               2: 2-byte addressing
 *                               3: 3-byte addressing
 *
 * @return the handle to the i2c device, 0 if no handle
 */
hs_i2c_handler_t hs_i2c_init(uint32_t hz, uint16_t deviceAddr, uint8_t deviceAddrWidth, uint8_t deviceWidth)
{
  hs_i2c_master_t *this = NULL;

  if (I2CD0.state != I2C_STOP)
    return (hs_i2c_handler_t)this;

  this = (hs_i2c_master_t *)hs_malloc(sizeof(hs_i2c_master_t), __MT_Z_GENERAL);
  if (!this)
    return (hs_i2c_handler_t)this;

  this->addr = deviceAddr;
  this->alen = deviceWidth;

  this->i2cp = &I2CD0;
  this->cfg.op_mode = deviceAddrWidth == 7 ? OPMODE_I2C_MASTER : OPMODE_SMBUS_HOST;
  this->cfg.clock_speed = hz;
  this->cfg.con_mode = CONMODE_I2C_NO_AUTO_WR;
  this->cfg.timer_control_channel = 0;
  this->cfg.i2c_callback = NULL;
  i2cStart(this->i2cp, &this->cfg);

  return (hs_i2c_handler_t)this;
}

/*
 * @brief I2C master close the i2c slave device
 *
 * @return none
 */
void hs_i2c_uninit(hs_i2c_handler_t handle)
{
  hs_i2c_master_t *this = (hs_i2c_master_t *)handle;

  i2cStop(this->i2cp);
}

/*
 * @brief I2C master read data from the i2c slave device
 *
 * @param[in]  handle the handle to the i2c device
 * @param[in]  offset the offset (internal register address) in i2c slave device
 * @param[out] buf    pointer to the receive buffer
 * @param[in]  len    bytes to read
 *
 * @return read bytes
 */
int hs_i2c_read(hs_i2c_handler_t handle, uint32_t offset, uint8_t *buf, uint32_t len)
{
  systime_t tmo = MS2ST(len);
  msg_t ret;
  hs_i2c_master_t *this = (hs_i2c_master_t *)handle;

  if (tmo < MS2ST(10))
    tmo = MS2ST(10);
  i2cAcquireBus(this->i2cp);

  if (0 == this->alen) {
    ret = i2cMasterReceiveTimeout(this->i2cp,
                                  this->addr,
                                  buf, len, tmo);
  }
  else {
    uint8_t alen = this->alen;
    uint8_t abuf[3];
    i2c_address_to_buffer(abuf, offset, alen);
    ret = i2cMasterTransmitTimeout(this->i2cp,
                                   this->addr,
                                   abuf, alen,
                                   buf, len, tmo);
  }

  i2cReleaseBus(this->i2cp);

  if (ret < 0)
    return 0;
  else
    return len;
}

/*
 * @brief I2C master write data to the i2c slave device.
 *
 * @param[in] handle the handle to the i2c device
 * @param[in] offset the offset (internal register address) in i2c slave device
 * @param[in] buf    pointer to the transmit buffer
 * @param[in] len    bytes to write
 *
 * @return written bytes
 */
int hs_i2c_write(hs_i2c_handler_t handle, uint32_t offset, const uint8_t *buf, uint32_t len)
{
  systime_t tmo = MS2ST(2*len);
  msg_t ret;
  hs_i2c_master_t *this = (hs_i2c_master_t *)handle;

  if (tmo < MS2ST(10))
    tmo = MS2ST(10);
  i2cAcquireBus(this->i2cp);

  if (0 == this->alen) {
    ret = i2cMasterTransmitTimeout(this->i2cp,
                                   this->addr,
                                   buf, len, NULL, 0, tmo);
  }
  else {
    uint8_t *ibuf;
    uint8_t alen = this->alen;

    if ((ibuf = (uint8_t*)osBmemAlloc(alen + len)) == NULL) {
      i2cReleaseBus(this->i2cp);
      return 0;
    }
   
    i2c_address_to_buffer(ibuf, offset, alen);
    memcpy(ibuf + alen, buf, len);
    ret = i2cMasterTransmitTimeout(this->i2cp,
                                   this->addr,
                                   ibuf, alen+len, NULL, 0, tmo);

    osBmemFree(ibuf);
  }

  i2cReleaseBus(this->i2cp);

  if (ret < 0)
    return 0;
  else
    return len;
}

/*
 * @brief I2C master write data to the i2c memory device, eeprom.
 * @note the eeprom page size is @EEPROM_PAGE_SIZE.
 *
 * @param[in] handle the handle to the i2c device
 * @param[in] offset the offset in eeprom (eeprom's internal address)
 * @param[in] buf    pointer to the transmit buffer
 * @param[in] len    bytes to write
 *
 * @return written bytes
 */
int hs_i2c_write_eeprom(hs_i2c_handler_t handle, uint32_t offset, const uint8_t *buf, uint32_t len)
{
  systime_t tmo = MS2ST(2*len);
  msg_t ret;
  int written = 0;
  hs_i2c_master_t *this = (hs_i2c_master_t *)handle;
  uint8_t *ibuf;
  uint8_t alen = this->alen;
  uint8_t page_size = EEPROM_PAGE_SIZE;
  uint32_t first_write_size, write_size, last_write_size = 0, num_writes, page;

  /* eeprom requires 1,2,3-byte addressing */
  if ((this->addr & 0xF8) == I2C_DEV_TYPE_EEPROM) {
    if (0 == alen)
      return 0;
  }

  /* Calculate first write size in first page */
  first_write_size = (((offset/page_size) + 1)*page_size) - offset;

  if ((ibuf = (uint8_t*)osBmemAlloc(alen + page_size)) == NULL)
    return 0;
   
  /* calculate size of last write */
  if (len > first_write_size)
    last_write_size = (len-first_write_size) % page_size;
  
  /* Calculate how many writes we need */
  if (len > first_write_size)
    num_writes = ((len-first_write_size) / page_size) + 2;
  else
    num_writes = 1;  

  i2cAcquireBus(this->i2cp);
     
  for (page=0; page<num_writes; page++) {
    if (page == 0)
      write_size = first_write_size;
    else if (page == (num_writes-1))
      write_size = last_write_size;
    else
      write_size = page_size;

    if (write_size > 0) {
      i2c_address_to_buffer(ibuf, offset, alen);
      memcpy(ibuf + alen, buf + written, write_size);
      ret = i2cMasterTransmitTimeout(this->i2cp,
                                     this->addr,
                                     ibuf, alen+write_size, NULL, 0, tmo);
      if (ret >= 0) {
        written += write_size;
        offset += write_size;   // Increment address for next write
      }
      else {
        break;
      }

      /* delay for the next page write or next read*/
      chThdSleep(MS2ST(page_size/5));
    }
  }

  i2cReleaseBus(this->i2cp);

  osBmemFree(ibuf);

  return written;
}

#endif

/** @} */
