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

#ifndef CAN_IO_WIRING_H_
#define CAN_IO_WIRING_H_

// CAN IO wiring
#define CAN_IO_WIRING_CFG_REG       0   // offset for config register
#define CAN_IO_WIRING_MODE_normal	0	// all CAN IOs connected to pins
#define CAN_IO_WIRING_MODE_internal 1   // FPGA internal can bus, no external connection
#define CAN_IO_WIRING_MODE_mixed    2   // FPGA internal can bus, connect to external bus via pins of CAN_instance0


/* CAN IO wiring: set mode */
void can_io_wiring_rxtx_mode_set(uint32_t mode);

/* CAN IO wiring: get mode */
uint32_t can_io_wiring_rxtx_mode_get();

/* CAN IO wiring: print mode */
void can_io_wiring_rxtx_mode_print();


#endif /*CAN_IO_WIRING_H_*/
