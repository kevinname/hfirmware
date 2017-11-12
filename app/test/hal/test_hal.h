/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
                            2014..2015 pingping.wu@huntersun.com.cn

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
 * @file    test_hal.h
 * @brief   Tests support header.
 *
 * @addtogroup test
 * @{
 */

#ifndef _TEST_HAL_H_
#define _TEST_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

void cmd_adcSetAttr(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adcSetTiming(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adcAddChn(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adcStart(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adcStop(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adcScan(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_i2c(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_hs6760(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_sf(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_erase(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_SetMode(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_ReadStatus(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sfwrite(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sfRead(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sfClk(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_audioperformance(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_sd(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_rtc(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_avcodec(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_i2s(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dma(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_usbhost(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_rf(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_tim(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_spi(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_codec(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fm(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_pin(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_btrf(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_wdt(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_anaSetReg(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_anaGetReg(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_pmuSetVoltage(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_rfpllSetVoltage(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_pllClock(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_audioRecord(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_audioTestS(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_audioTestE(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_audioAlgRecord(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_sfTestWrite(BaseSequentialStream *chp, int argc, char *argv[]);
        
#ifdef __cplusplus
}
#endif

#define  HAL_TEST_CMD             \
        {"adcattr",   cmd_adcSetAttr,         "set adc attribute, Usage: see from \'adcattr ?\'"},    \
        {"adctiming", cmd_adcSetTiming,       "set adc timing, Usage: see from \'adctiming ?\'"},    \
        {"adcchn",    cmd_adcAddChn,          "set adc chn-feather, Usage: see from \'adcchn ?\'"},    \
        {"adcstart",  cmd_adcStart,           "adc start, Usage: adcstart mode [cnt]"},    \
        {"adcstop",   cmd_adcStop,            "adc stop, Usage: adcstop"},    \
        {"adcscan",   cmd_adcScan,            "adc scan every case, Usage: adcscan"},    \
                                                                                         \
        {"i2c",       cmd_i2c,                "i2c test, Usage: i2c [dev_addr offset count]"},   \
        {"hs6760",    cmd_hs6760,             "hs6760, Usage: hs6760 [init|freq]"},  \
        {"sf",        cmd_sf,                 "sf test,  Usage: sf  [speed offset count]"},       \
        {"sfes",      cmd_erase,              "erase flash, Usage: sferase offset length"}, \
        {"sfset",     cmd_SetMode,            "set flash mode, Usage: sfmode mode"}, \
        {"sfrds",     cmd_ReadStatus,         "read flash status, Usage: sfrds"}, \
        {"sfrd",      cmd_sfRead,             "read data from flash, Usage: sfrd offset len"}, \
        {"sfwr",      cmd_sfwrite,            "write data to flash, Usage: sfwr offset [pattern]"}, \
        {"sfclk",     cmd_sfClk,              "set sflash clock, Usage: sfclk clock"}, \
        {"sftwr",     cmd_sfTestWrite,        "test flash writing, Usage: sftwr"}, \
                                                                                         \
        {"sd",        cmd_sd,                 "SD card test, using fatfs, Usage: sd"}, \
        {"acodec",    cmd_avcodec,            "mp3 decoder, Usage: avcodec [input file]"}, \
        {"audrec",    cmd_audioRecord,        "audio recorder, Usage: audrec source sample"}, \
        {"audts",     cmd_audioTestS,         "Usage: audts mode sample"}, \
        {"audte",     cmd_audioTestE,         "Usage: audte"}, \
        {"audalg",    cmd_audioAlgRecord,     "test audio alg, Usage: audalg source alg"}, \
        {"codec",     cmd_codec,              "codec test, Usage: codec"}, \
        {"i2s",       cmd_i2s,                "i2s test, play wav file, Usage: i2s [file name]"}, \
        {"dma",       cmd_dma,                "dma test, Usage: dma"}, \
        {"usbhost",   cmd_usbhost,            "usbhost test, Usage: usbhost"}, \
        {"rtc",       cmd_rtc,                "rtc test, Usage: rtc [date | alarm | sleep | wakeup]"}, \
        {"rf",        cmd_rf,                 "2.4G RF test, Usage: rf ...."}, \
        {"spi",       cmd_spi,                "spi test, Usage: spi [cs]"}, \
        {"fm",        cmd_fm,                 "fm test, Usage: fm "}, \
        {"tim",       cmd_tim,                "timer test, Usage: tim [dma | ]"}, \
        {"gpio",      cmd_gpio,               "gpio test, Usage: gpio [num | input/output ]"},  \
        {"pin",       cmd_pin,                "pin set/clr, Usage: pin"}, \
        {"btrf",      cmd_btrf,               "bt rf calibration test"},      \
        {"wdt",       cmd_wdt,                "watch test, Usage: wdt kick/init"},      \
          \
        {"pmuvolt",   cmd_pmuSetVoltage,      "set pmu voltage, Usage: pmuvolt index"},      \
        {"rfpllvolt", cmd_rfpllSetVoltage,    "set rf pll voltage, Usage: rfpllvolt index"},   \
        {"pllclk",    cmd_pllClock,           "set pll clock, Usage: pllclk index"}

#endif /* _TEST_HAL_H_ */

