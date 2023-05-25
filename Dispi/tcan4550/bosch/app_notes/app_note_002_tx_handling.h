/* Application Note 002 - Tx handling */

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


#ifndef APP_NOTE_002_TX_HANDLING_H_
#define APP_NOTE_002_TX_HANDLING_H_

//MZ#include "../global_includes.h"

/* Example 1: Configure and use Tx FIFO*/
void m_can_an002_transmit_messages_tx_fifo();

/* Example 2: Configure and use Tx QUEUE and Dedicated Tx Buffers */
void m_can_an002_transmit_messages_tx_queue_and_ded_tx_buffers();

/* Example 3: Demonstration of Tx Event FIFO operation */
void m_can_an002_tx_event_fifo_handling();

/* Example 4: Demonstrates Cancelation of a requested Transmission */
void m_can_an002_tx_buffer_cancellation();

#endif /* APP_NOTE_002_TX_HANDLING_H_ */
