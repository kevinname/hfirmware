/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.
                 Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

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

/**
 * @file    sf.c
 * @brief   SPI flash interface Driver code.
 *
 * @addtogroup SF
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_SF || defined(__DOXYGEN__)

static const SFConfig g_stSfDefaultCfg = 
{
    NULL,
    WIDTH_8BIT,
    SELECT_CS0,
    CLKMODE_0,
    LOW_ACTIVE,
    48000000,
};

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   SPI flash interface Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sfInit(void) {
  sf_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p SFDriver structure.
 *
 * @param[out] sfp     pointer to the @p SFDriver object
 *
 * @init
 */
void sfObjectInit(SFDriver *sfp) { 
  sfp->state = SF_STOP;
  sfp->config = &g_stSfDefaultCfg;
  
#if SF_USE_MUTUAL_EXCLUSION
#if CH_CFG_USE_MUTEXES
  chMtxObjectInit(&sfp->mutex);
#else
  chSemObjectInit(&sfp->semaphore, 1);
#endif
#endif /* SF_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the SPI flash interface.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] config    pointer to the @p SFConfig object
 *
 * @api
 */
int sfStart(SFDriver *sfp, const SFConfig *config) {
  int res;
  
  if((sfp == NULL) || (config == NULL)) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  
  if((sfp->state != SF_STOP) && (sfp->state != SF_READY))
  {
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }
  
  sfp->config = config;
  res = sf_lld_start(sfp);
  if(res == 0)
    sfp->state = SF_READY;
  
  osalSysUnlock();

  return res;
}

/**
 * @brief Deactivates the SPI flash interface.
 * @note  Deactivating the peripheral also enforces a release of the slave
 *        select line.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @api
 */
int sfStop(SFDriver *sfp) {
  int res;
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  if((sfp->state != SF_STOP) && (sfp->state != SF_READY) && (sfp->state != SF_FOUND))
  {    
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }
  
  res = sf_lld_stop(sfp);
  if(res == 0) {
    sfp->state = SF_STOP;
  }
  
  osalSysUnlock();
  return res;
}

/**
 * @brief probe the SPI flash.
 * @note  
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @api
 */
int sfProbe(SFDriver *sfp, systime_t to) {
  int res;
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  if((sfp->state != SF_READY) && (sfp->state != SF_FOUND)) {
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }
  
  sfp->timeout = to;
  res = sf_lld_probe(sfp);
  if(res >= 0)
    sfp->state = SF_FOUND;
  
  osalSysUnlock();
  return res;
}

/**
 * @brief erase the SPI flash.
 * @note  
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] offset   the base address to erase
 * @param[in] len      the length of space to erase,if len == 0, then chip erase
 *
 * @api
 */
int sfErase(SFDriver *sfp, uint32_t offset, size_t len, systime_t to) {
  int res;

  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();  
  sfp->timeout = to;  
  res = sf_lld_erase(sfp, offset, len);

  osalSysUnlock();

  return res;
}

/**
 * @brief read data from the SPI flash.
 * @note  
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] rdbuf    the base address to erase
 * @param[in] len      the length of space to erase
 *
 * @api
 */
int sfRead(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len, systime_t to) {
  int res;

  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  
  sfp->timeout = to;
  res = sf_lld_read(sfp, offset, rdbuf, len);
  
  osalSysUnlock();

  return res;
}

/**
 * @brief write data from the SPI flash.
 * @note  
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] wrbuf    the base address to erase
 * @param[in] len      the length of space to erase
 *
 * @api
 */
int sfWrite(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len, systime_t to) {
  int res;

  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  
  sfp->timeout = to;
  res = sf_lld_write(sfp, offset, wrbuf, len);
  osalSysUnlock();

  return res;
}

uint32_t sfReadStatus(SFDriver *sfp, systime_t to) {

  uint32_t status;
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  sfp->timeout = to;
  status = sf_read_status(sfp);
  osalSysUnlock();
  
  return status;
}

int sfQuadEn(SFDriver *sfp, sfqe_t qe, systime_t to) {
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  if(sfp->state != SF_FOUND) {
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }

  sfp->timeout = to;
  sf_lld_QuadEn(sfp, qe);
  osalSysUnlock();
  
  return HAL_SUCCESS;
}  

int sfDeepPD(SFDriver *sfp, systime_t to) {
  int res;
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  if((sfp->state != SF_READY) && (sfp->state != SF_FOUND)) {
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }
  
  sfp->timeout = to;
  res = sf_lld_deepPd(sfp);
  
  osalSysUnlock();
  return res;
}

int sfReleasePD(SFDriver *sfp, systime_t to) {
  int res;
  
  if(sfp == NULL) {
    return SF_RES_ERROR_NULLPTR;
  }

  osalSysLock();
  if(sfp->state != SF_READY) {
    osalSysUnlock();
    return SF_RES_ERROR_STATE;
  }
  
  sfp->timeout = to;
  res = sf_lld_releasePd(sfp);
  
  osalSysUnlock();
  return res;
}


#if SF_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the SPI flash interface bus.
 * @details This function tries to gain ownership to the SPI bus, if the bus
 *          is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option @p SF_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @api
 */
void sfAcquireBus(SFDriver *sfp) {

  osalDbgCheck(sfp != NULL);

#if CH_CFG_USE_MUTEXES
  chMtxLock(&sfp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemWait(&sfp->semaphore);
#endif
}

/**
 * @brief   Releases exclusive access to the SPI bus.
 * @pre     In order to use this function the option @p SF_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @api
 */
void sfReleaseBus(SFDriver *sfp) {

  osalDbgCheck(sfp != NULL);

#if CH_CFG_USE_MUTEXES
  (void)sfp;
  chMtxUnlock(&sfp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemSignal(&sfp->semaphore);
#endif
}
#endif /* SF_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_SF */

/** @} */
