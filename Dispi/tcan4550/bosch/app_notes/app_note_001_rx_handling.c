/* M_CAN Application Note 001 - Rx handling */

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

// Simple example to demonstrate how to configure the M_CAN for receiving messages
void m_can_an001_receive_messages_simple()
{
	/* Description:
	 * 	- Two M_CAN Nodes participate in the Test
	 * 	- M_CAN 0 transmits messages (Tx messages are hard coded in the structure TX_messages)
	 * 	- M_CAN 1 is the receiver. M_CAN 1 receives all messages in FIFO0 as per global filter configuration
	 * 	- Messages received by M_CAN 1 are displayed
	 *
	 *  - Hint: M_CAN Interrupt Service Routine copies Messages from M_CAN to Software-List
	 */

	// Structure defines the 16 messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//direction		  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	brs    / 	dlc  /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x120     , .mm=0x1 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x121	  , .mm=0x2 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x122	  , .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x123	  , .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x124	  , .mm=0x5 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x125     , .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x126	  , .mm=0x20,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x127	  , .mm=0x30,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x128     , .mm=0x40,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x129     , .mm=0x50,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x12a     , .mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x12b	  , .mm=0x60,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  }	},
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x12c	  , .mm=0x65,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x12d	  , .mm=0x70,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1111FFFF, .mm=0x75,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1ABCDECF, .mm=0x80,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x12345678, .mm=0x81,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x12ABBA21, .mm=0x82,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1FFFFFFF, .mm=0x83,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
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

	printf("\n\n=== Entered function m_can_an001_receive_messages_simple() ===\n");

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

    // Configure TX Buffers
    // Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
    m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , MAX_TX_BUFFER_ELEMS,  0             , BYTE64); // apply configuration in M_CAN

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

	printf("\n[M_CAN_%d] Number of messages transmitted: %d\n", tx_node, tx_msg_count);

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



// Example to demonstrate Acceptance filtering and handling of received messages
void m_can_an001_receive_messages_with_filtering()
{
	/* Description:
	 *  - Two M_CAN Nodes participate in the Test
	 *  - M_CAN 0 transmits messages (Tx messages are hard coded in the structure TX_messages)
	 *  - M_CAN 1 is the receiver. M_CAN 1 receives messages in FIFO0, FIFO1 or in a dedicated Rx buffer as per the global/standard/extended filters configured
	 *  - Messages received by M_CAN 1 are displayed
	 *
	 *  - Hint: M_CAN Interrupt Service Routine copies Messages from M_CAN to Software-List
	 */

	/* type definition for Standard message ID filter element */
	typedef struct {
		unsigned int				 	index; // filter element will be stored in the message RAM at an offset calculated with this index
		unsigned int   					SFID1; // Standard Filter ID 1
		unsigned int   					SFID2; // Standard Filter ID 2
		SFT_Standard_Filter_Type_enum	SFT;   // Standard Filter Type
		Filter_Configuration_enum 		SFEC;  // Standard Filter Element Configuration
	} struct_standard_id_filter;

	/* Standard ID filter parameters for M_CAN 1 */
	const struct_standard_id_filter M_CAN_1_std_filters[]={
	//index/ SFID1/ SFID2/ 		    SFT		    /    		SFEC
		{ 0, 0x017, 0x019, STD_FILTER_TYPE_RANGE, FILTER_ELEMENT_REJECT_ID       }, // reject messages with IDs in range 0x17 to 0x19
		{ 1, 0x014, 0x01A, STD_FILTER_TYPE_RANGE, FILTER_ELEMENT_STORE_IN_FIFO0  }, // filter messages with IDs in range 0x014 to 0x1A and store in FIFO0
		{ 2, 0x184, 0x187, STD_FILTER_TYPE_DUAL , FILTER_ELEMENT_STORE_IN_FIFO1  }, // filter messages with IDs 0x184 or 0x187 store in FIFO1
		{ 3, 0x189, 0x189, STD_FILTER_TYPE_DUAL , FILTER_ELEMENT_STORE_IN_FIFO0  }, // filter message with ID 0x189 and store in FIFO0
		{ 4, 0x200, 0x39F, STD_FILTER_TYPE_CLASSIC, FILTER_ELEMENT_STORE_IN_FIFO0}, // filter messages according to the ID and mask and store in FIFO0
		{ 5, 0x201,	0x39F, STD_FILTER_TYPE_CLASSIC, FILTER_ELEMENT_REJECT_ID 	 }, // reject messages according to the ID and mask
		{ 6, 0x325, 0x02,  STD_FILTER_TYPE_DUAL, FILTER_ELEMENT_STORE_IN_RX_BUFFER_OR_DEBUG_MSG }, // filter messages with ID 0x325 and store in dedicated Rx buffer at index 0x2
		{ 7, 0x326, 0x05,  STD_FILTER_TYPE_DUAL, FILTER_ELEMENT_STORE_IN_RX_BUFFER_OR_DEBUG_MSG }, // filter messages with ID 0x326 and store in dedicated Rx buffer at index 0x5
	};

	/* type definition for Extended message ID filter element */
	typedef struct {
			unsigned int 					index;	// filter element will be stored in the message RAM at an offset calculated with this index
			uint32_t   						EFID1;	// extended filter ID 1
			uint32_t  						EFID2;	// extended filter ID 2
			EFT_Extended_Filter_Type_enum	EFT;	// extended filter type
			Filter_Configuration_enum 		EFEC;	// extended filter element configuration
		} struct_extended_id_filter;

	/* Extended ID filter parameters for M_CAN 1 */
	const struct_extended_id_filter M_CAN_1_xtd_filters[]={
	//index/   EFID1  /     EFID2   / 		   EFT	         /    		EFEC
		{  8, 0x12222222,       0x3E, EXTD_FILTER_TYPE_DUAL  , FILTER_ELEMENT_STORE_IN_RX_BUFFER_OR_DEBUG_MSG }, // filter message with id 0x12222222 and store in dedicated Rx buffer at index 0x3E
		{  9, 0x19999999, 0x1BBBBBBB, EXTD_FILTER_TYPE_RANGE , FILTER_ELEMENT_STORE_IN_FIFO1 }, // filter messages with ids in range 0x19999999 to 0x1BBBBBBB and store in FIFO1
		{ 10, 0x1FFABCDE, 0x1FFABCDE, EXTD_FILTER_TYPE_DUAL  , FILTER_ELEMENT_STORE_IN_FIFO0 }, // filter message with id 0x1FFABCDE and store in FIFO-0
	    { 11, 0x16666666, 0x16666666, EXTD_FILTER_TYPE_RANGE_XIDAM_MASK_NOT_APPLIED , FILTER_ELEMENT_STORE_IN_FIFO1 } // filter message with id 0x16666666 and store in FIFO1
	};

	// messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//direction		  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	brs    / 	dlc  /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x014, 		.mm=0x1 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=1  , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x015, 		.mm=0x2 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x016, 		.mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x017, 		.mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x018, 		.mm=0x5 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x019, 		.mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x01A, 		.mm=0x20,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x184, 		.mm=0x30,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x187, 		.mm=0x40,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=9  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x189, 		.mm=0x50,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=10 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x200, 		.mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=8  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x201, 		.mm=0x60,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=7  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  }	},
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x220, 		.mm=0x65,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x221, 		.mm=0x70,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x240, 		.mm=0x75,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x241, 		.mm=0x80,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x260, 		.mm=0x85,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x0261, 	.mm=0x86,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x0325, 	.mm=0x87,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x0326, 	.mm=0x88,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x0327, 	.mm=0x89,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=6  , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x12222222, .mm=0x90,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=5  , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x19999999, .mm=0x91,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=4  , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1FFABCDE, .mm=0x92,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=3  , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x16666666, .mm=0x93,  .efc=FALSE, .fdf=TRUE, .brs=TRUE,  .dlc=2  , .data={  0x11, 0x22  } },
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

	printf("\n\n=== Entered function m_can_an001_receive_messages_with_filtering() ===\n");

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

    // Configure TX Buffers
    // Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
    m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , MAX_TX_BUFFER_ELEMS, 0              , BYTE64); // apply configuration in M_CAN

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

   	//Configure GFC to accept Non-matching frames in Rx FIFO1
    m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO1, REJECT_NON_MATCHING_FRAMES, TRUE, TRUE);
    // configure standard message Id filter usage
    m_can_filter_init_standard_id(&global.can[rx_node], MAX_11_BIT_FILTER_ELEMS);
    // configure extended message Id filter usage
    m_can_filter_init_extended_id(&global.can[rx_node], MAX_29_BIT_FILTER_ELEMS);

    // RX FIFO Configuration
    // Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size
    m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, 10       , BYTE64);	// configure RX FIFO0
    m_can_rx_fifo_init(&global.can[rx_node], 1       , MAX_RX_FIFO_1_ELEMS, 10       , BYTE64);	// configure RX FIFO1

    // RX dedicated Buffer Configuration
    m_can_rx_dedicated_buffers_init(&global.can[rx_node], BYTE64);

    // write the Standard Id Filters for M_CAN 1 into the message RAM
    for(i=0; i<ARRAYSIZE(M_CAN_1_std_filters);i++) {
    	m_can_filter_write_standard_id(&global.can[rx_node], M_CAN_1_std_filters[i].index, M_CAN_1_std_filters[i].SFT,
    							M_CAN_1_std_filters[i].SFEC, M_CAN_1_std_filters[i].SFID1, M_CAN_1_std_filters[i].SFID2);
    }

    // write the Extended message Id Filters for M_CAN 1 into the message RAM
    for(i=0; i<ARRAYSIZE(M_CAN_1_xtd_filters);i++) {
    	m_can_filter_write_extended_id(&global.can[rx_node], M_CAN_1_xtd_filters[i].index, M_CAN_1_xtd_filters[i].EFT,
    								 M_CAN_1_xtd_filters[i].EFEC, M_CAN_1_xtd_filters[i].EFID1, M_CAN_1_xtd_filters[i].EFID2);
    }


    /* ======== Step - 2: Software related Initialization ===== */
    // reset the software message list that holds the received messages
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

	printf("\n[M_CAN_%d] Number of messages transmitted: %d\n", tx_node, tx_msg_count);


	/* Hint: A transmitted message will be received by M_CAN-1 in the Rx FIFO 0/1 or in a dedicated Rx Buffer depending on a matching filer element.
	 * 		 Interrupt flags triggered are:
	 * 		 	- IR.RF0N: Rx FIFO 0 New Message
	 * 		 	- IR.RF1N: Rx FIFO 1 New Message
	 * 		 	- IR.DRX:  Message Stored To Dedicated Rx Buffer
	 *       The interrupt service routine associated with these interrupt flags, copies the message to the software internal message list. */


	/* ======== STEP - 5: Print the received messages ======== */
	printf("\n[M_CAN_%d] Received messages:\n", rx_node);
	rx_msg_counter = 0;

	// while "not all messages transmitted" => wait to ensure all the messages received by M_CAN will be displayed */
	while(reg_get(&global.can[tx_node], ADR_M_CAN_TXBRP) != 0)
	{
		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			rx_msg_counter++;
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node])); // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 		 // Remove Head Message from Message List
		} // while
	} //while(1)
}



// Example to demonstrate handling of a high priority messages
void m_can_an001_high_priority_message_handling()
{
	/* Description: demonstrates one approach to handle the reception of a high priority message.
	 *  - Acknowledging the read of a high priority message has to be done with care. (See Application Note.)
	 *  - Approach followed in this example: All messages currently in RX FIFO will be read and acknowledged:
	 *  	1. When HPM interrupt occurs      -> To ensure that the high priority message is Read
	 *  	2. When FIFO watermark is reached -> To ensure that the messages are not too old
	 *
	 *  - Two M_CAN Nodes participate in the Test.
	 *  - M_CAN 0 transmits messages and M_CAN 1 receives all messages in RX FIFO0.
	 *  - M_CAN 1 uses a filter to trigger (IR.HPM) the reception of high priority messages.
	 *  - Messages received by M_CAN 1 are displayed
	 *
	 *  - Hint: M_CAN Interrupt Service Routine copies Messages from M_CAN to Software-List
	 */

	/* type definition for Standard message ID filter element */
	typedef struct {
		unsigned int				 	index; // filter element will be stored in the message RAM at an offset calculated with this index
		unsigned int   					SFID1; // Standard Filter ID 1
		unsigned int   					SFID2; // Standard Filter ID 2
		SFT_Standard_Filter_Type_enum	SFT;   // Standard Filter Type
		Filter_Configuration_enum 		SFEC;  // Standard Filter Element Configuration
	}struct_standard_id_filter;

	/* Standard id filter parameters for M_CAN 1: Used to identify the High Priority Message */
	const struct_standard_id_filter M_CAN_1_std_filter={
	//index/ SFID1/ SFID2/ 		    SFT		    /    		SFEC
	     0, 0x019, 0x019, STD_FILTER_TYPE_DUAL, FILTER_ELEMENT_SET_PRIORITY_AND_STORE_IN_FIFO0   // Set priority and store message with ID 0x19 in FIFO0
	};

	// Messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//direction		  / 	esi   / 	  idType       / 	 remote	  /   msg_id / msg mrkr/ evnt-fifo ctrl/  fdf  / 	brs    /  dlc  /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x015, .mm=0x2 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=2 , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x016, .mm=0x3 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=3 , .data={  0x11, 0x22, 0x33  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x017, .mm=0x4 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=4 , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x018, .mm=0x5 ,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=5 , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x020, .mm=0x10,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=6 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x01A, .mm=0x20,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=7 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x184, .mm=0x30,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=8 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x185, .mm=0x40,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=9 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x186, .mm=0x50,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=10, .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x187, .mm=0x52,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=9 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x189, .mm=0x55,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=8 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x200, .mm=0x60,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=7 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77  }	},
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x201, .mm=0x65,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=6 , .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x202, .mm=0x70,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=5 , .data={  0x11, 0x22, 0x33, 0x44, 0x55  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x203, .mm=0x75,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=4 , .data={  0x11, 0x22, 0x33, 0x44  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x204, .mm=0x80,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=3 , .data={  0x11, 0x22, 0x33  } },
		/* High Priority Message should be the last in the message list to ensure all messages are read out from FIFO 0 in this app-note */
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x019, .mm=0x85,  .efc=FALSE, .fdf=TRUE, .brs=TRUE, .dlc=2 , .data={  0x11, 0x22  } },
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

	printf("\n\n=== Entered function m_can_an001_high_priority_message_handling() ===\n");

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

	// Configure TX Buffers
	// Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
    m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , MAX_TX_BUFFER_ELEMS, 0              , BYTE64); // apply configuration in M_CAN

    /* ===== Configure Rx Node => M_CAN 1 ===== */
    global.can[rx_node].ena = TRUE; // enable node (software representation)

    // define the bit timings for Rx node
    global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

    // set the bit timings for Rx node
    m_can_set_bit_timing(&global.can[rx_node]);

    // IMPORTANT: initialize the interrupt registers for Rx node. Enable only HPM and RF0WL interrupt flags
    m_can_interrupt_init(&global.can[rx_node],
                        IR_HPM_HIGH_PRIORITY_MESSAGE + IR_RF0W_RX_FIFO0_WATERMARK_REACHED,		// Assign Interrupts HPM and RF0WL to INT_LINE_0
    					0x0,						// Assign NO Interrupts to INT_LINE_1
    					0xFFFFFFFF,					// Enable TX Buffer Transmission Interrupt
    					0xFFFFFFFF);				// Enable TX Buffer Cancellation Interrupt

    //Configure GFC to accept Non-matching frames in Rx FIFO0
    m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, REJECT_NON_MATCHING_FRAMES, TRUE, TRUE);
    // configure standard message Id filter usage: only 1 filter element used
    m_can_filter_init_standard_id(&global.can[rx_node], 1);

    // RX FIFO Configuration
    // Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size
    m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, 8       , BYTE64);	// configure RX FIFO0

    // write the Standard Id Filters for M_CAN 1 into the message RAM
    m_can_filter_write_standard_id(&global.can[rx_node], M_CAN_1_std_filter.index, M_CAN_1_std_filter.SFT, M_CAN_1_std_filter.SFEC, M_CAN_1_std_filter.SFID1, M_CAN_1_std_filter.SFID2);


    /* ======== Step - 2: Software related Initialization ===== */
    // reset the software message list that holds the received messages
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

	printf("\n[M_CAN_%d] Number of messages transmitted: %d\n", tx_node, tx_msg_count);


	/* Hint: A transmitted message will be received in the Rx FIFO 0 of M_CAN-1 depending on a matching standard ID filer element or Global filter configurations
	 * 		 Enabled Interrupt flags are:
	 * 		 	- IR.HPM:  High priority message
	 * 		 	- IR.RF0W: Rx FIFO 0 Watermark reached
	 *       The interrupt service routine associated with these interrupt flags, copies the message to the software internal message list. */


	/* ======== STEP - 5: Print the received messages ======== */
	printf("\n[M_CAN_%d] Received messages:\n", rx_node);

	// while "not all messages received" => wait to ensure all the messages received by M_CAN will be displayed */
	while(rx_msg_counter < tx_msg_count)
	{
		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			rx_msg_counter++;
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node])); // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 		 // Remove Head Message from Message List
		} // while
	} //
}



// Example demonstrates timeout counter usage with Rx FIFO0
void m_can_an001_timeoutcounter_usage_with_rx_fifo()
{
	/* Description:
	 * 	- Two M_CAN Nodes participate in the Test
	 * 	- M_CAN 0 transmits messages and M_CAN 1 receives messages in its Rx FIFO0
	 * 	- Rx FIFO0 controls the timeout counter.
	 * 	- IF messages are not read out from Rx FIFO0  after a certain amount of time, a timeout occurs. Based on the timeout the M_CAN sets the interrupt flag IR.TOO.
	 * 	  In this example the IR.TOO handled by simply reseting it. In a real application the user would need to react on the interrupt more specifically:
	 * 	..e.g. also read out RX FIFO0 immediately, or just delete the messages from RX FIFO0 because they are too old.
	 * 	- Received messages are displayed
	 */

	// Structure defines the messages that will be transmitted by M_CAN 0
	can_msg_struct TX_messages[]={
		//direction		  / 	esi   / 	  idType       / 	 remote	  /   	msg_id    / msg mrkr/ evnt-fifo ctrl/  fdf  / 	brs    / 	dlc  /    	data
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x120     , .mm=0x1 ,  .efc=FALSE, .fdf=TRUE,  .brs=FALSE,  .dlc=1 , .data={  0x11  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=standard_id, .remote=FALSE, .id=0x121	  , .mm=0x2 ,  .efc=FALSE, .fdf=FALSE, .brs=FALSE,  .dlc=2 , .data={  0x11, 0x22  } },
		{.direction=tx_dir, .esi=FALSE, .idtype=extended_id, .remote=FALSE, .id=0x1FFABCDE, .mm=0x3 ,  .efc=FALSE, .fdf=TRUE,  .brs=FALSE,  .dlc=10, .data={  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20  } }
		};

	const int tx_node = 0, rx_node = 1;
	int i, tx_msg_count = 0, rx_msg_counter = 0;

	// Nominal BT config preset - 0.5  Mbit/s @ 40 MHz
	const bt_config_struct nominal_bt_preset = {.brp=1, .prop_seg=47, .phase_seg1=16, .phase_seg2=16, .sjw=16, .tdc=0};

	// Data Phase BT config preset - 2.0 Mbit @ 40 MHz
	const bt_config_struct data_bt_preset = {.brp=1, .prop_seg=0, .phase_seg1=13, .phase_seg2=6, .sjw=6, .tdc=1, .tdc_offset=14, .tdc_filter_window=0};

	// loop to disable all M_CAN nodes in Software
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
    	global.can[i].ena = FALSE;
    }

	printf("\n\n=== Entered function m_can_an001_timeoutcounter_usage_with_rx_fifo() ===\n");

	/* ======== Step - 1: Configure the M_CANs ======== */
	// set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
	m_can_set_config_change_enable(&global.can[tx_node]);
	m_can_set_config_change_enable(&global.can[rx_node]);

	/* ===== Configure Tx Node => M_CAN 0 ===== */
    global.can[tx_node].ena = TRUE; // enable node (software representation)

    // define the bit timings for Tx node
	global.can[tx_node].bt_config.fd_ena  = TRUE;
	global.can[tx_node].bt_config.brs_ena = FALSE;
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

    // Configure TX Buffers
    // Parameters:       M_CAN node          , FIFO_true_QUEUE_false, no. FIFO elements  , no. ded Buffers, datafield size
    m_can_tx_buffer_init(&global.can[tx_node], TRUE                 , MAX_TX_BUFFER_ELEMS,  0             , BYTE64); // apply configuration in M_CAN

    /* ===== Configure Rx Node => M_CAN 1 ===== */
    global.can[rx_node].ena = TRUE; // enable node (software representation)

    // define the bit timings for Rx node
    global.can[rx_node].bt_config = global.can[tx_node].bt_config; // set same bit timings for the Tx and Rx nodes

    // set the bit timings for Rx node
    m_can_set_bit_timing(&global.can[rx_node]);

	// initialize the interrupt registers for Rx node
	m_can_interrupt_init(&global.can[rx_node],
                        IR_TOO_TIMEOUT_OCCURRED,// Assign to INT_LINE_0
						0x0,					// Assign to INT_LINE_1: NO Interrupts
						0xFFFFFFFF,				// Enable TX Buffer Transmission Interrupt
						0xFFFFFFFF);			// Enable TX Buffer Cancellation Interrupt

	// set Global filter parameters in Rx Node to accept all messages in Rx FIFO0
  	m_can_global_filter_configuration(&global.can[rx_node], ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0, TRUE, TRUE);

  	/* RX FIFO Configuration
     * Parameters:     M_CAN node          , FIFO 0/1, FIFO_size          , watermark, element Size */
	m_can_rx_fifo_init(&global.can[rx_node], 0       , MAX_RX_FIFO_0_ELEMS, 10       , BYTE64);

	// Configure and enable timeout counter: Timeout controlled by Rx FIFO0
	// Parameters:                                 can_ptr             , prescaler,  tscc_tss_timestamp_select                , tocc_tos_timeout_select                 , to_period, to_counter_enable
	m_can_timestampcounter_and_timeoutcounter_init(&global.can[rx_node], 0x10      , TSCC_TSS_TIMESTAMP_COUNTER_VALUE_ALWAYS_0, TOCC_TOS_TIMEOUT_CONTROLLED_BY_RX_FIFO_0, 0xFFFF   , TRUE);


    /* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
	// reset CCE and INIT: M_CANs will participate on the CAN bus
	m_can_reset_config_change_enable_and_reset_init(&global.can[tx_node]);
	m_can_reset_config_change_enable_and_reset_init(&global.can[rx_node]);


	/* ======== STEP - 4: Transmit messages via Tx FIFO ======== */
	while (tx_msg_count < ARRAYSIZE(TX_messages)) // loop to send messages via Tx FIFO
	{
		tx_msg_count += m_can_tx_fifo_queue_msg_transmit(&global.can[tx_node],  &TX_messages[tx_msg_count]);
	}

	printf("\n[M_CAN_%d] Number of messages transmitted: %d\n", tx_node, tx_msg_count);


	/* NOTE : Timeout is controlled by Rx FIFO0 in this example. If the received message in Rx FIFO0 is not read out even after a certain amount of time,
	 *        then the timeout will occur and interrupt flag IR.TOO will be set.
	 *        Some reasons for the delay in reading out a received message is typically an overload situation.
	 *        As example the M_CAN could assign his low priority interrupts (e.g. new Message in RX FIFO0) to InterruptLine0
	 *        and his high priority interrupts (e.g. TimeOut occured) to InterruptLine1.
	 *        If now the overall system is overloaded, the host CPU could not serve the low priority interrupts for a while. Consequently the IR.TOO could occur.
	 */


	/* ======== STEP - 5: Wait for IR.TOO to occur ======== */
	printf("Waiting for TimeOut Interrupt (IR.TOO) to occur...\n");

	while (global.can[rx_node].internal_test.timeout_occurred == FALSE)
	{
		// do nothing, just wait
	}

	if (global.can[rx_node].internal_test.timeout_occurred == TRUE) {
		printf("IR.TOO occurred :-)\n");
		// reset variable
		global.can[rx_node].internal_test.timeout_occurred = FALSE;
	} else {
		printf("This should never occur: global.can[rx_node].internal_test.timeout_occured == FALSE");
	}


	/* ======== STEP - 6: Print the received messages ======== */
	printf("\n[M_CAN_%d] Received messages:\n", rx_node);

	// while "not all messages received" => wait to ensure all the messages received by M_CAN will be displayed */
	while(rx_msg_counter < tx_msg_count)
	{
		// Copy all RX Messages from RX FIFO_0 to a Msg Data Structure, tell FIFO_0 that Msgs are read
		m_can_rx_fifo_copy_msg_to_msg_list(&global.can[rx_node], 0);

		while ( msg_list_is_empty(&global.can[rx_node]) == FALSE )
		{ // list not empty
			rx_msg_counter++;
			printf("[Message %2d]", rx_msg_counter);
			print_msg(msg_list_get_head_msg(&global.can[rx_node]));  // print the message
			msg_list_remove_head_msg(&global.can[rx_node]); 		 // Remove Head Message from Message List
		} // while
	} //while

}
