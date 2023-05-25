/* Type definitions */
 
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

#ifndef TYPES_H_
#define TYPES_H_

#include "global_settings.h"  // has to be included prior to types.h, otherwise the Software doesn work with Xilinx Microblaze/SDK


typedef unsigned int can_reg_val_t;   // 4 byte, width of CAN Register values

/*  This definition calculates the number of elements of an array by
 *  deviding the total array size by the size of the first element.
 *
 *  \param x array variable
 */
#define ARRAYSIZE(x) ((int)(sizeof(x)/sizeof(x[0])))

#if __ALTERA
 typedef enum
 {
 	FALSE = 0,
	TRUE
 } boolean;
#elif __XILINX
 // Background: xbasic_types.h already defines FALSE = 0, TRUE = 1
 //             nevertheless, our code needs the type boolean
 //             now MFALSE = FALSE, MTRUE = TRUE.
 typedef enum
 {
 	MFALSE = 0,
 	MTRUE
 } boolean;
#else
 // MZ
 typedef enum
 {
 	FALSE = 0,
	TRUE
 } boolean;

#endif

#endif
