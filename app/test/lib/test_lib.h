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
 * @file    test_lib.h
 * @brief   Tests support header.
 *
 * @addtogroup test
 * @{
 */

#ifndef _TEST_LIB_H_
#define _TEST_LIB_H_


#ifdef __cplusplus
extern "C" {
#endif

void cmd_memAlloc(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_memFree(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_vtimer(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_semTest(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_messageTest(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_cfgread(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_cfgwrite(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_cfgflush(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_isMount(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsLs(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsPwd(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsCd(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsMkdir(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsCat(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsCdDrive(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsDel(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_fsFormat(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_pwmSet(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_pwmExit(BaseSequentialStream *chp, int argc, char *argv[]);

void cmd_eqSetFreq(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_eqSetGain(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_eqSetPoint(BaseSequentialStream *chp, int argc, char *argv[]);

//void cmd_rfcommTest(BaseSequentialStream *chp, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#define  LIB_TEST_CMD             \
        {"malloc",      cmd_memAlloc,        "alloc memory, Usage: malloc type size"},    \
        {"mfree",       cmd_memFree,         "free memory, Usage: mfree address"},       \
        {"sema",        cmd_semTest,         "semaphore test, Usage: sema [num]"},       \
        {"message",     cmd_messageTest,     "message test, Usage: message"},       \
        {"vtimer",      cmd_vtimer,          "virt-timer test, Usage: vtimer type ms [cnt]"},  \
          \
        {"cfgread",     cmd_cfgread,         "read config data, Usage: cfgread index len"},  \
        {"cfgwrite",    cmd_cfgwrite,        "write config data, Usage: cfgwrite index len [pattern]"},  \
        {"cfgflush",    cmd_cfgflush,        "flush config data, Usage: cfgflush [type]"},  \
          \
        {"mount",       cmd_isMount,         "mount fat fs, Usage: mount"},  \
        {"pwd",         cmd_fsPwd,           "get current dir, Usage: pwd"},  \
        {"cd",          cmd_fsCd,            "change current dir, Usage: cd dir"}, \
        {"cdd",         cmd_fsCdDrive,       "change current drive, Usage: cdd drive"}, \
        {"mkdir",       cmd_fsMkdir,         "create a dir, Usage: mkdir dir"}, \
        {"cat",         cmd_fsCat,           "display a file, Usage: Usage: cat [-s/-b/>] file"}, \
        {"ls",          cmd_fsLs,            "list files of current dir, Usage: ls"},              \
        {"del",         cmd_fsDel,           "delete file of current dir, Usage: del file"},              \
        {"format",      cmd_fsFormat,        "format file system, Usage: format"},              \
          \
        {"pwmset",      cmd_pwmSet,          "set and start pwm, Usage: pwmset period active_width"}, \
        {"pwmexit",     cmd_pwmExit,         "pwm exit, Usage: pwmexit"}, \
          \
        {"eqfreq",      cmd_eqSetFreq,       "set frequency of seven points, Usage: eqfreq freq1 ... freq7"}, \
        {"eqgain",      cmd_eqSetGain,       "set gain of seven points, Usage: eqgain gain1 ... gain7"}, \
        {"eqpoint",     cmd_eqSetPoint,      "set EQ of a point, Usage: eqpoint pointIdx freq gain bandWidth"}


#endif /* _TEST_LIB_H_ */

