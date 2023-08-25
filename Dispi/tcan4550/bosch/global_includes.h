/* All needed header files */

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

#ifndef M_CAN_INCLUDES_H
#define M_CAN_INCLUDES_H

/* Global Definitions */
#include "global_settings.h"  // has to be included prior to types.h, otherwise the Software doesn't work with Xilinx Microblaze/SDK
#include "types.h"            // user defined types
#include "global_defines.h"   // helpful defines used all over the software

/* C99 data types */
#include <stdint.h>
/* printf uint32_t format macro */
#include <inttypes.h>
/* Common C header files */
//#define NDEBUG // If this is defined, asserts are turned OFF. Comment out this line to enable asserts.
#include "assert.h"
#include "stdio.h"
#include "string.h" /* memset */

// MZ -----------------------------------
#define STAMPA_DBG
#include "utili.h"
//#include "../../cod/tcan4550.h"

// Molto verboso
//#define STAMPA_REGISTRI

// sostituisce la scrittura a registro con quella a gruppi
//#define MRAM_OTTIMIZZATA		1

extern void mcan_tx_fifo_empty_cb(void) ;
extern void mcan_rx_fifo_msg_cb(void) ;
extern void mcan_copia_in_mram(
    uint32_t indir,
    const void * v,
    int dim) ;

#define CAN_NUMBER_OF_PRESENT_NODES     2

// sul tcan4550 i registri del m_can partono da qua
#define REG_BASE        0x1000

extern uint32_t * questo_reg_leggi(
    uint16_t numreg,
    uint16_t primo) ;

static inline uint32_t IORD_32DIRECT(
    uint32_t base,
    uint16_t reg)
{
    if ( base ) {
        // MRAM
        reg += base ;
    }
    else {
        // Registro
        reg += REG_BASE ;
    }

    uint32_t * pV = questo_reg_leggi(1, reg) ;
#ifdef STAMPA_REGISTRI
    DBG_PRINTF("[%04X] -> 0x%08X", reg, *pV) ;
#endif
    return *pV ;
}

extern bool questo_reg_scrivi(
    uint16_t numreg,
    uint16_t primo,
    const uint32_t * val) ;

static inline void IOWR_32DIRECT(
    uint32_t base,
    uint16_t reg,
    uint32_t value)
{
    if ( base ) {
        // MRAM
        reg += base ;
    }
    else {
        // Registro
        reg += REG_BASE ;
    }
#ifdef STAMPA_REGISTRI
    DBG_PRINTF("[%04X] <- 0x%08X", reg, value) ;
#endif
    (void) questo_reg_scrivi(1, reg, &value) ;
}

extern void m_can_disable_interrupt(int can_id) ;
extern void m_can_enable_interrupt(int can_id) ;

// ----------------------------------- MZ

#if __ALTERA
/* ALTERA C header file(s) */
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/alt_irq.h"

#elif __XILINX
/* XILINX C header files */
#include "xparameters.h"
#include "xil_cache.h"
#include "xbasic_types.h"
#include "xil_io.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"

/* Xilinx C header file(s) - modified by Bosch */
#include "xilinx/xilinx_specific.h" // Xilinx specific redefines (important)
#include "xilinx/xilinx_gpio.h"
#include "xilinx/xilinx_intcontroller.h"
#endif // __XILINX

/* Common User Defined header files */
#include "peripherals.h"     // FPGA board peripheral specific functions
#include "can_io_wiring/can_io_wiring.h"   // CAN IO Wiring Module drivers

/* M_CAN IP related header files */
#include "m_can/m_can_regdef.h"
#include "m_can/m_can.h"
#include "m_can/m_can_irq_handling.h"
#include "m_can/m_can_helper_func.h"

/* M_CAN Application Examples - header files */
#include "app_notes/app_note_001_rx_handling.h"
#include "app_notes/app_note_002_tx_handling.h"

#ifdef __PNS // Plug and Secure for CAN available
/* PnS related header files */
#include "pns_driver/pns_regdef.h"
#include "pns_driver/pns.h"
#endif

#if __ALTERA && __BOSCH_INTERNAL_TESTS
// these test cases are only developed under/for Altera
#include "m_can_tests_internal/m_can_test_internal_menu.h"
#include "m_can_tests_internal/m_can_random_msg_gen.h"
#include "m_can_tests_internal/m_can_test_help_func.h"
#include "m_can_tests_internal/m_can_test_01_normal_tx_rx.h"
#include "m_can_tests_internal/m_can_test_01_normal_tx_rx_old.h"
#include "m_can_tests_internal/m_can_test_02_CCCR.h"
#include "m_can_tests_internal/m_can_test_03_timeout.h"
#include "m_can_tests_internal/m_can_memory_dump.h"
#include "m_can_tests_internal/m_can_test_x1_tx_cancelation.h"
#include "m_can_tests_internal/m_ttcan_test_01_basic.h"
#endif

#endif /*M_CAN_INCLUDES_H*/
