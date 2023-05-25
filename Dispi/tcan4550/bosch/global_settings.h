/* global setting */

/* Copyright (c) 2018, Robert Bosch GmbH, Gerlingen, Germany
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*     1. Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*     2. Redistributions in binary form must reproduce the above copyright
*        notice, this list of conditions and the following disclaimer in the
*        documentation and/or other materials provided with the distribution.
*     3. Neither the name of the Robert Bosch GmbH nor the
*        names of its contributors may be used to endorse or promote products
*        derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL the Robert Bosch GmbH BE LIABLE FOR ANY
*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
   Note: This software is snapshot of an ongoing project and serves as example.
*/

#ifndef GLOBAL_SETTINGS_H_
#define GLOBAL_SETTINGS_H_

// Hints for settings
// 0 = FALSE
// 1 = TRUE

// SET THE TYPE OF SoftcoreCPU USED
#define __ALTERA 0  // Altera NIOS II
#define __XILINX 0  // Xilinx Microblaze

// SET THE ENDIANNESS of your CPU here
#define __LITTLE_ENDIAN 1  // (Altera NIOS II, Xilinx Microblaze when AXI4 Bussystem used)
#define __BIG_ENDIAN    0  // (Xilinx Microblaze when PLB Bussystem used)


// MISC Defines
#define __DE1_SOC 0  // HELP DEFINE FOR DE1-SOC Board, E.g. used to control the 7segment display
#define __BOSCH_INTERNAL_TESTS 0 // If enabled (1), Bosch internal tests are also included

#endif // GLOBAL_SETTINGS_H_
