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
 * @file    shell.c
 * @brief   Simple CLI shell code.
 *
 * @addtogroup SHELL
 * @{
 */

#include "lib.h"

#define SDK_VERSION       "0.6.B1335"

static char *_strtok(char *str, const char *delim, char **saveptr) {
  char *token;
  if (str)
    *saveptr = str;
  token = *saveptr;

  if (!token)
    return NULL;

  token += strspn(token, delim);
  *saveptr = strpbrk(token, delim);
  if (*saveptr)
    *(*saveptr)++ = '\0';

  return *token ? token : NULL;
}

static void usage(BaseSequentialStream *chp, char *p) {

  chprintf(chp, "Usage: %s\r\n", p);
}

static void list_commands(BaseSequentialStream *chp, const ShellCommand *scp) {

  while (scp->sc_name != NULL) {
    if(strcmp(scp->sc_name, SHELL_CMD_SPLIT) == 0)
      //chprintf(chp, "\r\n    %s:\r\n", scp->desciption);
      chprintf(chp, "\r\n", scp->desciption);
    else
      chprintf(chp, "%16s\t%s\r\n", scp->sc_name, scp->desciption);      

    scp++;
  }
}

static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "info");
    return;
  }

  chprintf(chp, "SDK Version:  %s\r\n", SDK_VERSION);
  chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef PORT_COMPILER_NAME
  chprintf(chp, "Compiler:     %s\r\n", PORT_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s\r\n", PORT_ARCHITECTURE_NAME);
#ifdef PORT_INFO
  chprintf(chp, "Port Info:    %s\r\n", PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
}

static void cmd_systime(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "systime");
    return;
  }
  chprintf(chp, "%lu\r\n", (unsigned long)chVTGetSystemTime());
}

/**
 * @brief   Array of the default commands.
 */
static const ShellCommand local_commands[] = {
  {"info",    cmd_info,       "System information, Usage: info"},
  {"systime", cmd_systime,    "Current system time, Usage: systime"},
  {NULL, NULL, NULL}
};

static bool cmdexec(const ShellCommand *scp, BaseSequentialStream *chp,
                      char *name, int argc, char *argv[]) {

  while (scp->sc_name != NULL) {
    if (strcasecmp(scp->sc_name, name) == 0) {
      scp->sc_function(chp, argc, argv);
      return false;
    }
    scp++;
  }
  return true;
}

/**
 * @brief   Shell thread function.
 *
 * @param[in] p         pointer to a @p BaseSequentialStream object
 */
static void _shell_thread(void *p) 
{
  int n;
  BaseSequentialStream *chp = ((ShellConfig *)p)->sc_channel;
  const ShellCommand *scp = ((ShellConfig *)p)->sc_commands;
  char *lp, *cmd, *tokp, line[SHELL_MAX_LINE_LENGTH];
  char *args[SHELL_MAX_ARGUMENTS + 1];

  chRegSetThreadName("shell");
  chprintf(chp, "\r\nHuntersun Soc Test Shell\r\n");
  while (true) {
    chp = ((ShellConfig *)p)->sc_channel;
    
    chprintf(chp, "\e[1;32mHS6601> \e[0m");
    if (shellGetLine(chp, line, sizeof(line))) {
      chprintf(chp, "\r\nGet input error!\r\n");
      continue; //while(1);break;
    }
    lp = _strtok(line, " \t", &tokp);
    cmd = lp;
    n = 0;
    while ((lp = _strtok(NULL, " \t", &tokp)) != NULL) {
      if (n >= SHELL_MAX_ARGUMENTS) {
        chprintf(chp, "too many arguments\r\n");
        cmd = NULL;
        break;
      }
      args[n++] = lp;
    }
    args[n] = NULL;
    if (cmd != NULL) {
      if (strcasecmp(cmd, "exit") == 0) {
        if (n > 0) {
          usage(chp, "exit");
          continue;
        }
        break;
      }
      else if (strcasecmp(cmd, "help") == 0) {
        if (n > 0) {
          usage(chp, "help");
          continue;
        }
        chprintf(chp, "Commands:\r\n            help\thelp\r\n            exit\tExit shell, Usage: exit\r\n");
        list_commands(chp, local_commands);
        chprintf(chp, "\r\n");
        if (scp != NULL)
          list_commands(chp, scp);
        chprintf(chp, "\r\n");
      }
      else if (cmdexec(local_commands, chp, cmd, n, args) &&
          ((scp == NULL) || cmdexec(scp, chp, cmd, n, args))) {
        chprintf(chp, "%s", cmd);
        chprintf(chp, " ? Use \'help\' to get commands\r\n");
      }
    }
  }
}

/**
 * @brief   Reads a whole line from the input channel.
 *
 * @param[in] chp       pointer to a @p BaseSequentialStream object
 * @param[in] line      pointer to the line buffer
 * @param[in] size      buffer maximum length
 * @return              The operation status.
 * @retval true         the channel was reset or CTRL-D pressed.
 * @retval false        operation successful.
 *
 * @api
 */
bool shellGetLine(BaseSequentialStream *chp, char *line, unsigned size) {
  char *p = line;

  memset(line, 0, size);
  while (true) {
    char c;

    if (chSequentialStreamRead(chp, (uint8_t *)&c, 1) == 0) {
      msleep(10);
      return false;
    }
    if (c == 4) {
      chprintf(chp, "^D");
      return true;
    }
    if ((c == 8) || (c == 127)) {
      if (p != line) {
        chSequentialStreamPut(chp, c);
        chSequentialStreamPut(chp, 0x20);
        chSequentialStreamPut(chp, c);
        p--;
      }
      continue;
    }
    if (c == '\r') {
      chprintf(chp, "\r\n");
      *p = 0;
      return false;
    }
    if (c < 0x20)
      continue;
    if (p < line + size - 1) {
      chSequentialStreamPut(chp, c);
      *p++ = (char)c;
    }
  }
}

#ifdef TEST_ENABLE

typedef struct
{
  ShellConfig    stShellCfg;
  osThreadId     pstThd;

  uint32_t       u32UsbSerialOK;
}hs_shellinfo_t;

static hs_shellinfo_t *g_pstSInfo;

int hs_shell_init(const ShellCommand * cmd)
{
  osThreadDef_t stThdDef;

  if(g_pstSInfo)
    return -1;

  g_pstSInfo = hs_malloc(sizeof(hs_shellinfo_t), __MT_Z_GENERAL);
  if(!g_pstSInfo)
    return -1;

  g_pstSInfo->stShellCfg.sc_channel = (BaseSequentialStream  *)TEST_IO_DEVICE;
  g_pstSInfo->stShellCfg.sc_commands = cmd;
  
  stThdDef.pthread   = (os_pthread)_shell_thread;
  stThdDef.stacksize = 1024;
  stThdDef.tpriority = osPriorityRealtime;     
  g_pstSInfo->pstThd  = oshalThreadCreate(&stThdDef, &g_pstSInfo->stShellCfg); 
  if(!g_pstSInfo->pstThd)
  {
    hs_free(g_pstSInfo);
    return -1; 
  }

  return 0;
}

void hs_shell_uninit(void)
{
  if((!g_pstSInfo) || (!g_pstSInfo->pstThd))
    return ;
  
  if (chThdTerminatedX(g_pstSInfo->pstThd)) 
  {
    chThdRelease(g_pstSInfo->pstThd); 
    hs_free(g_pstSInfo);
    g_pstSInfo = NULL;
  }
}

#if HAL_USE_USB_SERIAL
void hs_shell_usbSerialOpen(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((g_pstSInfo->u32UsbSerialOK) || (!g_pstSInfo->pstThd) || (!g_pstSInfo))
    return ;

  #if HS_SHELL_USE_USBSERIAL
  hs_printf("\r\nPlease open the usb-serial to be continue!\r\n......\r\n");
  g_pstSInfo->stShellCfg.sc_channel = (BaseSequentialStream *)&SDU1;
  chSysLock();
  chIQPutI(&SD1.iqueue, '\r');
  chSysUnlock();
  g_pstSInfo->u32UsbSerialOK = 1;
  #endif
}

void hs_shell_usbSerialClose(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((g_pstSInfo->u32UsbSerialOK == 0) || (!g_pstSInfo->pstThd) || (!g_pstSInfo))
    return ;

  chprintf((BaseSequentialStream *)&SDU1, "\r\nPlease open the UART to be continue!\r\n......\r\n");
  g_pstSInfo->stShellCfg.sc_channel = TEST_IO_DEVICE;
  chSysLock();
  chIQPutI(&SDU1.iqueue, '\r');
  chSysUnlock();
  g_pstSInfo->u32UsbSerialOK = 0;
}
#endif

#endif
/** @} */
