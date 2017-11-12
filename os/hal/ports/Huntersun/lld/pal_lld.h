/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn

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
 * @file    HS66xx/pal_lld.h
 * @brief   HS66xx PAL low level driver header.
 *
 * @addtogroup PAL
 * @{
 */

#ifndef _PAL_LLD_H_
#define _PAL_LLD_H_

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Unsupported modes and specific modes                                      */
/*===========================================================================*/

#undef PAL_MODE_RESET
#undef PAL_MODE_UNCONNECTED
#undef PAL_MODE_INPUT
#undef PAL_MODE_INPUT_PULLUP
#undef PAL_MODE_INPUT_PULLDOWN
#undef PAL_MODE_INPUT_ANALOG
#undef PAL_MODE_OUTPUT_PUSHPULL
#undef PAL_MODE_OUTPUT_OPENDRAIN

/**
 * @name    HS66XX-specific I/O PAD number
 * @{
 */
#define    PAD_FUNC_JTAG             (0)
#define    PAD_FUNC_DEBUG            (1)
#define    PAD_FUNC_TIMER2_3         (2)
#define    PAD_FUNC_SFLASH           (3)
#define    PAD_FUNC_SD_USB           (4)
#define    PAD_FUNC_GPIO             (5)
#define    PAD_FUNC_I2S_TEST         (6)
#define    PAD_FUNC_PCM_TEST         (7)
#define    PAD_FUNC_I2C_SCK          (8)
#define    PAD_FUNC_I2C_SDA          (9)
#define    PAD_FUNC_SPI0_MISO        (10)
#define    PAD_FUNC_SPI0_MOSI        (11)
#define    PAD_FUNC_SPI0_CSN         (12)
#define    PAD_FUNC_SPI0_SCK         (13)
#define    PAD_FUNC_SPI1_MISO        (14)
#define    PAD_FUNC_SPI1_MOSI        (15)
#define    PAD_FUNC_SPI1_CSN         (16)
#define    PAD_FUNC_SPI1_SCK         (17)
#define    PAD_FUNC_UART0_RX         (18)
#define    PAD_FUNC_UART0_TX         (19)
#define    PAD_FUNC_UART0_SIR_RX     (20)
#define    PAD_FUNC_UART0_SIR_TX     (21)
#define    PAD_FUNC_UART1_RX         (22)
#define    PAD_FUNC_UART1_TX         (23)
#define    PAD_FUNC_UART1_CTS        (24)
#define    PAD_FUNC_UART1_RTS        (25)
#define    PAD_FUNC_RTC_TI1_EVENT    (26)
#define    PAD_FUNC_RTC_TI2_EVENT    (27)
#define    PAD_FUNC_RTC_AFO_ALARM    (28)
#define    PAD_FUNC_RTC_AFO_CALIB    (29)
#define    PAD_FUNC_I2S_SCK_TX       (30)
#define    PAD_FUNC_I2S_SCK_RX       (31)
#define    PAD_FUNC_I2S_WS_TX        (32)
#define    PAD_FUNC_I2S_WS_RX        (33)        
#define    PAD_FUNC_I2S_SDI          (34)        
#define    PAD_FUNC_I2S_SDO          (35)
#define    PAD_FUNC_TIMER1_CH0       (36)
#define    PAD_FUNC_TIMER1_CH1       (37)
#define    PAD_FUNC_TIMER1_CH2       (38)
#define    PAD_FUNC_TIMER1_CH3       (39)
#define    PAD_FUNC_TIMER1_BKIN      (40)
#define    PAD_FUNC_TIMER1_TOGGLE0   (47)
#define    PAD_FUNC_TIMER1_TOGGLE1   (48)
#define    PAD_FUNC_TIMER1_TOGGLE2   (49)
#define    PAD_FUNC_SD_DETECT        (42)
#define    PAD_FUNC_SD_WP            (43)
#define    PAD_FUNC_DMIC_CLK         (44)
#define    PAD_FUNC_DMIC_IN          (45)
#define    PAD_CODEC_CLK12M          (46)

/**
 * @name    HS66XX-specific I/O mode flags
 * @{
 */
#define PAL_HS_MODE_MASK             (1 << 0)
#define PAL_HS_MODE_INPUT            (1 << 0)
#define PAL_HS_MODE_OUTPUT           (0 << 0)

#define PAL_HS_PUDR_MASK             (3 << 1)
#define PAL_HS_PUDR_NOPUPDR          (0 << 1)
#define PAL_HS_PUDR_PULLUP           (1 << 1)
#define PAL_HS_PUDR_PULLDOWN         (2 << 1)

#define PAL_HS_DRCAP_MASK            (3 << 3)
#define PAL_HS_DRCAP(n)              ((n&3) << 3)

#define PAL_HS_ALTERNATE_MASK        (0x3f << 5)
#define PAL_HS_ALTERNATE(n)          ((n&0x3f) << 5)


/**
 * @brief   Alternate function.
 *
 * @param[in] n         alternate function selector
 */
#define PAL_MODE_ALTERNATE(n)       PAL_HS_ALTERNATE(n)
/** @} */


/**
 * @brief   pad drive capability function.
 *
 * @param[in] n         drive capability selector
 */
#define PAL_MODE_DRIVE_CAP(n)     PAL_HS_DRCAP(n) 
/** @} */


/**
 * @name    Standard I/O mode flags
 * @{
 */
/**
 * @brief   This mode is implemented as input.
 */
#define PAL_MODE_RESET                  PAL_HS_MODE_INPUT

/**
 * @brief   This mode is implemented as input with pull-up.
 */
#define PAL_MODE_UNCONNECTED            PAL_MODE_INPUT_PULLUP

/**
 * @brief   Regular input high-Z pad.
 */
#define PAL_MODE_INPUT                  (PAL_HS_MODE_INPUT |             \
                                        PAL_HS_PUDR_NOPUPDR)

/**
 * @brief   Input pad with weak pull up resistor.
 */
#define PAL_MODE_INPUT_PULLUP           (PAL_HS_MODE_INPUT |             \
                                         PAL_HS_PUDR_PULLUP)

/**
 * @brief   Input pad with weak pull down resistor.
 */
#define PAL_MODE_INPUT_PULLDOWN         (PAL_HS_MODE_INPUT |             \
                                         PAL_HS_PUDR_PULLDOWN)

/**
 * @brief   Regular output high-Z pad.
 */
#define PAL_MODE_OUTPUT                 (PAL_HS_MODE_OUTPUT |             \
                                        PAL_HS_PUDR_NOPUPDR)

/**
 * @brief   Pullup output pad.
 */
#define PAL_MODE_OUTPUT_PULLUP         (PAL_HS_MODE_OUTPUT |            \
                                         PAL_HS_PUDR_PULLUP)

/**
 * @brief   pulldown output pad.
 */
#define PAL_MODE_OUTPUT_PULLDOWN       (PAL_HS_MODE_OUTPUT |            \
                                         PAL_HS_PUDR_PULLDOWN)

/** @} */


#if !defined(HS_GPIO_INTR_PRIORITY) || defined(__DOXYGEN__)
#define HS_GPIO_INTR_PRIORITY        3
#endif

/*===========================================================================*/
/* I/O Ports Types and constants.                                            */
/*===========================================================================*/

/**
 * @brief   PAL port setup info.
 */
typedef struct {
  uint32_t      data;
  uint32_t      out;
  uint32_t      altfunc;
  uint32_t      inten;
  uint32_t      inttype;
  uint32_t      intpol;
} hs_gpio_setup_t;


/**
 * @brief   PAL interrupt type .
 */
typedef enum {
  FALLING_EDGE = 0,
  RISING_EDGE,
  DUAL_EDGE,
  LOW_LEVEL,
  HIGH_LEVEL,
}hs_gpio_inter_t;

/**
 * @brief   HS PAL static initializer.
 * @details An instance of this structure must be passed to @p palInit() at
 *          system startup time in order to initialize the digital I/O
 *          subsystem. This represents only the initial setup, specific pads
 *          or whole ports can be reprogrammed at later time.
 */
typedef struct {
  hs_gpio_setup_t    gpio0;
  hs_gpio_setup_t    gpio1;
} PALConfig;

/**
 * @brief   Width, in bits, of an I/O port.
 */
#define PAL_IOPORTS_WIDTH 16

/**
 * @brief   Whole port mask.
 * @details This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFFFF)

/**
 * @brief Digital I/O port sized unsigned type.
 */
typedef uint32_t ioportmask_t;

/**
 * @brief   Digital I/O modes.
 */
typedef uint32_t iomode_t;

/**
 * @brief   Port Identifier.
 * @details This type can be a scalar or some kind of pointer, do not make
 *          any assumption about it, use the provided macros when populating
 *          variables of this type.
 */
typedef HS_GPIO_Type * ioportid_t;

typedef void (*ioevent_t)(ioportid_t port, uint8_t pad);
/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/* The low level driver wraps the definitions already present in the HS   */
/* firmware library.                                                         */
/*===========================================================================*/

/**
 * @brief   PAL port 0 identifier.
 */
#if HS_HAS_GPIO0 || defined(__DOXYGEN__)
#define IOPORT0         HS_GPIO0
#endif

/**
 * @brief   PAL port 1 identifier.
 */
#if HS_HAS_GPIO1 || defined(__DOXYGEN__)
#define IOPORT1         HS_GPIO1
#endif

/*===========================================================================*/
/* Implementation, some of the following macros could be implemented as      */
/* functions, if so please put them in pal_lld.c.                            */
/*===========================================================================*/

/**
 * @brief   PAL ports subsystem initialization.
 *
 * @notapi
 */
#define pal_lld_init() _pal_lld_init()

/**
 * @brief   Reads an I/O port.
 * @details This function is implemented by reading the PAL DATA register.
 * @note    This function is not meant to be invoked directly by the application
 *          code.
 *
 * @param[in] port      port identifier
 * @return              The port bits.
 *
 * @notapi
 */
#define pal_lld_readport(port) ((port)->DATA)

/**
 * @brief   Reads the output latch.
 * @details This function is implemented by reading the PAL DATAOUT register.
 * @note    This function is not meant to be invoked directly by the application
 *          code.
 *
 * @param[in] port      port identifier
 * @return              The latched logical states.
 *
 * @notapi
 */
#define pal_lld_readlatch(port) ((port)->DATAOUT)

/**
 * @brief   Writes on a I/O port.
 * @details This function is implemented by writing the PAL DATAOUT register.
 * @note    Writing on pads programmed as pull-up or pull-down has the side
 *          effect to modify the resistor setting because the output latched
 *          data is used for the resistor selection.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits) ((port)->DATAOUT = (bits))

/**
 * @brief   Pads group mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as output as recommended by
 *          the MSP430x1xx Family User's Guide.
 * @note    This function does not alter the @p PxSEL registers. Alternate
 *          functions setup must be handled by device-specific code.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      mode
 *
 * @notapi
 */
#define pal_lld_setgroupmode(port, mask, offset, mode) \
   (_pal_lld_setgroupmode(port, mask << offset, mode))

#define pal_lld_regevent(pad, lvl, event)   (_pal_lld_regevent(pad, lvl, event))

extern const PALConfig pal_default_config;

#ifdef __cplusplus
extern "C" {
#endif
  void _pal_lld_init(void);
        
  void _pal_lld_setgroupmode(ioportid_t port,
                             ioportmask_t mask,
                             iomode_t mode);
  void _pal_lld_regevent(uint8_t pad, hs_gpio_inter_t lvl, ioevent_t event);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL */

#endif /* _PAL_LLD_H_ */

/** @} */
