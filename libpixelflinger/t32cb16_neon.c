/* libs/pixelflinger/t32cb16_neon.S                                                       
**
** Copyright 2009, 0xlab.org - http://0xlab.org
** Copyright 2009, The Android Open Source Project
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
*/

#include <arm_neon.h>

void scanline_t32cb16_neon(uint16_t * __restrict dst,
                           uint32_t * __restrict src, size_t count)
{
    if (count >= 8) {
        uint16_t * __restrict keep_dst = 0;
        
        asm volatile (
            "ands       ip, %[count], #7            \n\t"
            "vmov.u8    d31, #1<<7                  \n\t"
            "vld1.16    {q12}, [%[dst]]             \n\t"
            "vld4.8     {d0-d3}, [%[src]]           \n\t"
            "moveq      ip, #8                      \n\t"
            "mov        %[keep_dst], %[dst]         \n\t"
                      
            "add        %[src], %[src], ip, LSL#2   \n\t"
            "add        %[dst], %[dst], ip, LSL#1   \n\t"
            "subs       %[count], %[count], ip      \n\t"
            "b          9f                          \n\t"
            // LOOP
            "2:                                         \n\t"
                      
            "vld1.16    {q12}, [%[dst]]!            \n\t"
            "vld4.8     {d0-d3}, [%[src]]!          \n\t"
            "vst1.16    {q10}, [%[keep_dst]]        \n\t"
            "sub        %[keep_dst], %[dst], #8*2   \n\t"
            "subs       %[count], %[count], #8      \n\t"
            "9:                                         \n\t"
            "pld        [%[dst],#32]                \n\t"
            // expand 0565 q12 to 8888 {d4-d7}
            "vmovn.u16  d4, q12                     \n\t"
            "vshr.u16   q11, q12, #5                \n\t"
            "vshr.u16   q10, q12, #6+5              \n\t"
            "vmovn.u16  d5, q11                     \n\t"
            "vmovn.u16  d6, q10                     \n\t"
            "vshl.u8    d4, d4, #3                  \n\t"
            "vshl.u8    d5, d5, #2                  \n\t"
            "vshl.u8    d6, d6, #3                  \n\t"
                      
            "vmovl.u8   q14, d31                    \n\t"
            "vmovl.u8   q13, d31                    \n\t"
            "vmovl.u8   q12, d31                    \n\t"
                      
            // duplicate in 4/2/1 & 8pix vsns
            "vmvn.8     d30, d3                     \n\t"
            "vmlal.u8   q14, d30, d6                \n\t"
            "vmlal.u8   q13, d30, d5                \n\t"
            "vmlal.u8   q12, d30, d4                \n\t"
            "vshr.u16   q8, q14, #5                 \n\t"
            "vshr.u16   q9, q13, #6                 \n\t"
            "vaddhn.u16 d6, q14, q8                 \n\t"
            "vshr.u16   q8, q12, #5                 \n\t"
            "vaddhn.u16 d5, q13, q9                 \n\t"
            "vqadd.u8   d6, d6, d0                  \n\t"  // moved up
            "vaddhn.u16 d4, q12, q8                 \n\t"
            // intentionally don't calculate alpha
            // result in d4-d6
                      
            "vqadd.u8   d5, d5, d1                  \n\t"
            "vqadd.u8   d4, d4, d2                  \n\t"
                      
            // pack 8888 {d4-d6} to 0565 q10
            "vshll.u8   q10, d6, #8                 \n\t"
            "vshll.u8   q3, d5, #8                  \n\t"
            "vshll.u8   q2, d4, #8                  \n\t"
            "vsri.u16   q10, q3, #5                 \n\t"
            "vsri.u16   q10, q2, #11                \n\t"
                      
            "bne        2b                          \n\t"
                      
            "1:                                         \n\t"
            "vst1.16      {q10}, [%[keep_dst]]      \n\t"
            : [count] "+r" (count)
            : [dst] "r" (dst), [keep_dst] "r" (keep_dst), [src] "r" (src) 
            : "ip", "cc", "memory",
              "d0",  "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
        );
    }
    else {
        // handle count < 8
        uint16_t * __restrict keep_dst = 0;
        
        asm volatile (
            "vmov.u8    d31, #1<<7                  \n\t"
            "mov        %[keep_dst], %[dst]         \n\t"
                      
            "tst        %[count], #4                \n\t"
            "beq        14f                         \n\t"
            "vld1.16    {d25}, [%[dst]]!            \n\t"
            "vld1.32    {q1}, [%[src]]!             \n\t"
                      
            "14:                                        \n\t"
            "tst        %[count], #2                \n\t"
            "beq        12f                         \n\t"
            "vld1.32    {d24[1]}, [%[dst]]!         \n\t"
            "vld1.32    {d1}, [%[src]]!             \n\t"
                      
            "12:                                        \n\t"
            "tst        %[count], #1                \n\t"
            "beq        11f                         \n\t"
            "vld1.16    {d24[1]}, [%[dst]]!         \n\t"
            "vld1.32    {d0[1]}, [%[src]]!          \n\t"
                      
            "11:                                        \n\t"
            // unzips achieve the same as a vld4 operation
            "vuzpq.u16  q0, q1                      \n\t"
            "vuzp.u8    d0, d1                      \n\t"
            "vuzp.u8    d2, d3                      \n\t"
            // expand 0565 q12 to 8888 {d4-d7}
            "vmovn.u16  d4, q12                     \n\t"
            "vshr.u16   q11, q12, #5                \n\t"
            "vshr.u16   q10, q12, #6+5              \n\t"
            "vmovn.u16  d5, q11                     \n\t"
            "vmovn.u16  d6, q10                     \n\t"
            "vshl.u8    d4, d4, #3                  \n\t"
            "vshl.u8    d5, d5, #2                  \n\t"
            "vshl.u8    d6, d6, #3                  \n\t"
                      
            "vmovl.u8   q14, d31                    \n\t"
            "vmovl.u8   q13, d31                    \n\t"
            "vmovl.u8   q12, d31                    \n\t"
                      
            // duplicate in 4/2/1 & 8pix vsns
            "vmvn.8     d30, d3                     \n\t"
            "vmlal.u8   q14, d30, d6                \n\t"
            "vmlal.u8   q13, d30, d5                \n\t"
            "vmlal.u8   q12, d30, d4                \n\t"
            "vshr.u16   q8, q14, #5                 \n\t"
            "vshr.u16   q9, q13, #6                 \n\t"
            "vaddhn.u16 d6, q14, q8                 \n\t"
            "vshr.u16   q8, q12, #5                 \n\t"
            "vaddhn.u16 d5, q13, q9                 \n\t"
            "vqadd.u8   d6, d6, d0                  \n\t"  // moved up
            "vaddhn.u16 d4, q12, q8                 \n\t"
            // intentionally don't calculate alpha
            // result in d4-d6
                      
            "vqadd.u8   d5, d5, d1                  \n\t"
            "vqadd.u8   d4, d4, d2                  \n\t"
                      
            // pack 8888 {d4-d6} to 0565 q10
            "vshll.u8   q10, d6, #8                 \n\t"
            "vshll.u8   q3, d5, #8                  \n\t"
            "vshll.u8   q2, d4, #8                  \n\t"
            "vsri.u16   q10, q3, #5                 \n\t"
            "vsri.u16   q10, q2, #11                \n\t"
                      
            // store
            "tst        %[count], #4                \n\t"
            "beq        24f                         \n\t"
            "vst1.16    {d21}, [%[keep_dst]]!       \n\t"
                      
            "24:                                        \n\t"
            "tst        %[count], #2                \n\t"
            "beq        22f                         \n\t"
            "vst1.32    {d20[1]}, [%[keep_dst]]!    \n\t"
                      
            "22:                                        \n\t"
            "tst        %[count], #1                \n\t"
            "beq        21f                         \n\t"
            "vst1.16    {d20[1]}, [%[keep_dst]]!    \n\t"
                      
            "21:                                        \n\t"
            : [count] "+r" (count)
            : [dst] "r" (dst), [keep_dst] "r" (keep_dst), [src] "r" (src)
            : "ip", "cc", "memory",
              "d0",  "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
        );
    }
}
