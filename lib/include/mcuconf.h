/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

#define HS6601_MCUCONF
#undef  HS6601_FPGA

#define HS_PRODUCT_SOUNDBOX       0
#define HS_PRODUCT_HEADPHONE      1
#define HS_PRODUCT_SDTEST         2

#define HS_BT_AUDIO               1
#define HS_BT_DATA_TRANS          2
#define HS_BT_DONGLE              3

#if HS_PRODUCT_TYPE == HS_PRODUCT_HEADPHONE

#define HS_USE_FM                 FALSE   //FM player
#define HS_USE_MP3                FALSE   //MP3 player
#define HS_USE_AUX                FALSE   //linein
#define HS_USE_PLAYER             FALSE
#define HS_USE_TONE               TRUE    //Tone indicator
#define HS_USE_CONF               TRUE    //Configuration system in flash
#define HS_SHELL_USE_USBSERIAL    (FALSE &&  HAL_USE_USB_SERIAL)

/* HAL */
#define HAL_USE_AUDIO             TRUE
#define HAL_USE_I2S               TRUE
#define HAL_USE_CODEC             TRUE
#define HS_I2S_USE_I2S0           TRUE
#define HS_I2S_USE_STATISTIC      TRUE
#define HS_CODEC_USE_DRV       	  TRUE
#define HS_SERIAL_USE_UART0       TRUE
#define HS_SERIAL_USE_UART1       TRUE
#define HS_RTC_USE_RTC0           FALSE
#define HS_ICU_USE_TIM1           TRUE
#define HS_GPT_USE_TIM0           FALSE
#define HS_PWM_USE_TIM2           TRUE
#define HAL_USE_FM                FALSE
#define HS_FM_USE_MAX2829         FALSE
#define HAL_USE_RF                FALSE
#define HAL_USE_WDT               FALSE

#define HAL_USE_SDC               FALSE
#define HAL_USE_FATFS             FALSE

#define HAL_USE_USB               FALSE
#define HAL_USE_USB_SERIAL        (FALSE && HAL_USE_USB)
#define HAL_USE_USB_AUDIO         (FALSE && HAL_USE_USB)
#define HAL_USE_USB_STORAGE       (FALSE && HAL_USE_USB)
#define HAL_USE_USB_HOST_STORAGE  (FALSE && HAL_USE_USB)
#define HAL_USE_USB_BULK_HS6200   (FALSE && HAL_USE_USB)

#endif

#if HS_PRODUCT_TYPE == HS_PRODUCT_SOUNDBOX

/* APP */
#define HS_USE_FM                 TRUE //FM player
#define HS_USE_MP3                TRUE //MP3 player
#define HS_USE_AUX                TRUE //linein
#define HS_USE_PLAYER             TRUE
#define HS_USE_TONE               TRUE //Tone indicator
#define HS_USE_CONF               TRUE //Configuration system in flash
#define HS_SHELL_USE_USBSERIAL    (TRUE && HAL_USE_USB_SERIAL) 
#define HS_USE_LEDDISP            TRUE

/* HAL */
#define HAL_USE_AUDIO             TRUE
#define HAL_USE_I2S               TRUE
#define HAL_USE_CODEC             TRUE
#define HAL_USE_FM                TRUE
#define HAL_USE_RF                TRUE
#define HAL_USE_WDT               FALSE //FALSE

#define HS_I2S_USE_I2S0           TRUE
#define HS_I2S_USE_STATISTIC      TRUE
#define HS_CODEC_USE_DRV       	  TRUE
#define HS_SERIAL_USE_UART0       TRUE
#define HS_SERIAL_USE_UART1       TRUE

#define HS_RTC_USE_RTC0           TRUE

#define HS_PWM_USE_TIM0           TRUE
#define HS_PWM_USE_TIM1           TRUE
#define HS_ICU_USE_TIM2           TRUE


#define HS_FM_USE_MAX2829         TRUE


#define HAL_USE_SDC               TRUE
#define HAL_USE_FATFS             TRUE

#define HAL_USE_USB               TRUE
#define HAL_USE_USB_SERIAL        (FALSE && HAL_USE_USB)
#define HAL_USE_USB_AUDIO         (TRUE && HAL_USE_USB)
#define HAL_USE_USB_STORAGE       (TRUE && HAL_USE_USB)
#define HAL_USE_USB_HOST_STORAGE  (TRUE && HAL_USE_USB)
#define HAL_USE_USB_BULK_HS6200   (FALSE && HAL_USE_USB)
#endif

#if HS_PRODUCT_TYPE == HS_PRODUCT_SDTEST

/* APP */
#define HS_USE_FM                 FALSE //FM player
#define HS_USE_MP3                FALSE //MP3 player
#define HS_USE_AUX                FALSE //linein
#define HS_USE_PLAYER             FALSE
#define HS_USE_TONE               FALSE //Tone indicator
#define HS_USE_CONF               TRUE  //Configuration system in flash
#define HS_SHELL_USE_USBSERIAL    (FALSE && HAL_USE_USB_SERIAL)

/* HAL */
#define HAL_USE_AUDIO             TRUE
#define HAL_USE_I2S               TRUE
#define HAL_USE_CODEC             TRUE
#define HS_I2S_USE_I2S0           TRUE
#define HS_I2S_USE_STATISTIC      TRUE
#define HS_CODEC_USE_DRV       	  TRUE
#define HS_SERIAL_USE_UART0       TRUE
#define HS_SERIAL_USE_UART1       TRUE
#define HS_RTC_USE_RTC0           TRUE
#define HS_ICU_USE_TIM1           TRUE
#define HS_GPT_USE_TIM0           TRUE
#define HS_PWM_USE_TIM2           TRUE
#define HAL_USE_FM                TRUE
#define HS_FM_USE_MAX2829         FALSE
#define HAL_USE_RF                TRUE
#define HAL_USE_WDT               TRUE

#define HAL_USE_SDC               TRUE
#define HAL_USE_FATFS             TRUE

#define HAL_USE_USB               TRUE
#define HAL_USE_USB_SERIAL        (FALSE && HAL_USE_USB)
#define HAL_USE_USB_AUDIO         (FALSE && HAL_USE_USB)
#define HAL_USE_USB_STORAGE       (TRUE && HAL_USE_USB)
#define HAL_USE_USB_HOST_STORAGE  (FALSE && HAL_USE_USB)
#define HAL_USE_USB_BULK_HS6200   (FALSE && HAL_USE_USB)
#endif

#ifdef HS6601_FPGA
#define HS_CODEC_USE_WM8753       TRUE
#define HS_CODEC_USE_INSIDE       TRUE
#else
#define HS_CODEC_USE_WM8753       FALSE
#define HS_CODEC_USE_INSIDE       TRUE
#endif

#define HS_SERIAL_UART0_IRQ_PRIORITY    3
#define HS_SERIAL_UART1_IRQ_PRIORITY    3
#define HS_BT_IRQ_PRIORITY              1
#define HS_TICK_IRQ_PRIORITY            2
#define HS_GPT_TIM1_IRQ_PRIORITY        3
#define HS_GPT_TIM2_IRQ_PRIORITY        3
#define HS_PWM_TIM1_IRQ_PRIORITY        3
#define HS_PWM_TIM2_IRQ_PRIORITY        3
#define HS_ICU_TIM1_IRQ_PRIORITY        3
#define HS_ICU_TIM2_IRQ_PRIORITY        3

#endif /* _MCUCONF_H_ */
