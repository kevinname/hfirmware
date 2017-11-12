/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    hs6601/cmparams.h
 * @brief   Andes N10 parameters for the STM32F4xx.
 *
 * @defgroup Andes N10 hs6601 Specific Parameters
 * @ingroup ARMCMx_SPECIFIC
 * @details This file contains the Andes N10 specific parameters for the
 *          hs6601 platform.
 * @{
 */

#ifndef _ANPARAMS_H_
#define _ANPARAMS_H_

/**
 * @brief   Andes core model.
 */
#define ANDES_MODEL            4

/**
 * @brief   Floating Point unit presence.
 */
#define ANDES_HAS_FPU          0

/**
 * @brief   Number of bits in priority masks.
 */
#define ANDES_PRIORITY_BITS    2

/**
 * @brief   Number of interrupt vectors.
 * @note    This number does not include the 16 system vectors and must be
 *          rounded to a multiple of 8.
 */
#define ANDES_NUM_VECTORS      32

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)
#include "board.h"
#endif /* !defined(_FROM_ASM_) */

#endif /* _ANPARAMS_H_ */

/** @} */
