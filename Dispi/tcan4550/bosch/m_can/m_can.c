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

#include "../global_includes.h"

/*
 * MZ
 *
 * TCAN4550 / SLLSF91 - DECEMBER 2018 / pag 73
 *
 * ... a 1 should not  be written to CCCR.CSR (Clock Stop Request) as this will interfere with normal operation.
 * If a Read-Modify-Write operation is performed in Standby mode a CSR = 1 will be read
 * back but a 0 should be written to it
 *
 * Scrivo due funzioni per tenere conto di CSR
 */

extern can_struct * get_can_struct(void) ;

static void scrivi_cccr(M_CAN_CCCR_union cccr_reg)
{
    cccr_reg.bits.CSR = 0 ;

    reg_set(get_can_struct(), ADR_M_CAN_CCCR, cccr_reg.value) ;
}

static boolean cccr_set_and_check(M_CAN_CCCR_union reg_value)
{
    M_CAN_CCCR_union read_back_value ;

    reg_value.bits.CSR = 0 ;
    reg_set(get_can_struct(), ADR_M_CAN_CCCR, reg_value.value) ;

    // check written value
    read_back_value.value = reg_get(get_can_struct(), ADR_M_CAN_CCCR) ;
    read_back_value.bits.CSR = 0 ;
    if ( read_back_value.value == reg_value.value ) {
        return TRUE ;
    }
    else {
        if ( ERROR_PRINT ) {
            DBG_PRINTF(
                "[Error] cccr_set_and_check: w_value= 0x%8.8x, r_value= 0x%8.8x",
                reg_value.value,
                read_back_value.value) ;
        }
        return FALSE ; // error
    }
}

/* Print M_CAN Version from core release register */
void m_can_print_version(can_struct * can_ptr)
{
#ifdef DBG_ABIL
    // M_CAN core release register value
    M_CAN_CREL_union crel ;

    // read the registers
    crel.value = IORD_32DIRECT(can_ptr->base, ADR_M_CAN_CREL) ;

    // print the register contents
    DBG_PRINTF("%d.%d.%d from ",
               crel.bits.REL, crel.bits.STEP, crel.bits.SUBSTEP) ;
    DBG_PRINTF("%2.2x.%2.2x.201%1.1x",
               crel.bits.DAY, crel.bits.MON, crel.bits.YEAR) ;
#else
    INUTILE(can_ptr) ;
#endif
}

/* Write to can register */
void reg_set(
    can_struct * can_ptr,
    uint16_t reg_addr_offset,
    uint32_t reg_value)
{
    IOWR_32DIRECT(can_ptr->base, reg_addr_offset, reg_value) ;
}

/* Read from can register */
uint32_t reg_get(
    can_struct * can_ptr,
    uint16_t reg_addr_offset)
{
    return (uint32_t) IORD_32DIRECT(can_ptr->base, reg_addr_offset) ;
}

/* Add a value to a register (Read -> Add -> Write Back) */
void reg_add(
    can_struct * can_ptr,
    uint16_t reg_addr_offset,
    uint16_t add_value)
{
    // declare
    uint16_t can_reg_val ;
    // read
    can_reg_val = reg_get(can_ptr, reg_addr_offset) ;
    // modify (ADD)
    can_reg_val = can_reg_val | add_value ;
    // write back
    reg_set(can_ptr, reg_addr_offset, can_reg_val) ;
}

/* Write to can register and check write by Readback */
boolean reg_set_and_check(
    can_struct * can_ptr,
    uint16_t reg_addr_offset,
    uint32_t reg_value)
{
    uint32_t read_back_value ;

    IOWR_32DIRECT(can_ptr->base, reg_addr_offset, reg_value) ;

    // check written value
    read_back_value = reg_get(can_ptr, reg_addr_offset) ;
    if ( read_back_value == reg_value ) {
        return TRUE ;
    }
    else {
        if ( ERROR_PRINT ) {
            DBG_PRINTF(
                "[Error] reg_set_and_check: can_id= %d, reg_addr= 0x%3.3x, w_value= 0x%8.8x, r_value= 0x%8.8x",
                can_ptr->id,
                reg_addr_offset,
                reg_value,
                read_back_value) ;
        }
        return FALSE ; // error
    }
}

/* set the init bit in M_CAN instance */
void m_can_set_init(can_struct * can_ptr)
{
    // M_CAN is set into init mode (CCCR.INIT := 1).

    M_CAN_CCCR_union cccr_reg ;
    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;    // init with current value

    // check if init bit is not yet set
    if ( cccr_reg.bits.INIT == 0 ) {
        // not yet set => SET INIT
        cccr_reg.bits.INIT = 1 ;     // Initialization

        // write to CCCR
        //MZ reg_set(can_ptr, ADR_M_CAN_CCCR, cccr_reg.value) ;
        scrivi_cccr(cccr_reg) ;

        // check if init is already active (is synced to CCLK and back to HCLK domain)
        do {
            cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;
        } while ( cccr_reg.bits.INIT == 0 ) ;

        // DEBUG
        //DBG_PRINTF("%d", can_ptr->id);
    }
}

/* reset the init bit in M_CAN instance */
void m_can_reset_init(can_struct * can_ptr)
{
    // M_CAN is reset from init mode (CCCR.INIT := 0).

    M_CAN_CCCR_union cccr_reg ;
    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;    // init with current value

    // check if init bit is not yet reset
    if ( cccr_reg.bits.INIT == 1 ) {
        // not yet reset => RESET INIT
        cccr_reg.bits.INIT = 0 ;     // Initialization

        // write to CCCR
        //MZ reg_set(can_ptr, ADR_M_CAN_CCCR, cccr_reg.value) ;
        scrivi_cccr(cccr_reg) ;

        // check if init is already inactive (is synced to CCLK and back to HCLK domain)
        do {
            cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;
        } while ( cccr_reg.bits.INIT == 1 ) ;
    }
}

/* set configuration change enable for CAN instance */
void m_can_set_config_change_enable(can_struct * can_ptr)
{
    // M_CAN is set into the configuration change enable mode.

    M_CAN_CCCR_union cccr_reg ;

    /* 1. set init bit */
    m_can_set_init(can_ptr) ;

    /* 2. set CCE bit */
    // init with current value
    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;
    // set CCE
    cccr_reg.bits.CCE = 1 ;         // Configuration Change Enable
    //MZ reg_set_and_check(can_ptr, ADR_M_CAN_CCCR, cccr_reg.value) ;
    cccr_set_and_check(cccr_reg) ;
}

/* reset configuration change enable for CAN instance (init not reset!) */
void m_can_reset_config_change_enable(can_struct * can_ptr)
{
    // M_CAN CCE (configuration change enable) is reset.
    // Attention: After returning from this function, the M_CAN is still in init-Mode (i.e. init='1')

    M_CAN_CCCR_union cccr_reg ;

    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;    // init with current value
    cccr_reg.bits.CCE = 0 ;     // Configuration Change Enable

    // write CCCR
    //MZ reg_set_and_check(can_ptr, ADR_M_CAN_CCCR, cccr_reg.value) ;
    cccr_set_and_check(cccr_reg) ;

    //if (DEBUG_PRINT) DBG_PRINTF("[M_CAN_%d] CCCR_reg = 0x%8.8x", can->id, reg_get(can, ADR_M_CAN_CCCR));
}

/* reset configuration change enable AND reset init bit for CAN instance */
void m_can_reset_config_change_enable_and_reset_init(can_struct * can_ptr)
{
    // M_CAN CCE (configuration change enable) is reset.
    // M_CAN INIT is reset ==> this means the CAN instance is connected to the CAN bus!

    // MZ
    m_can_reset_config_change_enable(can_ptr) ;

    /* 1. reset init bit */
    m_can_reset_init(can_ptr) ;

    /* 2. set CCE bit */
    // no further action required, since:
    // M_CAN 3.2 Manual, Chapter 3.1.1: "CCCR.CCE is automatically reset when CCCR.INIT is reset"
}

/* Set Nominal Bit Timing of CAN instance (Arbitration Phase) */
boolean m_can_set_bit_timing_nominal(can_struct * can_ptr)
{
    /* Description:
     *  - Configure Nominal Bit Timing (Arbitration Phase)
     *  - Bit timing settings are taken from the software representation of the M_CAN node
     *  - M_CAN instance has to be in CCE mode
     *
     * Parameter: can_ptr - pointer to the M_CAN node, where the configuration should be applied
     *
     * Return: TRUE  - Nominal Bit Timing set successfully
     *         FALSE - Nominal Bit Timing set FAILED
     */

    M_CAN_NBTP_union bt_reg ;

    bt_reg.value = 0 ;       // init with 0

    bt_reg.bits.NTSEG2 = can_ptr->bt_config.nominal.phase_seg2 - 1 ; // The time segment after  the sample point
    bt_reg.bits.NTSEG1 = can_ptr->bt_config.nominal.phase_seg1
                         + can_ptr->bt_config.nominal.prop_seg - 1 ;                                     // The time segment before the sample point
    bt_reg.bits.NSJW = can_ptr->bt_config.nominal.sjw - 1 ;         // (Re) Synchronization Jump Width
    bt_reg.bits.NBRP = can_ptr->bt_config.nominal.brp - 1 ;         // Bit Rate Prescaler

    if ( reg_set_and_check(can_ptr, ADR_M_CAN_NBTP, bt_reg.value) == TRUE ) {
        if ( DEBUG_PRINT ) {
            DBG_PRINTF(" M_CAN_%d: Nominal Bit Rate configured correctly.",
                       can_ptr->id) ;
        }
        return TRUE ;
    }
    else {
        if ( ERROR_PRINT ) {
            DBG_PRINTF(" M_CAN_%d: Nominal Bit Rate configuration ERROR!",
                       can_ptr->id) ;
        }
        return FALSE ;
    }
}

/* Set Data Bit Timing of CAN instance (Data Phase) */
boolean m_can_set_bit_timing_data(can_struct * can_ptr)
{
    /* Description:
     *  - Configure Data Bit Timing (Data Phase)
     *  - Bit timing settings are taken from the software representation of the M_CAN node
     *  - M_CAN instance has to be in CCE mode
     *
     * Parameter: can_ptr - pointer to the M_CAN node, where the configuration should be applied
     *
     * Return: TRUE  - Data Bit Timing set successfully
     *         FALSE - Data Bit Timing set FAILED
     */

    M_CAN_DBTP_union bt_reg ;
    M_CAN_TDCR_union tdcr_reg ;

    bt_reg.value = 0 ;          // init with 0
    tdcr_reg.value = 0 ;         // init with 0

    bt_reg.bits.DTSEG2 = can_ptr->bt_config.data.phase_seg2 - 1 ; // The time segment after  the sample point
    bt_reg.bits.DTSEG1 = can_ptr->bt_config.data.phase_seg1
                         + can_ptr->bt_config.data.prop_seg - 1 ;                                  // The time segment before the sample point
    bt_reg.bits.DSJW = can_ptr->bt_config.data.sjw - 1 ;         // (Re) Synchronization Jump Width
    bt_reg.bits.DBRP = can_ptr->bt_config.data.brp - 1 ;         // Bit Rate Prescaler
    bt_reg.bits.TDC = can_ptr->bt_config.data.tdc ;              // Transceiver Delay Compensation

    tdcr_reg.bits.TDCO = can_ptr->bt_config.data.tdc_offset ;    // Transmitter Delay Compensation Offset

    if ( (reg_set_and_check(can_ptr, ADR_M_CAN_DBTP,
                            bt_reg.value) == TRUE) &&
         (reg_set_and_check(can_ptr, ADR_M_CAN_TDCR,
                            tdcr_reg.value) == TRUE) ) {
        if ( DEBUG_PRINT ) {
            DBG_PRINTF(" M_CAN_%d: Data Bit Rate configured correctly.",
                       can_ptr->id) ;
        }
        return TRUE ;
    }
    else {
        if ( ERROR_PRINT ) {
            DBG_PRINTF(" M_CAN_%d: Data Bit Rate configuration ERROR!",
                       can_ptr->id) ;
        }
        return FALSE ;
    }
}

/* Set Nominal and Data Bit Timing and CAN mode (FD, Classic) */
void m_can_set_bit_timing(can_struct * can_ptr)
{
    /*
     * Description: sets the nominal bit timing
     *              sets the data    bit timings
     *              and enables the CAN mode in CCCR register (Classical CAN, FD, or FD with BRS)
     *
     * Parameter:
     *  - can_ptr: pointer to the M_CAN node, where the configuration should be applied
     */

    // check if M_CAN is in configuration change enable mode before setting the bit timings
#ifndef NDEBUG  // Posso stampare
    M_CAN_CCCR_union cccr_reg ;
    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;
    assert_print_error(
        cccr_reg.bits.CCE == 1,
        "Bit timings can be set only when configuration change is enabled. (CCCR.INIT =1 and CCCR.CCE=1)") ;
#endif
    // Set Bit Timing of CAN instance for Arbitration Phase
    m_can_set_bit_timing_nominal(can_ptr) ;

    // Set Bit Timing of CAN instance for Data Phase (if FD is enabled)
    if ( can_ptr->bt_config.fd_ena ) {
        m_can_set_bit_timing_data(can_ptr) ;
    }

    // ENABLE   the correct CAN mode (Classic, FDF, FDF+BRS)
    m_can_enable_CAN_mode(can_ptr) ;
}

/* Initialize Interrupt Registers */
void m_can_interrupt_init(
    can_struct * can_ptr,
    uint32_t ir_line0_select,
    uint32_t ir_line1_select,
    uint32_t tx_buffer_transmission_ir_enable,
    uint32_t tx_buffer_cancel_finished_ir_enable)
{
    /* Description: Enable Interrupts and Interrupt-Line (only Line0 available in this FPGA Demo)
     *
     * Parameters
     *  - can_ptr:          pointer to the M_CAN node, where the configuration should be applied
     *  - ir_line0_select:  IR flags that should be signaled via interrupt line 0: m_can_int0, (use defines, e.g. IR_Rx_FIFO0_New_Message + IR_Rx_FIFO0_Full)
     *  - ir_line1_select:  IR flags that should be signaled via interrupt line 1: m_can_int1, (use defines, e.g. IR_Rx_FIFO1_New_Message + IR_Rx_FIFO1_Full)
     *  - tx_buffer_transmission_int_enable:    content of TXBTIE register
     *  - tx_buffer_cancel_finished_int_enable: content of TXBCIE register
     *
     * Hint: - content for IE  register is derived from ir_line0_select and ir_line1_select
     *       - content for ILE register is derived from ir_line0_select and ir_line1_select
     */

    uint32_t ir_enable ;
    uint32_t ir_line_enable_reg ;
    M_CAN_IE_union ie_reg_old ; // ie_reg value that is in the M_CAN before reconfiguring it

    // an interrupt flag can only be assigned to ir_line0 or ir_line1 (e.g. assignment of an ir_flag to both ir_lines at the same is NOT possible)
    assert_print_error(
        (ir_line0_select & ir_line1_select) == 0,
        "Interrupt flags cannot be assigned to both interrupt lines at the same time.") ;

    // check if only valid interrupt flags are selected (avoid setting of reserved bit in IR register)
    assert_print_error(
        ( (ir_line0_select
           + ir_line1_select) | INTERRUPT_ALL_SIGNALS ) ==
        INTERRUPT_ALL_SIGNALS,
        "Invalid interrupt bits set in the int_line_select parameter.") ;

    // Derive content for Interrupt Enable register
    // - Interrupts assigned to ir_line0/1 are enabled
    // - all not assigned interrupts are DISabled
    ir_enable = ir_line0_select | ir_line1_select ;

    // Clear Interrupt flags history (OPTIONAL cleanup during reconfiguration)
    // a) read the IE register to get the curently enabled interrupts
    ie_reg_old.value = reg_get(can_ptr, ADR_M_CAN_IE) ;
    // b) Clear all interrupt flags in IR register which are NOT Enabled.
    //    Reason: IR Flags in IR register are set independent of IE Register.
    //            This means, IR flags may be set in the past, but were not processed, because the IR was not enabled.
    //            To clear the IR register history, the NOT enable Interrupt flags are cleared.
    //            E.g. IR_flag RXFIFO1_newmessage was set in the past but never cleard, because this IR was not enabled
    //                 If we enable IE for RX_FIFO1_newmessage later in this function, the ISR will think there is a new message, but there no message.
    reg_set(can_ptr, ADR_M_CAN_IR, ~ie_reg_old.value) ;  // Clear NOT(ie_reg_old) IR flags

    // Write Interrupt Enable register
    reg_set_and_check(can_ptr, ADR_M_CAN_IE, ir_enable) ;

    // Derive content for Interrupt Line Enable register
    // disable all interrupt lines
    ir_line_enable_reg = ILE_DISABLE_INTERRUPT_LINES ;
    // enable interrupt line 0: if ir_line0_select contains interrupt flags => enable interrupt line0
    if ( ir_line0_select != 0 ) {
        ir_line_enable_reg += ILE_ENABLE_INTERRUPT_LINE_0 ;
    }
    // enable interrupt line 1: if ir_line1_select contains interrupt flags => enable interrupt line1
    if ( ir_line1_select != 0 ) {
        ir_line_enable_reg += ILE_ENABLE_INTERRUPT_LINE_1 ;
    }

    // Write Interrupt Line Enable register
    reg_set_and_check(can_ptr, ADR_M_CAN_ILE, ir_line_enable_reg) ;

    // Assign Interrupt flags to INT_LINE_0 or INT_LINE_1
    // Interrupts flags that are not assigned by the user via ir_line0/1_selcect will be assigned to interrupt line 0
    reg_set_and_check( can_ptr, ADR_M_CAN_ILS,
                       (ir_line1_select & INTERRUPT_ALL_SIGNALS) ) ;                      // AND with INTERRUPT_ALL_SIGNALS masks out the reserved bits

    // Write TX Buffer Transmission Interrupt Enable
    reg_set_and_check(can_ptr,
                      ADR_M_CAN_TXBTIE,
                      tx_buffer_transmission_ir_enable) ;

    // Write TX Buffer Cancellation Finished Interrupt Enable
    reg_set_and_check(can_ptr,
                      ADR_M_CAN_TXBCIE,
                      tx_buffer_cancel_finished_ir_enable) ;
}

/* Enable the CAN mode: Classical, FD, FD with Bit Rate Switch */
void m_can_enable_CAN_mode(can_struct * can_ptr)
{
    // M_CAN CC Control Register (CCCR)
    M_CAN_CCCR_union cccr_reg ;

    // init with current value
    cccr_reg.value = reg_get(can_ptr, ADR_M_CAN_CCCR) ;

    // FD Operation enable/disable
    if ( can_ptr->bt_config.fd_ena ) {
        cccr_reg.bits.FDOE = 1 ;
    }
    else {
        cccr_reg.bits.FDOE = 0 ;
    }

    // BRS enable/disable
    // Hint: The M_CAN IP evalutes the BRSE-Bit only if FDOE=1
    if ( can_ptr->bt_config.brs_ena ) {
        cccr_reg.bits.BRSE = 1 ;
    }
    else {
        cccr_reg.bits.BRSE = 0 ;
    }

    if ( can_ptr->autotx ) {
        cccr_reg.bits.DAR = 0 ;
    }
    else {
        cccr_reg.bits.DAR = 1 ;
    }

    if ( can_ptr->lback_abil ) {
        cccr_reg.bits.TEST = 1 ;

        if ( can_ptr->lback_intrnl ) {
            cccr_reg.bits.MON = 1 ;
        }
    }

    // write back modified register value
    //MZ reg_set_and_check(can_ptr, ADR_M_CAN_CCCR, cccr_reg.value) ;
    cccr_set_and_check(cccr_reg) ;

    if ( can_ptr->lback_abil ) {
        M_CAN_TEST_union test ;
        test.value = reg_get(can_ptr, ADR_M_CAN_TEST) ;
        test.bits.LBCK = 1 ;
        reg_set_and_check(can_ptr, ADR_M_CAN_TEST, test.value) ;
    }
}

/* Convert DLC of the can frame into a payload length in byte */
int convert_DLC_to_data_length(uint16_t dlc)
{
    if ( dlc <= 8 ) {
        return dlc ;
    }
    else {
        switch ( dlc ) {
        case 9:
            return 12 ;
        case 10:
            return 16 ;
        case 11:
            return 20 ;
        case 12:
            return 24 ;
        case 13:
            return 32 ;
        case 14:
            return 48 ;
        case 15:
            return 64 ;
        default:
            // error, dlc has invalid value!
            if ( ERROR_PRINT ) {
                DBG_PRINTF("ERROR: DLC (= %" PRIu16 ") has invalid value!",
                           dlc) ;
            }
            assert_print_error(
                (dlc <= CAN_FD_MAX_DLC),
                "convert_DLC_to_data_length called with a DLC > CAN_FD_MAX_DLC !") ;
            return 0 ;
        } // switch
    }
} // function convert

/* Convert element_size (of an RX/TX Element in Message RAM) to payload/data_length in byte */
uint16_t convert_element_size_to_data_length(
    data_field_size_enum data_field_size)
{
    switch ( data_field_size ) {
    case BYTE8:
        return 8 ;
    case BYTE12:
        return 12 ;
    case BYTE16:
        return 16 ;
    case BYTE20:
        return 20 ;
    case BYTE24:
        return 24 ;
    case BYTE32:
        return 32 ;
    case BYTE48:
        return 48 ;
    case BYTE64:
        return 64 ;
    default:
        // error, data_field_size has invalid value!
        if ( ERROR_PRINT ) {
            DBG_PRINTF("ERROR: data_field_size (= %d) has invalid value!",
                       data_field_size) ;
        }
        assert_print_error(
            FALSE,
            "convert_element_size_to_data_length called with not defined data_field_size_enum value !") ;

        return 0 ;
    } // switch
} // function convert

/* Copy RX-Message from Message RAM to the Msg Data Structure given as Pointer */
void m_can_copy_msg_from_msg_ram(
    can_struct * can_ptr,
    uint32_t msg_addr_in_msg_ram,
    can_msg_struct * msg_ptr,
    rx_info_struct rx_info)
{
    int i ;           // word counter
    uint32_t ram_word ;  // help variable, the currently read out Message RAM word
    int byte_index ;
    int bytes_to_read ;

    // init for while loop
    bytes_to_read = CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME ; // will be overwritten in the while loop
    i = 0 ;

    msg_ptr->direction = rx_dir ; // store that this message was received
    msg_ptr->rx_info = rx_info ;  // copy rx information (e.g. rx buffer type)

    // loop through Msg. RAM words of RX Buffer Element
    while ( bytes_to_read > 0 ) {
        ram_word = IORD_32DIRECT(msg_addr_in_msg_ram,
                                 i * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;                   // Read current Msg. RAM word

        if ( i == 0 ) {       // Msg. RAM word T0
            // extract ID Type and Msg.ID
            if ( (ram_word & RX_BUF_XTD) == 0 ) {
                msg_ptr->idtype = standard_id ;
                msg_ptr->id =
                    (uint32_t) ( (ram_word
                                  & RX_BUF_STDID_MASK) >> RX_BUF_STDID_SHIFT ) ;
            }
            else {
                msg_ptr->idtype = extended_id ;
                msg_ptr->id = (ram_word & RX_BUF_EXTID_MASK) ;
            }

            // extract: remote frame yes/no
            if ( (ram_word & RX_BUF_RTR) == 0 ) {
                msg_ptr->remote = FALSE ;
            }
            else {
                msg_ptr->remote = TRUE ;
            }

            // extract: ESI Bit
            if ( (ram_word & (uint32_t) RX_BUF_ESI) == 0 ) {
                msg_ptr->esi = FALSE ;
            }
            else {
                msg_ptr->esi = TRUE ;
            }
        }
        else if ( i == 1 ) { // Msg. RAM word T1
            // extract DLC
            msg_ptr->dlc = (ram_word & RX_BUF_DLC_MASK) >> RX_BUF_DLC_SHIFT ;

            // init bytes to read to be able to exit loop
            bytes_to_read = convert_DLC_to_data_length(msg_ptr->dlc) ;

            // extract: FDF Bit
            if ( (ram_word & RX_BUF_FDF) == 0 ) {
                msg_ptr->fdf = FALSE ;
            }
            else {
                msg_ptr->fdf = TRUE ;
            }

            // extract: BRS Bit
            if ( (ram_word & RX_BUF_BRS) == 0 ) {
                msg_ptr->brs = FALSE ;
            }
            else {
                msg_ptr->brs = TRUE ;
            }
        }
        else {   // Msg. RAM word T2 .. MAX_RX_BUF_ELEM_SIZE_WORD
            byte_index = (i - 2) * 4 ;
            msg_ptr->data[byte_index + 0] = (ram_word & 0x000000FF) >> 0 ;
            msg_ptr->data[byte_index + 1] = (ram_word & 0x0000FF00) >> 8 ;
            msg_ptr->data[byte_index + 2] = (ram_word & 0x00FF0000) >> 16 ;
            msg_ptr->data[byte_index + 3] = (ram_word & 0xFF000000) >> 24 ;

            // while-loop stops, when message payload bytes read completely
            bytes_to_read = bytes_to_read - 4 ;
        }

        // actions at the end of the while-loop -----------
        i++ ; // progress to next word
    } // while loop

    // update message statistics
    update_msg_statistics_at_mram_access(can_ptr, msg_ptr) ;
} // function

/* Copy TX-Message into Message RAM */
void m_can_write_msg_to_msg_ram(
    can_struct * can_ptr,
    uint32_t msg_addr_in_msg_ram,
    can_msg_struct * msg_ptr)
{
    // msg_addr_in_msg_ram: start address of TX Element in Msg.RAM

    uint32_t ram_word ;  // help variable, the currently read out Message RAM word
    int payload_bytes_to_write ;
    uint32_t curr_payload_word ;

    // Check if Message is a TX Message (check flags in msg_object, use an assert)
    assert_print_error(
        msg_ptr->direction == tx_dir,
        "Direction of the message not properly set in SW. Message has to be Tx Message with direction=tx_dir") ;

    // T0
    if ( msg_ptr->idtype == standard_id ) {
        ram_word = msg_ptr->id << TX_BUF_STDID_SHIFT ;        // shift Standard Identifier to position 28..18
    }
    else {
        ram_word = msg_ptr->id + TX_BUF_XTD ;                 // EXT ID + set EXT-ID Bit
    }

    if ( msg_ptr->remote == TRUE ) {
        ram_word = ram_word + TX_BUF_RTR ;
    }

    if ( msg_ptr->esi == TRUE ) {
        ram_word = ram_word + (uint32_t) TX_BUF_ESI ;
    }

    IOWR_32DIRECT(msg_addr_in_msg_ram,
                  0 * M_CAN_RAM_WORD_WIDTH_IN_BYTE,
                  ram_word) ;                                                     // Write T0 to RAM

    // T1
    ram_word = (msg_ptr->dlc << TX_BUF_DLC_SHIFT) & TX_BUF_DLC_MASK ;

    if ( msg_ptr->fdf == TRUE ) {
        ram_word = ram_word + TX_BUF_FDF ;
    }

    if ( msg_ptr->brs == TRUE ) {
        ram_word = ram_word + TX_BUF_BRS ;
    }

    if ( msg_ptr->efc == TRUE ) {
        ram_word = ram_word + TX_BUF_EFC ;
        ram_word = ram_word
                   + ( (msg_ptr->mm << TX_BUF_MM_SHIFT) & TX_BUF_MM_MASK ) ;
    }

    IOWR_32DIRECT(msg_addr_in_msg_ram,
                  1 * M_CAN_RAM_WORD_WIDTH_IN_BYTE,
                  ram_word) ;                                                     // Write T1 to RAM

    // T2 .. MAX_TX_BUF_ELEM_SIZE_WORD
    payload_bytes_to_write = convert_DLC_to_data_length(msg_ptr->dlc) ;
#ifndef MRAM_OTTIMIZZATA
    curr_payload_word = 0 ;

    while ( payload_bytes_to_write > 0 ) {
        ram_word = (msg_ptr->data[(curr_payload_word << 2) + 0] << 0)
                   + (msg_ptr->data[(curr_payload_word << 2) + 1] << 8)
                   + (msg_ptr->data[(curr_payload_word << 2) + 2] << 16)
                   + (msg_ptr->data[(curr_payload_word << 2) + 3] << 24) ;
        IOWR_32DIRECT(msg_addr_in_msg_ram,
                      (curr_payload_word + 2) * M_CAN_RAM_WORD_WIDTH_IN_BYTE,
                      ram_word) ;                                                                         // Write T2 to RAM

        // actions for next loop
        curr_payload_word++ ;
        payload_bytes_to_write = payload_bytes_to_write - 4 ;
    }
#else
    // MZ
    mcan_copia_in_mram(msg_addr_in_msg_ram + 2 * M_CAN_RAM_WORD_WIDTH_IN_BYTE,
                       msg_ptr->data,
                       payload_bytes_to_write) ;
#endif
    // update message statistics
    update_msg_statistics_at_mram_access(can_ptr, msg_ptr) ;
}

/* Configure the dedicated Rx Buffers */
void m_can_rx_dedicated_buffers_init(
    can_struct * can_ptr,
    data_field_size_enum
    dedicated_rx_buffer_data_field_size)
{
    /* Description
    * - function configures the data structures used by a dedicated Rx Buffer
    *
    * Parameters
    * - can_ptr:         pointer to the M_CAN node, where the configuration should be applied
    * - dedicated_rx_buffer_data_field_size:  maximum data field size that should be stored in a dedicated Rx Buffer
    *                                     (configure BYTE64 if you are unsure, as this is the largest data field allowed in CAN FD)
    */

    // configure the start address for RX Buffers
    M_CAN_RXBC_union rxbc_reg ;
    uint32_t rel_start_addr_word ;

    // calculate start address, that is relative to Message_RAM Base, convert it to word address
    rel_start_addr_word = CONV_BYTE2WORD_ADDR(
        can_ptr->mram_sa.RXBC_RBSA - can_ptr->mram_base) ;
    rxbc_reg.value = 0 ;
    rxbc_reg.bits.RBSA = rel_start_addr_word ;
    reg_set_and_check(can_ptr, ADR_M_CAN_RXBC, rxbc_reg.value) ;

    // configure the RX Buffer Element Size
    M_CAN_RXESC_union rxesc_reg ;

    rxesc_reg.value = reg_get(can_ptr, ADR_M_CAN_RXESC) ; // load current value
    rxesc_reg.bits.RBDS = dedicated_rx_buffer_data_field_size ; // update value with Rx Buffer data field size
    reg_set_and_check(can_ptr, ADR_M_CAN_RXESC, rxesc_reg.value) ; // write configuration to M_CAN

    // store element size also in CAN structure
    can_ptr->elem_size_word.rx_buffer = RX_BUF_ELEM_HEADER_WORD
                                        + (convert_element_size_to_data_length(
                                               dedicated_rx_buffer_data_field_size)
                                           >>
                                           2) ;                                                                                                  // data field size is a multiple of 4
}

/* Initialize an RX FIFO in the M_CAN */
void m_can_rx_fifo_init(
    can_struct * can_ptr,
    int rx_fifo_number,
    int fifo_size_elems,
    int fifo_watermark,
    data_field_size_enum fifo_data_field_size)
{
    /* Description: Rx FIFO Configuration for RX_FIFO_0 and RX_FIFO_1
     * Parameters: - can_ptr:         pointer to the M_CAN node, where the configuration should be applied
     *             - rx_fifo_number:  0: RX FIFO_0, 1: RX_FIFO_1
     *             - fifo_size_elems: Rx FIFO Size in number of RX Elements
     *             - fifo_watermark:  Watermark in number of RX Elements
     *             - fifo_data_field_size: maximum data field size that should be stored in this RX FIFO
     *                                     (configure BYTE64 if you are unsure, as this is the largest data field allowed in CAN FD)
     */

    // rx_fifo_number has to be either 0 or 1, otherwise program aborts
    assert_print_error(
        (rx_fifo_number == 0 || rx_fifo_number == 1),
        "Invalid Rx FIFO Number. M_CAN can configure only Rx FIFO0 and Rx FIFO1 !!") ;

    // use two variables (FIFO_0, FIFO_1) as the configuration registers may be different in a future M_CAN Version
    M_CAN_RXF0C_union rxf0c_reg ;
    M_CAN_RXF1C_union rxf1c_reg ;
    uint32_t rel_start_addr_word ;

    // RX FIFO Configuration
    if ( rx_fifo_number == 0 ) {
        // RX_FIFO_0 ============================

        // calculate start address, that is relative to Message_RAM Base, convert it to a word address for the M_CAN
        rel_start_addr_word = CONV_BYTE2WORD_ADDR(
            can_ptr->mram_sa.RXF0C_F0SA - can_ptr->mram_base) ;

        // configure and store the RX_FIFO_0 configuration
        rxf0c_reg.value = 0 ;                       // init with 0
        rxf0c_reg.bits.F0OM = 1 ;                   // Rx FIFO Mode: Overwrite
        rxf0c_reg.bits.F0WM = fifo_watermark ;      // Rx FIFO Watermark
        rxf0c_reg.bits.F0S = fifo_size_elems ;     // Rx FIFO Size in RX Elements
        rxf0c_reg.bits.F0SA = rel_start_addr_word ; // Rx FIFO Start Address
        reg_set_and_check(can_ptr, ADR_M_CAN_RXF0C, rxf0c_reg.value) ;
    }
    else if ( rx_fifo_number == 1 ) {
        // RX_FIFO_1 ============================

        // calculate start address, that is relative to Message_RAM Base, convert it to a word address for the M_CAN
        rel_start_addr_word = CONV_BYTE2WORD_ADDR(
            can_ptr->mram_sa.RXF1C_F1SA - can_ptr->mram_base) ;

        // configure and store the RX_FIFO_1 configuration
        rxf1c_reg.value = 0 ;                       // init with 0
        rxf1c_reg.bits.F1OM = 1 ;                   // Rx FIFO 0 Mode: Overwrite
        rxf1c_reg.bits.F1WM = fifo_watermark ;      // Rx FIFO 0 Watermark
        rxf1c_reg.bits.F1S = fifo_size_elems ;     // Rx FIFO 0 Size in RX Elements
        rxf1c_reg.bits.F1SA = rel_start_addr_word ; // Rx FIFO 0 Start Address
        reg_set_and_check(can_ptr, ADR_M_CAN_RXF1C, rxf1c_reg.value) ;
    }

    // set data element size for RX FIFO
    //   - memory is provisioned in m_can.h for the maximum data field size,
    //   - this means any valid data field size can be set here, without exceeding the memory ranges reserved in the Message RAM
    M_CAN_RXESC_union rxesc_reg ;
    rxesc_reg.value = reg_get(can_ptr, ADR_M_CAN_RXESC) ; // load current value

    // configure and store data field size
    if ( rx_fifo_number == 0 ) { // RX_FIFO_0
        rxesc_reg.bits.F0DS = fifo_data_field_size ;
        // additionally store RX BUFFER element size also in CAN structure
        can_ptr->elem_size_word.rx_fifo0 = RX_BUF_ELEM_HEADER_WORD
                                           + (
            convert_element_size_to_data_length(
                fifo_data_field_size) >> 2) ;                                                                                        // data field size is a multiple of 4
    }
    else if ( rx_fifo_number == 1 ) { // RX_FIFO_1
        rxesc_reg.bits.F1DS = fifo_data_field_size ;
        // additionally store RX BUFFER element size also in CAN structure
        can_ptr->elem_size_word.rx_fifo1 = RX_BUF_ELEM_HEADER_WORD
                                           + (
            convert_element_size_to_data_length(
                fifo_data_field_size) >> 2) ;                                                                                        // data field size is a multiple of 4
    }

    reg_set_and_check(can_ptr, ADR_M_CAN_RXESC, rxesc_reg.value) ;
}

/* Copy Message from Dedicated Rx buffer to the Software Message List */
void m_can_rx_dedicated_buffer_copy_msg_to_msg_list(
    can_struct * can_ptr,
    int ded_buffer_index)
{
    /* Description: copies a message from a dedicated Rx buffer into a message list in software
     *
     * Parameters:
     *  - can_ptr:          pointer to the M_CAN node, whose dedicated Rx buffer should be read
     *  - ded_buffer_index: index of the dedicated Rx buffer
     * */
    uint32_t ram_read_addr ;
    rx_info_struct rx_info ;

    // Calculate RAM address where RX Msg is located
    ram_read_addr = can_ptr->mram_sa.RXBC_RBSA
                    + (ded_buffer_index * can_ptr->elem_size_word.rx_buffer
                       * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;

    // store information regarding the origin of the message
    rx_info.rx_via = DEDICATED_RX_BUFFER ;
    rx_info.buffer_index = ded_buffer_index ;

    /* Copy Message from Message RAM to Message List */
    copy_msg_from_msg_ram_to_msg_list(can_ptr, ram_read_addr, rx_info) ;
}

/* Copy all Messages from RX FIFO_i to the Software Message List */
unsigned int m_can_rx_fifo_copy_msg_to_msg_list(
    can_struct * can_ptr,
    int rx_fifo_number)
{
    /* Description
     * - ALL messages that are currently in the RX_FIFO are copied to rx_msg_list.
     * - The Messages are removed from RX FIFO.
     * - If no message is in the RX_FIFO, nothing is done.
     *
     * Parameters
     * - can_ptr:         pointer to the M_CAN node, where the configuration should be applied
     * - rx_fifo_number:  0: RX FIFO_0, 1: RX_FIFO_1
     *
     * Return Value
     *   messages read from the addressed RX FIFO
     */

    // rx_fifo_number has to be either 0 or 1, otherwise program aborts
    assert_print_error(
        (rx_fifo_number == 0 || rx_fifo_number == 1),
        "Invalid Rx FIFO Number. M_CAN can configure only Rx FIFO0 and Rx FIFO1 !!") ;

    M_CAN_RXF0S_union rxf0s_reg ;
    M_CAN_RXF1S_union rxf1s_reg ;
    uint32_t ram_read_addr = 0 ;
    int msgs_read = 0 ; // number of messages read from FIFO
    rx_info_struct rx_info ;

    switch ( rx_fifo_number ) {
    case 0:     /* ========= do processing of messages in Rx FIFO 0 ========= */

        // read Rx FIFO 0 Status
        rxf0s_reg.value = reg_get(can_ptr, ADR_M_CAN_RXF0S) ;

        // read ALL the messages from the RX FIFO 0
        // while the Fill Level is !=0 read messages
        while ( rxf0s_reg.bits.F0FL ) {
            msgs_read++ ;

            // Calculate RAM address where RX Msg is located
            ram_read_addr = can_ptr->mram_sa.RXF0C_F0SA
                            + (rxf0s_reg.bits.F0GI
                               * can_ptr->elem_size_word.rx_fifo0
                               * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;

            // store information regarding the origin of the message
            rx_info.rx_via = FIFO_0 ;
            rx_info.buffer_index = rxf0s_reg.bits.F0GI ;

            /* Copy Message from Message RAM to Message List */
            copy_msg_from_msg_ram_to_msg_list(can_ptr, ram_read_addr, rx_info) ;

            //DEBUG:
            if ( DEBUG_PRINT ) {
                DBG_PRINTF(
                    "[M_CAN_%d] RX_FIFO_%d new Msg, Copied Msg. at FIFO_GetIndex=%d to MsgList (F0FL=%d, F0PI=%d)",
                    can_ptr->id,
                    rx_fifo_number,
                    rxf0s_reg.bits.F0GI,
                    rxf0s_reg.bits.F0FL,
                    rxf0s_reg.bits.F0PI) ;
            }

            // Acknowledge READ to free FIFO 0
            reg_set(can_ptr, ADR_M_CAN_RXF0A, rxf0s_reg.bits.F0GI) ;

            // read Rx FIFO 0 Status
            rxf0s_reg.value = reg_get(can_ptr, ADR_M_CAN_RXF0S) ;
        }        //while
        break ;

    case 1:     /* ========= do processing of messages in Rx FIFO 1 ========= */

        // read Rx FIFO 1 Status
        rxf1s_reg.value = reg_get(can_ptr, ADR_M_CAN_RXF1S) ;

        // read ALL the messages from the RX FIFO 1
        // while the Fill Level is !=0 read messages
        while ( rxf1s_reg.bits.F1FL ) {
            msgs_read++ ;

            // Calculate RAM address where RX Msg is located
            ram_read_addr = can_ptr->mram_sa.RXF1C_F1SA
                            + (rxf1s_reg.bits.F1GI
                               * can_ptr->elem_size_word.rx_fifo1
                               * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;

            // store information regarding the origin of the message
            rx_info.rx_via = FIFO_1 ;
            rx_info.buffer_index = rxf1s_reg.bits.F1GI ;

            /* Copy Message from Message RAM to Message List */
            copy_msg_from_msg_ram_to_msg_list(can_ptr, ram_read_addr, rx_info) ;

            //DEBUG:
            if ( DEBUG_PRINT ) {
                DBG_PRINTF(
                    "[M_CAN_%d] RX_FIFO_%d new Msg, Copied Msg. at FIFO_GetIndex=%d to MsgList (F1FL=%d, F1PI=%d)",
                    can_ptr->id,
                    rx_fifo_number,
                    rxf1s_reg.bits.F1GI,
                    rxf1s_reg.bits.F1FL,
                    rxf1s_reg.bits.F1PI) ;
            }

            // Acknowledge READ to free FIFO 1
            reg_set(can_ptr, ADR_M_CAN_RXF1A, rxf1s_reg.bits.F1GI) ;

            // read Rx FIFO 1 Status
            rxf1s_reg.value = reg_get(can_ptr, ADR_M_CAN_RXF1S) ;
        }        //while
        break ;

    default:     // need not handle. Program would have terminated in the assert statement at the begining of the function.
        break ;
    }

    // return the number messages read from RX FIFO
    return msgs_read ;
} // function m_can_rx_fifo_msg_read_to_msg_list

/* Configure Transmit-Buffer Section */
void m_can_tx_buffer_init(
    can_struct * can_ptr,
    boolean FIFO_true_QUEUE_false,
    unsigned int fifo_queue_size,
    unsigned int ded_buffers_number,
    data_field_size_enum data_field_size)
{
    /* Description: Tx Buffers Configuration. Tx FIFO/Queue and deidcated Tx buffers.
     * Parameters:
     *  - can_ptr              : pointer to the M_CAN node, where the configuration should be applied
     *  - FIFO_true_QUEUE_false: TRUE = TX FIFO operation, FALSE = TX QUEUE operation
     *  - fifo_queue_size      : number of elements in tx FIFO/Queue
     *  - ded_buffers_number   : number of dedicated Tx Buffers
     *  - data_field_size      : maximum data field size that should be stored in the Tx Buffer element
     *                                     (configure BYTE64 if you are unsure, as this is the largest data field allowed in CAN FD)
     * Note:
     *  - it has to hold: (ded_buffers_number + fifo_queue_size) <= MAX_TX_BUFFER_ELEMS
     *  - in the Message RAM, the Tx dedicated buffers comes first (index 0 and following) followed by Tx Queue/FIFO buffer elements
     */

    M_CAN_TXBC_union txbc_reg ;
    uint32_t rel_start_addr_word ; // relative start address

    // check for erroneous configuration
    assert_print_error(
        (ded_buffers_number + fifo_queue_size) <= MAX_TX_BUFFER_ELEMS,
        "TX Buffer Config: More elements configured than existing!! ") ;

    // Populate values into global structure. This global structure is used in some M_CAN test functions.
    can_ptr->tx_config.FIFO_true_QUEUE_false = FIFO_true_QUEUE_false ; // Tx fifo operation
    can_ptr->tx_config.fifo_queue_size = fifo_queue_size ;
    can_ptr->tx_config.ded_buffers_number = ded_buffers_number ;      // No dedicated Tx buffer
    can_ptr->tx_config.data_field_size = data_field_size ;

    // calculate start address, that is relative to Message_RAM Base
    // conversion to word address
    rel_start_addr_word = CONV_BYTE2WORD_ADDR(
        can_ptr->mram_sa.TXBC_TBSA - can_ptr->mram_base) ;

    txbc_reg.value = 0 ;                          // init with 0
    txbc_reg.bits.TBSA = rel_start_addr_word ;   // Tx Buffers Start Address
    txbc_reg.bits.NDTB = ded_buffers_number ;    // Number of Dedicated Transmit Buffers
    txbc_reg.bits.TFQS = fifo_queue_size ;       // Transmit FIFO/Queue Size
    if ( FIFO_true_QUEUE_false ) {
        txbc_reg.bits.TFQM = 0 ;                 // Tx FIFO  Operation
    }
    else {
        txbc_reg.bits.TFQM = 1 ;                 // Tx QUEUE Operation
    }

    reg_set_and_check(can_ptr, ADR_M_CAN_TXBC, txbc_reg.value) ;

    // set data element size for TX Buffers
    //   - memory is provisioned in m_can.h for the maximum data field size,
    //   - this means any valid data field size can be set here, without leaving memory range reserved for FIFO 0
    //   - in a real system memory reservation would be calculated based on data field size
    M_CAN_TXESC_union txesc_reg ;
    txesc_reg.value = reg_get(can_ptr, ADR_M_CAN_TXESC) ; // load current value

    txesc_reg.bits.TBDS = data_field_size ;
    reg_set_and_check(can_ptr, ADR_M_CAN_TXESC, txesc_reg.value) ;

    // store element size also in CAN structure
    can_ptr->elem_size_word.tx_buffer = TX_BUF_ELEM_HEADER_WORD
                                        + (convert_element_size_to_data_length(
                                               data_field_size) >> 2) ;                                                      // data field size is a multiple of 4
} // function m_can_tx_buf_ini()

/* Get the number of Free Elements in TX FIFO */
uint16_t m_can_tx_fifo_get_num_of_free_elems(can_struct * can_ptr)
{
    /* Description: Returns the number of consecutive free TX FIFO elements, range 0 to 32.
     * This value is not provided by the M_CAN for a TX QUEUE (value provided by M_CAN is then 0).
     */
    M_CAN_TXFQS_union txfqs ;

    // Check if the function is called for TX FIFO configuration.
    assert_print_error(
        can_ptr->tx_config.FIFO_true_QUEUE_false == TRUE,
        "Tx FIFO free elements is checked for Tx QUEUE. Not allowed !!") ;

    // read Tx FIFO 0 Status
    txfqs.value = reg_get(can_ptr, ADR_M_CAN_TXFQS) ;

    // return TX_FIFO FREE LEVEL
    return txfqs.bits.TFFL ;
}

/* Check if a Transmit buffer has a pending TX Request */
boolean m_can_tx_is_tx_buffer_req_pending(
    can_struct * can_ptr,
    int tx_buff_index)
{
    /* Description:
     * return value: TRUE:  TX buffer "tx_buff_index" has a  pending tx request
     *               FALSE: TX buffer "tx_buff_index" has NO pending tx request
     */

    uint32_t txbrp_reg ;
    uint32_t mask ;

    // read Tx Buffer Request Pending register
    txbrp_reg = reg_get(can_ptr, ADR_M_CAN_TXBRP) ;
    // generate mask for the given tx buffer index
    mask = 1 << tx_buff_index ;

    // CHECK IF TX BUFFER has a pending request
    if ( (txbrp_reg & mask) == 0 ) {
        // NO pending request for this message buffer
        return FALSE ; // NO pending tx request
    }
    else {
        // pending request available
        return TRUE ; // tx request IS pending
    } // if
}

/* Copy Transmit Message to TX buffer - NO CHECK is performed if the buffer has already a pending tx request, NO Transmission is requested */
void m_can_tx_write_msg_to_tx_buffer(
    can_struct * can_ptr,
    can_msg_struct * tx_msg_ptr,
    int tx_buff_index)
{
    /* Description:
     * - copies a message to the message buffer (in Message RAM)
     * - NO request of transmission
     * - NO CHECK is performed if the buffer has already a pending tx request
     */

    uint32_t ram_write_addr = 0 ;

    // Calculate RAM address where next Dedicated TX Msg has to be located
    ram_write_addr = can_ptr->mram_sa.TXBC_TBSA
                     + (tx_buff_index * can_ptr->elem_size_word.tx_buffer
                        * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;

    // Copy TX-Message into Message RAM
    m_can_write_msg_to_msg_ram(can_ptr, ram_write_addr, tx_msg_ptr) ;
}

/* Request Transmission of TX Message in TX buffer - NO CHECK is performed if the buffer has already a pending tx request */
void m_can_tx_msg_request_tx(
    can_struct * can_ptr,
    int tx_buff_index)
{
    /* Description:
     * - request of transmission for a tx_buffer
     * - NO CHECK is performed if the buffer has already a pending tx request
     */

    uint32_t mask ;

    // generate mask for the given tx buffer index
    mask = 1 << tx_buff_index ;

    // Add TX request
    reg_set(can_ptr, ADR_M_CAN_TXBAR, mask) ; // set "add request bit"
}

/* Copy Transmit Message to TX buffer and request Transmission - NO CHECK is performed if the buffer has already a pending tx request*/
void m_can_tx_write_msg_to_tx_buffer_and_request_tx(
    can_struct * can_ptr,
    can_msg_struct * tx_msg_ptr,
    int tx_buff_index)
{
    /* Description:
     * - copies a message to the message buffer (in Message RAM) and requests its transmission
     * - NO CHECK is performed if the buffer has already a pending tx request
     */

    /* Copy Transmit Message to TX buffer - NO CHECK is performed if the buffer has already a pending tx request, NO Transmission is requested */
    m_can_tx_write_msg_to_tx_buffer(can_ptr, tx_msg_ptr, tx_buff_index) ;

    /* Request Transmission of TX Message in TX buffer - NO CHECK is performed if the buffer has already a pending tx request */
    m_can_tx_msg_request_tx(can_ptr, tx_buff_index) ;
}

/* Copy Tx Message to dedicated TX buffer and Request transmission - it CHECKs if buffer is free */
int m_can_tx_dedicated_msg_transmit(
    can_struct * can_ptr,
    can_msg_struct * tx_msg_ptr,
    int tx_buff_index)
{
    /* Description:
     * return value: number of tx requests set: 0= tx_buffer has a pending request, nothing requested
     *                                          1= one transmission requested
     */

    int msgs_sent ;

    /* Check if a Transmit buffer has a pending TX Request */
    if ( m_can_tx_is_tx_buffer_req_pending(can_ptr, tx_buff_index) == FALSE ) {
        // NO pending request for this message buffer => write msg to buffer and request it
        // Copy Transmit Message to tx buffer and request it - NO CHECK if buffer free or not!
        m_can_tx_write_msg_to_tx_buffer_and_request_tx(can_ptr,
                                                       tx_msg_ptr,
                                                       tx_buff_index) ;
        msgs_sent = 1 ; // one transmission requested
    }
    else {
        // pending request available
        msgs_sent = 0 ; // no transmission requested
    } // if

    return msgs_sent ;
}

/* Copy Tx Message into FIFO/Queue and Request transmission */
int m_can_tx_fifo_queue_msg_transmit(
    can_struct * can_ptr,
    can_msg_struct * tx_msg_ptr)
{
    /* Description
     *  can_ptr      : copy message to this node
     *  tx_msg_ptr   : message to be copied
     *  return value : number of tx requests set: 0= FIFO/Queue is full, nothing copied or requested
     *                                            1= one message transmission requested
     */

    M_CAN_TXFQS_union txfqs ;
    int msgs_sent ;

    // read Tx FIFO Status
    txfqs.value = reg_get(can_ptr, ADR_M_CAN_TXFQS) ;

    // check if FIFO/QUEUE is full
    //    HINT: Checking txfqs.bits.TFFL (Free Level) does not work in case of QUEUE!
    //          Therefore here Full Flag is checked.
    if ( txfqs.bits.TFQF == 0 ) {
        // FIFO/Queue NOT FULL
        // Copy Transmit Message to tx buffer AND request it - NO CHECK if buffer free or not!
        m_can_tx_write_msg_to_tx_buffer_and_request_tx(can_ptr,
                                                       tx_msg_ptr,
                                                       txfqs.bits.TFQPI) ;
        msgs_sent = 1 ;
    }
    else {
        // FIFO/Queue IS FULL => no tx message requested
        msgs_sent = 0 ;
    }

    return msgs_sent ;
} // function m_an_tx_fifo_queue_msg_write()

/* Cancel a Tx buffer transmission request */
void m_can_tx_buffer_request_cancelation(
    can_struct * can_ptr,
    int tx_buf_index)
{
    /* Description: request the cancellation for ONE Tx buffer
     *
     * Parameters
     *  - can_ptr:		pointer to the M_CAN node, whose tx request has to be canceled
     *  - tx_buf_index: index of the Tx buffer whose request should be canceled
     */

    uint32_t mask ;

    // generate mask for the given tx buffer index
    mask = 1 << tx_buf_index ;

    // write to Cancellation Register
    reg_set(can_ptr, ADR_M_CAN_TXBCR, mask) ;
}

/* Checks if a Tx buffer cancellation request has been finished or not */
boolean m_can_tx_buffer_is_cancelation_finshed(
    can_struct * can_ptr,
    int tx_buf_index)
{
    /* Description: tells if a Tx buffer cancellation request is complete or not
     *
     * Parameters
     *  - can_ptr:		pointer to the M_CAN node, whose request has to be canceled
     *  - buffer_index: index of the Tx buffer whose cancellation status should be checked
     *
     * Return
     *  - TRUE:	 Cancellation finished
     *  - FALSE: Cancellation NOT finished
     */

    uint32_t mask ;
    uint32_t txbcf_reg ;

    mask = 1 << tx_buf_index ; // generate mask for the given tx buffer index
    txbcf_reg = reg_get(can_ptr, ADR_M_CAN_TXBCF) ;

    // check if cancellation bit for tx_buf_index is set
    if ( txbcf_reg & mask ) {
        return TRUE ;    // cancellation is finished
    }
    else {
        return FALSE ;   // cancellation is not finished
    }
}

/* Checks if a Tx buffer transmission has occurred or not */
boolean m_can_tx_buffer_transmission_occured(
    can_struct * can_ptr,
    int tx_buf_index)
{
    /* Description: tells if a Tx buffer transmission has occurred or not
     *
     * Parameters
     *  - can_ptr:		pointer to the M_CAN node, whose request has to be canceled
     *  - buffer_index: index of the Tx buffer whose request should be canceled
     *
     * Return
     *  - TRUE:	 Transmission occurred
     *  - FALSE: Transmission did not occur
     */

    uint32_t mask ;
    uint32_t txbto_reg ;

    mask = 1 << tx_buf_index ;
    txbto_reg = reg_get(can_ptr, ADR_M_CAN_TXBTO) ;

    if ( txbto_reg & mask ) {
        return TRUE ;    // tx buffer transmission occurred
    }
    else {
        return FALSE ;   // tx buffer transmission not occurred
    }
}

/* Configures the Global Filter Settings (GFC Register) */
void m_can_global_filter_configuration(
    can_struct * can_ptr,
    GFC_accept_non_matching_frames_enum anfs,
    GFC_accept_non_matching_frames_enum anfe,
    boolean rrfs,
    boolean rrfe)
{
    /* Description: Global Filter Configuration (GFC)
     * Parameters:
     * - can_ptr:  pointer to the M_CAN node, where the configuration should be applied
     * - anfs:     accept/reject non-matching standard(11-bits) frames
     * - anfe:     accept/reject non-matching extended(29-bits) frames
     * - rrfs:     reject remote frames standard. TRUE: reject remote standard frames, FALSE: filter remote standard frames
     * - rrfe:     reject remote frames extended. TRUE: reject remote extended frames, FALSE: filter remote extended frames
     */

    M_CAN_GFC_union gfc_reg ;

    // prepare the register value to be written to the M_CAN
    gfc_reg.value = 0 ;          // init with 0
    gfc_reg.bits.ANFS = anfs ;   // accept non matching frames standard
    gfc_reg.bits.ANFE = anfe ;   // accept non matching frames extended

    if ( rrfs == TRUE ) {
        gfc_reg.bits.RRFS = 1 ;  //  1 = Reject all remote frames with 11-bit std IDs
    }
    else if ( rrfs == FALSE ) {
        gfc_reg.bits.RRFS = 0 ;  //  0 = Filter all remote frames with 11-bit std IDs
    }

    if ( rrfe == TRUE ) {
        gfc_reg.bits.RRFE = 1 ;  //  1= Reject all remote frames with 29-bit xtd IDs
    }
    else if ( rrfe == FALSE ) {
        gfc_reg.bits.RRFE = 0 ;  //  0 = Filter all remote frames with 29-bit xtd IDs
    }

    // write configuration to M_CAN
    reg_set_and_check(can_ptr, ADR_M_CAN_GFC, gfc_reg.value) ;
}

/* Configure standard ID filter usage (SIDFC) */
void m_can_filter_init_standard_id(
    can_struct * can_ptr,
    unsigned int number_of_filters)
{
    /* Description: Configures the register SIDFC for the 11-bit Standard Message ID Filter elements.
     *
     * Parameters
     * - can_ptr:            pointer to the M_CAN node, where the configuration should be applied
     * - number_of_filters:  the number of standard message ID filter elements
     */

    // number_of_filters has to be less or equal to MAX_11_BIT_FILTER_ELEMS, otherwise program aborts
    assert_print_error( (number_of_filters <= MAX_11_BIT_FILTER_ELEMS),
                        "Invalid Number of Standard ID Filters configured !!" ) ;

    M_CAN_SIDFC_union sidfc_reg ;
    uint32_t rel_filter_start_addr_word ;

    // calculate start address, that is relative to Message_RAM Base
    // conversion to word address
    rel_filter_start_addr_word = CONV_BYTE2WORD_ADDR(
        can_ptr->mram_sa.SIDFC_FLSSA - can_ptr->mram_base) ;

    sidfc_reg.value = 0 ;                                 // init with 0
    sidfc_reg.bits.FLSSA = rel_filter_start_addr_word ;   // convert to word address!
    sidfc_reg.bits.LSS = number_of_filters ;             // size of the list in elements

    reg_set(can_ptr, ADR_M_CAN_SIDFC, sidfc_reg.value) ;
}

/* Configure extended ID filter usage (XIDFC) */
void m_can_filter_init_extended_id(
    can_struct * can_ptr,
    unsigned int number_of_filters)
{
    /* Description
     * - function configures the register XIDFC for the 29-bit extended message id filter elements.
     *
     * Parameters
     * - can_ptr:            pointer to the M_CAN node, where the configuration should be applied
     * - number_of_filters:  the number of extended message ID filter elements
     */

    // number_of_filters has to be less or equal to MAX_29_BIT_FILTER_ELEMS, otherwise program aborts
    assert_print_error( (number_of_filters <= MAX_29_BIT_FILTER_ELEMS),
                        "Invalid Number of Extended ID Filters configured !!" ) ;

    M_CAN_XIDFC_union xidfc_reg ;
    uint32_t rel_filter_start_addr_word ;

    // calculate start address, that is relative to Message_RAM Base and convert to word address
    rel_filter_start_addr_word = CONV_BYTE2WORD_ADDR(
        can_ptr->mram_sa.XIDFC_FLESA - can_ptr->mram_base) ;

    // prepare the register value to be written to the M_CAN
    xidfc_reg.value = 0 ;                                  // init with 0
    xidfc_reg.bits.FLESA = rel_filter_start_addr_word ;    // word address
    xidfc_reg.bits.LSE = number_of_filters ;              // size of the list in elements

    reg_set_and_check(can_ptr, ADR_M_CAN_XIDFC, xidfc_reg.value) ; // write configuration to M_CAN
}

/* Write a Standard Identifier Filter in Message RAM */
void m_can_filter_write_standard_id(
    can_struct * can_ptr,
    int filter_index,
    SFT_Standard_Filter_Type_enum sft,
    Filter_Configuration_enum sfec,
    int sfid1,
    int sfid2)
{
    /* Description: Writes a 11-bit standard id filter element in the Message RAM
     * Parameters:
     * - can_ptr:      pointer to the M_CAN node, where the configuration should be applied
     * - filter_index: index at which the filter element should be written in the '11-bit Filter' section of Message RAM
     * - sft:          Standard Filter Type: Range Filter, Dual ID filter, Classic filter, Filter element disabled
     * - sfec:         Standard Filter element configuration
     * - sfid1:        Standard Filter ID 1
     * - sfid2:		   Standard Filter ID 2
     */

    M_CAN_STD_ID_FILT_ELEMENT_union filter_elem ;
    unsigned int filter_address_offset ; // address offset inside the standard id filter section of the Message RAM

    // prepare Standard Message ID Filter Element
    filter_elem.value = 0 ;              // init with 0
    filter_elem.bits.SFID2 = sfid2 ;     // Standard Filter ID 2
    filter_elem.bits.SFID1 = sfid1 ;     // Standard Filter ID 1
    filter_elem.bits.SFEC = sfec ;      // Standard Filter Element Configuration
    filter_elem.bits.SFT = sft ;        // Standard Filter Type

    // calculate address offset inside the standard id filter section of the Message RAM
    filter_address_offset = filter_index * STD_ID_FILTER_ELEM_SIZE_WORD
                            * M_CAN_RAM_WORD_WIDTH_IN_BYTE ;
    // write filter element to Message RAM
    IOWR_32DIRECT(can_ptr->mram_sa.SIDFC_FLSSA,
                  filter_address_offset,
                  filter_elem.value) ;                                                     // Write the std id filter into Message RAM
}

/* Write Extended ID Filter in Message RAM */
void m_can_filter_write_extended_id(
    can_struct * can_ptr,
    int filter_index,
    EFT_Extended_Filter_Type_enum eft,
    Filter_Configuration_enum efec,
    int efid1,
    int efid2)
{
    /* Description:
     *  - Writes a 29-bit extended id filter element in the Message RAM
     *  - Size of an Extended Id filter element is 2 words. So 2 words are written into the Message RAM for each filter element
     *
     * Parameters:
     * - can_ptr:      pointer to the M_CAN node, where the configuration should be applied
     * - filter_index: index at which the filter element should be written in the '29-bit Filter' section of Message RAM
     * - eft:          Extended Filter Type. 00: Range Filter, 01: Dual ID filter, 10: Classic filter, 11: Range Filter; XIDAM mask not applied
     * - sfec:         Extended Filter element configuration
     * - sfid1:        Extended Filter ID 1
     * - sfid2:		   Extended Filter ID 2
     */

    M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_1_union xtd_filter_elem_word_1 ;
    M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_2_union xtd_filter_elem_word_2 ;
    uint32_t filter_address_offset_word_1, filter_address_offset_word_2 ; // address offsets inside the extended id filter section of the Message RAM

    // prepare Extended Message ID Filter Element
    xtd_filter_elem_word_1.value = 0 ;           // init with 0
    xtd_filter_elem_word_1.bits.EFEC = efec ;   // Extended Filter Element Configuration
    xtd_filter_elem_word_1.bits.EFID1 = efid1 ;  // Extended Filter ID 1
    filter_address_offset_word_1 = filter_index
                                   * EXT_ID_FILTER_ELEM_SIZE_WORD
                                   * M_CAN_RAM_WORD_WIDTH_IN_BYTE ;                                            // Address offset for word 1

    xtd_filter_elem_word_2.value = 0 ;           // init with 0
    xtd_filter_elem_word_2.bits.EFT = eft ;     // Extended Filter Type
    xtd_filter_elem_word_2.bits.EFID2 = efid2 ;  // Extended Filter ID 2
    filter_address_offset_word_2 = filter_address_offset_word_1
                                   + M_CAN_RAM_WORD_WIDTH_IN_BYTE ;                             // Address offset for word 2

    //write first word of the filter element into Message RAM
    IOWR_32DIRECT(can_ptr->mram_sa.XIDFC_FLESA,
                  filter_address_offset_word_1,
                  xtd_filter_elem_word_1.value) ;

    //write second word of the filter element into Message RAM
    IOWR_32DIRECT(can_ptr->mram_sa.XIDFC_FLESA,
                  filter_address_offset_word_2,
                  xtd_filter_elem_word_2.value) ;
}

/* Tx Event FIFO Configuration */
void m_can_tx_event_fifo_init(
    can_struct * can_ptr,
    int fifo_size_elems,
    int fifo_watermark)
{
    /* Description: function configures the register TXEFC - Tx Event FIFO Configuration
     *
     * Parameters
     *  - can_ptr:          pointer to the M_CAN node, where the configuration should be applied
     *  - fifo_size_elems:  number of elements in the Tx Event FIFO.
     *  - fifo_watermark:	watermark in number of Tx FIFO elements
     */

    M_CAN_TXEFC_union txefc_reg ;
    uint32_t rel_start_addr_word ;

    // calculate start address, that is relative to Message_RAM Base
    // conversion to word address
    rel_start_addr_word = CONV_BYTE2WORD_ADDR(
        can_ptr->mram_sa.TXEFC_EFSA - can_ptr->mram_base) ;

    txefc_reg.value = 0 ;                            // init with 0
    txefc_reg.bits.EFWM = fifo_watermark ;           // Tx Event FIFO Watermark
    txefc_reg.bits.EFS = fifo_size_elems ;          // Tx Event FIFO Size - number of FIFO elements
    txefc_reg.bits.EFSA = rel_start_addr_word ;      // Tx Event FIFO Start Address

    reg_set_and_check(can_ptr, ADR_M_CAN_TXEFC, txefc_reg.value) ;
}

/* Copy all Event Elements from TX Event FIFO to the Software Event List */
int m_can_tx_event_fifo_copy_element_to_tx_event_list(can_struct * can_ptr)
{
    /* Description:
     *  - ALL elements that are currently in Tx Event FIFO are copied to tx_event_list.
     *  - The elements are removed from Tx Event FIFO
     *  - If no element is in the Tx Event FIFO, nothing is done.
     *
     * Return Value
     *  - number of elements read from Tx Event FIFO
     */

    M_CAN_TXEFS_union txefs_reg ;
    uint32_t ram_read_addr = 0 ;
    int tx_event_elements_read = 0 ; // number of elements read from Tx Event FIFO

    txefs_reg.value = reg_get(can_ptr, ADR_M_CAN_TXEFS) ;

    // read ALL the elements from the Tx Event FIFO
    while ( txefs_reg.bits.EFFL ) { // while the Fill Level is !=0 read elements
        tx_event_elements_read++ ;

        // Calculate RAM address where Tx Event FIFO element is located, that should be read now
        ram_read_addr = can_ptr->mram_sa.TXEFC_EFSA
                        + (txefs_reg.bits.EFGI * TX_EVENT_FIFO_ELEM_SIZE_WORD
                           * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;

        /* Copy Tx Event FIFO element from Message RAM to tx_event_list */
        copy_event_element_from_msg_ram_to_tx_event_list(can_ptr, ram_read_addr) ;

        // Acknowledge READ to free Tx Event FIFO
        reg_set(can_ptr, ADR_M_CAN_TXEFA, txefs_reg.bits.EFGI) ;

        // read Tx Event FIFO Status
        txefs_reg.value = reg_get(can_ptr, ADR_M_CAN_TXEFS) ;
    }

    return tx_event_elements_read ; // return the number elements read from Tx Event FIFO
}

/* Copy a Tx Event FIFO Element from Message RAM to the Tx Event Element Data Structure given as Pointer */
void m_can_copy_tx_event_element_from_msg_ram(
    can_struct * can_ptr,
    uint32_t msg_addr_in_msg_ram,
    tx_event_element_struct *
    tx_event_element_ptr)
{
    /* Description: reads a Tx Event FIFO element from Message RAM and populates it in a pointer
     *
     * Parameters
     *  - can_ptr:              pointer to the M_CAN node, where the configuration should be applied
     *  - msg_addr_in_msg_ram:  address of the Tx Event element in the message RAM
     *  - tx_event_element_ptr: pointer to which the Tx Event element should be populated
     */

    M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_1_union tx_fifo_element_word_1 ;
    M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_2_union tx_fifo_element_word_2 ;

    INUTILE(can_ptr) ;

    // Read word_1
    tx_fifo_element_word_1.value = (uint32_t) IORD_32DIRECT(
        msg_addr_in_msg_ram,
        0
        * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;                                                                           // Read current RAM word

    // extract ESI from word_1 - TRUE/FALSE
    if ( tx_fifo_element_word_1.bits.ESI == 0 ) {
        tx_event_element_ptr->esi = FALSE ;
    }
    else {
        tx_event_element_ptr->esi = TRUE ;
    }

    // extract XTD and Msg.ID from word_1
    if ( tx_fifo_element_word_1.bits.XTD == 0 ) {
        tx_event_element_ptr->idtype = standard_id ;
        tx_event_element_ptr->id =
            (uint32_t) ( (tx_fifo_element_word_1.bits.ID
                          & TX_EVENT_FIFO_STDID_MASK)
                         >> TX_EVENT_FIFO_STDID_SHIFT ) ;
    }
    else {
        tx_event_element_ptr->idtype = extended_id ;
        tx_event_element_ptr->id = tx_fifo_element_word_1.bits.ID ;
    }

    // extract RTR from word_1 - TRUE/FALSE
    if ( tx_fifo_element_word_1.bits.RTR == 0 ) {
        tx_event_element_ptr->remote = FALSE ;
    }
    else {
        tx_event_element_ptr->remote = TRUE ;
    }

    // Read word_2
    tx_fifo_element_word_2.value = (uint32_t) IORD_32DIRECT(
        msg_addr_in_msg_ram,
        1
        * M_CAN_RAM_WORD_WIDTH_IN_BYTE) ;                                                                           // Read next RAM word

    // extract the Message Marker from word_2
    tx_event_element_ptr->mm = tx_fifo_element_word_2.bits.MM ;

    // extract the Event Type from word_2
    tx_event_element_ptr->et =
        (tx_event_fifo_elem_event_type_enum) tx_fifo_element_word_2.bits.ET ;

    // extract FDF from word_2 - TRUE/FALSE
    if ( tx_fifo_element_word_2.bits.FDF == 0 ) {
        tx_event_element_ptr->fdf = FALSE ;
    }
    else {
        tx_event_element_ptr->fdf = TRUE ;
    }

    // extract BRS from word_2 - TRUE/FALSE
    if ( tx_fifo_element_word_2.bits.BRS == 0 ) {
        tx_event_element_ptr->brs = FALSE ;
    }
    else {
        tx_event_element_ptr->brs = TRUE ;
    }

    // extract DLC from word_2
    tx_event_element_ptr->dlc = tx_fifo_element_word_2.bits.DLC ;

    // extract Tx Timestamp from word_2
    tx_event_element_ptr->txts = tx_fifo_element_word_2.bits.TXTS ;
}

/* Configure Time Stamp Usage  AND  Time Out Usage */
void m_can_timestampcounter_and_timeoutcounter_init(
    can_struct * can_ptr,
    int ts_counter_prescaler,
    tscc_tss_timestamp_select_enum
    ts_select,
    tocc_tos_timeout_select_enum
    to_select,
    uint16_t to_period,
    boolean to_counter_enable)
{
    /* Description: configures the registers for the
     *              1. Time Stamp Counter
     *              2. Time Out   Counter
     *
     * Remark: These (TimeOut, TimeStamp) use a shared parameter (the Prescaler).
     *         Because of that, the only safe way to configure these is to use a single function call.
     *
     * Parameters
     *  - can_ptr:                pointer to the M_CAN node, where the configuration should be applied
     *  for TimeStamp Counter (Register TSCC)
     *  - ts_counter_prescaler:	  Timestamp Counter Prescaler (Hint: in M_CAN 3.2.1 and before the TimeOutCounter uses also this prescaler)
     *  - ts_select:              Timestamp Select
     *  for TimeOut Counter   (Register TOCC)
     *  - to_select:              Timeout Select
     *  - to_period:              Timeout Period. Start value of the timeout counter (down counter)
     *  - to_counter_enable:      TRUE=> enable timeout counter, FALSE=> disable
     */

    // Check if ts_counter_prescaler <= TSCC_TCP_MAX_VALUE and => TSCC_TCP_MIN_VALUE
    assert_print_error(
        ts_counter_prescaler <= TSCC_TCP_MAX_VALUE && ts_counter_prescaler >=
        TSCC_TCP_MIN_VALUE,
        "Invalid Time stamp counter prescaler value. Valid values are [1..16]") ;

    /* Timestamp counter configuration */
    M_CAN_TSCC_union tscc_reg ;
    tscc_reg.value = 0 ;                        // init with 0
    tscc_reg.bits.TCP = ts_counter_prescaler - 1 ; // timestamp counter prescaler
    tscc_reg.bits.TSS = ts_select ;              // timestamp select
    reg_set_and_check(can_ptr, ADR_M_CAN_TSCC, tscc_reg.value) ;

    /* Timeout counter configuration */
    M_CAN_TOCC_union tocc_reg ;
    tocc_reg.value = 0 ;                 // init with 0
    tocc_reg.bits.TOS = to_select ;      // timeout select
    tocc_reg.bits.TOP = to_period ;      // timeout period
    if ( to_counter_enable == TRUE ) {
        tocc_reg.bits.ETOC = 1 ;          // enable timeout counter
    }
    else if ( to_counter_enable == FALSE ) {
        tocc_reg.bits.ETOC = 0 ;          // disable timeout counter
    }
    reg_set_and_check(can_ptr, ADR_M_CAN_TOCC, tocc_reg.value) ;
}

/* Read Time Stamp Value */
uint32_t m_can_timestamp_read_value(can_struct * can_ptr)
{
    // Description : returns the Timestamp Counter value

    M_CAN_TSCV_union tscv_reg ;

    // read the contents in TSCV register
    tscv_reg.value = reg_get(can_ptr, ADR_M_CAN_TSCV) ;

    return tscv_reg.bits.TSC ;
}

/* Check correctness of Message RAM Partitioning */
void m_can_check_msg_ram_partitioning(can_struct * can_ptr)
{
#ifdef NDEBUG
    INUTILE(can_ptr) ;
#else
    int i ;

    // Description: Check if the Message RAM Partitioning prepared with defines in m_can.h
    //              is realizable with the given Message RAM size
    for ( i = 0 ; i < CAN_NUMBER_OF_PRESENT_NODES ; ++i ) {
        // EXPRESSIONS HAVE TO BE TRUE, otherwise program aborts
        assert_print_error(
            ( REL_TXBC_END <=
              ( (can_ptr->mram_size_words * M_CAN_RAM_WORD_WIDTH_IN_BYTE) - 1 ) ),
            "M_CAN is configured to use more MessageRAM than allowed by define >RAM_BYTES_PER_M_CAN=%d<!",
            REL_TXBC_END) ;
    }
#endif
}

/* Init struct that contains Message RAM Partitioning for one CAN node */
void m_can_init_msg_ram_partitioning(can_struct * can_ptr)
{
    // initialize the Start Addresses for all types of elements for a CAN node
    // - ABSOLUTE Start Addresses!
    // - BYTE addressing
    can_ptr->mram_sa.SIDFC_FLSSA = can_ptr->mram_base + REL_SIDFC_FLSSA ;
    can_ptr->mram_sa.XIDFC_FLESA = can_ptr->mram_base + REL_XIDFC_FLESA ;
    can_ptr->mram_sa.RXF0C_F0SA = can_ptr->mram_base + REL_RXF0C_F0SA ;
    can_ptr->mram_sa.RXF1C_F1SA = can_ptr->mram_base + REL_RXF1C_F1SA ;
    can_ptr->mram_sa.RXBC_RBSA = can_ptr->mram_base + REL_RXBC_RBSA ;
    can_ptr->mram_sa.TXEFC_EFSA = can_ptr->mram_base + REL_TXEFC_EFSA ;
    can_ptr->mram_sa.TXBC_TBSA = can_ptr->mram_base + REL_TXBC_TBSA ;

    // Check correctness of Message RAM Partitioning
    m_can_check_msg_ram_partitioning(can_ptr) ;
}

// Function decodes (prints) verbosely the occurred M_CAN Interrupt
void m_can_ir_print(
    int m_can_id,
    int ir_value)
{
    M_CAN_IR_union ir_reg_union ;
    ir_reg_union.value = ir_value ;

    INUTILE(m_can_id) ;

    DBG_PRINTF("[M_CAN_%d] IR occurred.           IR reg: 0x%8.8x",
               m_can_id,
               ir_value) ;

    if ( DEBUG_PRINT ) {
        /* decode IR - Interrupt Register with help of a union */
        //  0: false, >0: true
        if ( ir_reg_union.bits.RF0N ) {
            DBG_PRINTF(" RF0N (bit 00, Rx FIFO 0 New Message)\n") ;
        }
        if ( ir_reg_union.bits.RF0W ) {
            DBG_PRINTF(" RF0W (bit 01, Rx FIFO 0 Watermark Reached)\n") ;
        }
        if ( ir_reg_union.bits.RF0F ) {
            DBG_PRINTF(" RF0F (bit 02, Rx FIFO 0 Full)\n") ;
        }
        if ( ir_reg_union.bits.RF0L ) {
            DBG_PRINTF(" RF0L (bit 03, Rx FIFO 0 Message Lost)\n") ;
        }
        if ( ir_reg_union.bits.RF1N ) {
            DBG_PRINTF(" RF1N (bit 04, Rx FIFO 1 New Message)\n") ;
        }
        if ( ir_reg_union.bits.RF1W ) {
            DBG_PRINTF(" RF1W (bit 05, Rx FIFO 1 Watermark Reached)\n") ;
        }
        if ( ir_reg_union.bits.RF1F ) {
            DBG_PRINTF(" RF1F (bit 06, Rx FIFO 1 Full)\n") ;
        }
        if ( ir_reg_union.bits.RF1L ) {
            DBG_PRINTF(" RF1L (bit 07, Rx FIFO 1 Message Lost)\n") ;
        }
        if ( ir_reg_union.bits.HPM ) {
            DBG_PRINTF(" HPM  (bit 08, High Priority Message)\n") ;
        }
        if ( ir_reg_union.bits.TC ) {
            DBG_PRINTF(" TC   (bit 09, Transmission Completed)\n") ;
        }
        if ( ir_reg_union.bits.TCF ) {
            DBG_PRINTF(" TCF  (bit 10, Transmission Cancellation Finished)\n") ;
        }
        if ( ir_reg_union.bits.TFE ) {
            DBG_PRINTF(" TFE  (bit 11, Tx FIFO Empty)\n") ;
        }
        if ( ir_reg_union.bits.TEFN ) {
            DBG_PRINTF(" TEFN (bit 12, Tx Event FIFO New Entry)\n") ;
        }
        if ( ir_reg_union.bits.TEFW ) {
            DBG_PRINTF(" TEFW (bit 13, Tx Event FIFO Watermark Reached)\n") ;
        }
        if ( ir_reg_union.bits.TEFF ) {
            DBG_PRINTF(" TEFF (bit 14, Tx Event FIFO Full)\n") ;
        }
        if ( ir_reg_union.bits.TEFL ) {
            DBG_PRINTF(" TEFL (bit 15, Tx Event FIFO Element Lost)\n") ;
        }
        if ( ir_reg_union.bits.TSW ) {
            DBG_PRINTF(" TSW  (bit 16, Timestamp Wraparound)\n") ;
        }
        if ( ir_reg_union.bits.MRAF ) {
            DBG_PRINTF(" MRAF (bit 17, Unprocessed Message Discarded)\n") ;
        }
        if ( ir_reg_union.bits.TOO ) {
            DBG_PRINTF(" TOO  (bit 18, Timeout Occurred)\n") ;
        }
        if ( ir_reg_union.bits.DRX ) {
            DBG_PRINTF(
                " DRX  (bit 19, Message stored to Dedicated RX Buffer)\n") ;
        }
        if ( ir_reg_union.bits.BEC ) {
            DBG_PRINTF(" BEC  (bit 20, Bit Error Corrected)\n") ;
        }
        if ( ir_reg_union.bits.BEU ) {
            DBG_PRINTF(" BEU  (bit 21, Bit Error Uncorrected)\n") ;
        }
        if ( ir_reg_union.bits.ELO ) {
            DBG_PRINTF(" ELO  (bit 22, Error Logging Overflow)\n") ;
        }
        if ( ir_reg_union.bits.EP ) {
            DBG_PRINTF(" EP   (bit 23, Error Passive)\n") ;
        }
        if ( ir_reg_union.bits.EW ) {
            DBG_PRINTF(" EW   (bit 24, Warning Status)\n") ;
        }
        if ( ir_reg_union.bits.BO ) {
            DBG_PRINTF(" BO   (bit 25, Bus_Off Status)\n") ;
        }
        if ( ir_reg_union.bits.WDI ) {
            DBG_PRINTF(" WDI  (bit 26, Watchdog Interrupt)\n") ;
        }
        if ( ir_reg_union.bits.PEA ) {
            DBG_PRINTF(" PEA  (bit 27, Protocol Error in Arbitration Phase)\n") ;
        }
        if ( ir_reg_union.bits.PED ) {
            DBG_PRINTF(" PED  (bit 28, Protocol Error in Data Phase)\n") ;
        }
        if ( ir_reg_union.bits.ARA ) {
            DBG_PRINTF(" ARA  (bit 29, Access to Reserved Address)\n") ;
        }
    } // if print verbose
}

/* Check the M_CAN & M_TTCAN Register properties */
void m_can_register_test(
    can_struct * can_ptr,
    boolean reset_value_test_only)
{
    /* Description:
     * This test function performs the following tests. Both M_CAN and M_TTCAN registers are tested.
     * 1. reset_value_test => - Test if the registers contain proper reset values
     * 2. write_test       => - Test if a Normal WRITE is possible to writable bits(RW) in a register
     *                        - Test if a WRITE is possible to write Protected bits(RP) in a register
     *                        - Test if a Read Only bit is writable in a register
     *                        - Does not modify register content (i.e. restores previous content after test)
     *                        - Therefore M_CAN or M_TTCAN is set temporarily to INIT and CCE Mode! (= M_CAN is disconnected from the bus)
     * Parameters:
     *  - can_ptr             :  pointer to the M_CAN node, of which the registers should be tested
     *  - m_can_regs_only     :  TRUE  => Check only M_CAN specific registers
     *                           FALSE => Check M_CAN and M_TTCAN specific registers
     *  - reset_value_test_only:  TRUE  => Perform only the reset value test
     *                           FALSE => Perform all tests. Reset value and write tests
     * Note:
     *  - function tests M_CAN or M_TTCAN register set based on "can_ptr->is_m_ttcan_node == TRUE/FALSE"
     *	 - Some bits (special cases) are not checked for the writeability to keep the test understandable.
     *	   The comments in the structure definition of m_can_register_properties mention the excluded bits.
     */

    typedef const struct {
        char name[8] ;
        uint16_t addr ;
        unsigned int value ;
        unsigned int mask_check_write ;         // 1= bit is normal writable
        unsigned int mask_check_write_protected ; // 1= bit is only writable in protected mode
        boolean m_can_only ;                    // TRUE=> M_CAN register, FALSE=> M_TTCAN specific register
        boolean apply_normal_write_test ;       // TRUE=> Test the normal writable bits in an RW register
        boolean apply_protected_write_test ;    // TRUE=> Test the write protected bits in an RP register
    } register_properties ;

    // MZ pro stack
    static const
    register_properties m_can_register_properties[] = {
        //name,    addr,             value,            mask_wr,    mask_wp,   m_can, apply_wr, apply_wp
        // MISC ------------------------- 7
        { "CREL  ", ADR_M_CAN_CREL, DEF_M_CAN_CREL, 0x00000000, 0x00000000,
          TRUE,
          FALSE, FALSE },
        { "ENDN  ", ADR_M_CAN_ENDN, DEF_M_CAN_ENDN, 0x00000000, 0x00000000,
          TRUE,
          FALSE, FALSE },
        { "DBTP  ", ADR_M_CAN_DBTP, DEF_M_CAN_DBTP, 0x00000000, 0x009F1FFF,
          TRUE,
          FALSE, TRUE  },
        { "TEST  ", ADR_M_CAN_TEST, DEF_M_CAN_TEST, 0x00000000, 0x00000070,
          TRUE,
          FALSE, FALSE },
        { "RWD   ", ADR_M_CAN_RWD, DEF_M_CAN_RWD, 0x00000000, 0x0000FFFF, TRUE,
          FALSE, TRUE  },
        //MZ{"CCCR  ", ADR_M_CAN_CCCR  , DEF_M_CAN_CCCR  , 0x00000011, 0x0000F3C7, TRUE, FALSE, TRUE  }, // INIT bit(RW) has an update delay. Nevertheless, INIT and CCE bits are implicitly checked during configuration change enable. Therefore, omit these the bits from write test.
        // MZ metto falso perche' contiene i bit di protezione!
        { "CCCR  ", ADR_M_CAN_CCCR, DEF_M_CAN_CCCR, 0x00000011, 0x0000F3C7,
          TRUE,
          FALSE, FALSE },
        { "NBTP  ", ADR_M_CAN_NBTP, DEF_M_CAN_NBTP, 0x00000000, 0xFFFFFF7F,
          TRUE,
          FALSE, TRUE  },
        // Timestamp & Timeout ---------- 4
        { "TSCC  ", ADR_M_CAN_TSCC, DEF_M_CAN_TSCC, 0x00000000, 0x000F0003,
          TRUE,
          FALSE, TRUE  },
        { "TSCV  ", ADR_M_CAN_TSCV, DEF_M_CAN_TSCV, 0x00000000, 0x00000000,
          TRUE,
          FALSE, FALSE },
        { "TOCC  ", ADR_M_CAN_TOCC, DEF_M_CAN_TOCC, 0x00000000, 0xFFFF0007,
          TRUE,
          FALSE, TRUE  },
        { "TOCV  ", ADR_M_CAN_TOCV, DEF_M_CAN_TOCV, 0x00000000, 0x00000000,
          TRUE,
          FALSE, FALSE },
        // Misc      -------------------- 3
        { "ECR   ", ADR_M_CAN_ECR, DEF_M_CAN_ECR, 0x00000000, 0x00000000, TRUE,
          FALSE, FALSE },
        { "PSR   ", ADR_M_CAN_PSR, DEF_M_CAN_PSR, 0x00000000, 0x00000000, TRUE,
          FALSE, FALSE },
        { "TDCR  ", ADR_M_CAN_TDCR, DEF_M_CAN_TDCR, 0x00000000, 0x00007F7F,
          TRUE,
          FALSE, TRUE  },
        // Interrupt -------------------- 4
        { "IR    ", ADR_M_CAN_IR, DEF_M_CAN_IR, 0x3FFFFFFF, 0x3FFFFFFF, TRUE,
          FALSE, FALSE },                                                                             // the register is writable. But writing a 1 clears the bit position and so impossible to validate the write. So, omit this register from Write tests
        { "IE    ", ADR_M_CAN_IE, DEF_M_CAN_IE, 0x3FFFFFFF, 0x3FFFFFFF, TRUE,
          TRUE, FALSE },
        { "ILS   ", ADR_M_CAN_ILS, DEF_M_CAN_ILS, 0x3FFFFFFF, 0x3FFFFFFF, TRUE,
          TRUE, FALSE },
        { "ILE   ", ADR_M_CAN_ILE, DEF_M_CAN_ILE, 0x00000003, 0x00000003, TRUE,
          TRUE, FALSE },
        // Rx Message Handler ----------- 15
        { "GFC   ", ADR_M_CAN_GFC, DEF_M_CAN_GFC, 0x00000000, 0x0000003F, TRUE,
          FALSE, TRUE  },
        { "SIDFC ", ADR_M_CAN_SIDFC, DEF_M_CAN_SIDFC, 0x00000000, 0x00FFFFFC,
          TRUE, FALSE, TRUE  },
        { "XIDFC ", ADR_M_CAN_XIDFC, DEF_M_CAN_XIDFC, 0x00000000, 0x007FFFFC,
          TRUE, FALSE, TRUE  },
        { "XIDAM ", ADR_M_CAN_XIDAM, DEF_M_CAN_XIDAM, 0x00000000, 0x1FFFFFFF,
          TRUE, FALSE, TRUE  },
        { "HPMS  ", ADR_M_CAN_HPMS, DEF_M_CAN_HPMS, 0x00000000, 0x00000000,
          TRUE,
          FALSE, FALSE },
        { "NDAT1 ", ADR_M_CAN_NDAT1, DEF_M_CAN_NDAT1, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, FALSE, FALSE },                                                                        // the register is writable. But writing a 1 clears the bit position and so impossible to validate the write. So, omit this register from write tests
        { "NDAT2 ", ADR_M_CAN_NDAT2, DEF_M_CAN_NDAT2, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, FALSE, FALSE },                                                                        // the register is writable. But writing a 1 clears the bit position and so impossible to validate the write. So, omit this register from write tests
        { "RXF0C ", ADR_M_CAN_RXF0C, DEF_M_CAN_RXF0C, 0x00000000, 0xFF7FFFFC,
          TRUE, FALSE, TRUE  },
        { "RXF0S ", ADR_M_CAN_RXF0S, DEF_M_CAN_RXF0S, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "RXF0A ", ADR_M_CAN_RXF0A, DEF_M_CAN_RXF0A, 0x0000003F, 0x0000003F,
          TRUE, TRUE, FALSE },
        { "RXBC  ", ADR_M_CAN_RXBC, DEF_M_CAN_RXBC, 0x00000000, 0x0000FFFC,
          TRUE,
          FALSE, TRUE  },
        { "RXF1C ", ADR_M_CAN_RXF1C, DEF_M_CAN_RXF1C, 0x00000000, 0xFF7FFFFC,
          TRUE, FALSE, TRUE  },
        { "RXF1S ", ADR_M_CAN_RXF1S, DEF_M_CAN_RXF1S, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "RXF1A ", ADR_M_CAN_RXF1A, DEF_M_CAN_RXF1A, 0x0000003F, 0x0000003F,
          TRUE, TRUE, FALSE },
        { "RXESC ", ADR_M_CAN_RXESC, DEF_M_CAN_RXESC, 0x00000000, 0x00000777,
          TRUE, FALSE, TRUE  },
        // Tx Message Handler ----------- 13
        { "TXBC  ", ADR_M_CAN_TXBC, DEF_M_CAN_TXBC, 0x00000000, 0x7F3FFFFC,
          TRUE,
          FALSE, TRUE  },
        { "TXFQS ", ADR_M_CAN_TXFQS, DEF_M_CAN_TXFQS, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "TXESC ", ADR_M_CAN_TXESC, DEF_M_CAN_TXESC, 0x00000000, 0x00000007,
          TRUE, FALSE, TRUE  },
        { "TXBRP ", ADR_M_CAN_TXBRP, DEF_M_CAN_TXBRP, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "TXBAR ", ADR_M_CAN_TXBAR, DEF_M_CAN_TXBAR, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, FALSE, FALSE },                                                                        // bits in this register are writable only for those Tx buffers that are configured via TXBC. Omit this register from write tests as this is a special case.
        { "TXBCR ", ADR_M_CAN_TXBCR, DEF_M_CAN_TXBCR, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, FALSE, FALSE },                                                                        // bits in this register are writable only for those Tx buffers that are configured via TXBC. Omit this register from write tests as this is a special case.
        { "TXBTO ", ADR_M_CAN_TXBTO, DEF_M_CAN_TXBTO, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "TXBCF ", ADR_M_CAN_TXBCF, DEF_M_CAN_TXBCF, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "TXBTIE", ADR_M_CAN_TXBTIE, DEF_M_CAN_TXBTIE, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, TRUE, FALSE },
        { "TXBCIE", ADR_M_CAN_TXBCIE, DEF_M_CAN_TXBCIE, 0xFFFFFFFF, 0xFFFFFFFF,
          TRUE, TRUE, FALSE },
        { "TXEFC ", ADR_M_CAN_TXEFC, DEF_M_CAN_TXEFC, 0x00000000, 0x3F3FFFFC,
          TRUE, FALSE, TRUE  },
        { "TXEFS ", ADR_M_CAN_TXEFS, DEF_M_CAN_TXEFS, 0x00000000, 0x00000000,
          TRUE, FALSE, FALSE },
        { "TXEFA ", ADR_M_CAN_TXEFA, DEF_M_CAN_TXEFA, 0x0000001F, 0x0000001F,
          TRUE, TRUE, FALSE },
        // M_TTCAN specific registers ---- 17
        // TT Registers have to be at the END of this array!
        { "TTTMC ", ADR_M_CAN_TTTMC, DEF_M_CAN_TTTMC, 0x00000000, 0x007FFFFC,
          FALSE, FALSE, TRUE  },
        { "TTRMC ", ADR_M_CAN_TTRMC, DEF_M_CAN_TTRMC, 0x00000000, 0xDFFFFFFF,
          FALSE, FALSE, TRUE  },
        { "TTOCF ", ADR_M_CAN_TTOCF, DEF_M_CAN_TTOCF, 0x00000000, 0x07FFFFFB,
          FALSE, FALSE, TRUE  },
        { "TTMLM ", ADR_M_CAN_TTMLM, DEF_M_CAN_TTMLM, 0x00000000, 0x0FFF0FFF,
          FALSE, FALSE, TRUE  },
        { "TURCF ", ADR_M_CAN_TURCF, DEF_M_CAN_TURCF, 0x00000000, 0xBFFFFFFF,
          FALSE, FALSE, FALSE },                                                                        // some bits in this register can be set only on specific conditions and 0 is an illegal value for a few bits. So omitting to keep the test simple and understandable.
        { "TTOCN ", ADR_M_CAN_TTOCN, DEF_M_CAN_TTOCN, 0x00002FFC, 0x00002FFC,
          FALSE, FALSE, FALSE },                                                                        // bits NIG, ECS and SGT can be set only on specific conditions. Omitting these bits from normal write test for simplicity of test. So the mask value is set accordingly
        { "TTGTP ", ADR_M_CAN_TTGTP, DEF_M_CAN_TTGTP, 0xFFFFFFFF, 0xFFFFFFFF,
          FALSE, FALSE, FALSE },                                                                        // all the bits in this register are write-protected and are writable only on specific conditions. Omit this register from write tests as this is a special case.
        { "TTTMK ", ADR_M_CAN_TTTMK, DEF_M_CAN_TTTMK, 0x007FFFFF, 0x007FFFFF,
          FALSE, TRUE, FALSE },
        { "TTIR  ", ADR_M_CAN_TTIR, DEF_M_CAN_TTIR, 0xFFFFFFFF, 0xFFFFFFFF,
          FALSE, FALSE, FALSE },                                                                        // the register is writable. But writing a 1 clears the bit position and so impossible to validate the write. Therefore, Mask value omits all the bits from write test.
        { "TTIE  ", ADR_M_CAN_TTIE, DEF_M_CAN_TTIE, 0x0007FFFF, 0x0007FFFF,
          FALSE, TRUE, FALSE },
        { "TTILS ", ADR_M_CAN_TTILS, DEF_M_CAN_TTILS, 0x0007FFFF, 0x0007FFFF,
          FALSE, TRUE, FALSE },
        { "TTOST ", ADR_M_CAN_TTOST, DEF_M_CAN_TTOST, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
        { "TURNA ", ADR_M_CAN_TURNA, DEF_M_CAN_TURNA, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
        { "TTLGT ", ADR_M_CAN_TTLGT, DEF_M_CAN_TTLGT, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
        { "TTCTC ", ADR_M_CAN_TTCTC, DEF_M_CAN_TTCTC, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
        { "TTCPT ", ADR_M_CAN_TTCPT, DEF_M_CAN_TTCPT, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
        { "TTCSM ", ADR_M_CAN_TTCSM, DEF_M_CAN_TTCSM, 0x00000000, 0x00000000,
          FALSE, FALSE, FALSE },
    } ;

    unsigned int i, j, number_of_registers_to_check = 0 ;
    uint32_t value_read_reg, expected_value_reg, original_value_reg,
             read_value_witten_to_reg, masked_result, set_value_reg ;
    boolean reg_reset_values_ok = TRUE, reg_write_ok = TRUE,
            reg_write_prot_ok = TRUE ;

    if ( can_ptr->is_m_ttcan_node == FALSE ) {
        // M_CAN register test
        DBG_PRINTF(" M_CAN Version 3.2.x Register Test\n") ;
        DBG_PRINTF(" M_CAN Base_Addr: 0x%08x", can_ptr->base) ;

        // this loop counts the number of m_can specific registers
        for ( i = 0 ; i < (int) ARRAYSIZE(m_can_register_properties) ; i++ ) {
            if ( m_can_register_properties[i].m_can_only == TRUE ) {
                number_of_registers_to_check++ ;
            }
        }
    }
    else {
        // M_TTCAN register test
        DBG_PRINTF(" M_TTCAN Version 3.2.x Register Test\n") ;
        DBG_PRINTF(" M_CAN Base_Addr: 0x%08x", can_ptr->base) ;
        number_of_registers_to_check = (int) ARRAYSIZE(
            m_can_register_properties) ;
    }

    /* Test - 1 => Test if the registers contain proper reset values
     *   - read value from each register and check if the register contains the proper reset value */
    DBG_PRINTF(" => Reset value Test: \n") ;

    // for all register do...
    for ( i = 0 ; i < number_of_registers_to_check ; i++ ) {
        value_read_reg = reg_get(can_ptr, m_can_register_properties[i].addr) ;
        expected_value_reg = m_can_register_properties[i].value ;
        if ( value_read_reg != expected_value_reg ) {
            DBG_PRINTF(
                "      Read Error:  %s (0x%03x): read: 0x%08x, expected read: 0x%08x)",
                m_can_register_properties[i].name,
                m_can_register_properties[i].addr,
                value_read_reg,
                expected_value_reg) ;
            reg_reset_values_ok = FALSE ;
        } // if
    } // for

    if ( reg_reset_values_ok == TRUE ) {
        DBG_PRINTF("PASS") ;
    }                                                      // Test-1 completed

    if ( reset_value_test_only == TRUE ) {
        DBG_PRINTF("\n => Number of registers checked: %u",
                   number_of_registers_to_check) ;
        return ; // after the reset_value_test, do not proceed to the write test
    } // if

    /* Test - 2 => Test if a Normal WRITE is possible to writable bits(RW) in a register
     *   - write '1' to all the normal writable bits (RW) and check if write happened
     *   - write '0' to all the normal writable bits (RW) and check if write happened */
    DBG_PRINTF("\n => Normal writable Bits Test: ") ;

    // for all register do...
    for ( i = 0 ; i < number_of_registers_to_check ; i++ ) {
        if ( m_can_register_properties[i].apply_normal_write_test == TRUE ) {
            // read the value of the register. This value will be restored at the end of test.
            original_value_reg = reg_get(can_ptr,
                                         m_can_register_properties[i].addr) ;

            /* this loop runs twice
             *  - run 1: write '1' to all Normal Writable bits and check if write was possible
             *  - run 2: write '0' to all Normal Writable bits and check if write was possible */
            for ( j = 0 ; j < 2 ; j++ ) {
                if ( j == 0 ) { // loop run-1
                    set_value_reg =
                        m_can_register_properties[i].mask_check_write ;
                }
                else {   // loop run-2
                    set_value_reg = 0x0 ;
                }

                reg_set(can_ptr,
                        m_can_register_properties[i].addr,
                        set_value_reg) ;                                          // write to the Normal Writable bits of the register

                read_value_witten_to_reg = reg_get(
                    can_ptr,
                    m_can_register_properties[i]
                    .addr) ;                                                                  // read the value that was just written in the register

                // do a bitwise AND with the WRITE Mask and the 'new value read' from register and compare
                masked_result = read_value_witten_to_reg
                                & m_can_register_properties[i].mask_check_write ;
                if ( set_value_reg != masked_result ) {
                    reg_write_ok = FALSE ;
                    DBG_PRINTF(
                        "\n      Write Error: %s (0x%03x): written: 0x%08x, read: 0x%08x, expected read: 0x%08x",
                        (char *) &m_can_register_properties[i].name,
                        m_can_register_properties[i].addr,
                        set_value_reg,
                        read_value_witten_to_reg,
                        set_value_reg) ;
                } // if
            } // for

            // set the register value to its initial value as the test is complete
            reg_set_and_check(can_ptr,
                              m_can_register_properties[i].addr,
                              original_value_reg) ;
        } // if RW register
    } //for

    if ( reg_write_ok == TRUE ) {
        DBG_PRINTF("PASS") ;
    }                                               // Test-2 completed message

    /* Test - 3 => Test if a WRITE is possible to write Protected bits(RP) in a register
     * Steps followed :
     *      -   enable configuration change mode to enable writes to RP registers
     *      -	clear all the bits
     *      -   write '1' to all the write protected bits (RP) and check if the write happened
     *      -   write '0' to all the write protected bits (RP) and check if the write happened
     *      -   disable configuration change mode */
    DBG_PRINTF("\n => Protected writable Bits Test: ") ;

    // for all registers
    for ( i = 0 ; i < number_of_registers_to_check ; i++ ) {
        if ( m_can_register_properties[i].apply_protected_write_test ==
             TRUE ) {
            // read the original value in the register. This value will be restored at the end of test
            original_value_reg = reg_get(can_ptr,
                                         m_can_register_properties[i].addr) ;

            // set CCCR.INIT=1 and CCCR.CCE=1 so that the RP bits can be write enabled.
            m_can_set_config_change_enable(can_ptr) ;

            /* this loop runs twice
            *   - run 1: write '1' to all write protected bits and check if write was possible
            *   - run 2: write '0' to all write protected bits and check if write was possible
            */
            for ( j = 0 ; j < 2 ; j++ ) {
                if ( j == 0 ) { // loop run-1
                    set_value_reg =
                        m_can_register_properties[i].mask_check_write_protected ;
                }
                else {    // loop run-2
                    set_value_reg = 0x0 ;
                }

                reg_set(can_ptr,
                        m_can_register_properties[i].addr,
                        set_value_reg) ;                                          // write to the write protected bits of the register

                read_value_witten_to_reg = reg_get(
                    can_ptr,
                    m_can_register_properties[i]
                    .addr) ;                                                                  // read the value that was just written in the register

                // do a bitwise AND with the Write Mask and the 'new value read' from register and compare
                masked_result = read_value_witten_to_reg
                                & m_can_register_properties[i].
                                mask_check_write_protected ;
                if ( set_value_reg != masked_result ) {
                    reg_write_prot_ok = FALSE ;
                    DBG_PRINTF(
                        "\n      Write Error: %s (0x%03x): written: 0x%08x, read: 0x%08x, expected read: 0x%08x",
                        (char *) &m_can_register_properties[i].name,
                        m_can_register_properties[i].addr,
                        set_value_reg,
                        read_value_witten_to_reg,
                        set_value_reg) ;
                } // if
            } // for

            reg_set_and_check(can_ptr,
                              m_can_register_properties[i].addr,
                              original_value_reg) ;                                          // restore the initial value in the register

            // set CCCR.INIT=0 and CCCR.CCE=0. Disable the configuration change mode
            m_can_reset_config_change_enable(can_ptr) ;
        } //if
    } //for

    if ( reg_write_prot_ok == TRUE ) {
        DBG_PRINTF("PASS") ;
    }                                                     // Test-3 completed message

    /* Test - 4: Test if a READ ONLY Bit is writable => not implemented yet
     * Description:
     * In normal operation mode (init = '1')
     *  - write '0' to read only bits => check that this did not happen
     *  - write '1' to read only bits => check that this did not happen
     */

    // end of test case, print number tested registers
    DBG_PRINTF("\n => Number of registers checked: %u",
               number_of_registers_to_check) ;
} // end of m_can_register_test()
