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

/**
 * @file    shell.h
 * @brief   Simple CLI shell header.
 *
 * @addtogroup SHELL
 * @{
 */

#ifndef _SHELL_H_
#define _SHELL_H_

#define SHELL_CMD_SPLIT             "split"
#define SHELL_SPLIT_LINE(mod)       {SHELL_CMD_SPLIT, NULL, mod}

/**
 * @brief   Shell maximum input line length.
 */
#if !defined(SHELL_MAX_LINE_LENGTH) || defined(__DOXYGEN__)
#define SHELL_MAX_LINE_LENGTH       64
#endif

/**
 * @brief   Shell maximum arguments per command.
 */
#if !defined(SHELL_MAX_ARGUMENTS) || defined(__DOXYGEN__)
#define SHELL_MAX_ARGUMENTS         10
#endif

/**
 * @brief   Command handler function type.
 */
typedef void (*shellcmd_t)(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * @brief   Custom command entry type.
 */
typedef struct {
  const char            *sc_name;           /**< @brief Command name.       */
  shellcmd_t            sc_function;        /**< @brief Command function.   */
  const char            *desciption;
} ShellCommand;

/**
 * @brief   Shell descriptor type.
 */
typedef struct {
  BaseSequentialStream  *sc_channel;        /**< @brief I/O channel associated
                                                 to the shell.              */
  const ShellCommand    *sc_commands;       /**< @brief Shell extra commands
                                                 table.                     */
} ShellConfig;

#ifdef __cplusplus
extern "C" {
#endif
  bool shellGetLine(BaseSequentialStream *chp, char *line, unsigned size);

  #ifdef TEST_ENABLE
  int  hs_shell_init(const ShellCommand * cmd) __attribute__((used));
  void hs_shell_uninit(void) __attribute__((used));
  void hs_shell_usbSerialOpen (uint16_t u16Idx, void *parg);
  void hs_shell_usbSerialClose(uint16_t u16Idx, void *parg);
  #endif
#ifdef __cplusplus
}
#endif

#endif /* _SHELL_H_ */

/** @} */
