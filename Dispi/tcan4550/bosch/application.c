/* M_CAN/M_TTCAN main program */

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

#include "global_includes.h"
#include "m_can_info.h"
can_global_struct global;

// XILINX or ALTERA specific declarations START ******************************************
#if __ALTERA
volatile int can_IR_capture;
volatile int timer2_capture;
#elif __XILINX
	XGpio XGpioInputButtons;     // The driver instance for a GPIO Device configured as Input
	XGpio XGpioOutputLEDs;       // The driver instance for a GPIO Device configured as Output
	XIntc intc; 				 // The driver instance for the interrupt controller
#endif // __XILINX
// XILINX or ALTERA specific END *********************************************************

// SPECIFIC FUNCTIONS START **************************************************************

#if __ALTERA
/* M_CAN_0_BASE;
	        	global.can[i].mram_base = M_CAN_0_MRAM_BASE;
	        	global.can[i].mram_size_words = M_CAN_0_MRAM_SIZE_WORDS;*/

const struct m_can_info m_can_info_list[CAN_NUMBER_OF_PRESENT_NODES] = {
		{
				.int_ctrl_id = M_CAN_0_IRQ_INTERRUPT_CONTROLLER_ID,
				.int_num = M_CAN_0_IRQ,
				.m_can_base = M_CAN_0_BASE,
				.mram_base = M_CAN_0_MRAM_BASE,
				.mram_size_words = M_CAN_0_MRAM_SIZE_WORDS
		},
		{
				.int_ctrl_id = M_CAN_1_IRQ_INTERRUPT_CONTROLLER_ID,
				.int_num = M_CAN_1_IRQ,
				.m_can_base = M_CAN_1_BASE,
				.mram_base = M_CAN_1_MRAM_BASE,
				.mram_size_words = M_CAN_1_MRAM_SIZE_WORDS
		},
		{
				.int_ctrl_id = M_CAN_2_IRQ_INTERRUPT_CONTROLLER_ID,
				.int_num = M_CAN_2_IRQ,
				.m_can_base = M_CAN_2_BASE,
				.mram_base = M_CAN_2_MRAM_BASE,
				.mram_size_words = M_CAN_2_MRAM_SIZE_WORDS
		},
		{
				.int_ctrl_id = M_CAN_3_IRQ_INTERRUPT_CONTROLLER_ID,
				.int_num = M_CAN_3_IRQ,
				.m_can_base = M_CAN_3_BASE,
				.mram_base = M_CAN_3_MRAM_BASE,
				.mram_size_words = M_CAN_3_MRAM_SIZE_WORDS
		},

};
#elif __XILINX
const struct m_can_info m_can_info_list[CAN_NUMBER_OF_PRESENT_NODES] = {
		{
				.int_ctrl_id = XPAR_INTC_0_DEVICE_ID, // Not used
				.int_num = XPAR_MICROBLAZE_0_AXI_INTC_M_CAN_APB_0_M_CAN_INT0_INTR,
				.m_can_base = XPAR_M_CAN_APB_0_BASEADDR,
				.mram_base = M_CAN_0_MRAM_BASE,
				.mram_size_words = M_CAN_0_MRAM_SIZE_WORDS
		},
		{
				.int_ctrl_id = XPAR_INTC_0_DEVICE_ID, // Not used
				.int_num =  XPAR_MICROBLAZE_0_AXI_INTC_M_CAN_APB_1_M_CAN_INT0_INTR,
				.m_can_base =  XPAR_M_CAN_APB_1_BASEADDR,
				.mram_base = M_CAN_1_MRAM_BASE,
				.mram_size_words = M_CAN_1_MRAM_SIZE_WORDS
		}};
#else
	#error "Code can only be compiled for Altera Nios2 Devices or Xilinx devices"
#endif


#if __ALTERA
/* Interrupt Service Routine */
static void handle_timer32_2_interrupts(void* context) {
	//volatile int* timer2_capture_ptr = (volatile int*) context;

	/* Stop timer */
	timer_stop(TIMER32_2_BASE);

	// enter user code to be executed upon interrupt here
	global.fd_test_next_message_trigger = TRUE;

	//printf("IR Timer2\n");

	/* Write a '0' in order to serve interrupt source */
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER32_2_BASE, 0x0);
}
/* Initialize the timer */
static void init_timer32_2_IRQ() {
	/* Recast the edge_capture pointer to match the alt_irq_register() function
	 * prototype. */
	void* timer2_capture_ptr = (void*) &timer2_capture;

	/* Register the interrupt handler. */
	alt_ic_isr_register(TIMER32_2_IRQ_INTERRUPT_CONTROLLER_ID, TIMER32_2_IRQ,
			handle_timer32_2_interrupts, timer2_capture_ptr, 0x0);
}

/* Interrupt Service Routine */
static void handle_M_CAN_interrupts(void* context) {
	// context variable is set to the CAN node that caused the interrupt
	can_struct *can_ptr = (can_struct*)context;
	m_can_process_IRQ(can_ptr);
	// optionally also process the M_TTCAN Interrupts
	if (can_ptr->is_m_ttcan_node == TRUE) {
		m_can_process_TT_IRQ(can_ptr);
	}
}

/* Register the ISR for the M_CAN nodes */
static void register_m_can_interrupt(struct m_can_info *node_info, can_struct *can_ptr)
{
	alt_ic_isr_register(node_info->int_ctrl_id, node_info->int_num, handle_M_CAN_interrupts, can_ptr, 0UL);
}

#elif __XILINX
/* Interrupt Service Routine */
static void handle_M_CAN_interrupts(void* context) {
	/* context variable is set to the CAN node that triggered the interrupt */
	can_struct *can_ptr = (can_struct*)context;
	m_can_process_IRQ(can_ptr);
	// optionally also process the M_TTCAN Interrupts
	if (can_ptr->is_m_ttcan_node == TRUE) {
		m_can_process_TT_IRQ(can_ptr);
	}
	/* Acknowledge the Interrupt */
	XIntc_Acknowledge(&intc, XPAR_INTC_0_DEVICE_ID);
}

/* Register the ISR for the M_CAN nodes */
static void register_m_can_interrupt(struct m_can_info *node_info, can_struct *can_ptr)
{
	XIntc_Connect(&intc, node_info->int_num,
			(XInterruptHandler)handle_M_CAN_interrupts, (void*)can_ptr);
	XIntc_Enable(&intc, node_info->int_num);
}

void AssertCallback (const char8 *File, s32 Line) {
	printf("Assert in %s Line %d\r\n", File, Line);
	fflush(stdout);
}
#endif // __XILINX

// SPECIFIC FUNCTIONS END ***************************************************************


/* main function ======================================== */
int main(void)
{
    unsigned int i;
    const struct m_can_info *node_info;

	printf("= %s started ======================\n", __FUNCTION__);
    // Check length of data types

	assert_print_error((sizeof(uint8_t)  == 1), "Unexpected sizeof(uint8_t). (expected: 1, was %zu)", sizeof(uint8_t));
	assert_print_error((sizeof(uint16_t)  == 2), "Unexpected sizeof(uint16_t). (expected: 2, was %zu)", sizeof(uint16_t));
	assert_print_error((sizeof(uint32_t) == 4), "Unexpected sizeof(uint32_t). (expected: 4, was %zu)", sizeof(uint32_t));

	// Check if CAN info structure is filled
	assert_print_error(((sizeof(m_can_info_list)/sizeof(m_can_info_list[0])) == CAN_NUMBER_OF_PRESENT_NODES),
			"Array for info structures has not the expected amount of Elements.\n Expexted amount is "DEFINE2STRING(CAN_NUMBER_OF_PRESENT_NODES)
			" but instead read %u.", (sizeof(m_can_info_list)/sizeof(m_can_info_list[0])));

   // XILINX or ALTERA specific START **************************************************
#if __ALTERA
	init_timer32_2_IRQ();
#elif __XILINX
   Xil_ICacheEnable();
   Xil_DCacheEnable();

   Xil_AssertSetCallback(AssertCallback);

   // Xilinx: IR-Controller Init + Register M_CAN Handler (This is APPLICATION SPECIFIC!)

   printf("Init InterruptController xps_intc_0...: ");
   int status;
   status = IntcInterruptSetup(&intc, XPAR_INTC_0_DEVICE_ID);
   //status = IntcInterruptSetup(&intc, XPAR_XPS_INTC_0_DEVICE_ID);
   if (status == 0) {
	   printf("PASSED\r\n");
   } else {
		printf("FAILED !!!\r\n");
   }

   printf("\r\n");
   // XILINX or ALTERA specific END ***************************************************************
#endif // __XILINX

	// Set up FPGA board ****************************************************************
#if __ALTERA
	led_set(0); 		// Switch off LEDs
	sevenseg_clear(); 	// clear sevensegment display
	pio_set(0); 		// pio_out = '0'
#endif

	/*  CAN struct: initialize with basic properties ================================== */
    // set board id (has to be unique in a network scenario)
    global.board_id = 0;

    // MUST: can[x].id has to be equal to the ARRAY_ELEMENT_NUMBER, i.e. can[x].id=x !
    for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {

        global.can[i].id        = i;  // id = array element number, used to access other arrays
        global.can[i].id_global = (global.board_id << 2) + global.can[i].id;

        // Get info struct
        node_info = &(m_can_info_list[i]);

        // Initialise Base Address, and Message RAM
	    global.can[i].base = node_info->m_can_base;
	    global.can[i].mram_base = node_info->mram_base;
	    global.can[i].mram_size_words = node_info->mram_size_words;

        // Init IRQ for M_CAN[i]
	    register_m_can_interrupt((struct m_can_info*)node_info, &global.can[i]);

    	// Initialize the Start Addresses for all types of elements for a CAN node (RX FIFO, TX Buffer, ...)
        // Requires the mram_base address to be already set!
        m_can_init_msg_ram_partitioning(&global.can[i]);

		// Start M_CAN RAM Check
		ram_check_reset_value(global.can[i].id, global.can[i].mram_base, global.can[i].mram_size_words, M_CAN_RAM_WORD_WIDTH_IN_BYTE);

        // set node property: M_CAN or M_TTCAN node? < TO BE SET BY USER >
        global.can[i].is_m_ttcan_node = FALSE; // M_CAN
        //global.can[i].is_m_ttcan_node = TRUE; // M_TTCAN

        // enable all nodes by default; => enable/disable the used nodes in the individual test cases
        global.can[i].ena = TRUE;
    } // for each M_CAN

    /* Print relative Message RAM Start-Addresses */
    //print_mram_startaddresses();

    // Print M_CAN version (Core Release Register)
	for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
		printf("M_CAN_%d version: ", global.can[i].id);
			m_can_print_version(&global.can[i]);
			printf("\n");
	}
	printf("\n");

	/* CAN IO wiring config */
	//can_io_wiring_rxtx_mode_set(CAN_IO_WIRING_MODE_normal);
	can_io_wiring_rxtx_mode_set(CAN_IO_WIRING_MODE_internal);
	can_io_wiring_rxtx_mode_print();
	printf("\n");

	// Start initial register value test for M_CAN nodes
    for (i = 0; i < CAN_NUMBER_OF_PRESENT_NODES; ++i) {
        printf("M_CAN_%d Register Test (test for reset-values)\n", global.can[i].id);
      	m_can_register_test(&global.can[i], FALSE); // Test M_CAN/M_TTCAN Registers
        printf("\n");
    }


#if __ALTERA && __BOSCH_INTERNAL_TESTS
    // M_CAN Bosch Internal Test Cases - Menu: Allows Developer to select the internal test case to be executed
    m_can_test_internal_menu();
#endif


	// ===== Application Examples ============================================

    // Message Reception
	m_can_an001_receive_messages_simple();
	m_can_an001_receive_messages_with_filtering();
	m_can_an001_high_priority_message_handling();
    m_can_an001_timeoutcounter_usage_with_rx_fifo();

    // Message Transmission
	m_can_an002_transmit_messages_tx_fifo();
	m_can_an002_transmit_messages_tx_queue_and_ded_tx_buffers();
	m_can_an002_tx_event_fifo_handling();
	m_can_an002_tx_buffer_cancellation();

    fflush(stdout);

   // XILINX specific START ****************************************************************
#if __XILINX
   Xil_DCacheDisable();
   Xil_ICacheDisable();
#endif  // __XILINX
   // XILINX specific END   ****************************************************************

   return 0;
}
