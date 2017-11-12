/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    unitTest/unitTest.c
 * @brief   unit test.
 * @details 
 *
 * @addtogroup  unit test
 * @details 
 * @{
 */
#include "unitTest.h"

#ifdef HS_UNIT_TEST

void hs_unit_test(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
  
  hs_unittest_mem();
  //hs_unittest_oshal();
  //hs_unittest_drvhal();
  //...
}

#endif

/** @} */
