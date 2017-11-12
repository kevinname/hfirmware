/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 20156Huntersun Technologies
              wei.lu@huntersun.com.cn

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

#if defined ( __GNUC__ )
	.global xmp3_PolyphaseStereo
	.global xmp3_PolyphaseMono

	.equ RNDVAL,	(1 << ((32 - 12) + (6 - 1)))

	/* C64TOS - clip 64-bit accumulator to short (no rounding)
	 * xl, xh = value (lo 32, hi 32)
	 * input assumed to have 6 fraction bits
	 * sign = temp variable to use for sign
	 * maxPos = 0x00007fff (takes 2 instr. to generate - calculating
         * once and using repeatedly saves if you do several CTOS in a row)
	 */
    
	.macro C64TOS xl, xh, sign, maxPos
	
	srli	\xl, \xl, #(20+6)
	or_slli	\xl, \xl, \xh, #(12-6)
	srai	\sign, \xl, #31
	srai	r15, \xl, #15
	beq	    \sign, r15, 1f
	xor	    \xl, \sign, \maxPos
1:

	.endm // C64TOS

	/* ARM-version smlal lo, hi, x, y */
	.macro SMLAL lo, hi, x, y
	mulsr64	r16, \x, \y
	add45	r16, \lo         //sum.lo
	slt45	r16, \lo         //r15=C from sum.lo
	add 	\hi, \hi, r17    //sum.hi
	add 	\hi, \hi, r15    //sum.hi+C
	.endm

	/* MC0S - process 2 taps, 1 sample per channel (sample 0)
	 * x = vb1 offset
	 */

	.macro MC0S x

	lwi.bi	r12, [r2], #4
	lwi.bi	r14, [r2], #4
	lwi 	r0, [r1 +#(4*(\x))]
	lwi	    r3, [r1 +#(4*(23 - \x))]

   	SMLAL	r4, r5, r0, r12
	lwi	    r0, [r1 +#(4*(32 + \x))]
	subri	r14, r14, #0
   	SMLAL	r4, r5, r3, r14
	lwi 	r3, [r1 +#(4*(32 + 23 - \x))]

   	SMLAL	r8, r9, r0, r12
   	SMLAL	r8, r9, r3, r14

	.endm // MCOS

	/* MC1S - process 2 taps, 1 sample per channel (sample 16)
	 * x = vb1 offset
	 */
    
	.macro MC1S x

	lwi.bi	r12, [r2], #4
	lwi	    r0, [r1 +#(4*(\x))]
	lwi 	r3, [r1 +#(4*(32 + \x))]
	SMLAL	r4, r5, r0, r12
	SMLAL	r8, r9, r3, r12
    
	.endm // MC1S

	/* MC2S - process 2 taps, 2 samples per channel
	 * x = vb1 offset
	 */
    
	.macro MC2S x

	// load data as far as possible in advance of using it
	lwi.bi	r12, [r2], #4
	lwi.bi 	r14, [r2], #4
	lwi 	r0, [r1 +#(4*(\x))]
	lwi 	r3, [r1 +#(4*(23 - \x))]

	SMLAL	r4, r5, r0, r12
	SMLAL	r6, r7, r0, r14
	subri	r14, r14, #0
	SMLAL	r6, r7, r3, r12
	SMLAL	r4, r5, r3, r14

	lwi 	r3, [r1 +#(4*(32 + 23 - \x))]
	lwi 	r0, [r1 +#(4*(32 + \x))]

	SMLAL	r8, r9, r3, r14
	SMLAL	r10, r11, r3, r12
	subri	r14, r14, #0
	SMLAL	r8, r9, r0, r12
	SMLAL	r10, r11, r0, r14

	.endm // MC2S

	.section .mp3text

	/* void PolyphaseStereo(short *pcm, int *vbuf, const int *coefBase) */
    /* arm: r13=sp r14=lr                          sp[1]=pcm sp[0]=i
       nds: r31=sp r30=lp  r15=asm r16,r17=mulsr64   r18=pcm   r19=i */
	.func xmp3_PolyphaseStereo
xmp3_PolyphaseStereo:
	push25 $r14,#64    ! {$r6~$r14, $fp, $gp, $lp}
    
	mov55     	r18, r0                   // r18 = pcm pointer

	// special case, output sample 0
	move        r4, #RNDVAL               // load rndVal (low 32)
	move        r8, #RNDVAL               // load rndVal (low 32)
	movi        r5, #0
	movi        r9, #0

	MC0S    0
	MC0S    1
	MC0S    2
	MC0S    3
	MC0S    4
	MC0S    5
	MC0S    6
	MC0S    7

	mov55      r0, r18             // load pcm pointer
	movi       r14, #0x7f00
	ori        r14, r14, #0xff

	C64TOS    r4, r5, r12, r14
	C64TOS    r8, r9, r12, r14

	shi       r4, [r0 +#(2*0)]
	shi       r8, [r0 +#(2*1)]

	// special case, output sample 16
	addi       r2, r2, #(4*(256-16))    // coef = coefBase + 256 (was coefBase + 16 after MC0S block)
	addi       r1, r1, #(4*1024)        // vb1 = vbuf + 64*16

	move       r4, #RNDVAL              // load rndVal (low 32)
	move       r8, #RNDVAL              // load rndVal (low 32)
	movi       r5, #0
	movi       r9, #0

	MC1S    0
	MC1S    1
	MC1S    2
	MC1S    3
	MC1S    4
	MC1S    5
	MC1S    6
	MC1S    7

	mov55      r0, r18           // load pcm pointer
	movi       r14, #0x7f00
	ori        r14, r14, #0xff
    
	C64TOS  r4, r5, r12, r14
	C64TOS  r8, r9, r12, r14

	shi     r4, [r0 +#(2*(2*16+0))]
	shi     r8, [r0 +#(2*(2*16+1))]

	// main convolution loop: sum1L = samples 1, 2, 3, ... 15   sum2L = samples 31, 30, ... 17
	addi    r2, r2, -#(4*(264-16))    // coef = coefBase + 16 (was coefBase + 264 after MC1S block)
	addi    r1, r1, -#(4*(1024-64))   // vb1 = vbuf + 64 (was vbuf + 64*16 after MC1S block)
	movi    r19, #15                  // loop counter, count down
	addi    r0, r0, #(2*2)            // pcm+=2

LoopPS:
	mov55       r18, r0                   // r18 = pcm (pointer to pcm buffer)

	move        r4,  #RNDVAL              // load rndVal (low 32)
	move        r8,  #RNDVAL              // load rndVal (low 32)
	move        r6,  #RNDVAL              // load rndVal (low 32)
	move        r10, #RNDVAL              // load rndVal (low 32)

	movi        r5,  #0
	movi        r9,  #0
	movi        r7,  #0
	movi        r11, #0

	MC2S    0
	MC2S    1
	MC2S    2
	MC2S    3
	MC2S    4
	MC2S    5
	MC2S    6
	MC2S    7

	addi        r1, r1, #(4*64)    // vb1 += 64

	mov55       r0, r18            // load pcm pointer
	movi        r14, #0x7f00
	ori         r14, r14, #0xff

	C64TOS    r4, r5, r12, r14
	C64TOS    r8, r9, r12, r14
	C64TOS    r6, r7, r12, r14
	C64TOS    r10, r11, r12, r14

	add_slli   r14, r0, r19, #3     // r30 = r0 + 4*i (short offset)
	shi.bi     r6,  [r14], #2       // *(pcm + 2*2*i + 0)
	shi.bi     r10, [r14], #2       // *(pcm + 2*2*i + 1)

	shi.bi    r4, [r0], #2          // *(pcm + 0)
	shi.bi    r8, [r0], #2          // *(pcm + 1)

	addi      r19, r19, -#1
	bnez      r19, LoopPS

	pop25 $r14,#64    ! {$r6~$r14, $fp, $gp, $lp}
	.endfunc

	/* MONO PROCESSING */

	/* MC0M - process 2 taps, 1 sample (sample 0)
	 * x = vb1 offset
	 */
    
	.macro MC0M x

	lwi.bi	r12, [r2], #4
	lwi.bi	r14, [r2], #4
	lwi 	r0, [r1 +#(4*(\x))]
	lwi 	r3, [r1 +#(4*(23 - \x))]

	subri	r14, r14, #0
	SMLAL	r4, r5, r0, r12
	SMLAL	r4, r5, r3, r14

	.endm // MC0M

	/* MC1M - process 2 taps, 1 sample (sample 16)
	 * x = vb1 offset
	 */
    
	.macro MC1M x

	lwi.bi	r12, [r2], #4
	lwi 	r0, [r1 +#(4*(\x))]
	SMLAL	r4, r5, r0, r12
    
	.endm // MC1M

	/* MC2M - process 2 taps, 2 samples
	 * x = vb1 offset
	 */
	
	.macro MC2M x

	// load data as far as possible in advance of using it
	lwi.bi	r12, [r2], #4
	lwi.bi	r14, [r2], #4
	lwi 	r0, [r1 +#(4*(\x))]
	lwi 	r3, [r1 +#(4*(23 - \x))]

	SMLAL	r4, r5, r0, r12
	SMLAL	r6, r7, r0, r14
	subri	r14, r14, #0
	SMLAL	r4, r5, r3, r14
	SMLAL	r6, r7, r3, r12

	.endm // MC2M

	.section .mp3text
	/* void PolyphaseMono(short *pcm, int *vbuf, const int *coefBase) */
	.func xmp3_PolyphaseMono
xmp3_PolyphaseMono:
	push25 $r14,#64    ! {$r6~$r14, $fp, $gp, $lp}

	mov55     	r18, r0                   // r18 = pcm pointer

	// special case, output sample 0
	move        r4, #RNDVAL               // load rndVal (low 32)
	movi        r5, #0

	MC0M    0
	MC0M    1
	MC0M    2
	MC0M    3
	MC0M    4
	MC0M    5
	MC0M    6
	MC0M    7

	mov55      r0, r18             // load pcm pointer
	movi       r14, #0x7f00
	ori        r14, r14, #0xff

	C64TOS    r4, r5, r12, r14
	shi       r4, [r0 +#(2*0)]

	// special case, output sample 16
	addi        r2, r2, #(4*(256-16))    // coef = coefBase + 256 (was coefBase + 16 after MC0M block)
	addi        r1, r1, #(4*1024)        // vb1 = vbuf + 64*16

	move        r4, #RNDVAL              // load rndVal (low 32)
	movi        r5, #0

	MC1M    0
	MC1M    1
	MC1M    2
	MC1M    3
	MC1M    4
	MC1M    5
	MC1M    6
	MC1M    7

	mov55      r0, r18           // load pcm pointer
	movi       r14, #0x7f00
	ori        r14, r14, #0xff

	C64TOS    r4, r5, r12, r14

	shi       r4, [r0 +#(2*16)]

	// main convolution loop: sum1L = samples 1, 2, 3, ... 15   sum2L = samples 31, 30, ... 17
	addi      r2, r2, -#(4*(264-16))    // coef = coefBase + 16 (was coefBase + 264 after MC1M block)
	addi      r1, r1, -#(4*(1024-64))   // vb1 = vbuf + 64 (was vbuf + 64*16 after MC1M block)
	movi      r19, #15                  // loop counter, count down
	addi      r0, r0, #(2)              // pcm++

LoopPM:
	mov55      r18, r0                  // r18 = pcm (pointer to pcm buffer)

	move       r4, #RNDVAL              // load rndVal (low 32)
	move       r6, #RNDVAL              // load rndVal (low 32)
	movi       r5, #0
	movi       r7, #0

	MC2M    0
	MC2M    1
	MC2M    2
	MC2M    3
	MC2M    4
	MC2M    5
	MC2M    6
	MC2M    7

	addi       r1, r1, #(4*64)    // vb1 += 64

	mov55      r0, r18            // load pcm pointer
	movi       r14, #0x7f00
	ori        r14, r14, #0xff

	C64TOS    r4, r5, r12, r14
	C64TOS    r6, r7, r12, r14

	add_slli  r14, r0, r19, #2    // r14 = r0 + 2*i (short offset)*/
	shi.bi    r6, [r14], #2    	  // (pcm + 2*i + 0)
	shi.bi    r4, [r0], #2        // (pcm + 0) // pcm++

	addi      r19, r19, -#1
	bnez      r19, LoopPM

	pop25 $r14,#64    ! {$r6~$r14, $fp, $gp, $lp}
	.endfunc

	.end

#endif
