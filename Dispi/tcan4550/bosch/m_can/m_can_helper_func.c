/* M_CAN test cases - help functions */

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
#include "../m_can_info.h"

extern can_global_struct global;
extern struct m_can_info m_can_info_list[CAN_NUMBER_OF_PRESENT_NODES];
#if __XILINX
	extern XIntc intc; // the Xilinx interrupt controller
#endif // __XILINX

// data structure to hold messages after reception via CAN rx_node, RX processing picks up messages from here
rx_msg_list_struct    rx_msg_list[CAN_NUMBER_OF_PRESENT_NODES];
tx_event_list_struct  tx_event_list[CAN_NUMBER_OF_PRESENT_NODES];


#if __ALTERA
static void m_can_disable_interrupt(int can_id) {
	alt_u32 int_ctrl;
	alt_u32 int_id;

	assert_print_error((can_id < CAN_NUMBER_OF_PRESENT_NODES), "Tried to disable non-existent interrupt of id CAN %d", can_id);

	int_id = m_can_info_list[can_id].int_num;
	int_ctrl = m_can_info_list[can_id].int_ctrl_id;

	alt_ic_irq_disable(int_ctrl, int_id);
}

static void m_can_enable_interrupt(int can_id) {
	alt_u32 int_ctrl;
	alt_u32 int_id;

	assert_print_error((can_id < CAN_NUMBER_OF_PRESENT_NODES), "Tried to enable non-existent interrupt of id CAN %d", can_id);

	int_id = m_can_info_list[can_id].int_num;
	int_ctrl = m_can_info_list[can_id].int_ctrl_id;

	alt_ic_irq_enable(int_ctrl, int_id);
}
#elif __XILINX

static void m_can_disable_interrupt(int can_id)
{

	assert_print_error((can_id < CAN_NUMBER_OF_PRESENT_NODES), "Tried to disable non-existent interrupt of id CAN %d", can_id);
	XIntc_Disable(&intc, m_can_info_list[can_id].int_num);
}

static void m_can_enable_interrupt(int can_id)
{
	assert_print_error((can_id < CAN_NUMBER_OF_PRESENT_NODES), "Tried to enable non-existent interrupt of id CAN %d", can_id);
	XIntc_Enable(&intc, m_can_info_list[can_id].int_num);
}

#endif

/* compare messages for equal contents */
boolean msg_compare(can_msg_struct *msg1_ptr, can_msg_struct *msg2_ptr, M_CAN_operating_mode operating_mode)
{
	/* Description
	 * - Compares two messages for equality
	 *   => only the message content relevant fields of the can_msg_struct are compared
	 * - Based on the operation_mode parameter:
	 *   => some content relevant fields of can_msg_struct are included/excluded in comparison
	 *   => e.g. FDF, BRS
	 */

	int i, payload_bytes;

	/* compare message contents for equality */
	if ( (msg1_ptr->idtype == msg2_ptr->idtype) &&
		 (msg1_ptr->id     == msg2_ptr->id)     &&
	     (msg1_ptr->dlc    == msg2_ptr->dlc)
       )
	{
		// EQUAL Message ID, DLC

	    // compare Messsage Data (payload) for equality
		payload_bytes = convert_DLC_to_data_length(msg1_ptr->dlc);

		// In case of Classical_CAN only a max. of 8 bytes needs to be checked
		if ( (operating_mode == CLASSICAL_CAN) && (payload_bytes > CAN_CLASSIC_MAX_DLC) ) {
		    payload_bytes = CAN_CLASSIC_MAX_DLC;
		}

		for (i = 0; i < payload_bytes; ++i)
		{
			if (msg1_ptr->data[i] != msg2_ptr->data[i])
				return FALSE;
		}

	} else {
		// UN-EQUAL Message ID, DLC
		return FALSE;
	}

   // Bits FDF and BRS needs to be checked only if the corresponding mode is set
   if ((operating_mode == CAN_FD_WITH_BRS) || (operating_mode == CAN_FD_WITHOUT_BRS))
   {
       /* compare transmission format for equality */
       if ( (msg1_ptr->fdf == msg2_ptr->fdf) ) {
           // if fdf bit is equal, check if brs is set correctly
           if (msg1_ptr->fdf && operating_mode == CAN_FD_WITH_BRS) {
               if (msg1_ptr->brs != msg2_ptr->brs) {
                   // UN-EQUAL BRS
                   return FALSE;
               }
           } else {
			// no check - makes no sense
			// because the M_CAN ignores the BRS bit for transmission if FDF==FALSE,
			// consequently checking something that was ignored by the M_CAN and during transmission with the BRS in the received message makes no sense
           }
       } else {
           // UN-EQUAL FDF
           return FALSE;
       }
   }


	/* compare ESI Bit
	 *   comparison makes no sense, since we can control the ESI Bit in a TX Message only in case,
	 *   when the protocol controller is NOT in ERROR_PASSIVE or BUS_OFF state
	 */

	/* at this point in the function: the messages are equal */
	return TRUE;
}



/* calculate number of arbitration phase bits of a CAN frame (without dynamic stuff bits!) */
unsigned int frame_get_bit_count_of_nominal_bits(can_msg_struct *can_msg)
{
    /* Arbitration Phase (Without Dynamic Stuff Bits)
       - all bits/bit fields considered: e.g. SOF, ID, ACK, EOF, IFS(ITM)
       - Dynamic Stuff Bits not considered
       - Bit Counts:                  Standard ID   Extended ID
            Classic CAN             : 28            48
            CAN FD Payload<=16 Byte : 29            48
            CAN FD Payload >16 Byte : 29            48*/
    unsigned int nominal_bits_in_msg = 0;

    if (can_msg->id == standard_id) {
        // Classical CAN uses 28 bits for standard ID and CAN_FD uses 29 bits
        if (can_msg->fdf == TRUE) {
            nominal_bits_in_msg = 29; // Standard ID in CAN FD frame
        } else {
            nominal_bits_in_msg = 28; // Standard ID in Classical CAN frame
        }
    } else {
        // this means the frame has an extended ID consisting of 48 bits
        nominal_bits_in_msg = 48;
    }

    return nominal_bits_in_msg;
}



/* calculate the number of data phase bits of a CAN frame (without dynamic stuff bits!) */
unsigned int frame_get_bit_count_of_data_bits(can_msg_struct *can_msg)
{
    /* Data Phase (Without Dynamic Stuff Bits)
       - all bits/bit fields considered
       - Dynamic Stuff Bits not considered
       - Bit Counts:                Overhead                      Payload
                                    ---------------------------   -------------
                                    ESI,DLC,CRCdlm   CRC Field    Data (n bytes)
          Classic CAN             : 4                15           n*8
          CAN FD Payload<=16 Byte : 6                21           n*8
          CAN FD Payload >16 Byte : 6                25           n*8     */
    unsigned int data_bits_in_msg = 0, no_bytes_payload;
    unsigned int no_bits_esi_dlc_CRCdlm, no_bits_CRC_fld;

    no_bytes_payload = convert_DLC_to_data_length(can_msg->dlc);

    if (can_msg->fdf == TRUE) {
        // CAN FD Frame
        no_bits_esi_dlc_CRCdlm = 6;

        if ( no_bytes_payload > 16 ) {
            no_bits_CRC_fld = 25;
        } else {
            no_bits_CRC_fld = 21;
        }

    } else {
        // Classical CAN Frame
        no_bits_esi_dlc_CRCdlm = 4;
        no_bits_CRC_fld = 15;
    }

   data_bits_in_msg = no_bits_esi_dlc_CRCdlm + no_bits_CRC_fld + no_bytes_payload*8; // Total no. of bits in the message

    return data_bits_in_msg;
}



/* Message List ################################################# */

// MESSAGE LIST PROPERTIES:
//   list empty:       write_ptr == read_ptr  AND  full= FALSE
//   list full:        write_ptr == read_ptr  AND  full==TRUE
//   list partly full: write_ptr != read_ptr
//   who accesses the list?
//   - ISR:              adds elements    and updates list flags
//   - user code:        removes elements and updates list flags

/* Reset the RX MSG List */
void rx_msg_list_reset()
{
	/* Set internal "pointers" to consistent values - as needed for an empty list */
	int i;

	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		rx_msg_list[i].next_free_elem = 0;
		rx_msg_list[i].last_full_elem = 0;
		rx_msg_list[i].full = FALSE;
	}
}


/* Copy Message from Message RAM to Message List */
void copy_msg_from_msg_ram_to_msg_list(can_struct *can_ptr, uint32_t msg_addr_in_msg_ram, rx_info_struct rx_info)
{
	// Tasks: - Alarm: List full
	//        - copy msg to rx_msg_list

	// check if list is full
	if ( msg_list_is_full(can_ptr) )
	{
#if ERROR_PRINT
		DBG_PRINTF("[ERROR] Message List of M_CAN_%d is FULL! Message is dropped.", can_ptr->id);
#endif
		return;
	}

	// list is not full - at this point

	// copy message from CAN node to RX message list element
	m_can_copy_msg_from_msg_ram(can_ptr, msg_addr_in_msg_ram, msg_list_get_next_free_element(can_ptr), rx_info);

	// Add Message to Tail of Message List
	msg_list_add_msg_at_tail(can_ptr);

	// MZ: avviso
	mcan_rx_fifo_msg_cb() ;
}


/* Get Head Message from Message List */
can_msg_struct * msg_list_get_head_msg(can_struct *can_ptr)
{
	// Tasks: - Return Head Message from List (as Pointer)
	//        - Does NOT modify list

	// check if list is empty (there must be at least one element!)
	if ( msg_list_is_empty(can_ptr) ) {
		// list empty: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] access to emtpy list! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// list non-empty
	return &rx_msg_list[can_ptr->id].msg[rx_msg_list[can_ptr->id].last_full_elem];
} // get_head_msg_from_msg_list()


/* Remove Head Message from Message List */
void msg_list_remove_head_msg(can_struct *can_ptr)
{
	// Tasks: - Update List Status

	// Hint:  This is the only function removing messages from the list.
	//        This function is only called from normal user code and may be interrupted by an ISR => disable IRQ.

	// check if list is empty (there must be at least one element!)
	if ( msg_list_is_empty(can_ptr) ) {
		// list empty: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] access to emtpy list! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// disable Interrupt Source to prohibit list modification by IR Handler
    m_can_disable_interrupt(can_ptr->id);

	// progress in list - keep order of steps  to avoid inconsistency ========
	// This code is not Interrupted!

	// 1. OPERATION
	// increment last_full_elem (remove head msg)
	if ( rx_msg_list[can_ptr->id].last_full_elem == (RX_MESSAGE_LIST_ENTRIES-1) ) {
		rx_msg_list[can_ptr->id].last_full_elem = 0;
	} else {
		rx_msg_list[can_ptr->id].last_full_elem++;
	}

	// 2. OPERATION - Update full flag
	// If full flag is set, reset it, since an elemente was removed from list
	if (rx_msg_list[can_ptr->id].full) {
		rx_msg_list[can_ptr->id].full = FALSE;
	}

	// enable Interrupt Source (that was disable before)
    m_can_enable_interrupt(can_ptr->id);

} // remove_head_msg_from_msg_list()


/* Get Next Free Element Pointer of Message List */
can_msg_struct * msg_list_get_next_free_element(can_struct *can_ptr)
{
	// Tasks: - Return the pointer to the next free element
	//        - Does NOT modify list

	// check if list is full (there must be at least one free element!)
	if ( msg_list_is_full(can_ptr) ) {
		// list full: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] get_next_free_element_from_msg_list(), but list is full! Full Stop here! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// list not full
	return &rx_msg_list[can_ptr->id].msg[rx_msg_list[can_ptr->id].next_free_elem];
} // get_next_free_element_from_msg_list()


/* Add Message to Tail of Message List */
void msg_list_add_msg_at_tail(can_struct *can_ptr)
{
	// Tasks: - Update List Status, i.e. progress with next_free_element pointer
	//        - The current next_free_elem has to already contain the msg added,
	//          do msg copy before calling this function

	// Hint:  This is the only function adding messages to the list.
	//        This function is only called from an ISR, i.e. it is not interrupted.

	// check if list is full (there must be at least one free element!)
	if ( msg_list_is_full(can_ptr) ) {
		// list full: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] get_next_free_element_from_msg_list(), but list is full! Full Stop here! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// increment list pointer to next free element in List
	if ( rx_msg_list[can_ptr->id].next_free_elem == (RX_MESSAGE_LIST_ENTRIES-1) ) {
		rx_msg_list[can_ptr->id].next_free_elem = 0;
	} else {
		rx_msg_list[can_ptr->id].next_free_elem++;
	}

	// check if list got now full
	if ( rx_msg_list[can_ptr->id].next_free_elem == rx_msg_list[can_ptr->id].last_full_elem) {
		rx_msg_list[can_ptr->id].full = TRUE;
	}
} // add_msg_to_tail_of_msg_list()


/* Check, if Message List is EMPTY */
boolean msg_list_is_empty(can_struct *can_ptr)
{
	// Tasks: - Return List Status: TRUE=> EMPTY, FALSE=> non-EMPTY

	if ( (rx_msg_list[can_ptr->id].last_full_elem == rx_msg_list[can_ptr->id].next_free_elem) &&
		 (rx_msg_list[can_ptr->id].full == FALSE)
	   ) {
		// list empty
		return TRUE;
	} else {
		// list non-empty
		return FALSE;
	}

} // is_empty_msg_list()


/* Check, if Message List is FULL */
boolean msg_list_is_full(can_struct *can_ptr)
{
	// Tasks: - Return List Status: TRUE=> FULL, FALSE=> non-FULL
	return rx_msg_list[can_ptr->id].full;
} // is_full_msg_list()


/* Check, if there is any RX Message List in any of the Message Lists */
boolean is_any_msg_in_any_rx_msg_list()
{
	// Tasks: - Return all Lists Status:
	//          TRUE  => any Msg available,
	//          FALSE =>  no Msg available

	int can_id;

	// for all Lists (each M_CAN has one list)
    for (can_id = 0; can_id < CAN_NUMBER_OF_PRESENT_NODES; can_id++) {
    	// check if list is non-empty
    	if (msg_list_is_empty(&global.can[can_id]) == FALSE) {
    		// list non-empty
    		return TRUE;
    	}
    }

    // at this point: lists are empty
	return FALSE;

} // is_empty_msg_list()



/* Tx Event List ################################################# */

// TX EVENT FIFO LIST PROPERTIES:
//   list empty:       write_ptr == read_ptr  AND  full= FALSE
//   list full:        write_ptr == read_ptr  AND  full==TRUE
//   list partly full: write_ptr != read_ptr
//   who accesses the list?
//   - ISR:              adds elements    and updates list flags
//   - user code:        removes elements and updates list flags

/* Reset the Tx Event FIFO list */
void tx_event_list_reset()
{
	/* Description: Set internal "pointers" to consistent values - as needed for an empty list */
	int i;

	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		tx_event_list[i].next_free_elem = 0;
		tx_event_list[i].last_full_elem = 0;
		tx_event_list[i].full = FALSE;
	}
}


/* Copy a Tx Event FIFO element from Message RAM to TX Event List */
void copy_event_element_from_msg_ram_to_tx_event_list(can_struct *can_ptr, uint32_t tx_event_element_addr_in_msg_ram)
{
	/* Description: copies a Tx Event FIFO element from Message RAM and adds it to tx_event_list
	 *
	 * Parameters
	 * 	- can_ptr: pointer to the M_CAN node, where the configuration should be applied
	 * 	- msg_addr_in_msg_ram: address of the Tx Event FIFO element in the message RAM
	 */

	tx_event_element_struct * tx_event_fifo_element_ptr;

	// check if list is full
	if ( tx_event_list_is_full(can_ptr) )
	{
#if ERROR_PRINT
		DBG_PRINTF("[ERROR] Tx Event Message List of M_CAN_%d is FULL! Message is dropped.", can_ptr->id);
#endif
		return;
	}
	// list is not full - at this point

	// get the position in the list where the new element can be added
	tx_event_fifo_element_ptr = tx_event_list_get_next_free_element(can_ptr);

	// copy a Tx Event FIFO element from CAN node to tx_event_list
	m_can_copy_tx_event_element_from_msg_ram(can_ptr, tx_event_element_addr_in_msg_ram, tx_event_fifo_element_ptr);

	// Now the tx_event_element is already copied to the free element of the tx_event_list,
	// but the tx_event_list doesn't know that yet (= next_free/last_full status is not yet updated)

	// Add Tx Event Element to Tail of tx_event_list
	tx_event_list_add_element_at_tail(can_ptr); // this only updates the list status
}


/* Get Head element from Tx Event List */
tx_event_element_struct * tx_event_list_get_head_element(can_struct *can_ptr)
{
	/* Description:
	 * 	- Returns Head element from List (as Pointer)
     * 	- Does NOT modify list
     *
	 * Parameters
	 * 	- can_ptr: pointer to the M_CAN node, whose list is being accessed
	 */

	// check if list is empty (there must be at least one element!)
	if ( tx_event_list_is_empty(can_ptr) ) {
		DBG_PRINTF("[Error] access to emtpy list! (M_CAN_%d)", can_ptr->id);
		// list empty: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] access to emtpy list! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// list non-empty
	return &tx_event_list[can_ptr->id].tx_event_elem[tx_event_list[can_ptr->id].last_full_elem];
}


/* Remove Head element from Tx Event List */
void tx_event_list_remove_head_element(can_struct *can_ptr)
{
	/* Description: Update List Status
     *
	 * Parameters
	 * 	- can_ptr: pointer to the M_CAN node, whose list is being accessed
	 *
	 * Note:  This is the only function removing elements from the list.
	 *	 	  This function is only called from normal user code and may be interrupted by an ISR => disable IRQ.
	 */

	// check if list is empty (there must be at least one element!)
	assert_print_error(tx_event_list_is_empty(can_ptr) != TRUE, "[Error] access to emtpy Tx Event list!");
	//if ( tx_event_list_is_empty(can_ptr) ) {
	//	// list empty: FULL STOP;
	//	if (ERROR_PRINT) DBG_PRINTF("[Error] access to emtpy list! (M_CAN_%d)", can_ptr->id);
	//	while (TRUE) {/* do nothing */};
	//}

	// disable Interrupt that modifies list
    m_can_disable_interrupt(can_ptr->id);

	// progress in list - keep order of steps  to avoid inconsistency ========
	// This code is not Interrupted!

	// 1. OPERATION
	// increment last_full_elem (remove head msg)
	if ( tx_event_list[can_ptr->id].last_full_elem == (TX_EVENT_FIFO_LIST_ENTRIES-1) ) {
		tx_event_list[can_ptr->id].last_full_elem = 0;
	} else {
		tx_event_list[can_ptr->id].last_full_elem++;
	}


	// 2. OPERATION - Update full flag
	// If full flag is set, reset it, since an elemente was removed from list
	if (tx_event_list[can_ptr->id].full) {
		tx_event_list[can_ptr->id].full = FALSE;
	}

	// enable Interrupt that modifies list
	m_can_enable_interrupt(can_ptr->id);
}


/* Get Next Free Element Pointer of Tx Event List */
tx_event_element_struct * tx_event_list_get_next_free_element(can_struct *can_ptr)
{
	/* Description:
	 * 	- Return the pointer to the next free element of the tx_event_list
	 * 	- Does NOT modify list
	 *
	 * Parameters
	 * 	- can_ptr: pointer to the M_CAN node, whose list is being accessed
	 */

	// check if list is full (there must be at least one free element!)
	if ( tx_event_list_is_full(can_ptr) ) {
		// list full: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] tx_event_list_get_next_free_element(), but list is full! Full Stop here! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// list not full
	return &tx_event_list[can_ptr->id].tx_event_elem[tx_event_list[can_ptr->id].next_free_elem];

}


/* Add Element to Tail of Tx Event List */
void tx_event_list_add_element_at_tail(can_struct *can_ptr)
{
	/* Description:
	 * 	- Update List Status, i.e. progress with next_free_element pointer
	 * 	- The current next_free_elem has to already contain the tx_event_fifo_element added, do element copy before calling this function
	 *
	 * Parameters
	 * 	- can_ptr: pointer to the M_CAN node, whose list is being accessed
	 *
	 * Note:  This is the only function adding element to the list.
	 *        This function is only called from an ISR, i.e. it is not interrupted.
	 */

	// check if list is full (there must be at least one free element!)
	if ( tx_event_list_is_full(can_ptr) ) {
		// list full: FULL STOP;
#if ERROR_PRINT
		DBG_PRINTF("[Error] add_msg_to_tail_of_tx_event_list(), but list is full! Full Stop here! (M_CAN_%d)", can_ptr->id);
#endif
		while (TRUE) {/* do nothing */};
	}

	// increment list pointer to next free element in List
	if ( tx_event_list[can_ptr->id].next_free_elem == (TX_EVENT_FIFO_LIST_ENTRIES-1) ) {
		tx_event_list[can_ptr->id].next_free_elem = 0;
	} else {
		tx_event_list[can_ptr->id].next_free_elem++;
	}

	// check if list got now full
	if ( tx_event_list[can_ptr->id].next_free_elem == tx_event_list[can_ptr->id].last_full_elem) {
		tx_event_list[can_ptr->id].full = TRUE;
	}
}


/* Check, if Tx Event List is EMPTY */
boolean tx_event_list_is_empty(can_struct *can_ptr)
{
	/* Description: Return List Status: TRUE=> EMPTY, FALSE=> non-EMPTY
	 */

	if ( (tx_event_list[can_ptr->id].last_full_elem == tx_event_list[can_ptr->id].next_free_elem) &&
		 (tx_event_list[can_ptr->id].full == FALSE)
	   ) {
		// list empty
		return TRUE;
	} else {
		// list non-empty
		return FALSE;
	}
}


/* Check, if Tx Event List is FULL */
boolean tx_event_list_is_full(can_struct *can_ptr)
{
	// Description:  Return List Status: TRUE=> FULL, FALSE=> non-FULL
	return tx_event_list[can_ptr->id].full;
}


/* Statistics ########################################### */

/* Update Message Statistics */
void update_msg_statistics_at_mram_access(can_struct *can_ptr, can_msg_struct *msg_ptr)
{
	// Function updates the statistics messages sent (TX) and received (RX)

	// it this is an RX Message
	if (msg_ptr->direction == rx_dir) {

		can_ptr->stat.rx.msgs_mram++;  // increment message counter

	    if (msg_ptr->fdf) {
	    	can_ptr->stat.rx.fdf++;
	    }

	    if (msg_ptr->brs) {
	    	can_ptr->stat.rx.brs++;
	    }

	    if (msg_ptr->esi) {
	    	can_ptr->stat.rx.esi++;
	    }

	} // if RX-Message

	// it this is an TX Message
	if (msg_ptr->direction == tx_dir) {

		can_ptr->stat.tx.msgs_mram++;  // increment message counter

	    if (msg_ptr->fdf) {
	    	can_ptr->stat.tx.fdf++;
	    }

	    if (msg_ptr->brs) {
	    	can_ptr->stat.tx.brs++;
	    }

	    if (msg_ptr->esi) {
	    	can_ptr->stat.tx.esi++;
	    }

	} // if RX-Message


} // update statistics


/* Print Statistics */
void print_statistics(can_struct *can_ptr)
{
#ifndef DBG_ABIL
	INUTILE(can_ptr) ;
#else
	// Hint: As xil_DBG_PRINTF does not support the format specifier %u, instad %d is used.

	uint16_t tld_mean; // help variable

	DBG_PRINTF("Statistics M_CAN_%d", can_ptr->id);
	DBG_PRINTF("  TX         msgs_mram=%7u,    flags: fdf=%7u,         brs=%7u,         esi=%7u,  tx_evnt_elems=%7u",
			  (unsigned int)can_ptr->stat.tx.msgs_mram,
			  (unsigned int)can_ptr->stat.tx.fdf,
			  (unsigned int)can_ptr->stat.tx.brs,
			  (unsigned int)can_ptr->stat.tx.esi,
			  (unsigned int)can_ptr->stat.tx_event_elements
			  );
	DBG_PRINTF("  RX         msgs_mram=%7u,    flags: fdf=%7u,         brs=%7u,         esi=%7u",
			(unsigned int)can_ptr->stat.rx.msgs_mram,
			(unsigned int)can_ptr->stat.rx.fdf,
			(unsigned int)can_ptr->stat.rx.brs,
			(unsigned int)can_ptr->stat.rx.esi
			  );
	DBG_PRINTF("  RX  CHECK    msgs_ok=%7u,      msg_dupl=%7u,    msgs_err=%7u",
			(unsigned int)can_ptr->stat.rx_check.msgs_ok,
			(unsigned int)can_ptr->stat.rx_check.msgs_duplicate,
			(unsigned int)can_ptr->stat.rx_check.msgs_err
			  );

	// calculate tld_mean, avoid division by zero
	if (can_ptr->stat.tld.sum_cnt == 0) {
		tld_mean = 0;
	} else {
		tld_mean = can_ptr->stat.tld.sum/can_ptr->stat.tld.sum_cnt;
	}

	DBG_PRINTF("  TLD              min=%7u,           max=%7u,        mean=%7u,       count=%7u   (TLD values in clk periods)",
			(unsigned int)can_ptr->stat.tld.min,
			(unsigned int)can_ptr->stat.tld.max,
			(unsigned int)tld_mean,
			(unsigned int)can_ptr->stat.tld.sum_cnt
			  );
	DBG_PRINTF("  Protocol        boff=%7u,         ewarn=%7u,       epass=%7u,         pxe=%7u",
			(unsigned int)can_ptr->stat.protocol.boff,
			(unsigned int)can_ptr->stat.protocol.ewarn,
			(unsigned int)can_ptr->stat.protocol.epass,
			(unsigned int)can_ptr->stat.protocol.pxe

			);
	DBG_PRINTF("           LEC:  Stuff=%7u,          Form=%7u,         Ack=%7u,        Bit1=%7u, Bit0=%7u,  Crc=%7u",
			(unsigned int)can_ptr->stat.protocol.lec[1],
			(unsigned int)can_ptr->stat.protocol.lec[2],
			(unsigned int)can_ptr->stat.protocol.lec[3],
			(unsigned int)can_ptr->stat.protocol.lec[4],
			(unsigned int)can_ptr->stat.protocol.lec[5],
			(unsigned int)can_ptr->stat.protocol.lec[6]
			);
	DBG_PRINTF("           DLEC: Stuff=%7u,          Form=%7u,         Ack=%7u,        Bit1=%7u, Bit0=%7u,  Crc=%7u",
			(unsigned int)can_ptr->stat.protocol.dlec[1],
			(unsigned int)can_ptr->stat.protocol.dlec[2],
			(unsigned int)can_ptr->stat.protocol.dlec[3],
			(unsigned int)can_ptr->stat.protocol.dlec[4],
			(unsigned int)can_ptr->stat.protocol.dlec[5],
			(unsigned int)can_ptr->stat.protocol.dlec[6]
			);
	DBG_PRINTF("  HW Access Error  ARA=%7u,           BEU=%7u,         BEC=%7u,        MRAF=%7u",
			(unsigned int)can_ptr->stat.hw_access.ara,
			(unsigned int)can_ptr->stat.hw_access.beu,
			(unsigned int)can_ptr->stat.hw_access.bec,
			(unsigned int)can_ptr->stat.hw_access.mraf
			  );
#endif
} // function print stat



/* Reset Statistics */
void reset_statistic()
{
	int i;

	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		// init struct with zeros
		memset(&global.can[i].stat, 0, sizeof(can_statistics_struct));
		global.can[i].stat.tld.min = 9999;  // init with a large value to be able to simply set the first tld_min
	}
}


/* Misc ################################################# */

/* Print Message */
void print_msg(can_msg_struct *msg_ptr)
{
	int i;

	if (msg_ptr->direction == tx_dir) {
		DBG_PRINTF(" TX Msg:");
	} else if (msg_ptr->direction == rx_dir) {
		DBG_PRINTF(" RX Msg:");
		switch(msg_ptr->rx_info.rx_via)
		{
			case FIFO_0: DBG_PRINTF(" via FIFO 0   Index= %2d", msg_ptr->rx_info.buffer_index);
						 break;
			case FIFO_1: DBG_PRINTF(" via FIFO 1   Index= %2d", msg_ptr->rx_info.buffer_index);
						 break;
			case DEDICATED_RX_BUFFER : DBG_PRINTF(" via Ded buff Index= %2d", msg_ptr->rx_info.buffer_index);
									   break;
		}//switch
	} else {
		DBG_PRINTF(" Msg (direction not set!):");
	}

	DBG_PRINTF("  IDTYPE= ");
	if (msg_ptr->idtype == standard_id) {
		DBG_PRINTF("STD  ID= 0x%03x     ", (unsigned int)msg_ptr->id);
	} else {
		DBG_PRINTF("EXT  ID= 0x%08x", (unsigned int)msg_ptr->id);
	}

	DBG_PRINTF("  REMOTE= %d  FDF= %d  BRS= %d  ESI= %d  DLC= %2d  DataBytes[0..%2d]= 0x",
			msg_ptr->remote,
			msg_ptr->fdf,
			msg_ptr->brs,
			msg_ptr->esi,
			msg_ptr->dlc,
			convert_DLC_to_data_length(msg_ptr->dlc));

	for (i = 0; i < convert_DLC_to_data_length(msg_ptr->dlc); ++i) {
		DBG_PRINTF(" %02x", msg_ptr->data[i]);
	}

	DBG_PRINTF("\n");

} // function print_msg()


/* prints the content of one Tx Event Element */
void print_tx_event_fifo_element(tx_event_element_struct *tx_event_element_ptr)
{
	/* Description: prints the contents of one Tx Event FIFO Element
	 *
	 * Parameters
	 * 	- tx_event_element_ptr: pointer to the Tx Event FIFO element whose contents should be printed
	 */
	DBG_PRINTF("  Message Marker= 0x%02x  Time Stamp= 0x%04x",
			tx_event_element_ptr->mm,
			tx_event_element_ptr->txts);

	DBG_PRINTF("  IDTYPE= ");
	if (tx_event_element_ptr->idtype == standard_id) {
		DBG_PRINTF("STD");
	} else {
		DBG_PRINTF("EXT");
	}

	DBG_PRINTF("  ID= 0x%08x  ET= 0x%1x  REMOTE= %u  FDF= %u  BRS= %u  ESI= %u  DLC= %2u ",
			(unsigned int)tx_event_element_ptr->id,
			(unsigned int)tx_event_element_ptr->et,
			(unsigned int)tx_event_element_ptr->remote,
			(unsigned int)tx_event_element_ptr->fdf,
			(unsigned int)tx_event_element_ptr->brs,
			(unsigned int)tx_event_element_ptr->esi,
			(unsigned int)tx_event_element_ptr->dlc);

	DBG_PRINTF("\n");
}


/* Print relative Message RAM Start-Addresses */
void print_mram_startaddresses()
{
#ifdef DBG_ABIL
	int i;
    uint32_t rel_start_addr_word;

	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		DBG_PRINTF("M_CAN_%d: Message RAM Start Addresses (byte) from M_CAN point of view",i);

		// calculate start address, that is relative to Message_RAM Base + convert to word address
		rel_start_addr_word = (global.can[i].mram_sa.SIDFC_FLSSA - global.can[i].mram_base);
		DBG_PRINTF("  SIDFC_FLSSA = 0x%4.4x", (unsigned int)rel_start_addr_word);
		rel_start_addr_word = (global.can[i].mram_sa.XIDFC_FLESA - global.can[i].mram_base);
		DBG_PRINTF("  XIDFC_FLESA = 0x%4.4x", (unsigned int)rel_start_addr_word);

		rel_start_addr_word = (global.can[i].mram_sa.RXF0C_F0SA - global.can[i].mram_base);
		DBG_PRINTF("  RXF0C_F0SA  = 0x%4.4x", (unsigned int)rel_start_addr_word);
		rel_start_addr_word = (global.can[i].mram_sa.RXF1C_F1SA - global.can[i].mram_base);
		DBG_PRINTF("  RXF1C_F1SA  = 0x%4.4x", (unsigned int)rel_start_addr_word);
		rel_start_addr_word = (global.can[i].mram_sa.RXBC_RBSA - global.can[i].mram_base);
		DBG_PRINTF("  RXBC_RBSA   = 0x%4.4x", (unsigned int)rel_start_addr_word);

		rel_start_addr_word = (global.can[i].mram_sa.TXEFC_EFSA - global.can[i].mram_base);
		DBG_PRINTF("  TXEFC_EFSA  = 0x%4.4x", (unsigned int)rel_start_addr_word);
		rel_start_addr_word = (global.can[i].mram_sa.TXBC_TBSA - global.can[i].mram_base);
		DBG_PRINTF("  TXBC_TBSA   = 0x%4.4x", (unsigned int)rel_start_addr_word);
	}
#endif
} // function print_mram_startaddresses()


/*
 * RAM Check for initial Value (at system startup)
 */
int ram_check_reset_value(int m_can_id, int ram_base_addr, int ram_size_in_words, int ram_word_width_in_bytes) {

    int const ram_init_value = 0x0;
    int ram_error_count = 0; // 0: 0K, >0: ERR
    int i;
    int value;
#ifdef DBG_ABIL
    DBG_PRINTF("M_CAN_%d: RAM check starting", m_can_id);
    DBG_PRINTF(" RAM address:         0x%08x", ram_base_addr);
    DBG_PRINTF(" RAM size    in byte: 0x%08x (%d)", ram_size_in_words*ram_word_width_in_bytes, ram_size_in_words*ram_word_width_in_bytes);
    DBG_PRINTF(" RAM size    in word: 0x%08x (%d) (%d bytes/word)", ram_size_in_words, ram_size_in_words, ram_word_width_in_bytes);
    DBG_PRINTF(" Expected init-value: 0x%08x", ram_init_value);
#else
    INUTILE(m_can_id) ;
#endif
    for (i = 0; i < ram_size_in_words; i++) {
        value = (IORD_32DIRECT(ram_base_addr, i*ram_word_width_in_bytes));
        if (value != ram_init_value) {
            ram_error_count++;
            DBG_PRINTF(
                    " ERROR @ address: 0x%08x, is-value: 0x%08x, expected-value: 0x%08x)",
                    i * ram_word_width_in_bytes, value, ram_init_value);
        }
    }

    if (ram_error_count == 0) {
        DBG_PRINTF(" => Status: Passed (No errors)\n");
    } else {
        DBG_PRINTF(" => Status: Not Passed (%d RAM words are != init value 0x%08x)",
                ram_error_count, ram_init_value);
    }
    DBG_PRINTF("\n");

    return ram_error_count;
}


/*
 * RAM Print non-zero values
 */
int ram_content_print(int m_can_id, int ram_base_addr, int ram_size_in_words, int ram_word_width_in_bytes) {

    int const dont_print_value = 0; // don't print RAM words containing this value
    int ram_print_count = 0;        // count number of printed RAM words
    int i;
    int value;
#ifdef DBG_ABIL
    DBG_PRINTF("M_CAN_%d: RAM non-zero content print", m_can_id);
    DBG_PRINTF(" RAM address:     0x%08x", ram_base_addr);
    DBG_PRINTF(" RAM size (byte): 0x%08x (%d)", ram_size_in_words*ram_word_width_in_bytes, ram_size_in_words*ram_word_width_in_bytes);
    DBG_PRINTF(" RAM size (word): 0x%08x (%d) (%d bytes/word)", ram_size_in_words, ram_size_in_words, ram_word_width_in_bytes);
#else
    INUTILE(m_can_id) ;
#endif

    for (i = 0; i < ram_size_in_words; i++) {
        value = (IORD_32DIRECT(ram_base_addr, i*ram_word_width_in_bytes));
        if (value != dont_print_value) {
            ram_print_count++;
            DBG_PRINTF(" 0x%03x: 0x%08x", i * ram_word_width_in_bytes, value);
        }
    }

    if (ram_print_count == 0) {
        DBG_PRINTF(" => Status: all RAM words contain the value 0x%08x", dont_print_value);   }
    else {
        DBG_PRINTF(" => Status: %d RAM words printed.", ram_print_count);
    }
    DBG_PRINTF("\n");

    return ram_print_count;
}
