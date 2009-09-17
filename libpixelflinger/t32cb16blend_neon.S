/* libs/pixelflinger/t32cb16blend_neon.S
**
** (C) COPYRIGHT 2009 ARM Limited.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*/

// Possible we have been included optimistically (because target is armv7) but Neon
// is not available.
#ifdef __ARM_NEON__
    .text
    .align

    .global scanline_t32cb16blend_neon

// r0 = dst
// r1 = src
// r2 = count

//
// This function alpha blends a source and destination scanline, using the
// formula:
//
//     d = s + (((a + (a >> 7)) * d) >> 8)
//
// where d is the destination pixel,
//       s is the source pixel,
//       a is the alpha channel of the source pixel,
//
// The Neon implementation processes 16 pixels per iteration. The remaining 0 - 15
// pixels are processed in ARM code.
//


scanline_t32cb16blend_neon:
    push        {r4-r11, lr}                    // stack ARM regs

    vmov.u16    q15, #256                       // create alpha constant
    movs        r3, r2, lsr #4                  // calc. sixteens iterations
    vmov.u16    q14, #0x1f                      // create blue mask
    beq         2f                              // if r3 == 0, branch to singles

    // This loop processes 16 pixels per iteration. In the comments, references to
    // the first eight pixels are suffixed with "0" (red0, green0, blue0, alpha0),
    // the second eight are suffixed "1".

1:
    vld4.8      {d0, d1, d2, d3}, [r1]!         // load first 8 source pixels
    subs        r3, r3, #1                      // decrement sixteens counter
    vshr.u8     d0, d0, #3                      // shift down red0 to low 5 bits
    vshr.u8     d1, d1, #2                      // shift down green0 to low 6 bits
    vshr.u8     d2, d2, #3                      // shift down blue0 to low 5 bits

    vld1.16     {d20, d21, d22, d23}, [r0]      // load 16 dest pixels

    vshr.u8     d24, d3, #7                     // extract top bit of alpha0
    vaddl.u8    q12, d3, d24                    // add top bit into alpha0
    vsub.u16    q12, q15, q12                   // invert alpha0

    vshr.u16    q8, q10, #11                    // shift dst red0 to low 5 bits
    vshl.u16    q9, q10, #5                     // shift dst green0 to top 6 bits
    vand        q10, q10, q14                   // extract dst blue0
    vshr.u16    q9, q9, #10                     // shift dst green0 to low 6 bits

    vld4.8      {d4, d5, d6, d7}, [r1]!         // load second 8 source pixels

    vmul.u16    q8, q8, q12                     // multiply dst red0 by src alpha0
    vshr.u8     d4, d4, #3                      // shift down red1 to low 5 bits
    vmul.u16    q9, q9, q12                     // multiply dst green0 by src alpha0
    vshr.u8     d5, d5, #2                      // shift down green1 to low 6 bits
    vmul.u16    q10, q10, q12                   // multiply dst blue0 by src alpha0
    vshr.u8     d6, d6, #3                      // shift down blue1 to low 5 bits

    vshrn.u16   d16, q8, #8                     // shift red0 down to 5 bit range
    vshrn.u16   d18, q9, #8                     // shift green0 down to 6 bit range
    vshrn.u16   d20, q10, #8                    // shift blue0 down to 5 bit range

    vaddl.u8    q8, d16, d0                     // add source red0
    vaddl.u8    q9, d18, d1                     // add source green0
    vaddl.u8    q10, d20, d2                    // add source blue0

    vshr.u8     d26, d7, #7                     // extract top bit of alpha1
    vaddl.u8    q13, d7, d26                    // add top bit into alpha1
    vsub.u16    q13, q15, q13                   // invert alpha1

    vsli.u16    q10, q9, #5                     // shift & insert green0 into blue0
    vsli.u16    q10, q8, #11                    // shift & insert red0 into blue0

    vshr.u16    q8, q11, #11                    // shift dst red1 to low 5 bits
    vshl.u16    q9, q11, #5                     // shift dst green1 to top 6 bits
    vand        q11, q11, q14                   // extract dst blue1
    vshr.u16    q9, q9, #10                     // shift dst green1 to low 6 bits

    vmul.u16    q8, q8, q13                     // multiply dst red1 by src alpha1
    vmul.u16    q9, q9, q13                     // multiply dst green1 by src alpha1
    vmul.u16    q11, q11, q13                   // multiply dst blue1 by src alpha1

    vshrn.u16   d16, q8, #8                     // shift red1 down to 5 bit range
    vshrn.u16   d18, q9, #8                     // shift green1 down to 6 bit range
    vshrn.u16   d22, q11, #8                    // shift blue1 down to 5 bit range

    vaddl.u8    q8, d16, d4                     // add source red1
    vaddl.u8    q9, d18, d5                     // add source green1
    vaddl.u8    q11, d22, d6                    // add source blue1

    vsli.u16    q11, q9, #5                     // shift & insert green1 into blue1
    vsli.u16    q11, q8, #11                    // shift & insert red1 into blue1

    vst1.16     {d20, d21, d22, d23}, [r0]!     // write 16 pixels back to dst
    bne         1b                              // if count != 0, loop

2:
    ands        r3, r2, #15                     // calc. single iterations
    mov         r9, #0x1f                       // dest red/blue channel mask
    mov         r10, #0x3f                      // dest green channel mask
    beq         4f                              // if r3 == 0, exit

3:
    ldr         r4, [r1], #4                    // load source pixel
    subs        r3, r3, #1                      // decrement singles counter
    ldrh        r8, [r0]                        // load dest pixel

    mov         r5, r4, lsr #24                 // shift down alpha
    add         r5, r5, r5, lsr #7              // add in top bit
    rsb         r5, r5, #256                    // invert alpha
    mov         r5, r5, lsl #8                  // prescale alpha

    and         r11, r9, r4, lsr #3             // extract src red
    and         r12, r10, r4, lsr #10           // extract src green
    and         r4, r9, r4, lsr #19             // extract src blue

    and         r6, r9, r8, lsr #11             // extract dest red
    and         r7, r10, r8, lsr #5             // extract dest green
    and         r8, r9, r8                      // extract dest blue

    smlawb      r6, r5, r6, r11                 // r: ((dst*(alpha<<8))>>16) + src
    smlawb      r7, r5, r7, r12                 // g: ((dst*(alpha<<8))>>16) + src
    smlawb      r8, r5, r8, r4                  // b: ((dst*(alpha<<8))>>16) + src

    orr         r8, r8, r7, lsl #5              // or green into blue
    orr         r8, r8, r6, lsl #11             // or red into blue

    strh        r8, [r0], #2                    // store pixel to dest, update ptr
    bne         3b                              // if count != 0, loop
4:

    pop         {r4-r11, pc}                    // return

#endif     // __ARM_NEON__
