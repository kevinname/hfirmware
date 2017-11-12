/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    unitTest/unitTest.h
 * @brief   unit test.
 * @details 
 *
 * @addtogroup  unit test
 * @details 
 * @{
 */
#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

#ifdef HS_UNIT_TEST

#include "mem_test.h"


#define __assert(s)     osalDbgCheck((s))

 
void hs_unit_test(BaseSequentialStream *chp, int argc, char *argv[]);

#endif

#endif
 /** @} */
