/* XILINX Specific Redefines */
 
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

#ifndef XILINX_SPECIFIC_H
#define XILINX_PECIFIC_H
 
// Redefine some basic ALTERA things to operate in a XILINX SopC environment

#if __XILINX

/* ALTERA printf => XILINX printf: xil_printf = a smaller footprint printf */
#define printf xil_printf

// ALTERA Write ==> XILINX Write
#define IOWR_32DIRECT(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

// ALTERA Read ==> XILINX Read
#define IORD_32DIRECT(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + ((RegOffset)) )

#endif // __XILINX


#endif /*XILINX_SPECIFIC_H*/
