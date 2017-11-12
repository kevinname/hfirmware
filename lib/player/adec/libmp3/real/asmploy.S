/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Huntersun Technologies
              hongwei.li@huntersun.com.cn

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


/*
 * __int64 MADD64 (__int64 sum, int x, int y);
 *
*/
 .globl MADD64
 .section .text
 .type   MADD64, @function
MADD64:
 mtusr $r0, $d0.lo
 mtusr $r1, $d0.hi
 madds64 $d0, $r2, $r3
 mfusr $r0, $d0.lo
 mfusr $r1, $d0.hi
 ret
