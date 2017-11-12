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
 * @file    cmd.c
 * @brief   Tests support code.
 *
 * @addtogroup test
 * @{
 */
#if HAL_USE_IR
extern void cmd_nec(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
void hs_unit_test(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand commands[] = {
  {"meminfo",     cmd_mem,            "Current system memory status, Usage: mem"},
  {"thds",        cmd_threads,        "Print all the threads and its status, Usage: thds"},
  {"pfm",         cmd_performance,    "cpu performance, Usage: pfm"},
  {"event",       cmd_sendEvent,      "send message of event, Usage: event module_id message(hex)"},
  {"listen",      cmd_listenEvent,    "listen messages of event, Usage: listen m1(hex) ... mn(hex)"},
  {"chip",        cmd_chipStatus,     "get chip information, Usage: chip [date-string]"}, 

  {"mem8r",       cmd_mem8read,       "read memory , unit: byte, Usage: mem8r addr count"},
  {"mem8w",       cmd_mem8write,      "write memory , unit: byte, Usage: mem8w addr count"},

  {"mem16r",      cmd_mem16read,      "read memory , unit: half-word, Usage: mem16r addr count"},
  {"mem16w",      cmd_mem16write,     "write memory , unit: half-word, Usage: mem16w addr count"},

  {"mem32r",      cmd_mem32read,      "read memory , unit: word, Usage: mem32r addr count"},
  {"mem32w",      cmd_mem32write,     "write memory , unit: word, Usage: mem32w addr count"},

  {"anaset",      cmd_anaSetReg,      "set ana register, Usage: anaset startBit endBit val"}, 
  {"anaget",      cmd_anaGetReg,      "get ana register, Usage: anaget startBit endBit"},

  {"erase",       cmd_flashErase,     "flash, Usage: erase offset length"},
  {"apfm",        cmd_audioperformance,   "printf audio performance, Usage: apfm"}, 
  {"anainfo",     cmd_anainfo,        "Current digital-analog status, Usage: anainfo"},

  {"gio",         cmd_gpioTest,        "set gpio, Usage: gpio pin dir [val]"},

#if 0
  {"nec",         cmd_nec,            "NEC function shell"},
#endif

  #ifdef HS_UNIT_TEST
  {"unittest",    hs_unit_test,       "sdk unit test, Usage: unittest"},
  #endif

  #ifdef TEST_HAL_ENABLE
  SHELL_SPLIT_LINE("+ Hal"),
  HAL_TEST_CMD,
  #endif

  #ifdef TEST_LIB_ENABLE
  SHELL_SPLIT_LINE("+ Lib"),
  LIB_TEST_CMD,
  #endif

   #ifdef TEST_RT_ENABLE
  SHELL_SPLIT_LINE("+ RTOS"),
  {"rt_all",     cmd_rt_test,         "Auto test all the case of ChiBiRTOS core, Usage: rt_all"},
  #endif
  
  {NULL,          NULL,             NULL}
};


