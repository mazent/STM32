/* M_CAN driver */ 
 
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

#ifndef M_CAN_H_
#define M_CAN_H_

//MZ#include "../global_includes.h"

/* Each M_CAN node is a separate module in the SoC with its own Message RAM (MRAM)
 *
 * The base address of the module is defined by the hardware setup of the FPGA image
 * The address span of each M_CAN module contains the address space of the M_CAN registers and the MRAM
 *
 * The most significant address bit of the module selects between M_CAN and MRAM
 *  binary address: 0xx..x = M_CAN address space
 *  binary address: 1xx..x = MRAM  address space
 *
 * See the FPGA integration guide (PDF) for more details.
 *
 * ---------------------------------------------------------------------------------------------------------------------------
 * Example:
 *
 * Given from hardware synthesis:
 * 		A module (M_CAN + MRAM) is configured to have a byte address width of 16 bits (64k Memory).
 * 		Module base address = 0x1000000
 *
 * Address Space
 *   0x1000000    Module (M_CAN + MRAM) base address
 *   0x1000000    M_CAN base address
 *   0x1008000    MRAM  base address (= Module base address + 0x8000)
 *   0x100FFFF    MRAM  last byte
 *
 * The whole module has 16 bits address width resulting in a memory space of 64k Byte.
 * The offset of the MRAM inside the module is therefore 32k Bytes (hexadecimal: 0x8000)
 *
 */

#if __ALTERA
	// SET THESE VALUES BY USER - depending on your M_CAN-Component added to you SoPC

    // Message RAM Base Address
    #define M_CAN_0_MRAM_BASE        (M_CAN_0_BASE + (M_CAN_0_SPAN >> 1))
    #define M_CAN_1_MRAM_BASE        (M_CAN_1_BASE + (M_CAN_1_SPAN >> 1))
    #define M_CAN_2_MRAM_BASE        (M_CAN_2_BASE + (M_CAN_2_SPAN >> 1))
    #define M_CAN_3_MRAM_BASE        (M_CAN_3_BASE + (M_CAN_3_SPAN >> 1))

    // Message RAM Size in Words, Typical: 4096 words => 16384 byte
	// in Bosch FPGA package: Message RAM size = (module size / 2) bytes
	//                        size in words = size (bytes) / 4
    #define M_CAN_0_MRAM_SIZE_WORDS  (M_CAN_0_SPAN >> 3) // (M_CAN_0_SPAN / 8)
    #define M_CAN_1_MRAM_SIZE_WORDS  (M_CAN_1_SPAN >> 3) // (M_CAN_1_SPAN / 8)
    #define M_CAN_2_MRAM_SIZE_WORDS  (M_CAN_2_SPAN >> 3) // (M_CAN_2_SPAN / 8)
    #define M_CAN_3_MRAM_SIZE_WORDS  (M_CAN_3_SPAN >> 3) // (M_CAN_3_SPAN / 8)

	#define CAN_NUMBER_OF_PRESENT_NODES     4

	#define HOST_CLK_FREQ_HERTZ        TIMER32_0_FREQ
	#define CAN_CLK_FREQ_HERTZ         40000000  // m_can_cclk frequency, used to operate the M_CAN protocol controller

	//#define TIMER_TRIG_MSG_TX        TIMER32_2_BASE

#elif __XILINX
	// SET THESE VALUES BY USER - depending on your M_CAN-Component added to you SoPC

	#define M_CAN_0_BASE        (XPAR_M_CAN_APB_1_BASEADDR)
	#define M_CAN_1_BASE        (XPAR_M_CAN_APB_1_BASEADDR)

	#define M_CAN_0_SPAN (XPAR_M_CAN_APB_0_HIGHADDR-XPAR_M_CAN_APB_0_BASEADDR+1)
	#define M_CAN_1_SPAN (XPAR_M_CAN_APB_1_HIGHADDR-XPAR_M_CAN_APB_1_BASEADDR+1)

    #define M_CAN_0_MRAM_BASE        (XPAR_M_CAN_APB_0_BASEADDR + (M_CAN_0_SPAN >> 1))
    #define M_CAN_1_MRAM_BASE        (XPAR_M_CAN_APB_1_BASEADDR + (M_CAN_1_SPAN >> 1))

	#define M_CAN_0_MRAM_SIZE_WORDS  (M_CAN_0_SPAN >> 3) // (M_CAN_0_SPAN / 8)
    #define M_CAN_1_MRAM_SIZE_WORDS  (M_CAN_1_SPAN >> 3) // (M_CAN_1_SPAN / 8)

	#define CAN_NUMBER_OF_PRESENT_NODES 2

	#define CAN_CLK_FREQ_HERTZ         40000000  // m_can_cclk frequency, used to operate the M_CAN protocol controller

	#define CAN_IO_WIRING_BASE (XPAR_CAN_IO_WIRING_APB_0_BASEADDR)

#endif // __XILINX

#define GLOBAL_NODE_ID_MASK           0xF    // used to add include global node id into msg_id (used by random message generator), has to be right aligned!
#define GLOBAL_NODE_ID_MASK_WIDTH     0x4    // width of the mask 0xF => 4, 0x1F => 5
#define GLOBAL_NODE_MAX_COUNT         (GLOBAL_NODE_ID_MASK + 1)  // max number of global nodes participating on bus

// M_CAN Message RAM - Basic Properties
#define M_CAN_RAM_WORD_WIDTH_IN_BYTE  4
#define CONV_BYTE2WORD_ADDR(x)        ((x) >> 2)

// M_CAN Message RAM - Partitioning
// Reserved number of elements in Message RAM - used for calculation of start addresses within RAM Configuration
//   some element_numbers set to less than max, to stay alltogether below 4096 words of MessageRAM requirement
#define MAX_11_BIT_FILTER_ELEMS    2   //MZ 128   // maximum is 128 11-bit Filter
#define MAX_29_BIT_FILTER_ELEMS    1   //MZ  64   // maximum is  64 29-bit Filter
#define MAX_RX_FIFO_0_ELEMS        4   //MZ  64   // maximum is  64 Rx FIFO 0 elements
#define MAX_RX_FIFO_1_ELEMS        5   //MZ  32   // maximum is  64 Rx FIFO 1 elements
#define MAX_RX_BUFFER_ELEMS        0   //MZ  64   // maximum is  64 Rx Buffer
#define MAX_TX_EVENT_FIFO_ELEMS    3   //MZ  32   // maximum is  32 Tx Event FIFO elements
#define MAX_TX_BUFFER_ELEMS        10   //MZ  32   // maximum is  32 Tx Buffers

// element sizes when stored in message RAM
#define MAX_RX_BUF_ELEM_SIZE_WORD     18  // size of RX Buffer and FIFO Element
#define MAX_TX_BUF_ELEM_SIZE_WORD     18  // size of TX Buffer and FIFO Element
#define TX_EVENT_FIFO_ELEM_SIZE_WORD   2  // size of TX Event FIFO Element
#define STD_ID_FILTER_ELEM_SIZE_WORD   1  // Standard Message ID Filter Element
#define EXT_ID_FILTER_ELEM_SIZE_WORD   2  // Extended Message ID Filter Element

#define TX_BUF_ELEM_HEADER_WORD        2  // size of TX Buffer and FIFO Element, i.e. amount of words occupied before the payload
#define RX_BUF_ELEM_HEADER_WORD        2  // size of RX Buffer and FIFO Element, i.e. amount of words occupied before the payload


// relative Start Addresses (for all M_CAN Nodes valid) (in Byte)
#define REL_SIDFC_FLSSA           0
#define REL_XIDFC_FLESA          (REL_SIDFC_FLSSA + (MAX_11_BIT_FILTER_ELEMS * STD_ID_FILTER_ELEM_SIZE_WORD * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_RXF0C_F0SA           (REL_XIDFC_FLESA + (MAX_29_BIT_FILTER_ELEMS * EXT_ID_FILTER_ELEM_SIZE_WORD * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_RXF1C_F1SA           (REL_RXF0C_F0SA  + (MAX_RX_FIFO_0_ELEMS     * MAX_RX_BUF_ELEM_SIZE_WORD    * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_RXBC_RBSA            (REL_RXF1C_F1SA  + (MAX_RX_FIFO_1_ELEMS     * MAX_RX_BUF_ELEM_SIZE_WORD    * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_TXEFC_EFSA           (REL_RXBC_RBSA   + (MAX_RX_BUFFER_ELEMS     * MAX_RX_BUF_ELEM_SIZE_WORD    * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_TXBC_TBSA            (REL_TXEFC_EFSA  + (MAX_TX_EVENT_FIFO_ELEMS * TX_EVENT_FIFO_ELEM_SIZE_WORD * M_CAN_RAM_WORD_WIDTH_IN_BYTE))
#define REL_TXBC_END             (REL_TXBC_TBSA   + (MAX_TX_BUFFER_ELEMS     * MAX_TX_BUF_ELEM_SIZE_WORD    * M_CAN_RAM_WORD_WIDTH_IN_BYTE) - 1)


// Print
#ifndef NDEBUG
#	define DEBUG_PRINT 1	// 0: don't print, !=0: print Debug Outputs
#	define ERROR_PRINT 1	// 0: don't print, !=0: print Error Outputs
#else
#	define DEBUG_PRINT 		0
#	define ERROR_PRINT 		0
#endif

// the maximal data bytes allowed per CAN frame
#define CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME 64
#define CAN_FD_MAX_DLC                   15  // maximal allowed DLC value in CAN FD
#define CAN_CLASSIC_MAX_DLC               8  // maximal allowed DLC value in CAN FD

// size of the list buffering RX Messages in Software Memory (not M_CAN or Messag RAM) before they are checked for correctness
#define RX_MESSAGE_LIST_ENTRIES     10
// size of the list buffering TX Event FIFO elements in Software Memory (not M_CAN or Messag RAM) before they are checked for correctness
#define TX_EVENT_FIFO_LIST_ENTRIES  32


/* M_CAN Message RAM Partitioning - i.e. Start Addresses (BYTE) */
typedef struct {
	uint32_t SIDFC_FLSSA;
	uint32_t XIDFC_FLESA;
	uint32_t RXF0C_F0SA ;  // RX FIFO 0 Start Address
	uint32_t RXF1C_F1SA ;  // RX FIFO 1 Start Address
	uint32_t RXBC_RBSA  ;
	uint32_t TXEFC_EFSA ;
	uint32_t TXBC_TBSA  ;
} msg_ram_partitioning_struct;

// element size structure
typedef struct
{
     uint16_t  rx_fifo0;                      // element size in words
     uint16_t  rx_fifo1;                      // element size in words
     uint16_t  rx_buffer;                     // element size in words
     uint16_t  tx_buffer;                     // element size in words
} elem_size_word_struct;

/* protocol statistics struct */
typedef struct {
	uint32_t lec[LEC_NO_OF_ERROR_CODES];
	uint32_t dlec[LEC_NO_OF_ERROR_CODES];
	uint32_t boff;
	uint32_t ewarn;
	uint32_t epass;
	uint32_t pxe;
	boolean changed;
	// add further elements as needed
} protocol_stat_struct;

/* Message statistics struct */
typedef struct {
	uint32_t fdf;  // number of messages with fdf bit set
	uint32_t brs;  // number of messages with brs bit set
	uint32_t esi;  // number of messages with esi bit set
	uint32_t msgs_mram;	  // number of messages copied from/to the message RAM (should be equal to received or transmitted messages)
} message_stat_struct;

/* Message statistics struct for Message Validation (Check) Results */
typedef struct {
	uint32_t msgs_ok;        // number of messages validated by receiving node: message content     OK
	uint32_t msgs_err;       // number of messages validated by receiving node: message content NOT OK
	uint32_t msgs_duplicate; // number of messages validated by receiving node: last message received again
} message_stat_rx_validation_struct;

/* TLD statistics struct */
typedef struct {
	uint32_t min;     // transmitter loop delay (TLD): minimal observed value
	uint32_t max;     // transmitter loop delay (TLD): maximal observed value
	uint32_t sum;     // transmitter loop delay (TLD): the sum of all measured
	uint32_t sum_cnt; // transmitter loop delay (TLD): number of measurements summed up in tld_sum
} tld_stat_struct;

/* TLD statistics struct */
typedef struct {
	uint32_t ara;   // Access to Reserved Address (ARA)  Interrupt set
	uint32_t mraf;  // Message RAM Access Failure (MRAF) Interrupt set
	uint32_t beu;   // Bit Error Uncorrected      (BEU)  Interrupt set
	uint32_t bec;   // Bit Error Corrected        (BEC)  Interrupt set
} hardware_access_stat_struct;

/* DEBUG statistics struct */
typedef struct {
	uint32_t counter[4];
} debug_stat_struct;


/* statistics struct*/
typedef struct {
	protocol_stat_struct protocol;
	message_stat_struct rx;
	message_stat_struct tx;
    tld_stat_struct tld;
    message_stat_rx_validation_struct rx_check;
    hardware_access_stat_struct hw_access;
    debug_stat_struct debug; // for debugging
    uint32_t tx_event_elements; // number of elements read from tx event fifo
	// add further elements as needed, eg. rx/tx counters
} can_statistics_struct;

// Parameters for Bit Timing Configuration
typedef struct {
	uint16_t brp;	     // baudrate prescaler
	uint16_t prop_seg;   // propagation segment
	uint16_t phase_seg1; // phase1 segment
	uint16_t phase_seg2; // phase2 segment
	uint16_t sjw;        // (re) synchronization jumpwidth
	uint16_t tdc;        // transceiver delay compensation (1:yes, 0:no)
	uint16_t tdc_offset; // transceiver delay compensation offset
	uint16_t tdc_filter_window; // transceiver delay compensation filter window length
} bt_config_struct;

// BitTiming Parameters for a CAN FD node
typedef struct {
	bt_config_struct nominal;  // Arbitration Phase Bit Timing configuration
	bt_config_struct data;     // Data        Phase Bit Timing configuration
	boolean fd_ena;            // TRUE == FD Operation enabled
	boolean brs_ena;           // TRUE == Bit Rate Switch enabled (only evaluated in HW, if FD operation enabled)
} bt_config_canfd_struct;

// TX Buffer Configuration Parameters for a CAN FD node
typedef struct {
	boolean FIFO_true_QUEUE_false;        // select: TRUE=FIFO, FALSE=Queue
	uint32_t fifo_queue_size;                // Elements in FIFO/Queue
	uint32_t ded_buffers_number;             // Number of dedicated TX buffers
	data_field_size_enum data_field_size; // TX Buffer Data Field Size (8byte .. 64byte)
} tx_buff_config_struct;

// Internal Tests structure: variables used by Bosch internal tests
typedef struct {
	boolean timeout_occurred;   // flag for timeout occurred: yes/no.
	boolean rx_msg_received;   // flag for the reception of an RX msg (used in m_can_x1_tx_cancelation_test() )
	boolean to_cnt_ir_occured; // flag for occurrence of the time out counter Interrupt
} internal_test_struct;

// CAN structure - Properties of an M_CAN node
typedef struct
{
     boolean ena;                          // TRUE = node enabled/aktive, FALSE = node disabled/does not TX or RX frames
     uint32_t base;                           // base address of this CAN module
     uint8_t  id;                             // local  CAN-Node identifier, e.g. 0 or 2
     uint8_t  id_global;                      // global CAN-Node identifier, used to separate Nodes in a multi-board network
     uint32_t mram_base;                   // Absolute Byte Base Address of this M_CAN
     uint32_t mram_size_words;             // Size of the Message RAM: number of words
     msg_ram_partitioning_struct mram_sa;  // Absolute Byte Start Addresses for Element Types in Message RAM
     elem_size_word_struct elem_size_word; // Size of Elements in Message RAM (RX Elem. in FIFO0, in FIFO1, TX Buffer) given in words
     can_statistics_struct stat;           // Statistics, any kind
     bt_config_canfd_struct bt_config;     // Bit Timing Configuration
     tx_buff_config_struct tx_config;      // TX Buffer Configuration
     boolean is_m_ttcan_node;              // FALSE: M_CAN Node, TRUE: M_TTCAN Node
     internal_test_struct internal_test;
     // MZ
     boolean autotx;
     // https://e2e.ti.com/support/interface-group/interface/f/interface-forum/1127339/tcan4550-q1-loopback-test
     boolean lback_abil;
     boolean lback_intrnl;
} can_struct;

// CAN Message ID Type
typedef enum
{
    standard_id = 0,
    extended_id = 1,
    std_and_ext_id = 2
} can_id_type_enum;

// CAN Message Direction
typedef enum
{
    tx_dir = 0,  // transmission
    rx_dir = 1   // reception
} can_msg_dir_enum;

// CAN Message received via the following Type of RX Buffers
typedef enum
{
	FIFO_0 = 0,
	FIFO_1 = 1,
	DEDICATED_RX_BUFFER = 2
} rx_buffer_type_enum;

// M_CAN operating modes (regarding frame formats capable to receive and transmit)
typedef enum
{
    CLASSICAL_CAN       = 0,
    CAN_FD_WITHOUT_BRS  = 1,
    CAN_FD_WITH_BRS     = 2
}M_CAN_operating_mode;

// CAN Message receive Information: via which RX Buffers, etc.
typedef struct
{
	rx_buffer_type_enum rx_via;        // Type of RX Buffer
	unsigned short int  buffer_index;  // if ded. RX Buffer: buffer index, if RX FIFO: GetIndex
} rx_info_struct;

// CAN Message Struct
typedef struct
{
    boolean          remote;    // TRUE = Remote Frame, FALSE = Data Frame
    can_id_type_enum idtype;    // ID or IDEXT
    uint32_t            id;        // ID (11) or IDext (29)
    boolean          fdf;       // FD Format (TRUE = FD Foramt)
    boolean          brs;       // Bit Rate Switch (TRUE = with Bit Rate Switch)
    boolean          esi;       // Error State Indicator
    uint16_t             dlc;       // Data Length Code used in the frame on the bus: current CAN FD IP impl. delivers 8 data bytes to the software, but transmits up to 64 on the CAN bus
                                // to calculate the data length of a frame based on DLC use the function convert_DLC_to_length(*)
                                // this should be removed, when CAN FD IP can deliver DLC-number of bytes to the software
#ifdef MRAM_OTTIMIZZATA
    union {
    	uint8_t            data[CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME];
    	uint32_t dati[CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME / sizeof(uint32_t)];
    } ;
#else
    uint8_t            data[CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME];   // Data
#endif

    can_msg_dir_enum direction; // Message Direction: RX or TX
    rx_info_struct   rx_info;   // Information regarding the reception of the frame
    boolean          efc;       // Event FIFO Control (TRUE = Store TX Event FIFO element after transmission)
    uint8_t             mm;        // Message marker (will be copied to TX Event FIFO element)

} can_msg_struct;

/* global status struct ========================= */
typedef struct {
    uint8_t board_id; // unique ID of the FPGA-Board in a network scenario
    can_struct can[CAN_NUMBER_OF_PRESENT_NODES];
    boolean fd_test_next_message_trigger; // triggers the transmission of a new message, is set by a timer, reset flag in your test case
    // add further elements as needed
    uint32_t DEBUG[3]; // for debugging
} can_global_struct;

// TX Event FIFO Element Struct
typedef struct {
	boolean             esi; // Error State Indicator
	can_id_type_enum idtype; // ID (11) or IDEXT (29)
	boolean          remote; // Remote transmission request
	uint32_t                id; // ID (11) or IDext (29)
	uint8_t                 mm; // Message marker
	tx_event_fifo_elem_event_type_enum   et; // Event Type
	boolean             fdf; // FD Format
	boolean             brs; // Bit Rate Switch
	uint16_t                dlc; // Data Length Code used in the frame on the bus
	uint16_t               txts; // Tx Timestamp
}tx_event_element_struct;

/* logging structure for RX Messages */
typedef struct {
	int   next_free_elem;  // write pointer, points to the next free       list entry (message)
	int   last_full_elem;  // read  pointer, points to the last valid/full list entry (message)
	boolean full;		   // TRUE==full, FALSE==not full
	can_msg_struct msg[RX_MESSAGE_LIST_ENTRIES]; // List of RX-Messages
} rx_msg_list_struct;

/* logging structure for TX Event FIFO Elements */
typedef struct {
	int   next_free_elem;  // write pointer, points to the next free       list entry (message)
	int   last_full_elem;  // read  pointer, points to the last valid/full list entry (message)
	boolean full;          // TRUE==full, FALSE==not full
	tx_event_element_struct tx_event_elem[TX_EVENT_FIFO_LIST_ENTRIES]; // List of TX Event FIFO Elements
} tx_event_list_struct;



/* ---------------------------------------------------
 * Functions
 * --------------------------------------------------- */

/* Print M_CAN version */
void m_can_print_version(can_struct *can);

/* Write to can register */
void reg_set(can_struct *can_ptr, uint16_t reg_addr_offset, uint32_t reg_value);

/* Read from can register */
uint32_t reg_get(can_struct *can_ptr, uint16_t reg_addr_offset);

/* Add a value to a register (Read -> Add -> Write Back) */
void reg_add(can_struct *can, uint16_t reg_addr, uint16_t add_val);

/* Write to can register and check write by Readback */
boolean reg_set_and_check(can_struct *can, uint16_t reg, uint32_t val);

/* set the init bit in M_CAN instance */
void m_can_set_init (can_struct *can_ptr);

/* reset the init bit in M_CAN instance */
void m_can_reset_init (can_struct *can_ptr);

/* set configuration change enable for CAN instance */
void m_can_set_config_change_enable (can_struct *can_ptr);

/* reset configuration change enable for CAN instance (init not reset!) */
void m_can_reset_config_change_enable (can_struct *can_ptr);

/* reset configuration change enable for CAN instance */
void m_can_reset_config_change_enable_and_reset_init (can_struct *can_ptr);

/* Set Nominal Bit Timing of CAN instance (Arbitration Phase) */
boolean m_can_set_bit_timing_nominal (can_struct *can_ptr);

/* Set Data Bit Timing of CAN instance (Data Phase) */
boolean m_can_set_bit_timing_data (can_struct *can_ptr);

/* Set Nominal and Data Bit Timing and CAN mode (FD, Classic) */
void m_can_set_bit_timing(can_struct *can_ptr);

/* Initialize Interrupt Registers */
void m_can_interrupt_init(can_struct *can_ptr, uint32_t ir_line0_select, uint32_t ir_line1_select, uint32_t tx_buffer_transmission_ir_enable, uint32_t tx_buffer_cancel_finished_ir_enable);

/* Enable the CAN mode: classic, FD, FD with Bit Rate Switch */
void m_can_enable_CAN_mode (can_struct *can_ptr);

/* Convert DLC of the can frame into a payload length in byte */
int convert_DLC_to_data_length (uint16_t dlc);

/* Convert element_size (of an RX/TX Element in Message RAM) to payload/data_length in byte */
uint16_t convert_element_size_to_data_length (data_field_size_enum data_field_size);

/* Copy RX-Message from Message RAM to the Msg Data Structure given as Pointer */
void m_can_copy_msg_from_msg_ram(can_struct *can_ptr, uint32_t msg_addr_in_msg_ram, can_msg_struct *msg_ptr, rx_info_struct);

/* Copy TX-Message into Message RAM */
void m_can_write_msg_to_msg_ram(can_struct *can_ptr, uint32_t msg_addr_in_msg_ram, can_msg_struct *msg_ptr);

/* Configure the dedicated Rx Buffers */
void m_can_rx_dedicated_buffers_init(can_struct * can_ptr, data_field_size_enum dedicated_rx_buffer_data_field_size);

/* Initialize an RX FIFO in the M_CAN */
void m_can_rx_fifo_init(can_struct *can_ptr, int rx_fifo_number, int fifo_size_elems, int fifo_watermark, data_field_size_enum fifo_data_field_size);

/* Copy Message from Dedicated Rx buffer to the Software Message List */
void m_can_rx_dedicated_buffer_copy_msg_to_msg_list(can_struct *can_ptr, int ded_buffer_index);

/* Copy all Messages from from RX FIFO_i to the Software Message List */
unsigned int m_can_rx_fifo_copy_msg_to_msg_list(can_struct *can_ptr, int rx_fifo_number);

/* Configure Transmit-Buffer Section */
void m_can_tx_buffer_init(can_struct *can_ptr, boolean FIFO_true_QUEUE_false, unsigned int fifo_queue_size, unsigned int ded_buffers_number, data_field_size_enum data_field_size);

/* Get the number of Free Elements in TX FIFO */
uint16_t m_can_tx_fifo_get_num_of_free_elems(can_struct *can_ptr);

/* Check if a Transmit buffer has a pending TX Request */
boolean m_can_tx_is_tx_buffer_req_pending(can_struct *can_ptr, int tx_buff_index);

/* Copy Transmit Message to TX buffer - NO CHECK is performed if the buffer has already a pending tx request, NO Transmission is requested */
void m_can_tx_write_msg_to_tx_buffer(can_struct *can_ptr, can_msg_struct *tx_msg_ptr, int tx_buff_index);

/* Request Transmission of TX Message in TX buffer - NO CHECK is performed if the buffer has already a pending tx request */
void m_can_tx_msg_request_tx(can_struct *can_ptr, int tx_buff_index);

/* Copy Transmit Message to TX buffer and request Transmission */
void m_can_tx_write_msg_to_tx_buffer_and_request_tx(can_struct *can_ptr, can_msg_struct *tx_msg_ptr, int tx_buff_index);

/* Copy Transmit Message to dedicated TX buffer and request it - it CHECKs if buffer is free */
int m_can_tx_dedicated_msg_transmit(can_struct *can_ptr, can_msg_struct *tx_msg_ptr, int tx_buff_index);

/* Copy Transmit Message into FIFO/Queue and request transmission */
int m_can_tx_fifo_queue_msg_transmit(can_struct *can_ptr, can_msg_struct *tx_msg_ptr);

/* Cancel a Tx buffer transmission request */
void m_can_tx_buffer_request_cancelation(can_struct * can_ptr, int tx_buf_index);

/* Checks if a Tx buffer cancellation request has been finished or not */
boolean m_can_tx_buffer_is_cancelation_finshed(can_struct *can_ptr, int tx_buf_index);

/* Checks if a Tx buffer transmission has occurred or not */
boolean m_can_tx_buffer_transmission_occured(can_struct *can_ptr, int tx_buf_index);

/* Configures the Global Filter Settings (GFC Register) */
void m_can_global_filter_configuration(can_struct *can_ptr, GFC_accept_non_matching_frames_enum anfs, GFC_accept_non_matching_frames_enum anfe, boolean rrfs, boolean rrfe);

/* Configure standard ID filter usage (SIDFC) */
void m_can_filter_init_standard_id(can_struct *can_ptr, unsigned int number_of_filters);

/* Configure extended ID filter usage (XIDFC) */
void m_can_filter_init_extended_id(can_struct *can_ptr, unsigned int number_of_filters);

/* Write Standard Identifier Filter in Message RAM */
void m_can_filter_write_standard_id(can_struct *can_ptr, int filter_index, SFT_Standard_Filter_Type_enum sft, Filter_Configuration_enum sfec, int sfid1, int sfid2);

/* Write Extended Identifier Filter in Message RAM */
void m_can_filter_write_extended_id(can_struct *can_ptr, int filter_index, EFT_Extended_Filter_Type_enum eft, Filter_Configuration_enum efec, int efid1, int efid2);

/* Tx Event FIFO Configuration */
void m_can_tx_event_fifo_init(can_struct *can_ptr, int fifo_size_elems, int fifo_watermark);

/* Copy all Event Elements from TX Event FIFO to the Software Event List */
int m_can_tx_event_fifo_copy_element_to_tx_event_list(can_struct *can_ptr);

/* Copy a Tx Event FIFO Element from Message RAM to the Tx Event Element Data Structure given as Pointer */
void m_can_copy_tx_event_element_from_msg_ram(can_struct *can_ptr, uint32_t msg_addr_in_msg_ram, tx_event_element_struct *tx_event_element_ptr);

/* Configure Time Stamp Usage  AND  Time Out Usage */
void m_can_timestampcounter_and_timeoutcounter_init(can_struct *can_ptr, int ts_counter_prescaler, tscc_tss_timestamp_select_enum ts_select, tocc_tos_timeout_select_enum to_select, uint16_t to_period, boolean to_counter_enable);

/* Read Time Stamp Value */
uint32_t m_can_timestamp_read_value(can_struct *can_ptr);

/* Check correctness of Message RAM Partitioning */
void m_can_check_msg_ram_partitioning(can_struct *can_ptr);

/* Init struct that contains Message RAM Partitioning for one CAN node */
void m_can_init_msg_ram_partitioning(can_struct *can_ptr);



/* ------------------------------------------------
 * Declaration of the test cases
 * ------------------------------------------------ */

// Function decodes (prints) verbosely the occurred M_CAN Interrupt
void m_can_ir_print(int m_can_id, int ir_value);

/* Check the M_CAN & M_TTCAN Register properties */
void m_can_register_test(can_struct *can_ptr, boolean reset_value_test_only);


#endif /*M_CAN_H_*/
