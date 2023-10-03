/* M_CAN IRQ handling */
 
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

/* Handle IR of M_CAN Nodes */
void m_can_process_IRQ(can_struct *can_ptr)
{
	static M_CAN_IR_union ir_reg;
	M_CAN_PSR_union psr_reg;
	uint32_t tld_last; // last measured transmitter loop delay

	// Determine the IR-Flags that need to be processed
	ir_reg.value =  reg_get(can_ptr, ADR_M_CAN_IR) & reg_get(can_ptr, ADR_M_CAN_IE);
	
    // read PSR Register once at the beginning of this ISR
	psr_reg.value = reg_get(can_ptr, ADR_M_CAN_PSR); // TODO: PSR is read to often! Read PSR only when necessary later in the ISR. PSR is reset on read! (E.g. read psr when necessary because of IR Flag, then remember that it was read, so further IR don't need to read PSR again)

	// Print Info
	if (DEBUG_PRINT) {m_can_ir_print(can_ptr->id, ir_reg.value);}

	/* update Protocol Exception Statistics (because PSR Register is modified on read)	 */
	if (psr_reg.bits.PXE) { // Protocol Exception Event occurred
		can_ptr->stat.protocol.pxe++; // update statistics
	}


	/* Check and Process the individual Interrupt Flags 
	   Strategy:
	    - (1) Reset the IR flag, then (2) process the IR (e.g. copy the message from RX FIFO)
	      Advantage: you do not miss interrupts
	      E.g.: - Case: RX FIFO new message
	            - imagine we would do the following: (1) process IR, then (2) reset the IR flag
	            - after processing the IR flag (e.g. copy message from RX FIFO) BUT BEFORE resetting the IR flag a new message could arrive, the IR flag would be set
	            - our (2) step (reset IR flag) would clear also the newly set IR flag! ==> message wont be fetched from RX FIFO!
	            - only a further message will trigger that the IR flag is set again!
	*/

	// 00: Rx FIFO 0 New Message
	if (ir_reg.bits.RF0N) {
		// Get Messages from RX_FIFO_0

		/* Target: be able to read all messages with help of the RF0N Interrupt,
		 *         it should be not necessary to poll the FIFO
		 *
		 * Action: 1. reset interrupt RF0N before reading the messages from FIFO
		 *         2. read all messages from FIFO;
		 *         Hint: if a new message arrives after reseting the IR-Flag, but before the FIFO emptied
		 *               * the corresponding message is also read from the FIFO
		 *               * the IR-Flag is set by the M_CAN
		 *               => This means, it may rarely happen for this IR routine,
		 *                  that the RF0N IR-Flag is set, but no message is in the FIFO
		 */

		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF0N_RX_FIFO0_NEW_MESSAGE);

		// Copy RX Messages from RX FIFO_0 to a Msg Data Structure, tell FIFO_0 that Msgs are read
		m_can_rx_fifo_copy_msg_to_msg_list(can_ptr, 0);
	}

	// 01: Rx FIFO 0 Watermark Reached
	if (ir_reg.bits.RF0W) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF0W_RX_FIFO0_WATERMARK_REACHED);

		// Copy RX Messages from RX FIFO_0 to a Msg Data Structure, tell FIFO_0 that Msgs are read
		m_can_rx_fifo_copy_msg_to_msg_list(can_ptr, 0);
	}

	// 02: Rx FIFO 0 Full
	if (ir_reg.bits.RF0F) {
		// Reset the this IR Flag immediately
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF0F_RX_FIFO0_FULL);
		// Add here your Code to handle this case.
	}
	
	// 03: Rx FIFO 0 Message Lost
    if (ir_reg.bits.RF0L) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF0L_RX_FIFO0_MESSAGE_LOST);
		// TODO: Caputre this event with a statistic counter++
		if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: RX FIFO 0, Message Lost IR occurred",can_ptr->id);}
	}

	// 04: Rx FIFO 1 New Message
	if (ir_reg.bits.RF1N) {
		// Get Messages from RX_FIFO_1

		/* Target: be able to read all messages with help of the RF1N Interrupt,
		 *         it should be not necessary to poll the FIFO
		 *
		 * Action: 1. reset interrupt RF1N before reading the messages from FIFO
		 *         2. read all messages from FIFO;
		 *         Hint: if a new message arrives after reseting the IR-Flag, but before the FIFO emptied
		 *               * the corresponding message is also read from the FIFO
		 *               * the IR-Flag is set by the M_CAN
		 *               => This means, it may rarely happen for this IR routine,
		 *                  that the RF1N IR-Flag is set, but no message is in the FIFO
		 */

		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF1N_RX_FIFO1_NEW_MESSAGE);

		// Copy RX Messages from RX FIFO_1 to a Msg Data Structure, tell FIFO_1 that Msgs are read
		m_can_rx_fifo_copy_msg_to_msg_list(can_ptr, 1);
	}

	// 05: Rx FIFO 1 Watermark Reached
	if (ir_reg.bits.RF1W) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF1W_RX_FIFO1_WATERMARK_REACHED);
		// Add here your Code to handle this case.

		// Copy RX Messages from RX FIFO_1 to a Msg Data Structure, tell FIFO_1 that Msgs are read
		m_can_rx_fifo_copy_msg_to_msg_list(can_ptr, 1);
	}
	
	// 06: Rx FIFO 1 Full
	if (ir_reg.bits.RF1F) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF1F_RX_FIFO1_FULL);
		// Add here your Code to handle this case.
	}
	
	// 07: Rx FIFO 1 Message Lost
	if (ir_reg.bits.RF1L) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_RF1L_RX_FIFO1_MESSAGE_LOST);
		// Add here your Code to handle this case.
		if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: RX FIFO 1, Message Lost IR occurred",can_ptr->id);}
	}

	// 08: High Priority Message
	if (ir_reg.bits.HPM) {
		/* Hint: Each time, when a High Priority Message arrives
		*        (this means: a message ID filter element configured to generate priority event matches)
		 *       the Register HPMS is overwritten AND the IR.HPM flag is set.
		 *       In high load situation, if two high priority messages arrive back to back, it is possible that
		 *       the CPU misses to execute the interrupt before the second high priority message arrives.
		 *       As the register HPMS is overwritten, it is not possible to find out that there was an unhandled IR.HPM interrupt before
		 */

		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_HPM_HIGH_PRIORITY_MESSAGE);
		// Handle HPM Interrupt
		m_can_hpm_ir_handling(can_ptr);
	}

	// 09: Transmission Completed
	if (ir_reg.bits.TC) {  // Transmission Completed
		// update msg statistics

		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TC_TRANSMISSION_COMPLETED);

		// update TLD statistics (Transmitter Loop Delay)
		// hint:    (1) some TLD values may be captured multiple times, and (2) some may not be captured at all!
		// reasons: (1) some frames may have the FDF==0,                and (2) some interrupts may be missed
		// hint2:   (1) to avoid duplicates in the TLD statistics, send only frames with FDF=BRS=1 and enable TDC
		// alternative: monitor TDCV periodically by polling this register (i.e. every 5 seconds)
		if ((can_ptr->bt_config.fd_ena == TRUE) && (can_ptr->bt_config.data.tdc == 1)) {

			// until the first frame is sent in FD Format + TDC, TDCV==0
			// => exclude this initial value
			if (psr_reg.bits.TDCV != 0) {
				tld_last = psr_reg.bits.TDCV - can_ptr->bt_config.data.tdc_offset;
				//DBG_PRINTF("[M_CAN_%d] TDCV = %d, TLD = %d, TDC_offset = %d", can_ptr->id, psr_reg.bits.TDCV, tld_last, can_ptr->bt_config.data.tdc_offset);
				can_ptr->stat.tld.sum_cnt++;
				can_ptr->stat.tld.sum = can_ptr->stat.tld.sum + tld_last;
				if (tld_last > can_ptr->stat.tld.max) {
					can_ptr->stat.tld.max = tld_last;
				}
				if (tld_last < can_ptr->stat.tld.min) {
					can_ptr->stat.tld.min = tld_last;
				}
			}
		}
	}

	// 10: Transmission Cancellation Finished
	if (ir_reg.bits.TCF) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TCF_TRANSMISSION_CANCELLATION_FINISHED);
		// Add here your Code to handle this case.
	}
	
	// 11: Tx FIFO Empty
	if (ir_reg.bits.TFE) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TFE_TX_FIFO_EMPTY);
		// Add here your Code to handle this case.
		mcan_tx_fifo_empty_cb(can_ptr->id) ;
	}
	
	// 12: Tx Event FIFO New Entry
	if (ir_reg.bits.TEFN)
	{
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TEFN_TX_EVENT_FIFO_NEW_ENTRY);

		// get Tx Event Fifo Elements from Message RAM and copy it to a list
		can_ptr->stat.tx_event_elements += m_can_tx_event_fifo_copy_element_to_tx_event_list(can_ptr);
	}

	// 13: Tx Event FIFO Watermark Reached
	if (ir_reg.bits.TEFW) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TEFW_TX_EVENT_FIFO_WATERMARK_REACHED);
		// Add here your Code to handle this case.
	}
	
	// 14: Tx Event FIFO Full
	if (ir_reg.bits.TEFF) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TEFF_TX_EVENT_FIFO_FULL);
		// Add here your Code to handle this case.
	}
	
	// 15: Tx Event FIFO Element Lost
	if (ir_reg.bits.TEFL) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TEFL_TX_EVENT_FIFO_EVENT_LOST);
		// Add here your Code to handle this case.
		if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: TX Event FIFO Element Lost IR occurred",can_ptr->id);}
	}

	// 16: Timestamp Wraparound
	if (ir_reg.bits.TSW) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TSW_TIMESTAMP_WRAPAROUND);
		// Hint: add here code to keep track of the number of timestamp wraparounds, this is required if you evaluate the timestamps in the Elements of TX_Event_FIFO, RX Buffer, or RX FIFO
	}
	
	// 17: Message RAM Access Failure
	if (ir_reg.bits.MRAF) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_MRAF_MESSAGE_RAM_ACCESS_FAILURE);

		can_ptr->stat.hw_access.mraf++; // update statistics
		if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: Message RAM Access Failure occurred",can_ptr->id);}
	}

	// 18: Timeout Occurred
	if (ir_reg.bits.TOO) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_TOO_TIMEOUT_OCCURRED);

		// Hint: Users should add here code to handle a timeout situation. The cause for the timeout can be known from register TOCC.TOS
		can_ptr->internal_test.timeout_occurred = TRUE;
	}


	// 19: Message stored to Dedicated Rx Buffer
	if (ir_reg.bits.DRX) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_DRX_MESSAGE_STORED_TO_DEDICATED_RX_BUFFER);

		// get all Messages from dedicated rx buffers (and update statistic counter)
		m_can_rx_dedicated_buffer_process_new_msg(can_ptr);
	}

	// 20: Bit Error Corrected
	if (ir_reg.bits.BEC) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_BEC_BIT_ERROR_CORRECTED);

		can_ptr->stat.hw_access.bec++; // update statistics
		//if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: Message RAM Access - Bit Error Corrected occurred",can_ptr->id);}
	}

	// 21: Bit Error Uncorrected
	if (ir_reg.bits.BEU) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_BEU_BIT_ERROR_UNCORRECTED);

		can_ptr->stat.hw_access.beu++; // update statistics
		//if (ERROR_PRINT) {DBG_PRINTF("[Error] M_CAN_%d: Message RAM Access - Bit Error Uncorrected occurred",can_ptr->id);}
	}

	// 22: Error Logging Overflow
	if (ir_reg.bits.ELO) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_ELO_ERROR_LOGGING_OVERFLOW);
		// Add here your Code to handle this case.
	}
	
	// 23: Error Passive Status (Changed)
	if (ir_reg.bits.EP) {
	    // Reset the this IR Flag immediately, to not miss a new setting of the flag
	    reg_set(can_ptr, ADR_M_CAN_IR, IR_EP_ERROR_PASSIVE);

		// Check if Error Passive Status is set
		if (psr_reg.bits.EP) {
			can_ptr->stat.protocol.epass++; // update statistics
		}
		can_ptr->stat.protocol.changed = TRUE;
	}

	// 24: Error Warning Status (Changed)
	if (ir_reg.bits.EW) {
	    // Reset the this IR Flag immediately, to not miss a new setting of the flag
	    reg_set(can_ptr, ADR_M_CAN_IR, IR_EW_WARNING_STATUS);

		if (psr_reg.bits.EW) { // Error Warning is set
			can_ptr->stat.protocol.ewarn++; // update statistics
		}
		can_ptr->stat.protocol.changed = TRUE;
	}

	// 25: Bus_Off Status (Changed)
	if (ir_reg.bits.BO) {
	    // Reset the this IR Flag immediately, to not miss a new setting of the flag
	    reg_set(can_ptr, ADR_M_CAN_IR, IR_BO_BUS_OFF_STATUS);

		if (psr_reg.bits.BO) { // Bus Off is set
			can_ptr->stat.protocol.boff++; // update statistics
		}
		can_ptr->stat.protocol.changed = TRUE;
	}

	// 26: Watchdog Interrupt
	if (ir_reg.bits.WDI) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_WDI_WATCHDOG);
		// Add here your Code to handle this case.
	}
	
	// 27: Protocol Error in Arbitration Phase
	if (ir_reg.bits.PEA) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_PEA_PROTOCOL_ERROR_IN_ARBITRATION_PHASE);

		// set, when LEC /= 0, 7
		can_ptr->stat.protocol.lec[psr_reg.bits.LEC]++; // update statistics
		can_ptr->stat.protocol.changed = TRUE;
	}

	// 28: Protocol Error in Data Phase
	if (ir_reg.bits.PED) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_PED_PROTOCOL_ERROR_IN_DATA_PHASE);

		// set, when DLEC /= 0, 7
		can_ptr->stat.protocol.dlec[psr_reg.bits.DLEC]++; // update statistics
		can_ptr->stat.protocol.changed = TRUE;
	}

	// 29: Access to Reserved Address
	if (ir_reg.bits.ARA) {
		// Reset the this IR Flag immediately, to not miss a new setting of the flag
		reg_set(can_ptr, ADR_M_CAN_IR, IR_ARA_ACCESS_TO_RESERVED_ADDRESS);

		can_ptr->stat.hw_access.ara++; // update statistics
		//if (ERROR_PRINT) {DBG_PRINTF("M_CAN_%d: Access to reserved address IR is active!", can_ptr->id);}
	}

} // function m_can_process_IR


/* Read new Messages from Dedicated RX Buffers */
unsigned int m_can_rx_dedicated_buffer_process_new_msg(can_struct *can_ptr)
{
	/* Description:
	 *  - reads and acknowledges new messages from dedicated Rx buffers.
	 *  - ALL messages currently in the dedicated Rx buffers are read and acknowledged
	 *
	 * Parameters:
	 *  - can_ptr: pointer to the M_CAN node, whose dedicated Rx buffer is to be checked
	 */

	int ded_buffer_index, j, msgs_read = 0;
	uint32_t mask;
	uint32_t new_dat_reg[2];

	// read new dat regs from M_CAN
	new_dat_reg[0] = reg_get(can_ptr, ADR_M_CAN_NDAT1);
	new_dat_reg[1] = reg_get(can_ptr, ADR_M_CAN_NDAT2);

	// scan new_dat_reg for set bits = new messages
	for (j = 0; j < 2; ++j) {

		ded_buffer_index = j*32; // init the buffer index based on the new_dat register processed currently

		mask = 1; // init mask
		while (new_dat_reg[j]) { // processes new data reg ONLY as long as there are bits set

			if (mask & new_dat_reg[j]) {

				// Check if: "dedicated Rx Buffer index > MAX_RX_BUFFER_ELEMS"
				//           If true, this means the message is stored in a memory location NOT RESERVED for ded. Rx buffers! Some other memory region was overwritten!
				if (ded_buffer_index > MAX_RX_BUFFER_ELEMS) {
					if (ERROR_PRINT) {
						DBG_PRINTF("\n[Error] Message received in an Rx dedicated buffer %d. Memory reserved only for MAX_RX_BUFFER_ELEMS=%d", ded_buffer_index, MAX_RX_BUFFER_ELEMS);
					}
				}
				// assert did not work in an ISR => solved with if-statement above
				// assert_print_error(i <= MAX_RX_BUFFER_ELEMS, "Message received in an un-configured Rx dedicated buffer");

				m_can_rx_dedicated_buffer_copy_msg_to_msg_list(can_ptr, ded_buffer_index);
				msgs_read++;

				// Reset new_dat_bit in corresponding M_CAN
				if (j==0) { reg_set(can_ptr, ADR_M_CAN_NDAT1, mask); }
				else      { reg_set(can_ptr, ADR_M_CAN_NDAT2, mask); }
				// remove bit from local new_dat_reg[j]
				new_dat_reg[j]=new_dat_reg[j] & ~mask;

				//if (DEBUG_PRINT) DBG_PRINTF(" Ded. RxBuffer %d: new message", ded_buffer_index);
			} // if
			ded_buffer_index++;        // next element
			mask = mask << 1;          // shift mask to next element
		} // while
	} // for j

	return msgs_read;
} 


/* Handle HighPriorityMessage Interrupt */
unsigned int m_can_hpm_ir_handling(can_struct *can_ptr)
{
	/* Description
	 * - The HPM interrupt signals the arrival of a high priority message
	 * - Here the interrupt is handled exemplary in the following way:
	 *   Copy ALL messages currently in the RX_FIFO including the High Priority message to the rx_msg_list.
	 *
	 * Parameters
	 * - can_ptr:         pointer to the M_CAN node, where the configuration should be applied
	 *
	 * Return Value
	 *   messages read from the RX FIFO
	 */

    M_CAN_HPMS_union  hpms_reg;
    unsigned int msgs_read = 0, rx_fifo_number;

    hpms_reg.value = reg_get(can_ptr, ADR_M_CAN_HPMS);

    // identify which Rx FIFO received the High Priority Message by checking HPMS.MSI
    if (hpms_reg.bits.MSI == HPMS_MSI_MESSAGE_STORED_IN_FIFO0) {
    	rx_fifo_number = 0;
    } else if (hpms_reg.bits.MSI == HPMS_MSI_MESSAGE_STORED_IN_FIFO1) {
    	rx_fifo_number = 1;
    } else {
    	if (ERROR_PRINT) {
        	if (hpms_reg.bits.MSI == HPMS_MSI_FIFO_MESSAGE_LOST) {
        		DBG_PRINTF("\n[ERROR] [HPM Handling ISR] High Priority message lost.");
        	}
    	}

    	return msgs_read;
    }

    // copy all messages including the high priority message in the Rx FIFO to the message list
    msgs_read = m_can_rx_fifo_copy_msg_to_msg_list(can_ptr, rx_fifo_number);

    return msgs_read; // return the number messages read from RX FIFO

}


/* Handle IR of M_TTCAN Nodes */
void m_can_process_TT_IRQ(can_struct *can_ptr)
{
	static M_CAN_TTIR_union ir_reg;
	static M_CAN_TTIR_union ir_reset_reg;

	// Determine the IR-Flags that need to be processed
	ir_reg.value = reg_get(can_ptr, ADR_M_CAN_TTIR) & reg_get(can_ptr, ADR_M_CAN_TTIE);

	// init IR clear vector
	ir_reset_reg.value = 0;


	/* Check and Process the individual Interrupt Flags */

	//  0 :  Start of Basic Cycle
	if (ir_reg.bits.SBC) {
		// IR is just reset
		ir_reset_reg.bits.SBC = 1;   // reset IR Flag
	}

	//  1 :  Start of Matrix Cycle
	if (ir_reg.bits.SMC) {
		// debug print: IR occured
		DBG_PRINTF("M");
		// IR is just reset
		ir_reset_reg.bits.SMC = 1;   // reset IR Flag
	}

	//  2 :  Change of Synchronization Mode
	//  3 :  Start of Gap
	//  4 :  REG Time Mark Interrupt

	//  5 :  Trigger Time Mark Interrupt
	if (ir_reg.bits.TTMI) {
		// debug print: IR occurred
		DBG_PRINTF("T");
		// IR is just reset
		ir_reset_reg.bits.TTMI = 1;   // reset IR Flag
	}

	//  6 :  Stop Watch Event
	//  7 :  Global Time Wrap
	//  8 :  Global Time Discontinuity
	//  9 :  Global Time Error
	//  10:  Tx Count Underflow
	//  11:  Tx Count Overflow
	//  12:  Scheduling Error 1
	//  13:  Scheduling Error 2
	//  14:  Error Level Changed
	//  15:  Initialization Watch Trigger
	//  16:  Watch Trigger
	//  17:  Application Watchdog
	//  18:  Configuration Error

	// Reset the ALL IR Flags, as long as not all IRs are handled individually
	ir_reset_reg.value = 0xFFFFFFFF;
	reg_set(can_ptr, ADR_M_CAN_TTIR, ir_reset_reg.value);

} // function m_can_process_IR

