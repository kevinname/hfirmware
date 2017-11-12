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
 * @file    test.h
 * @brief   Tests support header.
 *
 * @addtogroup test
 * @{
 */

#ifndef _TEST_H_
#define _TEST_H_

#ifdef TEST_ENABLE

#ifdef TEST_RT_ENABLE
#include "test_rt.h"
#endif

#ifdef TEST_HAL_ENABLE
#include "test_hal.h"
#endif

#ifdef TEST_LIB_ENABLE
#include "test_lib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void hs_test_init(void);
void hs_test_chkExit(void);

#ifdef __cplusplus
}
#endif

#endif
#endif /* _TEST_H_ */

