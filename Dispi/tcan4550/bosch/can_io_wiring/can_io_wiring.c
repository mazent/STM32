/* CAN IO WIRING driver */
 
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

#include "../global_includes.h"

/* CAN IO wiring: set mode */
void can_io_wiring_rxtx_mode_set(uint32_t mode)
{
	IOWR_32DIRECT(CAN_IO_WIRING_BASE, CAN_IO_WIRING_CFG_REG, mode);
}

/* CAN IO wiring: get mode */
uint32_t can_io_wiring_rxtx_mode_get()
{
    return (uint32_t)IORD_32DIRECT(CAN_IO_WIRING_BASE, CAN_IO_WIRING_CFG_REG);
}

/* CAN IO wiring: print mode */
void can_io_wiring_rxtx_mode_print()
{
    uint32_t mode;

    //read mode from config register
    mode = can_io_wiring_rxtx_mode_get();
    printf("CAN IO wiring mode: ");
    switch (mode) {
		case CAN_IO_WIRING_MODE_normal:
			printf("normal   (all CAN RX/TX-IOs connected to FPGA pins)\n");
			break;
		case CAN_IO_WIRING_MODE_internal:
			printf("internal (FPGA internal CAN bus, no external RX/TX connection)\n");
			break;
		case CAN_IO_WIRING_MODE_mixed:
			printf("mixed    (FPGA internal CAN bus, connected to external bus via pins of CAN_instance0)\n");
			break;
		default:
			printf("UNKNOWN!!! (this should never happen)\n");
			break;
	}
}
