/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/i2c.h
 * @brief   i2c include file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */
#ifndef __LIB_I2C_H__
#define __LIB_I2C_H__

#include "lib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hs_i2c_handler_t;

#if HAL_USE_I2C
hs_i2c_handler_t hs_i2c_init(uint32_t hz, uint16_t deviceAddr, uint8_t deviceAddrWidth, uint8_t deviceWidth);
void hs_i2c_uninit(hs_i2c_handler_t handle);
int hs_i2c_read(hs_i2c_handler_t handle, uint32_t offset, uint8_t *buf, uint32_t len);
int hs_i2c_write(hs_i2c_handler_t handle, uint32_t offset, const uint8_t *buf, uint32_t len);
int hs_i2c_write_eeprom(hs_i2c_handler_t handle, uint32_t offset, const uint8_t *buf, uint32_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif
 /** @} */
