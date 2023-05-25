/* M_CAN Application Note 002 - Tx handling */

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

extern can_global_struct global;

#if __XILINX
	extern XIntc intc; // the Xilinx interrupt controller
#endif // __XILINX


/* Example 1: Configure and use Tx FIFO*/
void m_can_an002_transmit_messages_tx_fifo()
{
	/* Description:
	 * 	- M_CAN 0 transmits messages via Tx FIFO. (Tx messages are hard coded in the structure TX_messages)
	 * 	- Messages received by M_CAN 1 in its Rx FIFO0 are displayed
	 *
	 *  - Hint: M_CAN Interrupt Service Routine copies Messages from M_CAN to Software-List
	 */

	// Structure defines messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//direction		  / 	esi   / 	  idType       / 	 remote	  /   msg_id  / msg mrkr/evnt-fifo ctrl/   fdf  / 	brs    / 	dlc   /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x400 , .mm=0x1 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x200 , .mm=0x2 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x100 , .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x080 , .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x040 , .mm=0x5 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x020 , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x010 , .mm=0x20,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x008 , .mm=0x30,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } }
	};

	const int tx_node = 0, rx_node = 1;
	int i, rx_msg_counter = 0, tx_msg_count = 0;

	// Nominal BT config preset - 0.5  Mbit/s @ 40 MHz
	const bt_config_struct nominal_bt_preset = {.brp=1, .prop_seg=47, .phase_seg1=16, .phase_seg2=16, .sjw=16, .tdc=0};

	// Data Phase BT config preset - 2.0 Mbit @ 40 MHz
	const bt_config_struct data_bt_preset = {.brp=1, .prop_seg=0, .phase_seg1=13, .phase_seg2=6, .sjw=6, .tdc=1, .tdc_offset=14, .tdc_filter_window=0};

	// loop to disable all M_CAN nodes in Software
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		global.can[i].ena = FALSE;
	}

	printf("\n\n=== Entered function m_can_an002_transmit_messages_tx_fifo() ===\n");

	/* ======== Step - 1: Configure the M_CANs ======== */
	// set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
	m_can_set_config_change_enable(&global.can[tx_node]);
	m_can_set_config_change_enable(&global.can[rx_node]);

	/* ===== Configure Tx Node => M_CAN 0 ===== */
	global.can[tx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Tx node
	global.can[tx_node].bt_config.fd_ena  = TRUE;
	global.can[tx_node].bt_config.brs_ena = TRUE;
	global.can[tx_node].bt_config.nominal = nominal_bt_preset;
	global.can[tx_node].bt_config.data    = data_bt_preset;

	// set the bit timings for Tx node
	m_can_set_bit_timing(&global.can[tx_node]);

	// initialize the interrupt registers for Tx node
	m_can_interrupt_init(&global.can[tx_node],
			INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
			0x0,					// Assign to INT_LINE_1: NO Interrupts
			0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
			0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// Configure TX Buffers:
	// Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
	m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , 16				 ,  0             , BYTE64); // apply configuration in M_CAN

	/* ===== Configure Rx Node => M_CAN 1 ===== */
	global.can[rx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Rx node
	global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

	// set the bit timings for Rx node
	m_can_set_bit_timing(&global.can[rx_node]);

	// initialize the interrupt registers for Rx node
	m_can_interrupt_init(&global.can[rx_node],
			INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
			0x0,					// Assign to INT_LINE_1: NO Interrupts
			0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
			0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// set Global filter parameters in Rx Node to accept all messages in Rx FIFO0
	m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, TRUE, TRUE);

	/* RX FIFO Configuration
	 * Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size */
	m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, 10       , BYTE64);


	/* ======== Step - 2: Software related Initialisation ===== */
	/* reset the software message list that holds the received messages */
	rx_msg_list_reset();


	/* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
	// reset CCE and INIT: M_CANs will participate on the CAN bus
	m_can_reset_config_change_enable_and_reset_init(&global.can[tx_node]);
	m_can_reset_config_change_enable_and_reset_init(&global.can[rx_node]);


	/* ======== STEP - 4: Transmit messages via Tx FIFO ======== */
	while(tx_msg_count < ARRAYSIZE(TX_messages)) // loop to send messages via Tx FIFO
	{
		tx_msg_count += m_can_tx_fifo_queue_msg_transmit(&global.can[tx_node],  &TX_messages[tx_msg_count]);
	}
	printf("\n[M_CAN_%d] Number of messages transmitted : %d \n", tx_node, tx_msg_count);

	/* Hint: When a transmitted message is received in the Rx FIFO0, the interrupt flag IR.RF0N is set.
	 *       The interrupt service routine copies the message from the Rx FIFO0 to the software internal message list. */


	/* ======== STEP - 5: Print the received messages ======== */
	printf("\n[M_CAN_%d] Received messages:\n", rx_node);

	// loop to wait till all the transmitted messages are received
	while(rx_msg_counter < tx_msg_count)
	{
		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			rx_msg_counter++;
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node])); // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 	    // Remove Head Message from Message List
		} // while
	}//while
}


/* Example 2: Configure and use Tx QUEUE and Dedicated Tx Buffers */
void m_can_an002_transmit_messages_tx_queue_and_ded_tx_buffers()
{
	/* Description:
	 * 	- M_CAN 0 transmits messages via Tx Queue and dedicated Tx Buffers. (Tx messages are hard coded in the structure TX_messages)
	 * 	- Messages received by M_CAN 1 in its Rx FIFO0 are displayed
	 *
	 *  - Hint: M_CAN Interrupt Service Routine copies Messages from M_CAN to Software-List
	 */

	// Structure defines messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//   direction	  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	 brs   / 	dlc   /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x400     , .mm=0x1 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x200     , .mm=0x2 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x100     , .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x080     , .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x060     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x040     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x020     , .mm=0x20,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x010     , .mm=0x30,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x008     , .mm=0x40,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x004     , .mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x2a  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x002	  , .mm=0x60,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=11 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e  } }
	};

	const int tx_node = 0, rx_node = 1;
	int i, rx_msg_counter = 0, tx_msg_count = 0, last_tx_ded_buffer_index, tx_ded_buff_index, number_messages_via_ded_tx_buffer;

	// Nominal BT config preset - 0.5  Mbit/s @ 40 MHz
	const bt_config_struct nominal_bt_preset = {.brp=1, .prop_seg=47, .phase_seg1=16, .phase_seg2=16, .sjw=16, .tdc=0};

	// Data Phase BT config preset - 2.0 Mbit @ 40 MHz
	const bt_config_struct data_bt_preset = {.brp=1, .prop_seg=0, .phase_seg1=13, .phase_seg2=6, .sjw=6, .tdc=1, .tdc_offset=14, .tdc_filter_window=0};

	// loop to disable all M_CAN nodes in Software
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		global.can[i].ena = FALSE;
	}

	printf("\n\n=== Entered function m_can_an002_transmit_messages_tx_queue_and_ded_tx_buffers() ===\n");

	/* ======== Step - 1: Configure the M_CANs ======== */
	// set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
	m_can_set_config_change_enable(&global.can[tx_node]);
	m_can_set_config_change_enable(&global.can[rx_node]);

	/* ===== Configure Tx Node => M_CAN 0 ===== */
	global.can[tx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Tx node
	global.can[tx_node].bt_config.fd_ena  = TRUE;
	global.can[tx_node].bt_config.brs_ena = TRUE;
	global.can[tx_node].bt_config.nominal = nominal_bt_preset;
	global.can[tx_node].bt_config.data    = data_bt_preset;

	// set the bit timings for Tx node
	m_can_set_bit_timing(&global.can[tx_node]);

	// initialize the interrupt registers for Tx node
	m_can_interrupt_init(&global.can[tx_node],
			INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
			0x0,					// Assign to INT_LINE_1: NO Interrupts
			0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
			0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// Configure TX Buffers:
	// Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. Queue elements , no. ded Buffers, datafield size
	m_can_tx_buffer_init(&global.can[tx_node], FALSE                , 16                 , 16             , BYTE64); // apply configuration in M_CAN

	/* ===== Configure Rx Node => M_CAN 1 ===== */
	global.can[rx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Rx node
	global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

	// set the bit timings for Rx node
	m_can_set_bit_timing(&global.can[rx_node]);

	// initialize the interrupt registers for Rx node
	m_can_interrupt_init(&global.can[rx_node],
			INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
			0x0,					// Assign to INT_LINE_1: NO Interrupts
			0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
			0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// set Global filter parameters in Rx Node to accept all messages in Rx FIFO0
	m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, TRUE, TRUE);

	/* RX FIFO Configuration
	 * Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size */
	m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, 10       , BYTE64);


	/* ======== Step - 2: Software related Initialisation ===== */
	/* reset the software message list that holds the received messages */
	rx_msg_list_reset();


	/* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
	// reset CCE and INIT: M_CANs will participate on the CAN bus
	m_can_reset_config_change_enable_and_reset_init(&global.can[tx_node]);
	m_can_reset_config_change_enable_and_reset_init(&global.can[rx_node]);


	/* ======== STEP - 4: Transmit messages  ======== */
	// TX via dedicated TX Buffers
	last_tx_ded_buffer_index = (global.can[tx_node].tx_config.ded_buffers_number-1);
	tx_ded_buff_index = 0;
	number_messages_via_ded_tx_buffer = ARRAYSIZE(TX_messages) / 2;  // send half of the messages via dedicated tx buffers

	do {
		// Copy Transmit Message to dedicated buffer and request it
		tx_msg_count += m_can_tx_dedicated_msg_transmit(&global.can[tx_node], &TX_messages[tx_msg_count], tx_ded_buff_index);

		tx_ded_buff_index++;
	} while ((tx_ded_buff_index < last_tx_ded_buffer_index) &&    // stop if: end of dedicated buffer section is reached
			 (tx_msg_count < number_messages_via_ded_tx_buffer)); // stop if: number of messages intended to send via ded. tx buffer reached

	// Send the REMAINING messages via Tx Queue
	while(tx_msg_count < ARRAYSIZE(TX_messages)) // loop to send messages via Tx Queue
	{
		tx_msg_count += m_can_tx_fifo_queue_msg_transmit(&global.can[tx_node],  &TX_messages[tx_msg_count]);
	}

	printf("\n[M_CAN_%d] Number of messages transmitted : %d \n", tx_node, tx_msg_count);


	/* Hint: When a transmitted message is received in the Rx FIFO0, the interrupt flag IR.RF0N is set.
	 *       The interrupt service routine copies the message from the Rx FIFO0 to the software internal message list. */


	/* ======== STEP - 5: Print the received messages ======== */
	printf("\n[M_CAN_%d] Received messages:\n", rx_node);

	// loop to wait till all the transmitted messages are received
	while(rx_msg_counter < tx_msg_count)
	{
		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			rx_msg_counter++;
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node])); // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 		 // Remove Head Message from Message List
		} // while
	}//while
}


/* Example 3: Demonstration of Tx Event FIFO operation */
void m_can_an002_tx_event_fifo_handling()
{
	/* Description:
	 * 	- Two M_CAN Nodes participate in the Test
	 * 	- M_CAN_0 transmits messages (Tx messages are hard coded in the structure TX_messages)
	 * 	- Tx Event FIFO elements are configured for M_CAN_0.
	 * 	- M_CAN_1 is the receiver.
	 * 	- The contents in the Tx Event Fifo are displayed
	 *
	 * Hint:
	 *  - The Message Marker in the Buffer element will be copied into Tx Event FIFO element for identification of Tx message status
	 * 	- Only Tx messages with EFC=1 (Event FIFO Control) will be stored in the Tx Event FIFO
	 */

	/* This structure defines messages that will be transmitted by M_CAN 0
	 * Messages with efc = TRUE will be stored in Tx Event FIFO with its message marker */
	can_msg_struct TX_messages[]={
		//   direction	  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	 brs   / 	dlc   /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x400     , .mm=0x1 ,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x200     , .mm=0x2 ,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x100     , .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x080     , .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x060     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x040     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x020     , .mm=0x20,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x010     , .mm=0x30,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x008     , .mm=0x40,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x004     , .mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x2a  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1ABCBDEF, .mm=0x60,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=11 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1BBBBBBB, .mm=0x65,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=12 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x3a, 0x3b, 0x3c  } },
	};

	const int tx_node = 0, rx_node = 1;
	int i, msg_count = 0, tx_event_fifo_elem_counter = 0, tx_event_fifo_number_of_expected_elements = 0;

	// Nominal BT config preset - 0.5  Mbit/s @ 40 MHz
	const bt_config_struct nominal_bt_preset = {.brp=1, .prop_seg=47, .phase_seg1=16, .phase_seg2=16, .sjw=16, .tdc=0};

	// Data Phase BT config preset - 2.0 Mbit @ 40 MHz
	const bt_config_struct data_bt_preset = {.brp=1, .prop_seg=0, .phase_seg1=13, .phase_seg2=6, .sjw=6, .tdc=1, .tdc_offset=14, .tdc_filter_window=0};

	// loop to disable all M_CAN nodes
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
    	global.can[i].ena = FALSE;
    }

	printf("\n\n=== Entered function m_can_an002_tx_event_fifo_handling() ===\n");

	/* ======== Step - 1: Configure the M_CANs ======== */
	// set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
	m_can_set_config_change_enable(&global.can[tx_node]);
	m_can_set_config_change_enable(&global.can[rx_node]);

	/* ===== Configure Tx Node => M_CAN 0 ===== */
    global.can[tx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Tx node
	global.can[tx_node].bt_config.fd_ena  = TRUE;
	global.can[tx_node].bt_config.brs_ena = TRUE;
	global.can[tx_node].bt_config.nominal = nominal_bt_preset;
	global.can[tx_node].bt_config.data    = data_bt_preset;

	// set the bit timings for Tx node
	m_can_set_bit_timing(&global.can[tx_node]);

	// initialize the interrupt registers for Tx node
	m_can_interrupt_init(&global.can[tx_node],
						INTERRUPT_ALL_SIGNALS,	// Assign to INT_LINE_0: all
						0x0,					// Assign to INT_LINE_1: NO Interrupts
						0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
						0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

    // Configure TX Buffers
    // Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
    m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , MAX_TX_BUFFER_ELEMS, 0              , BYTE64); // apply Tx buffer configuration in M_CAN

    // Configure Tx event FIFO
    // Parameters:           can_ptr             , fifo_size_elements     , fifo_watermark
	m_can_tx_event_fifo_init(&global.can[tx_node], MAX_TX_EVENT_FIFO_ELEMS, 10            );

	// Configure registers for the timestamp
	// Hint: for Classical CAN: use internal OR external Timestamp Counter (in M_CAN Revision 3.2.1 or lower)
	//       for CAN FD:        use        ONLY external Timestamp Counter (in M_CAN Revision 3.2.1 or lower)
	m_can_timestampcounter_and_timeoutcounter_init(&global.can[tx_node], 0x9, TSCC_TSS_TIMESTAMP_COUNTER_VALUE_ACCORDING_TO_TCP, TOCC_TOS_CONTINUOUS_OPERATION, 0xFFFF, FALSE);

    /* ===== Configure Rx Node => M_CAN 1 ===== */
    global.can[rx_node].ena = TRUE; // enable node (software representation)

    // define the bit timings for Rx node
    global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

    // set the bit timings for Rx node
    m_can_set_bit_timing(&global.can[rx_node]);

    // initialize the interrupt registers for Rx node
    m_can_interrupt_init(&global.can[rx_node],
    					INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
    					0x0,					// Assign to INT_LINE_1: NO Interrupts
    					0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
    					0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

    // Configure GFC to accept Non-matching frames in Rx FIFO1
	m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, FALSE, FALSE);
	// RX FIFO Configuration
	// Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size
	m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS,     10   , BYTE64);


	/* ======== Step - 2: Software related Initializations ======== */
    // reset the software message list that holds the received messages
    rx_msg_list_reset();
    // reset the software message list that holds the Tx Event FIFO elements
    tx_event_list_reset();


    /* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
	// reset CCE and INIT: M_CANs will participate on the CAN bus
	m_can_reset_config_change_enable_and_reset_init(&global.can[tx_node]);
	m_can_reset_config_change_enable_and_reset_init(&global.can[rx_node]);


	/* ======== STEP - 4: Transmit messages via Tx FIFO ======== */
    while(msg_count < ARRAYSIZE(TX_messages))  // loop to send messages via Tx fifo
    {
    	// count the number of elements that should be stored in Tx Event FIFO
    	if (TX_messages[msg_count].efc == TRUE) {
    		tx_event_fifo_number_of_expected_elements++;
    	}
    	// transmit message
    	msg_count += m_can_tx_fifo_queue_msg_transmit(&global.can[tx_node],  &TX_messages[msg_count]);
    }
    printf("[M_CAN_%d] Number of messages transmitted: %d \n", tx_node, msg_count);
    printf("[M_CAN_%d] TX Event FIFO Entry requested for this Number of Messages: %d \n", tx_node, tx_event_fifo_number_of_expected_elements);

    /* Hint: Interrupt flag IR.TEFN is set when there is a new element in Tx Event FIFO.
     *       The interrupt service routine copies this new element from the Tx Event FIFO in message RAM to the software internal Tx Event list. */

    /* ======== STEP - 5: Print elements in the Tx Event List ======== */
    printf("\n Messages stored in the Tx Event FIFO:\n");
    while(tx_event_fifo_elem_counter < tx_event_fifo_number_of_expected_elements) // loop to wait till all suitable events are stored in the Tx Event FIFO
    { // instead of this loop we can have a while 1
    	while ( tx_event_list_is_empty(&global.can[tx_node]) == FALSE ) // loop to display all elements in list copied from Tx Event FIFO
    	{ // list not empty
			tx_event_fifo_elem_counter++;
    		printf("[Tx Event FIFO Element %d]", tx_event_fifo_elem_counter);
    		print_tx_event_fifo_element(tx_event_list_get_head_element(&global.can[tx_node]));
    		tx_event_list_remove_head_element(&global.can[tx_node]);	// Remove Head element from Tx Event FIFO List
    	} // while
    } // while
}



/* Example 4: Demonstrates Cancelation of a requested Transmission via Tx Buffer */
void m_can_an002_tx_buffer_cancellation()
{
	/* Description:
	 * - Two M_CAN Nodes participate in the Test: M_CAN_0, M_CAN_1
	 * - M_CAN_0 transmits 32 messages (Tx messages are hard coded in the structure TX_messages). M_CAN_1 is the receiver.
	 * - To have some pending tx message to be canceled, transmission is done the following way
	 *   - First  step: messages to be transmitted via dedicated tx buffers are written to the message RAM
	 *   - Second step: these messages are requested to be transmitted at once
	 * - Further messages are sent via the TX Queue
	 * - Some of the Tx messages are requested to be canceled through register TXBCR
	 * - The received messages are displayed
	 */

	//This structure defines the messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//   direction	  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	brs    / 	dlc  /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x004     , .mm=0x1 ,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x005     , .mm=0x2 ,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x006	  , .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x007	  , .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x008	  , .mm=0x5 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x009     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x010	  , .mm=0x20,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x020 	  , .mm=0x30,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x030     , .mm=0x40,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x040     , .mm=0x50,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x050     , .mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x060	  , .mm=0x60,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  }	},
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x070	  , .mm=0x65,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x080	  , .mm=0x70,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x090	  , .mm=0x75,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x100     , .mm=0x80,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x110     , .mm=0x81,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x120     , .mm=0x82,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x001	  , .mm=0x83,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x002	  , .mm=0x84,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x003	  , .mm=0x85,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1aaaaaaa, .mm=0x86,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1bbbbbbb, .mm=0x87,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1ccccccc, .mm=0x88,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1ddddddd, .mm=0x89,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1eeeeeee, .mm=0x8a,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1fffffff, .mm=0x8b,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1abcdeff, .mm=0x8c,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1fedcbaa, .mm=0x8d,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1f222222, .mm=0x8e,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1f333333, .mm=0x8f,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x10000000, .mm=0x90,  .efc=TRUE , .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
	};

	const unsigned int tx_node = 0, rx_node = 1;
	unsigned int i, rx_fifo_watermark = 10;
	unsigned int queue_size = 8;
	unsigned int num_of_dedicated_tx_buff = MAX_TX_BUFFER_ELEMS - queue_size;
	unsigned last_tx_ded_buffer_index, tx_ded_buff_index, msg_count, rx_msg_counter;
	boolean transmission_occurred = FALSE;
	uint32_t mask, request_transmission_mask_TXBAR = 0;
	uint32_t cancel_transmission_mask_TXBCR = 0;

	// Nominal BT config preset - 0.5  Mbit/s @ 40 MHz
	const bt_config_struct nominal_bt_preset = {.brp=1, .prop_seg=47, .phase_seg1=16, .phase_seg2=16, .sjw=16, .tdc=0};

	// Data Phase BT config preset - 2.0 Mbit @ 40 MHz
	const bt_config_struct data_bt_preset = {.brp=1, .prop_seg=0, .phase_seg1=13, .phase_seg2=6, .sjw=6, .tdc=1, .tdc_offset=14, .tdc_filter_window=0};

	// loop to disable all M_CAN nodes
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
    	global.can[i].ena = FALSE;
    }

	printf("\n\n=== Entered function m_can_an002_tx_buffer_cancellation() ===\n");

	/* ======== Step - 1: Configure the M_CANs ======== */
	// set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
	m_can_set_config_change_enable(&global.can[tx_node]);
	m_can_set_config_change_enable(&global.can[rx_node]);

	/* ===== Configure Tx Node => M_CAN 0 ===== */
    global.can[tx_node].ena = TRUE; // enable node (software representation)

	// define the bit timings for Tx node
	global.can[tx_node].bt_config.fd_ena  = TRUE;
	global.can[tx_node].bt_config.brs_ena = TRUE;
	global.can[tx_node].bt_config.nominal = nominal_bt_preset;
	global.can[tx_node].bt_config.data    = data_bt_preset;

	// set the bit timings for Tx node
	m_can_set_bit_timing(&global.can[tx_node]);

	// initialize the interrupt registers for Tx node
	m_can_interrupt_init(&global.can[tx_node],
						INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
						0x0,					// Assign to INT_LINE_1: NO Interrupts
						0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
						0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// TX Buffer Configuration
	// Parameters:       M_CAN node          , FIFO_true_QUEUE FALSE, no. FIFO elements, no. ded Buffers         , datafield size
	m_can_tx_buffer_init(&global.can[tx_node], FALSE                , queue_size       , num_of_dedicated_tx_buff, BYTE64); // apply configuration in M_CAN


	/* ===== Configure Rx Node => M_CAN 1 ===== */
    global.can[rx_node].ena = TRUE; // enable node (software representation)

    // define the bit timings for Rx node
    global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

    // set the bit timings for Rx node
    m_can_set_bit_timing(&global.can[rx_node]);

    // initialize the interrupt registers for Rx node
    m_can_interrupt_init(&global.can[rx_node],
    					INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,		// Assign to INT_LINE_0: all except the TimeStampWrapArround
    					0x0,					// Assign to INT_LINE_1: NO Interrupts
    					0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
    					0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

    //Configure GFC to accept Non-matching frames in Rx FIFO0
	m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, FALSE, FALSE);
	// RX FIFO Configuration
	// Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark        , element Size
	m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, rx_fifo_watermark, BYTE64);

	/* ===== Software related Initializations ===== */
    // reset the software message list that holds the received messages
    rx_msg_list_reset();


    /* ======== Step - 2: Connecting M_CANs to the CAN Bus ======== */
	// reset CCE and INIT: M_CANs will participate on the CAN bus
	m_can_reset_config_change_enable_and_reset_init(&global.can[tx_node]);
	m_can_reset_config_change_enable_and_reset_init(&global.can[rx_node]);


	/* ======== Step - 3: Write Tx messages into dedicated TX Buffers ========
	 *	 In the Message RAM, the Tx dedicated buffers comes first followed by Tx Queue buffer elements */
	last_tx_ded_buffer_index = (global.can[tx_node].tx_config.ded_buffers_number-1);
	tx_ded_buff_index = 0;
	msg_count = 0;

	// transmit messages via Tx dedicated buffers
	do {
		// check if Tx buffer is free. i.e.: Check if this Tx buffer has a pending TX Request
		if (m_can_tx_is_tx_buffer_req_pending(&global.can[tx_node], tx_ded_buff_index) == FALSE) {


			// Copy Tx Message to TX buffer - NO Transmission is requested
			m_can_tx_write_msg_to_tx_buffer(&global.can[tx_node], &TX_messages[msg_count], tx_ded_buff_index);

			// generate mask for the given tx buffer index
			mask = 1 << tx_ded_buff_index;

			// calculate mask_to_txbar to remember which all messages should be requested to be transmitted via TXBAR register
			request_transmission_mask_TXBAR = request_transmission_mask_TXBAR | mask;

			// increment number of msgs to be transmitted
			msg_count++;
		}
		tx_ded_buff_index++; // go to next dedicated buffer index

	} while (tx_ded_buff_index <= last_tx_ded_buffer_index); // stop if: last tx dedicated buffer element reached


	/* ======== Step - 4: Request transmission for messages in dedicated Tx buffers ======== */
	reg_set(&global.can[tx_node], ADR_M_CAN_TXBAR, request_transmission_mask_TXBAR); // set "add request bits" in TXBAR register


	/* ======== Step - 5: Write Tx messages into TX Queue and request transmission immediately ========
	 *   also a Tx Queue is used to transmit messages */

	// transmit REMAINING messages via Tx queue
	while (msg_count < ARRAYSIZE(TX_messages))
	{
		// write message to Tx Queue and request transmission immediately
		msg_count += m_can_tx_fifo_queue_msg_transmit(&global.can[tx_node],  &TX_messages[msg_count]);
	}


    /* ======== Step - 6: Request to cancel the transmission of some messages by writing to TXBCR register. ======== */
	// Cancel ALL pendig Tx Requests
	cancel_transmission_mask_TXBCR = reg_get(&global.can[tx_node], ADR_M_CAN_TXBRP);
	reg_set(&global.can[tx_node], ADR_M_CAN_TXBCR, cancel_transmission_mask_TXBCR);
    // Hint: use the following function to cancel tx requests one by one: m_can_tx_buffer_request_cancelation(can_struct * can_ptr, int tx_buf_index)


    /* ======== Step - 7: Print some information ======== */
	/* Some user information messages */
	printf("[M_CAN_%d] Number of messages transmitted: %d\n", tx_node, msg_count);
	printf("mask for TXBCR (Cancel Transmission)  : 0x%08x\n", cancel_transmission_mask_TXBCR);

	printf("\nCancellation Finished for Tx buffer indices : ");
	// for all the tx_messages
	for (i=0; i< ARRAYSIZE(TX_messages); i++) {
		if (m_can_tx_buffer_is_cancelation_finshed(&global.can[tx_node], i) == TRUE) {
			printf("%d ", i); // if cancellation finished, print the buffer index
		}
	}

	printf("\nTransmission Occurred in spite of cancellation request for Tx buffer indices : ");
	// for all the tx_messages
	for (i=0; i< ARRAYSIZE(TX_messages); i++) {
		if ( (m_can_tx_buffer_is_cancelation_finshed(&global.can[tx_node], i) == TRUE) &&
		     (m_can_tx_buffer_transmission_occured(&global.can[tx_node], i) == TRUE) )
		{
			printf("%d ", i); // if transmission occurred in spite of cancellation request, print the buffer index
			transmission_occurred = TRUE;
		}
	}
	if (transmission_occurred == FALSE)	{ printf("None\n"); }
	else { printf("\n"); }


    /* ======== STEP - 8: Print the received messages ======== */
	printf("\n[M_CAN_%d] Messages received:\n", rx_node);
	rx_msg_counter = 1;

	/* the following infinite loop acts as a wait to ensure all the messages received by M_CAN will be displayed. */
	while(1)
	{
		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node])); // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 		 // Remove Head Message from Message List
			rx_msg_counter++;
		} // while
	}
}
