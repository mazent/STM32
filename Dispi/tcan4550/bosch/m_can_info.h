/* Structure to store SoC related M_CAN IP Module properties */

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

#ifndef M_CAN_INFO_H_
#define M_CAN_INFO_H_

#include <stdint.h>
#include "global_settings.h"

#if __ALTERA
struct m_can_info
{
	alt_u32 int_ctrl_id;      // interrupt controller ID
	alt_u32 int_num;          // interrupt number
	alt_u32 m_can_base;       // M_CAN base address
	alt_u32 mram_base;        // Message RAM base address
	alt_u32 mram_size_words;  // Message RAM size (in words)
};

#elif __XILINX
struct m_can_info
{
	uint32_t int_ctrl_id;     // interrupt controller ID
	uint32_t int_num;         // interrupt number
	uint32_t m_can_base;      // M_CAN base address
	uint32_t mram_base;       // Message RAM base address
	uint32_t mram_size_words; // Message RAM size (in words)
};
#endif



#endif /* M_CAN_INFO_H_ */
