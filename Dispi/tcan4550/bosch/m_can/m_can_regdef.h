/* M_CAN and M_TTCAN IP Module V3.2.1 Register Definitions */

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

#ifndef M_CAN_REGDEF_H_
#define M_CAN_REGDEF_H_

// generate value with '1' set at position 'x'
#define BIT(x) (1 << (x))

// Sets bit a through bit b (inclusive), as long as 0 <= a <= 31 and 0 <= b <= 31.
// example: BITMASK_FROM_B_DOWNTO_A( 7,  4) = 0x00 00 00 F0
// example: BITMASK_FROM_B_DOWNTO_A(31, 28) = 0xF0 00 00 00
#define BITMASK_FROM_B_DOWNTO_A(b, a) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))

// ADDRESS and RESET_VALUE definitions for M_TTCAN Registers

// TODO: Rename: ADR_M_CAN_CREL ==> M_CAN_CREL_ADR
// TODO: Rename: DEF_M_CAN_CREL ==> M_CAN_CREL_RESET_VALUE

// MZ ++++++++++++++++++++++++++
// Accedo via spi quindi non ci sono volatili
#define VOLATILE
// MZ --------------------------

//  ~~~~~~~~~ M_CAN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// MISC -----------------------
#define ADR_M_CAN_CREL     0x00
//#define DEF_M_CAN_CREL   0x32380608  // M_CAN   Version 3_2_3, 08.06.2018
//#define DEF_M_CAN_CREL   0x32270530  // M_CAN   Version 3_2_2, 30.05.2017
//#define DEF_M_CAN_CREL   0x32270531  // M_TTCAN Version 3_2_2, 31.05.2017
#define DEF_M_CAN_CREL   0x32150320  // M_CAN   Version 3_2_1, 20.03.2015
//#define DEF_M_CAN_CREL   0x32150323  // M_TTCAN Version 3_2_1, 23.03.2015
//#define DEF_M_CAN_CREL   0x32041218  // M_CAN   Version 3_2_0, 18.12.2014
//#define DEF_M_CAN_CREL   0x32041219  // M_TTCAN Version 3_2_0, 19.12.2014


#define ADR_M_CAN_ENDN    0x004       // Endian Register (R)
#define DEF_M_CAN_ENDN    0x87654321

// address: 0x008 CUST Customer Register

#define ADR_M_CAN_DBTP    0x00C       // Data Bit Timing & Prescaler Register (RP)
#define DEF_M_CAN_DBTP    0x00000A33

#define ADR_M_CAN_TEST    0x010       // Test Register (RP)
#define DEF_M_CAN_TEST    0x00000080

#define ADR_M_CAN_RWD     0x014       // RAM Watchdog (RP)
#define DEF_M_CAN_RWD     0x00000000

#define ADR_M_CAN_CCCR    0x018       // CC Control Register (RP)
//MZ#define DEF_M_CAN_CCCR    0x00000001
#define DEF_M_CAN_CCCR    0x00000019

#define ADR_M_CAN_NBTP    0x01C       // Nominal Bit Timing & Prescaler Register (RP)
#define DEF_M_CAN_NBTP    0x06000A03

// Timestamp & Timeout ----------

#define ADR_M_CAN_TSCC    0x020       // Timestamp Counter Configuration (RP)
#define DEF_M_CAN_TSCC    0x00000000

#define ADR_M_CAN_TSCV    0x024       // Timestamp Counter Value (RC)
#define DEF_M_CAN_TSCV    0x00000000

#define ADR_M_CAN_TOCC    0x028       // Timeout Counter Configuration (RP)
#define DEF_M_CAN_TOCC    0xFFFF0000

#define ADR_M_CAN_TOCV    0x02C       // Timeout Counter Value (RC)
#define DEF_M_CAN_TOCV    0x0000FFFF

// Interrupt --------------------

// reserved address/range 0x030-03C
#define ADR_M_CAN_ECR     0x040       // Error Counter Register (R)
#define DEF_M_CAN_ECR     0x00000000

#define ADR_M_CAN_PSR     0x044       // Protocol Status Register (RXS)
#define DEF_M_CAN_PSR     0x00000707

#define ADR_M_CAN_TDCR    0x048       // Transmitter Delay Compensation (RP)
#define DEF_M_CAN_TDCR    0x00000000

// reserved address/range 0x048-04C
#define ADR_M_CAN_IR      0x050       // Interrupt Register (RW)
#define DEF_M_CAN_IR      0x00000000

#define ADR_M_CAN_IE      0x054       // Interrupt Enable (RW)
#define DEF_M_CAN_IE      0x00000000

#define ADR_M_CAN_ILS     0x058       // Interrupt Line Select (RW)
#define DEF_M_CAN_ILS     0x00000000

#define ADR_M_CAN_ILE     0x05C       // Interrupt Line Enable (RW)
#define DEF_M_CAN_ILE     0x00000000

// Rx Message Handler -------------

// reserved address/range 0x060-07C
#define ADR_M_CAN_GFC     0x080       // Global Filter Configuration (RP)
#define DEF_M_CAN_GFC     0x00000000

#define ADR_M_CAN_SIDFC   0x084       // Standard ID Filter Configuration (RP)
#define DEF_M_CAN_SIDFC   0x00000000

#define ADR_M_CAN_XIDFC   0x088       // Extended ID Filter Configuration (RP)
#define DEF_M_CAN_XIDFC   0x00000000

// reserved address/range 0x08C
#define ADR_M_CAN_XIDAM   0x090       // Extended ID AND Mask (RP)
#define DEF_M_CAN_XIDAM   0x1FFFFFFF

#define ADR_M_CAN_HPMS    0x094       // High Priority Message Status (R)
#define DEF_M_CAN_HPMS    0x00000000

#define ADR_M_CAN_NDAT1   0x098       // New Data 1 (RW)
#define DEF_M_CAN_NDAT1   0x00000000

#define ADR_M_CAN_NDAT2   0x09C       // New Data 2 (RW)
#define DEF_M_CAN_NDAT2   0x00000000

#define ADR_M_CAN_RXF0C   0x0A0       // Rx FIFO 0 Configuration (RP)
#define DEF_M_CAN_RXF0C   0x00000000

#define ADR_M_CAN_RXF0S   0x0A4       // Rx FIFO 0 Status (R)
#define DEF_M_CAN_RXF0S   0x00000000

#define ADR_M_CAN_RXF0A   0x0A8       // Rx FIFO 0 Acknowledge (RW)
#define DEF_M_CAN_RXF0A   0x00000000

#define ADR_M_CAN_RXBC    0x0AC       // Rx Buffer Configuration (RW)
#define DEF_M_CAN_RXBC    0x00000000

#define ADR_M_CAN_RXF1C   0x0B0       // Rx FIFO 1 Configuration (RP)
#define DEF_M_CAN_RXF1C   0x00000000

#define ADR_M_CAN_RXF1S   0x0B4       // Rx FIFO 1 Status (R)
#define DEF_M_CAN_RXF1S   0x00000000

#define ADR_M_CAN_RXF1A   0x0B8       // Rx FIFO 1 Acknowledge (RW)
#define DEF_M_CAN_RXF1A   0x00000000

#define ADR_M_CAN_RXESC   0x0BC       // Rx Buffer / FIFO Element Size Configuration (RW)
#define DEF_M_CAN_RXESC   0x00000000

// Tx Message Handler -------------

#define ADR_M_CAN_TXBC    0x0C0       // Tx Buffer Configuration (RP)
#define DEF_M_CAN_TXBC    0x00000000

#define ADR_M_CAN_TXFQS   0x0C4       // Tx FIFO/Queue Status (R)
#define DEF_M_CAN_TXFQS   0x00000000

#define ADR_M_CAN_TXESC   0x0C8       // Tx Buffer Element Size Configuration (RW)
#define DEF_M_CAN_TXESC   0x00000000

#define ADR_M_CAN_TXBRP   0x0CC       // Tx Buffer Request Pending (R)
#define DEF_M_CAN_TXBRP   0x00000000

#define ADR_M_CAN_TXBAR   0x0D0       // Tx Buffer Add Request (RW)
#define DEF_M_CAN_TXBAR   0x00000000

#define ADR_M_CAN_TXBCR   0x0D4       // Tx Buffer Cancellation Request (RW)
#define DEF_M_CAN_TXBCR   0x00000000

#define ADR_M_CAN_TXBTO   0x0D8       // Tx Buffer Transmission Occurred (R)
#define DEF_M_CAN_TXBTO   0x00000000

#define ADR_M_CAN_TXBCF   0x0DC       // Tx Buffer Cancellation Finished (R)
#define DEF_M_CAN_TXBCF   0x00000000

#define ADR_M_CAN_TXBTIE  0x0E0       // Tx Buffer Transmission Interrupt Enable (RW)
#define DEF_M_CAN_TXBTIE  0x00000000

#define ADR_M_CAN_TXBCIE  0x0E4       // Tx Buffer Cancellation Finished Interrupt Enable (RW)
#define DEF_M_CAN_TXBCIE  0x00000000

// reserved address/range 0x0E8-0EC
#define ADR_M_CAN_TXEFC   0x0F0       // Tx Event FIFO Configuration (RP)
#define DEF_M_CAN_TXEFC   0x00000000

#define ADR_M_CAN_TXEFS   0x0F4       // Tx Event FIFO Status (R)
#define DEF_M_CAN_TXEFS   0x00000000

#define ADR_M_CAN_TXEFA   0x0F8       // Tx Event FIFO Acknowledge (RW)
#define DEF_M_CAN_TXEFA   0x00000000

// reserved address/range 0x0FC

//  ~~~~~~~~~ M_TT_CAN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ADR_M_CAN_TTTMC   0x100       // TT Trigger Memory Configuration (RP)
#define DEF_M_CAN_TTTMC   0x00000000

#define ADR_M_CAN_TTRMC   0x104       // TT Reference Message Configuration (RP)
#define DEF_M_CAN_TTRMC   0x00000000

#define ADR_M_CAN_TTOCF   0x108       // TT Operation Configuration (RP)
#define DEF_M_CAN_TTOCF   0x00010000

#define ADR_M_CAN_TTMLM   0x10C       // TT Matrix Limits (RP)
#define DEF_M_CAN_TTMLM   0x00000000

#define ADR_M_CAN_TURCF   0x110       // TUR Configuration (RP)
#define DEF_M_CAN_TURCF   0x10000000

#define ADR_M_CAN_TTOCN   0x114       // TT Operation Control (RW)
#define DEF_M_CAN_TTOCN   0x00000000

#define ADR_M_CAN_TTGTP   0x118       // TT Global Time Preset (RW)
#define DEF_M_CAN_TTGTP   0x00000000

#define ADR_M_CAN_TTTMK   0x11C       // TT Time Mark (RW)
#define DEF_M_CAN_TTTMK   0x00000000

#define ADR_M_CAN_TTIR    0x120       // TT Interrupt Register (RW)
#define DEF_M_CAN_TTIR    0x00000000

#define ADR_M_CAN_TTIE    0x124       // TT Interrupt Enable (RW)
#define DEF_M_CAN_TTIE    0x00000000

#define ADR_M_CAN_TTILS   0x128       // TT Interrupt Line Select (RW)
#define DEF_M_CAN_TTILS   0x00000000

#define ADR_M_CAN_TTOST   0x12C       // TT Operation Status (R)
#define DEF_M_CAN_TTOST   0x00000080

#define ADR_M_CAN_TURNA   0x130       // TUR Numerator Actual (R)
#define DEF_M_CAN_TURNA   0x00010000

#define ADR_M_CAN_TTLGT   0x134       // TT Local & Global Time (R)
#define DEF_M_CAN_TTLGT   0x00000000

#define ADR_M_CAN_TTCTC   0x138       // TT Cycle Time & Count (R)
#define DEF_M_CAN_TTCTC   0x003F0000

#define ADR_M_CAN_TTCPT   0x13C       // TT Capture Time (R)
#define DEF_M_CAN_TTCPT   0x00000000

#define ADR_M_CAN_TTCSM   0x140       // TT Cycle Sync Mark (R)
#define DEF_M_CAN_TTCSM   0x00000000

// reserved address/range 0x144-1FC


// === Type Definition for REGISTERs =========================================

/* BITFIELDS
 * Bit fields vary widely from compiler to compiler, sorry.
 * With GCC, big endian machines lay out the bits big end first and little endian machines lay out the bits little end first.
 */

//  ~~~~~~~~~ M_CAN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if __LITTLE_ENDIAN
/* Type Definition for register CREL */
typedef union {
  VOLATILE unsigned int value ;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   DAY     :  8; //!< 07..00: Design Time Stamp, Day
    VOLATILE unsigned int   MON     :  8; //!< 15..08: Design Time Stamp, Month
    VOLATILE unsigned int   YEAR    :  4; //!< 19..16: Design Time Stamp, Year
    VOLATILE unsigned int   SUBSTEP :  4; //!< 23..20: Sub-Step of Core Release
    VOLATILE unsigned int   STEP    :  4; //!< 27..24: Step of Core Release
    VOLATILE unsigned int   REL     :  4; //!< 31..28: Core Release
  } bits;                                 //!< Bit representation
} M_CAN_CREL_union;

#elif __BIG_ENDIAN
/* Type Definition for register CREL */
typedef union {
  VOLATILE unsigned int value ;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   REL     :  4; //!< 31..28: Core Release
    VOLATILE unsigned int   STEP    :  4; //!< 27..24: Step of Core Release
    VOLATILE unsigned int   SUBSTEP :  4; //!< 23..20: Sub-Step of Core Release
    VOLATILE unsigned int   YEAR    :  4; //!< 19..16: Design Time Stamp, Year
    VOLATILE unsigned int   MON     :  8; //!< 15..08: Design Time Stamp, Month
    VOLATILE unsigned int   DAY     :  8; //!< 07..00: Design Time Stamp, Day
  } bits;                                 //!< Bit representation
} M_CAN_CREL_union;
#endif


#if __LITTLE_ENDIAN
/* Type Definition for register ENDN - Endian Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   ETV0    :  8; //!< 07..00: Byte0 has Value h#21
    VOLATILE unsigned int   ETV1    :  8; //!< 15..08: Byte1 has Value h#43
    VOLATILE unsigned int   ETV2    :  8; //!< 23..16: Byte2 has Value h#65
    VOLATILE unsigned int   ETV3    :  8; //!< 31..24: Byte3 has Value h#87
  } bits;                                 //!< Bit representation
} M_CAN_ENDN_union;

#elif __BIG_ENDIAN
/* Type Definition for register ENDN - Endian Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   ETV3    :  8; //!< 31..24: Byte3 has Value h#87
    VOLATILE unsigned int   ETV2    :  8; //!< 23..16: Byte2 has Value h#65
    VOLATILE unsigned int   ETV1    :  8; //!< 15..08: Byte1 has Value h#43
    VOLATILE unsigned int   ETV0    :  8; //!< 07..00: Byte0 has Value h#21
  } bits;                                 //!< Bit representation
} M_CAN_ENDN_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register DBTP - Data Bit Timing and Prescaler Register */
typedef union {
  VOLATILE unsigned int value; //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int DSJW   :4; //!< 03..00: Data (Re) Synchronization Jump Width
    VOLATILE unsigned int DTSEG2 :4; //!< 07..04: Data time segment after  sample point
    VOLATILE unsigned int DTSEG1 :5; //!< 12..08: Data time segment before sample point
    VOLATILE unsigned int res0   :3; //!< 15..13: reserved
    VOLATILE unsigned int DBRP   :5; //!< 20..16: Data Bit Rate Rate Prescaler
    VOLATILE unsigned int res1   :2; //!< 22..21: reserved
    VOLATILE unsigned int TDC    :1; //!<     23: Transceiver Delay Compensation
    VOLATILE unsigned int res2   :8; //!< 31..24: reserved
  } bits; //!< Bit representation
} M_CAN_DBTP_union;

#elif __BIG_ENDIAN
/* Type Definition for register FBTP - Fast Timing and Prescaler Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int res2   :8; //!< 31..24: reserved
    VOLATILE unsigned int TDC    :1; //!<     23: Transceiver Delay Compensation
    VOLATILE unsigned int res1   :2; //!< 22..21: reserved
    VOLATILE unsigned int DBRP   :5; //!< 20..16: Data Bit Rate Rate Prescaler
    VOLATILE unsigned int res0   :3; //!< 15..13: reserved
    VOLATILE unsigned int DTSEG1 :5; //!< 12..08: Data time segment before sample point
    VOLATILE unsigned int DTSEG2 :4; //!< 07..04: Data time segment after  sample point
    VOLATILE unsigned int DSJW   :4; //!< 03..00: Data (Re) Synchronization Jump Width
  } bits; //!< Bit representation
} M_CAN_DBTP_union;
#endif


#if __LITTLE_ENDIAN
/* Type Definition for register TEST - Test Register */
typedef union {
  VOLATILE unsigned int value; //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int res0 : 4; //!< 03..00: reserved
    VOLATILE unsigned int LBCK : 1; //!<     04: Loop Back Mode
    VOLATILE unsigned int TX   : 2; //!< 06..05: Control of Transmit Pin
    VOLATILE unsigned int RX   : 1; //!<     07: Receive Pin
    VOLATILE unsigned int res1 :24; //!< 31..08: reserved
  } bits; //!< Bit representation
} M_CAN_TEST_union;

#elif __BIG_ENDIAN
/* Type Definition for register TEST - Test Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int res1 :24; //!< 31..08: reserved
    VOLATILE unsigned int RX   : 1; //!<     07: Receive Pin
    VOLATILE unsigned int TX   : 2; //!< 06..05: Control of Transmit Pin
    VOLATILE unsigned int LBCK : 1; //!<     04: Loop Back Mode
    VOLATILE unsigned int res0 : 4; //!< 03..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TEST_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RWD - RAM Watchdog */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   WDC     :  8; //!< 07..00: Watchdog Configuration
    VOLATILE unsigned int   WDV     :  8; //!< 15..08: Watchdog Value
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RWD_union;

#elif __BIG_ENDIAN
/* Type Definition for register RWD - RAM Watchdog */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   WDV     :  8; //!< 15..08: Watchdog Value
    VOLATILE unsigned int   WDC     :  8; //!< 07..00: Watchdog Configuration
  } bits;                                 //!< Bit representation
} M_CAN_RWD_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register CCCR - CC Control Register */
typedef union {
  VOLATILE unsigned int value; //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int INIT : 1; //!<     00: Initialization
    VOLATILE unsigned int CCE  : 1; //!<     01: Configuration Change Enable
    VOLATILE unsigned int ASM  : 1; //!<     02: Restricted Operation Mode
    VOLATILE unsigned int CSA  : 1; //!<     03: Clock Stop Acknowledge
    VOLATILE unsigned int CSR  : 1; //!<     04: Clock Stop Request
    VOLATILE unsigned int MON  : 1; //!<     05: Bus Monitoring Mode
    VOLATILE unsigned int DAR  : 1; //!<     06: Disable Automatic Retransmission
    VOLATILE unsigned int TEST : 1; //!<     07: Test Mode Enable
    VOLATILE unsigned int FDOE : 1; //!<     08: FD Operation Enable
    VOLATILE unsigned int BRSE : 1; //!<     09: Bit Rate Switch Enable
    VOLATILE unsigned int res0 : 2; //!< 11..10: reserved
    VOLATILE unsigned int PXHD : 1; //!<     12: Protocol Exception Handling Disable
    VOLATILE unsigned int EFBI : 1; //!<     13: Edge Filtering During Bus Integration
    VOLATILE unsigned int TXP  : 1; //!<     14: Transmit Pause
    VOLATILE unsigned int NISO : 1; //!<     15: Non ISO Operation (0=ISO, 1=BoschSpecV1.0)
    VOLATILE unsigned int res1 :16; //!< 31..16: reserved
  } bits; //!< Bit representation CCCR
} M_CAN_CCCR_union;

#elif __BIG_ENDIAN
/* Type Definition for register CCCR - CC Control Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int res1 :16; //!< 31..16: reserved
    VOLATILE unsigned int NISO : 1; //!<     15: Non ISO Operation (0=ISO, 1=BoschSpecV1.0)
    VOLATILE unsigned int TXP  : 1; //!<     14: Transmit Pause
    VOLATILE unsigned int EFBI : 1; //!<     13: Edge Filtering During Bus Integration
    VOLATILE unsigned int PXHD : 1; //!<     12: Protocol Exception Handling Disable
    VOLATILE unsigned int res0 : 2; //!< 11..10: reserved
    VOLATILE unsigned int BRSE : 1; //!<     09: Bit Rate Switch Enable
    VOLATILE unsigned int FDOE : 1; //!<     08: FD Operation Enable
    VOLATILE unsigned int TEST : 1; //!<     07: Test Mode Enable
    VOLATILE unsigned int DAR  : 1; //!<     06: Disable Automatic Retransmission
    VOLATILE unsigned int MON  : 1; //!<     05: Bus Monitoring Mode
    VOLATILE unsigned int CSR  : 1; //!<     04: Clock Stop Request
    VOLATILE unsigned int CSA  : 1; //!<     03: Clock Stop Acknowledge
    VOLATILE unsigned int ASM  : 1; //!<     02: Restricted Operation Mode
    VOLATILE unsigned int CCE  : 1; //!<     01: Configuration Change Enable
    VOLATILE unsigned int INIT : 1; //!<     00: Initialization
  } bits;                                 //!< Bit representation
} M_CAN_CCCR_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register NBTP - Nominal Bit Timing & Prescaler Register */
typedef union {
  VOLATILE unsigned int value; //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int NTSEG2 : 7; //!< 06..00: Nominal time segment after the sample point
    VOLATILE unsigned int res0   : 1; //!<     07: reserved
    VOLATILE unsigned int NTSEG1 : 8; //!< 15..08: Nominal time segment before the sample point
    VOLATILE unsigned int NBRP   : 9; //!< 24..16: Nominal Bit Rate Prescaler
    VOLATILE unsigned int NSJW   : 7; //!< 31..25: Nominal (Re) Synchronization Jump Width
  } bits; //!< Bit representation
} M_CAN_NBTP_union;

#elif __BIG_ENDIAN
/* Type Definition for register BTP - Bit Timing & Prescaler Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int NSJW   : 7; //!< 31..25: Nominal (Re) Synchronization Jump Width
    VOLATILE unsigned int NBRP   : 9; //!< 24..16: Nominal Bit Rate Prescaler
    VOLATILE unsigned int NTSEG1 : 8; //!< 15..08: Nominal time segment before the sample point
    VOLATILE unsigned int res0   : 1; //!<     07: reserved
    VOLATILE unsigned int NTSEG2 : 7; //!< 06..00: Nominal time segment after the sample point
  } bits; //!< Bit representation
} M_CAN_NBTP_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TSCC - Timestamp Counter Configuration */
typedef union {
  VOLATILE unsigned int value;        //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int TSS   :2;  //!< 01..00: Timestamp Select
    VOLATILE unsigned int res0  :14; //!< 15..02: reserved
    VOLATILE unsigned int TCP   :4;  //!< 19..16: Timestamp Counter Prescaler
    VOLATILE unsigned int res1  :12; //!< 31..20: reserved
  } bits;                            //!< Bit representation
} M_CAN_TSCC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TSCC - Timestamp Counter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    : 12; //!< 31..20: reserved
    VOLATILE unsigned int   TCP     :  4; //!< 19..16: Timestamp Counter Prescaler
    VOLATILE unsigned int   res0    : 14; //!< 15..02: reserved
    VOLATILE unsigned int   TSS     :  2; //!< 01..00: Timestamp Select
  } bits;                                 //!< Bit representation
} M_CAN_TSCC_union;
#endif

/* TSCC: Timestamp Select */
typedef enum {
	TSCC_TSS_TIMESTAMP_COUNTER_VALUE_ALWAYS_0 = 0x0,
	TSCC_TSS_TIMESTAMP_COUNTER_VALUE_ACCORDING_TO_TCP = 0x1,
	TSCC_TSS_TIMESTAMP_COUNTER_VALUE_EXTERNAL_USED = 0x2
} tscc_tss_timestamp_select_enum;

#define TSCC_TCP_MIN_VALUE  1 // Minimum allowed value for TimeStamp Prescaler
#define TSCC_TCP_MAX_VALUE 16 // Maximum allowed value for TimeStamp Prescaler

#if __LITTLE_ENDIAN
/* Type Definition for register TSCV - Timestamp Counter Value */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TSC     : 16; //!< 15..00: Timestamp Counter
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TSCV_union;

#elif __BIG_ENDIAN
/* Type Definition for register TSCV - Timestamp Counter Value */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   TSC     : 16; //!< 15..00: Timestamp Counter
  } bits;                                 //!< Bit representation
} M_CAN_TSCV_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TOCC - Timeout Counter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   ETOC    :  1; //!<     00: Enable Timeout Counter
    VOLATILE unsigned int   TOS     :  2; //!< 02..01: Timeout Select
    VOLATILE unsigned int   res0    : 13; //!< 15..03: reserved
    VOLATILE unsigned int   TOP     : 16; //!< 31..16: Timeout Period
  } bits;                                 //!< Bit representation
} M_CAN_TOCC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TOCC - Timeout Counter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TOP     : 16; //!< 31..16: Timeout Period
    VOLATILE unsigned int   res0    : 13; //!< 15..03: reserved
    VOLATILE unsigned int   TOS     :  2; //!< 02..01: Timeout Select
    VOLATILE unsigned int   ETOC    :  1; //!<     00: Enable Timeout Counter (0: disabled, 1: enabled)
  } bits;                                 //!< Bit representation
} M_CAN_TOCC_union;
#endif

/* TOCC: Timeout Select */
typedef enum {
	TOCC_TOS_CONTINUOUS_OPERATION 				= 0x0,
	TOCC_TOS_TIMEOUT_CONTROLLED_BY_TX_EVENT_FIFO = 0x1,
	TOCC_TOS_TIMEOUT_CONTROLLED_BY_RX_FIFO_0 	= 0x2,
	TOCC_TOS_TIMEOUT_CONTROLLED_BY_RX_FIFO_1 	= 0x3
} tocc_tos_timeout_select_enum;

#if __LITTLE_ENDIAN
/* Type Definition for register TOCV - Timeout Counter Value */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TOC     : 16; //!< 15..00: Timeout Counter
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TOCV_union;

#elif __BIG_ENDIAN
/* Type Definition for register TOCV - Timeout Counter Value */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   TOC     : 16; //!< 15..00: Timeout Counter
  } bits;                                 //!< Bit representation
} M_CAN_TOCV_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register ECR - Error Counter Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TEC     :  8; //!< 07..00: Transmit Error Counter
    VOLATILE unsigned int   REC     :  7; //!< 14..08: Receive Error Counter
    VOLATILE unsigned int   RP      :  1; //!<     15: Receive Error Passive
    VOLATILE unsigned int   CEL     :  8; //!< 23..16: CAN Error Logging
    VOLATILE unsigned int   res0    :  8; //!< 31..24: reserved
  } bits;                                 //!< Bit representation
} M_CAN_ECR_union;

#elif __BIG_ENDIAN
/* Type Definition for register ECR - Error Counter Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  8; //!< 31..24: reserved
    VOLATILE unsigned int   CEL     :  8; //!< 23..16: CAN Error Logging
    VOLATILE unsigned int   RP      :  1; //!<     15: Receive Error Passive
    VOLATILE unsigned int   REC     :  7; //!< 14..08: Receive Error Counter
    VOLATILE unsigned int   TEC     :  8; //!< 07..00: Transmit Error Counter
  } bits;                                 //!< Bit representation
} M_CAN_ECR_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register PSR - Protocol Status Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   LEC     :  3; //!< 02..00: Last Error Code
    VOLATILE unsigned int   ACT     :  2; //!< 04..03: Activity
    VOLATILE unsigned int   EP      :  1; //!<     05: Error Passive
    VOLATILE unsigned int   EW      :  1; //!<     06: Warning Status
    VOLATILE unsigned int   BO      :  1; //!<     07: Bus_Off Status
    VOLATILE unsigned int   DLEC    :  3; //!< 10..08: Data Phase Last Error Code
    VOLATILE unsigned int   RESI    :  1; //!<     11: ESI flag of last received CAN FD Message
    VOLATILE unsigned int   RBRS    :  1; //!<     12: BRS flag of last received CAN FD Message
    VOLATILE unsigned int   RFDF    :  1; //!<     13: Received a CAN FD Message
    VOLATILE unsigned int   PXE     :  1; //!<     14: Protocol Exception Event
    VOLATILE unsigned int   res0    :  1; //!<     15: reserved
    VOLATILE unsigned int   TDCV    :  7; //!< 22..16: Transceiver Delay Compensation Value
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
  } bits; //!< Bit representation
} M_CAN_PSR_union;

#elif __BIG_ENDIAN
/* Type Definition for register PSR - Protocol Status Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
    VOLATILE unsigned int   TDCV    :  7; //!< 22..16: Transceiver Delay Compensation Value
    VOLATILE unsigned int   res0    :  1; //!<     15: reserved
    VOLATILE unsigned int   PXE     :  1; //!<     14: Protocol Exception Event
    VOLATILE unsigned int   RFDF    :  1; //!<     13: Received a CAN FD Message
    VOLATILE unsigned int   RBRS    :  1; //!<     12: BRS flag of last received CAN FD Message
    VOLATILE unsigned int   RESI    :  1; //!<     11: ESI flag of last received CAN FD Message
    VOLATILE unsigned int   DLEC    :  3; //!< 10..08: Data Phase Last Error Code
    VOLATILE unsigned int   BO      :  1; //!<     07: Bus_Off Status
    VOLATILE unsigned int   EW      :  1; //!<     06: Warning Status
    VOLATILE unsigned int   EP      :  1; //!<     05: Error Passive
    VOLATILE unsigned int   ACT     :  2; //!< 04..03: Activity
    VOLATILE unsigned int   LEC     :  3; //!< 02..00: Last Error Code
  } bits;                                 //!< Bit representation
} M_CAN_PSR_union;
#endif

/* PSR: Activity Encoding */
enum m_can_psr_activity {
  ACT_SYNCHRONIZING = 0,  // node is synchronizing on CAN communication
  ACT_IDLE = 1,           // node is neither Receiver nor Transmitter
  ACT_RECEIVER = 2,       // node is operating as Receiver
  ACT_TRANSMITTER = 3     // node is operating as Transmitter
};

#if __LITTLE_ENDIAN
/* Type Definition for register TDCR - Transmitter Delay Compensation Register */
typedef union {
  VOLATILE unsigned int value;          //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TDCF  :  7; //!< 06..00: Transmitter Delay Compensation Filter Window Length
    VOLATILE unsigned int   res0  :  1; //!<     07: reserved
    VOLATILE unsigned int   TDCO  :  7; //!< 14..08: Transmitter Delay Compensation Offset
    VOLATILE unsigned int   res1  : 17; //!< 31..15: reserved
  } bits;                               //!< Bit representation
} M_CAN_TDCR_union;

#elif __BIG_ENDIAN
/* Type Definition for register TDCR - Transmitter Delay Compensation Register */
typedef union {
  VOLATILE unsigned int value;          //!< Integer representation
  VOLATILE struct {
	VOLATILE unsigned int   res1  : 17; //!< 31..15: reserved
    VOLATILE unsigned int   TDCO  :  7; //!< 14..08: Transmitter Delay Compensation Offset
    VOLATILE unsigned int   res0  :  1; //!<     07: reserved
    VOLATILE unsigned int   TDCF  :  7; //!< 06..00: Transmitter Delay Compensation Filter Window Length
  } bits;                               //!< Bit representation
} M_CAN_TDCR_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register IR - Interrupt Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RF0N    :  1; //!<     00: Rx FIFO 0 New Message
    VOLATILE unsigned int   RF0W    :  1; //!<     01: Rx FIFO 0 Watermark Reached
    VOLATILE unsigned int   RF0F    :  1; //!<     02: Rx FIFO 0 Full
    VOLATILE unsigned int   RF0L    :  1; //!<     03: Rx FIFO 0 Message Lost
    VOLATILE unsigned int   RF1N    :  1; //!<     04: Rx FIFO 1 New Message
    VOLATILE unsigned int   RF1W    :  1; //!<     05: Rx FIFO 1 Watermark Reached
    VOLATILE unsigned int   RF1F    :  1; //!<     06: Rx FIFO 1 Full
    VOLATILE unsigned int   RF1L    :  1; //!<     07: Rx FIFO 1 Message Lost
    VOLATILE unsigned int   HPM     :  1; //!<     08: High Priority Message
    VOLATILE unsigned int   TC      :  1; //!<     09: Transmission Completed
    VOLATILE unsigned int   TCF     :  1; //!<     10: Transmission Cancellation Finished
    VOLATILE unsigned int   TFE     :  1; //!<     11: Tx FIFO Empty
    VOLATILE unsigned int   TEFN    :  1; //!<     12: Tx Event FIFO New Entry
    VOLATILE unsigned int   TEFW    :  1; //!<     13: Tx Event FIFO Watermark Reached
    VOLATILE unsigned int   TEFF    :  1; //!<     14: Tx Event FIFO Full
    VOLATILE unsigned int   TEFL    :  1; //!<     15: Tx Event FIFO Element Lost
    VOLATILE unsigned int   TSW     :  1; //!<     16: Timestamp Wraparound
    VOLATILE unsigned int   MRAF    :  1; //!<     17: Message RAM Access Failure
    VOLATILE unsigned int   TOO     :  1; //!<     18: Timeout Occurred
    VOLATILE unsigned int   DRX     :  1; //!<     19: Message stored to Dedicated Rx Buffer
    VOLATILE unsigned int   BEC     :  1; //!<     20: Bit Error Corrected
    VOLATILE unsigned int   BEU     :  1; //!<     21: Bit Error Uncorrected
    VOLATILE unsigned int   ELO     :  1; //!<     22: Error Logging Overflow
    VOLATILE unsigned int   EP      :  1; //!<     23: Error Passive
    VOLATILE unsigned int   EW      :  1; //!<     24: Warning Status
    VOLATILE unsigned int   BO      :  1; //!<     25: Bus_Off Status
    VOLATILE unsigned int   WDI     :  1; //!<     26: Watchdog Interrupt
    VOLATILE unsigned int   PEA     :  1; //!<     27: Protocol Error in Arbitration Phase
    VOLATILE unsigned int   PED     :  1; //!<     28: Protocol Error in Data        Phase
    VOLATILE unsigned int   ARA     :  1; //!<     29: Access to Reserved Address
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
  } bits;                                 //!< Bit representation
} M_CAN_IR_union;

#elif __BIG_ENDIAN
/* Type Definition for register IR - Interrupt Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
    VOLATILE unsigned int   ARA     :  1; //!<     29: Access to Reserved Address
    VOLATILE unsigned int   PED     :  1; //!<     28: Protocol Error in Data        Phase
    VOLATILE unsigned int   PEA     :  1; //!<     27: Protocol Error in Arbitration Phase
    VOLATILE unsigned int   WDI     :  1; //!<     26: Watchdog Interrupt
    VOLATILE unsigned int   BO      :  1; //!<     25: Bus_Off Status
    VOLATILE unsigned int   EW      :  1; //!<     24: Warning Status
    VOLATILE unsigned int   EP      :  1; //!<     23: Error Passive
    VOLATILE unsigned int   ELO     :  1; //!<     22: Error Logging Overflow
    VOLATILE unsigned int   BEU     :  1; //!<     21: Bit Error Uncorrected
    VOLATILE unsigned int   BEC     :  1; //!<     20: Bit Error Corrected
    VOLATILE unsigned int   DRX     :  1; //!<     19: Message stored to Dedicated Rx Buffer
    VOLATILE unsigned int   TOO     :  1; //!<     18: Timeout Occurred
    VOLATILE unsigned int   MRAF    :  1; //!<     17: Message RAM Access Failure
    VOLATILE unsigned int   TSW     :  1; //!<     16: Timestamp Wraparound
    VOLATILE unsigned int   TEFL    :  1; //!<     15: Tx Event FIFO Element Lost
    VOLATILE unsigned int   TEFF    :  1; //!<     14: Tx Event FIFO Full
    VOLATILE unsigned int   TEFW    :  1; //!<     13: Tx Event FIFO Watermark Reached
    VOLATILE unsigned int   TEFN    :  1; //!<     12: Tx Event FIFO New Entry
    VOLATILE unsigned int   TFE     :  1; //!<     11: Tx FIFO Empty
    VOLATILE unsigned int   TCF     :  1; //!<     10: Transmission Cancellation Finished
    VOLATILE unsigned int   TC      :  1; //!<     09: Transmission Completed
    VOLATILE unsigned int   HPM     :  1; //!<     08: High Priority Message
    VOLATILE unsigned int   RF1L    :  1; //!<     07: Rx FIFO 1 Message Lost
    VOLATILE unsigned int   RF1F    :  1; //!<     06: Rx FIFO 1 Full
    VOLATILE unsigned int   RF1W    :  1; //!<     05: Rx FIFO 1 Watermark Reached
    VOLATILE unsigned int   RF1N    :  1; //!<     04: Rx FIFO 1 New Message
    VOLATILE unsigned int   RF0L    :  1; //!<     03: Rx FIFO 0 Message Lost
    VOLATILE unsigned int   RF0F    :  1; //!<     02: Rx FIFO 0 Full
    VOLATILE unsigned int   RF0W    :  1; //!<     01: Rx FIFO 0 Watermark Reached
    VOLATILE unsigned int   RF0N    :  1; //!<     00: Rx FIFO 0 New Message
  } bits;                                 //!< Bit representation
} M_CAN_IR_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register IE - Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RF0NE   :  1; //!<     00: Rx FIFO 0 New Message Interrupt Enable
    VOLATILE unsigned int   RF0WE   :  1; //!<     01: Rx FIFO 0 Watermark Reached Interrupt Enable
    VOLATILE unsigned int   RF0FE   :  1; //!<     02: Rx FIFO 0 Full Interrupt Enable
    VOLATILE unsigned int   RF0LE   :  1; //!<     03: Rx FIFO 0 Message Lost Interrupt Enable
    VOLATILE unsigned int   RF1NE   :  1; //!<     04: Rx FIFO 1 New Message Interrupt Enable
    VOLATILE unsigned int   RF1WE   :  1; //!<     05: Rx FIFO 1 Watermark Reached Interrupt Enable
    VOLATILE unsigned int   RF1FE   :  1; //!<     06: Rx FIFO 1 Full Interrupt Enable
    VOLATILE unsigned int   RF1LE   :  1; //!<     07: Rx FIFO 1 Message Lost Interrupt Enable
    VOLATILE unsigned int   HPME    :  1; //!<     08: High Priority Message Interrupt Enable
    VOLATILE unsigned int   TCE     :  1; //!<     09: Transmission Completed Interrupt Enable
    VOLATILE unsigned int   TCFE    :  1; //!<     10: Transmission Cancellation Finished Interrupt Enable
    VOLATILE unsigned int   TFEE    :  1; //!<     11: Tx FIFO Empty Interrupt Enable
    VOLATILE unsigned int   TEFNE   :  1; //!<     12: Tx Event FIFO New Entry Interrupt Enable
    VOLATILE unsigned int   TEFWE   :  1; //!<     13: Tx Event FIFO Watermark Reached Interrupt Enable
    VOLATILE unsigned int   TEFFE   :  1; //!<     14: Tx Event FIFO Full Interrupt Enable
    VOLATILE unsigned int   TEFLE   :  1; //!<     15: Tx Event FIFO Element Lost Interrupt Enable
    VOLATILE unsigned int   TSWE    :  1; //!<     16: Timestamp Wraparound Interrupt Enable
    VOLATILE unsigned int   MRAFE   :  1; //!<     17: Message RAM Access Failure Interrupt Enable
    VOLATILE unsigned int   TOOE    :  1; //!<     18: Timeout Occurred Interrupt Enable
    VOLATILE unsigned int   DRXE    :  1; //!<     19: Message stored to Dedicated Rx Buffer Interrupt Enable
    VOLATILE unsigned int   BECE    :  1; //!<     20: Bit Error Corrected Interrupt Enable
    VOLATILE unsigned int   BEUE    :  1; //!<     21: Bit Error Uncorrected Interrupt Enable
    VOLATILE unsigned int   ELOE    :  1; //!<     22: Error Logging Overflow Interrupt Enable
    VOLATILE unsigned int   EPE     :  1; //!<     23: Error Passive Interrupt Enable
    VOLATILE unsigned int   EWE     :  1; //!<     24: Warning Status Interrupt Enable
    VOLATILE unsigned int   BOE     :  1; //!<     25: Bus_Off Status Interrupt Enable
    VOLATILE unsigned int   WDIE    :  1; //!<     26: Watchdog Interrupt Interrupt Enable
    VOLATILE unsigned int   PEAE    :  1; //!<     27: Protocol Error in Arbitration Phase Interrupt Enable
    VOLATILE unsigned int   PEDE    :  1; //!<     28: Protocol Error in Data        Phase Interrupt Enable
    VOLATILE unsigned int   ARAE    :  1; //!<     29: Access to Reserved Address Interrupt Enable
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
  } bits;                                 //!< Bit representation
} M_CAN_IE_union;

#elif __BIG_ENDIAN
/* Type Definition for register IE - Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
    VOLATILE unsigned int   ARAE    :  1; //!<     29: Access to Reserved Address Interrupt Enable
    VOLATILE unsigned int   PEDE    :  1; //!<     28: Protocol Error in Data        Phase Interrupt Enable
    VOLATILE unsigned int   PEAE    :  1; //!<     27: Protocol Error in Arbitration Phase Interrupt Enable
    VOLATILE unsigned int   WDIE    :  1; //!<     26: Watchdog Interrupt Interrupt Enable
    VOLATILE unsigned int   BOE     :  1; //!<     25: Bus_Off Status Interrupt Enable
    VOLATILE unsigned int   EWE     :  1; //!<     24: Warning Status Interrupt Enable
    VOLATILE unsigned int   EPE     :  1; //!<     23: Error Passive Interrupt Enable
    VOLATILE unsigned int   ELOE    :  1; //!<     22: Error Logging Overflow Interrupt Enable
    VOLATILE unsigned int   BEUE    :  1; //!<     21: Bit Error Uncorrected Interrupt Enable
    VOLATILE unsigned int   BECE    :  1; //!<     20: Bit Error Corrected Interrupt Enable
    VOLATILE unsigned int   DRXE    :  1; //!<     19: Message stored to Dedicated Rx Buffer Interrupt Enable
    VOLATILE unsigned int   TOOE    :  1; //!<     18: Timeout Occurred Interrupt Enable
    VOLATILE unsigned int   MRAFE   :  1; //!<     17: Message RAM Access Failure Interrupt Enable
    VOLATILE unsigned int   TSWE    :  1; //!<     16: Timestamp Wraparound Interrupt Enable
    VOLATILE unsigned int   TEFLE   :  1; //!<     15: Tx Event FIFO Element Lost Interrupt Enable
    VOLATILE unsigned int   TEFFE   :  1; //!<     14: Tx Event FIFO Full Interrupt Enable
    VOLATILE unsigned int   TEFWE   :  1; //!<     13: Tx Event FIFO Watermark Reached Interrupt Enable
    VOLATILE unsigned int   TEFNE   :  1; //!<     12: Tx Event FIFO New Entry Interrupt Enable
    VOLATILE unsigned int   TFEE    :  1; //!<     11: Tx FIFO Empty Interrupt Enable
    VOLATILE unsigned int   TCFE    :  1; //!<     10: Transmission Cancellation Finished Interrupt Enable
    VOLATILE unsigned int   TCE     :  1; //!<     09: Transmission Completed Interrupt Enable
    VOLATILE unsigned int   HPME    :  1; //!<     08: High Priority Message Interrupt Enable
    VOLATILE unsigned int   RF1LE   :  1; //!<     07: Rx FIFO 1 Message Lost Interrupt Enable
    VOLATILE unsigned int   RF1FE   :  1; //!<     06: Rx FIFO 1 Full Interrupt Enable
    VOLATILE unsigned int   RF1WE   :  1; //!<     05: Rx FIFO 1 Watermark Reached Interrupt Enable
    VOLATILE unsigned int   RF1NE   :  1; //!<     04: Rx FIFO 1 New Message Interrupt Enable
    VOLATILE unsigned int   RF0LE   :  1; //!<     03: Rx FIFO 0 Message Lost Interrupt Enable
    VOLATILE unsigned int   RF0FE   :  1; //!<     02: Rx FIFO 0 Full Interrupt Enable
    VOLATILE unsigned int   RF0WE   :  1; //!<     01: Rx FIFO 0 Watermark Reached Interrupt Enable
    VOLATILE unsigned int   RF0NE   :  1; //!<     00: Rx FIFO 0 New Message Interrupt Enable
  } bits;                                 //!< Bit representation
} M_CAN_IE_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register ILS - Interrupt Line Select */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RF0NL   :  1; //!<     00: Rx FIFO 0 New Message Interrupt Line
    VOLATILE unsigned int   RF0WL   :  1; //!<     01: Rx FIFO 0 Watermark Reached Interrupt Line
    VOLATILE unsigned int   RF0FL   :  1; //!<     02: Rx FIFO 0 Full Interrupt Line
    VOLATILE unsigned int   RF0LL   :  1; //!<     03: Rx FIFO 0 Message Lost Interrupt Line
    VOLATILE unsigned int   RF1NL   :  1; //!<     04: Rx FIFO 1 New Message Interrupt Line
    VOLATILE unsigned int   RF1WL   :  1; //!<     05: Rx FIFO 1 Watermark Reached Interrupt Line
    VOLATILE unsigned int   RF1FL   :  1; //!<     06: Rx FIFO 1 Full Interrupt Line
    VOLATILE unsigned int   RF1LL   :  1; //!<     07: Rx FIFO 1 Message Lost Interrupt Line
    VOLATILE unsigned int   HPML    :  1; //!<     08: High Priority Message Interrupt Line
    VOLATILE unsigned int   TCL     :  1; //!<     09: Transmission Completed Interrupt Line
    VOLATILE unsigned int   TCFL    :  1; //!<     10: Transmission Cancellation Finished Interrupt Line
    VOLATILE unsigned int   TFEL    :  1; //!<     11: Tx FIFO Empty Interrupt Line
    VOLATILE unsigned int   TEFNL   :  1; //!<     12: Tx Event FIFO New Entry Interrupt Line
    VOLATILE unsigned int   TEFWL   :  1; //!<     13: Tx Event FIFO Watermark Reached Interrupt Line
    VOLATILE unsigned int   TEFFL   :  1; //!<     14: Tx Event FIFO Full Interrupt Line
    VOLATILE unsigned int   TEFLL   :  1; //!<     15: Tx Event FIFO Element Lost Interrupt Line
    VOLATILE unsigned int   TSWL    :  1; //!<     16: Timestamp Wraparound Interrupt Line
    VOLATILE unsigned int   MRAFL   :  1; //!<     17: Message RAM Access Failure Interrupt Line
    VOLATILE unsigned int   TOOL    :  1; //!<     18: Timeout Occurred Interrupt Line
    VOLATILE unsigned int   DRXL    :  1; //!<     19: Message stored to Dedicated Rx Buffer Interrupt Line
    VOLATILE unsigned int   BECL    :  1; //!<     20: Bit Error Corrected Interrupt Line
    VOLATILE unsigned int   BEUL    :  1; //!<     21: Bit Error Uncorrected Interrupt Line
    VOLATILE unsigned int   ELOL    :  1; //!<     22: Error Logging Overflow Interrupt Line
    VOLATILE unsigned int   EPL     :  1; //!<     23: Error Passive Interrupt Line
    VOLATILE unsigned int   EWL     :  1; //!<     24: Warning Status Interrupt Line
    VOLATILE unsigned int   BOL     :  1; //!<     25: Bus_Off Status Interrupt Line
    VOLATILE unsigned int   WDIL    :  1; //!<     26: Watchdog Interrupt Interrupt Line
    VOLATILE unsigned int   PEAL    :  1; //!<     27: Protocol Error in Arbitration Phase Interrupt Line
    VOLATILE unsigned int   PEDL    :  1; //!<     28: Protocol Error in Data        Phase Interrupt Line
    VOLATILE unsigned int   ARAL    :  1; //!<     29: Access to Reserved Address Interrupt Line
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
  } bits;                                 //!< Bit representation
} M_CAN_ILS_union;

#elif __BIG_ENDIAN
/* Type Definition for register ILS - Interrupt Line Select */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 30..31: reserved
    VOLATILE unsigned int   ARAL    :  1; //!<     29: Access to Reserved Address Interrupt Line
    VOLATILE unsigned int   PEDL    :  1; //!<     28: Protocol Error in Data        Phase Interrupt Line
    VOLATILE unsigned int   PEAL    :  1; //!<     27: Protocol Error in Arbitration Phase Interrupt Line
    VOLATILE unsigned int   WDIL    :  1; //!<     26: Watchdog Interrupt Interrupt Line
    VOLATILE unsigned int   BOL     :  1; //!<     25: Bus_Off Status Interrupt Line
    VOLATILE unsigned int   EWL     :  1; //!<     24: Warning Status Interrupt Line
    VOLATILE unsigned int   EPL     :  1; //!<     23: Error Passive Interrupt Line
    VOLATILE unsigned int   ELOL    :  1; //!<     22: Error Logging Overflow Interrupt Line
    VOLATILE unsigned int   BEUL    :  1; //!<     21: Bit Error Uncorrected Interrupt Line
    VOLATILE unsigned int   BECL    :  1; //!<     20: Bit Error Corrected Interrupt Line
    VOLATILE unsigned int   DRXL    :  1; //!<     19: Message stored to Dedicated Rx Buffer Interrupt Line
    VOLATILE unsigned int   TOOL    :  1; //!<     18: Timeout Occurred Interrupt Line
    VOLATILE unsigned int   MRAFL   :  1; //!<     17: Message RAM Access Failure Interrupt Line
    VOLATILE unsigned int   TSWL    :  1; //!<     16: Timestamp Wraparound Interrupt Line
    VOLATILE unsigned int   TEFLL   :  1; //!<     15: Tx Event FIFO Element Lost Interrupt Line
    VOLATILE unsigned int   TEFFL   :  1; //!<     14: Tx Event FIFO Full Interrupt Line
    VOLATILE unsigned int   TEFWL   :  1; //!<     13: Tx Event FIFO Watermark Reached Interrupt Line
    VOLATILE unsigned int   TEFNL   :  1; //!<     12: Tx Event FIFO New Entry Interrupt Line
    VOLATILE unsigned int   TFEL    :  1; //!<     11: Tx FIFO Empty Interrupt Line
    VOLATILE unsigned int   TCFL    :  1; //!<     10: Transmission Cancellation Finished Interrupt Line
    VOLATILE unsigned int   TCL     :  1; //!<     09: Transmission Completed Interrupt Line
    VOLATILE unsigned int   HPML    :  1; //!<     08: High Priority Message Interrupt Line
    VOLATILE unsigned int   RF1LL   :  1; //!<     07: Rx FIFO 1 Message Lost Interrupt Line
    VOLATILE unsigned int   RF1FL   :  1; //!<     06: Rx FIFO 1 Full Interrupt Line
    VOLATILE unsigned int   RF1WL   :  1; //!<     05: Rx FIFO 1 Watermark Reached Interrupt Line
    VOLATILE unsigned int   RF1NL   :  1; //!<     04: Rx FIFO 1 New Message Interrupt Line
    VOLATILE unsigned int   RF0LL   :  1; //!<     03: Rx FIFO 0 Message Lost Interrupt Line
    VOLATILE unsigned int   RF0FL   :  1; //!<     02: Rx FIFO 0 Full Interrupt Line
    VOLATILE unsigned int   RF0WL   :  1; //!<     01: Rx FIFO 0 Watermark Reached Interrupt Line
    VOLATILE unsigned int   RF0NL   :  1; //!<     00: Rx FIFO 0 New Message Interrupt Line
  } bits;                                 //!< Bit representation
} M_CAN_ILS_union;
#endif

// Interrupt Register Bit Positions in registers IR, ILE, ILS
#define IR_ARA_ACCESS_TO_RESERVED_ADDRESS 				BIT(29)
#define IR_PED_PROTOCOL_ERROR_IN_DATA_PHASE 			BIT(28)
#define IR_PEA_PROTOCOL_ERROR_IN_ARBITRATION_PHASE 		BIT(27)
#define IR_WDI_WATCHDOG							 		BIT(26)
#define IR_BO_BUS_OFF_STATUS					 		BIT(25)
#define IR_EW_WARNING_STATUS					 		BIT(24)
#define IR_EP_ERROR_PASSIVE					 		    BIT(23)
#define IR_ELO_ERROR_LOGGING_OVERFLOW			 		BIT(22)
#define IR_BEU_BIT_ERROR_UNCORRECTED			 		BIT(21)
#define IR_BEC_BIT_ERROR_CORRECTED				 		BIT(20)
#define IR_DRX_MESSAGE_STORED_TO_DEDICATED_RX_BUFFER	BIT(19)
#define IR_TOO_TIMEOUT_OCCURRED							BIT(18)
#define IR_MRAF_MESSAGE_RAM_ACCESS_FAILURE				BIT(17)
#define IR_TSW_TIMESTAMP_WRAPAROUND						BIT(16)
#define IR_TEFL_TX_EVENT_FIFO_EVENT_LOST				BIT(15)
#define IR_TEFF_TX_EVENT_FIFO_FULL						BIT(14)
#define IR_TEFW_TX_EVENT_FIFO_WATERMARK_REACHED			BIT(13)
#define IR_TEFN_TX_EVENT_FIFO_NEW_ENTRY					BIT(12)
#define IR_TFE_TX_FIFO_EMPTY							BIT(11)
#define IR_TCF_TRANSMISSION_CANCELLATION_FINISHED		BIT(10)
#define IR_TC_TRANSMISSION_COMPLETED					BIT(9)
#define IR_HPM_HIGH_PRIORITY_MESSAGE					BIT(8)
#define IR_RF1L_RX_FIFO1_MESSAGE_LOST					BIT(7)
#define IR_RF1F_RX_FIFO1_FULL							BIT(6)
#define IR_RF1W_RX_FIFO1_WATERMARK_REACHED				BIT(5)
#define IR_RF1N_RX_FIFO1_NEW_MESSAGE					BIT(4)
#define IR_RF0L_RX_FIFO0_MESSAGE_LOST					BIT(3)
#define IR_RF0F_RX_FIFO0_FULL           				BIT(2)
#define IR_RF0W_RX_FIFO0_WATERMARK_REACHED				BIT(1)
#define IR_RF0N_RX_FIFO0_NEW_MESSAGE					BIT(0)

#define INTERRUPT_ALL_SIGNALS		0x3FFFFFFF

#if __LITTLE_ENDIAN
/* Type Definition for register ILE - Interrupt Line Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   EINT0   :  1; //!<     00: enable interrupt line 0
    VOLATILE unsigned int   EINT1   :  1; //!<     01: enable interrupt line 1
    VOLATILE unsigned int   res0    : 30; //!< 31..02: reserved
  } bits;                                 //!< Bit representation
} M_CAN_ILE_union;

#elif __BIG_ENDIAN
/* Type Definition for register ILE - Interrupt Line Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 30; //!< 31..02: reserved
    VOLATILE unsigned int   EINT1   :  1; //!<     01: enable interrupt line 1
    VOLATILE unsigned int   EINT0   :  1; //!<     00: enable interrupt line 0
  } bits;                                 //!< Bit representation
} M_CAN_ILE_union;
#endif

/* ILE defines */
#define ILE_DISABLE_INTERRUPT_LINES     0x0
#define ILE_ENABLE_INTERRUPT_LINE_0     0x1
#define ILE_ENABLE_INTERRUPT_LINE_1     0x2

#if __LITTLE_ENDIAN
/* Type Definition for register GFC - Global Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RRFE    :  1; //!<     00: reject remote frames extended
    VOLATILE unsigned int   RRFS    :  1; //!<     01: reject remote frames standard
    VOLATILE unsigned int   ANFE    :  2; //!< 03..02: Accept Non-matching frames extended
    VOLATILE unsigned int   ANFS    :  2; //!< 05..04: Accept Non-matching frames standard
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
  } bits;                                 //!< Bit representation
} M_CAN_GFC_union;

#elif __BIG_ENDIAN
/* Type Definition for register GFC - Global Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
    VOLATILE unsigned int   ANFS    :  2; //!< 05..04: Accept Non-matching frames standard
    VOLATILE unsigned int   ANFE    :  2; //!< 03..02: Accept Non-matching frames extended
    VOLATILE unsigned int   RRFS    :  1; //!<     01: reject remote frames standard
    VOLATILE unsigned int   RRFE    :  1; //!<     00: reject remote frames extended
  } bits;                                 //!< Bit representation
} M_CAN_GFC_union;
#endif

/* Accept Non-matching Frames (GFC Register) */
typedef enum {
  ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0 = 0x0,
  ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO1 = 0x1,
  REJECT_NON_MATCHING_FRAMES             = 0x3
} GFC_accept_non_matching_frames_enum;

#if __LITTLE_ENDIAN
/* Type Definition for register SIDFC - Standard ID Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   FLSSA   : 14; //!< 15..02: Filter List Standard Start Address
    VOLATILE unsigned int   LSS     :  8; //!< 23..16: List Size Standard
    VOLATILE unsigned int   res1    :  8; //!< 31..24: reserved
  } bits;                                 //!< Bit representation
} M_CAN_SIDFC_union;

#elif __BIG_ENDIAN
/* Type Definition for register SIDFC - Standard ID Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  8; //!< 31..24: reserved
    VOLATILE unsigned int   LSS     :  8; //!< 23..16: List Size Standard
    VOLATILE unsigned int   FLSSA   : 14; //!< 15..02: Filter List Standard Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_SIDFC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register XIDFC - Extended ID Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   FLESA   : 14; //!< 15..02: Filter List Extended Start Address
    VOLATILE unsigned int   LSE     :  7; //!< 22..16: List Size Extended
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
  } bits;                                 //!< Bit representation
} M_CAN_XIDFC_union;

#elif __BIG_ENDIAN
/* Type Definition for register XIDFC - Extended ID Filter Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
    VOLATILE unsigned int   LSE     :  7; //!< 22..16: List Size Extended
    VOLATILE unsigned int   FLESA   : 14; //!< 15..02: Filter List Extended Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_XIDFC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register XIDAM - Extended ID AND Mask */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   EIDM    : 29; //!< 28..00: Extended ID Mask
    VOLATILE unsigned int   res0    :  3; //!< 31..29: reserved
  } bits;                                 //!< Bit representation
} M_CAN_XIDAM_union;

#elif __BIG_ENDIAN
/* Type Definition for register XIDAM - Extended ID AND Mask */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  3; //!< 31..29: reserved
    VOLATILE unsigned int   EIDM    : 29; //!< 28..00: Extended ID Mask
  } bits;                                 //!< Bit representation
} M_CAN_XIDAM_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register HPMS - High Priority Message Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   BIDX    :  6; //!< 05..00: Buffer Index
    VOLATILE unsigned int   MSI     :  2; //!< 07..06: Message Storage Indicator
    VOLATILE unsigned int   FIDX    :  7; //!< 14..08: Filter Index
    VOLATILE unsigned int   FLST    :  1; //!<     15: Filter List
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_HPMS_union;

#elif __BIG_ENDIAN
/* Type Definition for register HPMS - High Priority Message Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   FLST    :  1; //!<     15: Filter List
    VOLATILE unsigned int   FIDX    :  7; //!< 14..08: Filter Index
    VOLATILE unsigned int   MSI     :  2; //!< 07..06: Message Storage Indicator
    VOLATILE unsigned int   BIDX    :  6; //!< 05..00: Buffer Index
  } bits;                                 //!< Bit representation
} M_CAN_HPMS_union;
#endif

/* HPMS Register: Message Storage Indicator (MSI) */
typedef enum {
	HPMS_MSI_NO_FIFO_SELECTED         = 0x0,
	HPMS_MSI_FIFO_MESSAGE_LOST        = 0x1,
	HPMS_MSI_MESSAGE_STORED_IN_FIFO0  = 0x2,
	HPMS_MSI_MESSAGE_STORED_IN_FIFO1  = 0x3
} MSI_Message_Storage_Indicator;

/* Type Definition for register NDAT1 - New Data 1 */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   NDAT    : 32; //!<     0: New Data flag of Rx Buffer 0 to 31
  } bits;                                 //!< Bit representation
} M_CAN_NDAT1_union;

/* Type Definition for register NDAT2 - New Data 2 */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   NDAT    : 32; //!<     0: New Data flag of Rx Buffer 32 to 63
  } bits;                                 //!< Bit representation
} M_CAN_NDAT2_union;

#if __LITTLE_ENDIAN
/* Type Definition for register RXF0C - Rx FIFO 0 Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   F0SA    : 14; //!< 15..02: Rx FIFO 0 Start Address
    VOLATILE unsigned int   F0S     :  7; //!< 22..16: Rx FIFO 0 Size
    VOLATILE unsigned int   res1    :  1; //!<     23: reserved
    VOLATILE unsigned int   F0WM    :  7; //!< 30..24: Rx FIFO 0 Watermark
    VOLATILE unsigned int   F0OM    :  1; //!<     31: Rx FIFO 0 Operation Mode (blocking/overwrite)
  } bits;                                 //!< Bit representation
} M_CAN_RXF0C_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF0C - Rx FIFO 0 Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F0OM    :  1; //!<     31: Rx FIFO 0 Operation Mode (blocking/overwrite)
    VOLATILE unsigned int   F0WM    :  7; //!< 30..24: Rx FIFO 0 Watermark
    VOLATILE unsigned int   res1    :  1; //!<     23: reserved
    VOLATILE unsigned int   F0S     :  7; //!< 22..16: Rx FIFO 0 Size
    VOLATILE unsigned int   F0SA    : 14; //!< 15..02: Rx FIFO 0 Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXF0C_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXF0S - Rx FIFO 0 Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F0FL    :  7; //!< 06..00: Rx FIFO 0 Fill Level
    VOLATILE unsigned int   res0    :  1; //!<     07: reserved
    VOLATILE unsigned int   F0GI    :  6; //!< 13..08: Rx FIFO 0 Get Index
    VOLATILE unsigned int   res1    :  2; //!< 15..14: reserved
    VOLATILE unsigned int   F0PI    :  6; //!< 21..16: Rx FIFO 0 Put Index
    VOLATILE unsigned int   res2    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   F0F     :  1; //!<     24: Rx FIFO 0 Full
    VOLATILE unsigned int   RF0L    :  1; //!<     25: Rx FIFO 0 Message Lost
    VOLATILE unsigned int   res3    :  6; //!< 31..26: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXF0S_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF0S - Rx FIFO 0 Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res3    :  6; //!< 31..26: reserved
    VOLATILE unsigned int   RF0L    :  1; //!<     25: Rx FIFO 0 Message Lost
    VOLATILE unsigned int   F0F     :  1; //!<     24: Rx FIFO 0 Full
    VOLATILE unsigned int   res2    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   F0PI    :  6; //!< 21..16: Rx FIFO 0 Put Index
    VOLATILE unsigned int   res1    :  2; //!< 15..14: reserved
    VOLATILE unsigned int   F0GI    :  6; //!< 13..08: Rx FIFO 0 Get Index
    VOLATILE unsigned int   res0    :  1; //!<     07: reserved
    VOLATILE unsigned int   F0FL    :  7; //!< 06..00: Rx FIFO 0 Fill Level
  } bits;                                 //!< Bit representation
} M_CAN_RXF0S_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXF0A - Rx FIFO 0 Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F0AI    :  6; //!< 05..00: Rx FIFO 0 Acknowledge Index
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXF0A_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF0A - Rx FIFO 0 Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
    VOLATILE unsigned int   F0AI    :  6; //!< 05..00: Rx FIFO 0 Acknowledge Index
  } bits;                                 //!< Bit representation
} M_CAN_RXF0A_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXBC - Rx Buffer Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   RBSA    : 14; //!< 15..02: Rx Buffer Start Address
    VOLATILE unsigned int   res2    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXBC_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXBC - Rx Buffer Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res2    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   RBSA    : 14; //!< 15..02: Rx Buffer Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXBC_union;
#endif


#if __LITTLE_ENDIAN
/* Type Definition for register RXF1C - Rx FIFO 1 Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   F1SA    : 14; //!< 15..02: Rx FIFO 1 Start Address
    VOLATILE unsigned int   F1S     :  7; //!< 22..16: Rx FIFO 1 Size
    VOLATILE unsigned int   res1    :  1; //!<     23: reserved
    VOLATILE unsigned int   F1WM    :  7; //!< 30..24: Rx FIFO 1 Watermark
    VOLATILE unsigned int   F1OM    :  1; //!<     31: Rx FIFO 1 Operation Mode (blocking=0/overwrite)
  } bits;                                 //!< Bit representation
} M_CAN_RXF1C_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF1C - Rx FIFO 1 Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F1OM    :  1; //!<     31: Rx FIFO 1 Operation Mode (blocking=0/overwrite)
    VOLATILE unsigned int   F1WM    :  7; //!< 30..24: Rx FIFO 1 Watermark
    VOLATILE unsigned int   res1    :  1; //!<     23: reserved
    VOLATILE unsigned int   F1S     :  7; //!< 22..16: Rx FIFO 1 Size
    VOLATILE unsigned int   F1SA    : 14; //!< 15..02: Rx FIFO 1 Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXF1C_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXF1S - Rx FIFO 1 Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F1FL    :  7; //!< 06..00: Rx FIFO 1 Fill Level
    VOLATILE unsigned int   res0    :  1; //!<     07: reserved
    VOLATILE unsigned int   F1GI    :  6; //!< 13..08: Rx FIFO 1 Get Index
    VOLATILE unsigned int   res1    :  2; //!< 15..14: reserved
    VOLATILE unsigned int   F1PI    :  6; //!< 21..16: Rx FIFO 1 Put Index
    VOLATILE unsigned int   res2    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   F1F     :  1; //!<     24: Rx FIFO 1 Full
    VOLATILE unsigned int   RF1L    :  1; //!<     25: Rx FIFO 1 Message Lost
    VOLATILE unsigned int   res3    :  4; //!< 29..26: reserved
    VOLATILE unsigned int   DMS     :  2; //!< 31..30: Debug Message Status
  } bits;                                 //!< Bit representation
} M_CAN_RXF1S_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF1S - Rx FIFO 1 Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   DMS     :  2; //!< 31..30: Debug Message Status
    VOLATILE unsigned int   res3    :  4; //!< 29..26: reserved
    VOLATILE unsigned int   RF1L    :  1; //!<     25: Rx FIFO 1 Message Lost
    VOLATILE unsigned int   F1F     :  1; //!<     24: Rx FIFO 1 Full
    VOLATILE unsigned int   res2    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   F1PI    :  6; //!< 21..16: Rx FIFO 1 Put Index
    VOLATILE unsigned int   res1    :  2; //!< 15..14: reserved
    VOLATILE unsigned int   F1GI    :  6; //!< 13..08: Rx FIFO 1 Get Index
    VOLATILE unsigned int   res0    :  1; //!<     07: reserved
    VOLATILE unsigned int   F1FL    :  7; //!< 06..00: Rx FIFO 1 Fill Level
  } bits;                                 //!< Bit representation
} M_CAN_RXF1S_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXF1A - Rx FIFO 1 Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F1AI    :  6; //!< 05..00: Rx FIFO 1 Acknowledge Index
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXF1A_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXF1A - Rx FIFO 1 Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 26; //!< 31..06: reserved
    VOLATILE unsigned int   F1AI    :  6; //!< 05..00: Rx FIFO 1 Acknowledge Index
  } bits;                                 //!< Bit representation
} M_CAN_RXF1A_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register RXESC - Rx Buffer / FIFO Element Size Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   F0DS    :  3; //!< 02..00: Rx FIFO 0 Data Filed Size
    VOLATILE unsigned int   res0    :  1; //!<     03: reserved
    VOLATILE unsigned int   F1DS    :  3; //!< 06..04: Rx FIFO 1 Date Field Size
    VOLATILE unsigned int   res1    :  1; //!<     07: reserved
    VOLATILE unsigned int   RBDS    :  3; //!< 10..08: Rx Buffer Data Field Size
    VOLATILE unsigned int   res2    : 21; //!< 31..11: reserved
  } bits;                                 //!< Bit representation
} M_CAN_RXESC_union;

#elif __BIG_ENDIAN
/* Type Definition for register RXESC - Rx Buffer / FIFO Element Size Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res2    : 21; //!< 31..11: reserved
    VOLATILE unsigned int   RBDS    :  3; //!< 10..08: Rx Buffer Data Field Size
    VOLATILE unsigned int   res1    :  1; //!<     07: reserved
    VOLATILE unsigned int   F1DS    :  3; //!< 06..04: Rx FIFO 1 Date Field Size
    VOLATILE unsigned int   res0    :  1; //!<     03: reserved
    VOLATILE unsigned int   F0DS    :  3; //!< 02..00: Rx FIFO 0 Data Filed Size
  } bits;                                 //!< Bit representation
} M_CAN_RXESC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TXBC - Tx Buffer Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   TBSA    : 14; //!< 15..02: Tx Buffers Start Address
    VOLATILE unsigned int   NDTB    :  6; //!< 21..16: Number of Dedicated Transmit Buffers
    VOLATILE unsigned int   res1    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   TFQS    :  6; //!< 29..24: Transmit FIFO/Queue Size
    VOLATILE unsigned int   TFQM    :  1; //!<     30: Tx FIFO/Queue Mode
    VOLATILE unsigned int   res2    :  1; //!<     31: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXBC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXBC - Tx Buffer Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res2    :  1; //!<     31: reserved
    VOLATILE unsigned int   TFQM    :  1; //!<     30: Tx FIFO/Queue Mode
    VOLATILE unsigned int   TFQS    :  6; //!< 29..24: Transmit FIFO/Queue Size
    VOLATILE unsigned int   res1    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   NDTB    :  6; //!< 21..16: Number of Dedicated Transmit Buffers
    VOLATILE unsigned int   TBSA    : 14; //!< 15..02: Tx Buffers Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXBC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TXFQS - Tx FIFO/Queue Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TFFL    :  6; //!< 05..00: Tx FIFO Free Level
    VOLATILE unsigned int   res0    :  2; //!< 07..06: reserved
    VOLATILE unsigned int   TFGI    :  5; //!< 12..08: Tx FIFO Get Index
    VOLATILE unsigned int   res1    :  3; //!< 23..20: reserved
    VOLATILE unsigned int   TFQPI   :  5; //!< 20..16: Tx FIFO/Queue Put Index
    VOLATILE unsigned int   TFQF    :  1; //!<     21: Tx FIFO/Queue Full (0=not full, 1=full)
    VOLATILE unsigned int   res2    : 10; //!< 31..22: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXFQS_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXFQS - Tx FIFO/Queue Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res2    : 10; //!< 31..22: reserved
    VOLATILE unsigned int   TFQF    :  1; //!<     21: Tx FIFO/Queue Full
    VOLATILE unsigned int   TFQPI   :  5; //!< 20..16: Tx FIFO/Queue Put Index
    VOLATILE unsigned int   res1    :  3; //!< 23..20: reserved
    VOLATILE unsigned int   TFGI    :  5; //!< 12..08: Tx FIFO Get Index
    VOLATILE unsigned int   res0    :  2; //!< 07..06: reserved
    VOLATILE unsigned int   TFFL    :  6; //!< 05..00: Tx FIFO Free Level
  } bits;                                 //!< Bit representation
} M_CAN_TXFQS_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TXESC - Tx Buffer / FIFO Element Size Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TBDS    :  3; //!< 02..00: Tx Buffer Data Filed Size
    VOLATILE unsigned int   res     : 29; //!< 03..31: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXESC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXESC - Tx Buffer / FIFO Element Size Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res     : 29; //!< 03..31: reserved
    VOLATILE unsigned int   TBDS    :  3; //!< 02..00: Tx Buffer Data Filed Size
  } bits;                                 //!< Bit representation
} M_CAN_TXESC_union;
#endif

/* Buffer/FIFO/Queue Data Field Size */
typedef enum {
  BYTE8 = 0,
  BYTE12 = 1,
  BYTE16 = 2,
  BYTE20 = 3,
  BYTE24 = 4,
  BYTE32 = 5,
  BYTE48 = 6,
  BYTE64 = 7
} data_field_size_enum;

/* Type Definition for register TXBRP - Tx Buffer Request Pending */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TRP     : 32; //!< 31..00: Transmission Request Pending
  } bits;                                 //!< Bit representation
} M_CAN_TXBRP_union;

/* Type Definition for register TXBAR - Tx Buffer Add Request */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   AR      : 32; //!< 31..00: Add Request
  } bits;                                 //!< Bit representation
} M_CAN_TXBAR_union;

/* Type Definition for register TXBCR - Tx Buffer Cancellation Request */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CR      : 32; //!< 31..00: Cancellation Request
  } bits;                                 //!< Bit representation
} M_CAN_TXBCR_union;

/* Type Definition for register TXBTO - Tx Buffer Transmission Occurred */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TO      : 32; //!< 31..00: Transmission Occurred
  } bits;                                 //!< Bit representation
} M_CAN_TXBTO_union;

/* Type Definition for register TXBCF - Tx Buffer Cancellation Finished */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CF     : 32; //!< 31..00: Cancellation Finished
  } bits;                                 //!< Bit representation
} M_CAN_TXBCF_union;

/* Type Definition for register TXBTIE - Tx Buffer Transmission Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int  TIE     : 32; //!< 31..00: Transmission Interrupt Enable
  } bits;                                 //!< Bit representation
} M_CAN_TXBTIE_union;

/* Type Definition for register TXBCIE - Tx Buffer Cancellation Finished Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int  CIE     : 32; //!< 31..00: Cancellation Finished Interrupt Enable
  } bits;                                 //!< Bit representation
} M_CAN_TXBCIE_union;

#if __LITTLE_ENDIAN
/* Type Definition for register TXEFC - Tx Event FIFO Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   EFSA    : 14; //!< 15..02: Event FIFO Start Address
    VOLATILE unsigned int   EFS     :  6; //!< 21..16: Event FIFO Size
    VOLATILE unsigned int   res1    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   EFWM    :  6; //!< 29..24: Event FIFO Watermark
    VOLATILE unsigned int   res2    :  2; //!< 31..30: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXEFC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXEFC - Tx Event FIFO Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res2    :  2; //!< 31..30: reserved
    VOLATILE unsigned int   EFWM    :  6; //!< 29..24: Event FIFO Watermark
    VOLATILE unsigned int   res1    :  2; //!< 23..22: reserved
    VOLATILE unsigned int   EFS     :  6; //!< 21..16: Event FIFO Size
    VOLATILE unsigned int   EFSA    : 14; //!< 15..02: Event FIFO Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXEFC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TXEFS - Tx Event FIFO Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   EFFL    :  6; //!< 05..00: Event FIFO Fill Level
    VOLATILE unsigned int   res0    :  2; //!< 07..06: reserved
    VOLATILE unsigned int   EFGI    :  5; //!< 12..08: Event FIFO Get Index
    VOLATILE unsigned int   res1    :  3; //!< 15..13: reserved
    VOLATILE unsigned int   EFPI    :  5; //!< 20..16: Event FIFO Put Index
    VOLATILE unsigned int   res2    :  3; //!< 23..21: reserved
    VOLATILE unsigned int   EFF     :  1; //!<     24: Event FIFO Full
    VOLATILE unsigned int   TEFL    :  1; //!<     25: Tx Event FIFO Element Lost
    VOLATILE unsigned int   res3    :  6; //!< 31..26: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXEFS_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXEFS - Tx Event FIFO Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res3    :  6; //!< 31..26: reserved
    VOLATILE unsigned int   TEFL    :  1; //!<     25: Tx Event FIFO Element Lost
    VOLATILE unsigned int   EFF     :  1; //!<     24: Event FIFO Full
    VOLATILE unsigned int   res2    :  3; //!< 23..21: reserved
    VOLATILE unsigned int   EFPI    :  5; //!< 20..16: Event FIFO Put Index
    VOLATILE unsigned int   res1    :  3; //!< 15..13: reserved
    VOLATILE unsigned int   EFGI    :  5; //!< 12..08: Event FIFO Get Index
    VOLATILE unsigned int   res0    :  2; //!< 07..06: reserved
    VOLATILE unsigned int   EFFL    :  6; //!< 05..00: Event FIFO Fill Level
  } bits;                                 //!< Bit representation
} M_CAN_TXEFS_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TXEFA - Tx Event FIFO Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   EFAI    :  5; //!< 04..00: Event FIFO Acknowledge Index
    VOLATILE unsigned int   res0    : 27; //!< 31..05: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TXEFA_union;

#elif __BIG_ENDIAN
/* Type Definition for register TXEFA - Tx Event FIFO Acknowledge */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 27; //!< 31..05: reserved
    VOLATILE unsigned int   EFAI    :  5; //!< 04..00: Event FIFO Acknowledge Index
  } bits;                                 //!< Bit representation
} M_CAN_TXEFA_union;
#endif

// Numer of Last Error Codes (LEC)
#define LEC_NO_OF_ERROR_CODES  8

/* LEC values - Last Error Code */
enum lec_type {
  LEC_NO_ERROR    = 0,
  LEC_STUFF_ERROR = 1,
  LEC_FORM_ERROR  = 2,
  LEC_ACK_ERROR   = 3,
  LEC_BIT1_ERROR  = 4,
  LEC_BIT0_ERROR  = 5,
  LEC_CRC_ERROR   = 6,
  LEC_NO_CHANGE   = 7,
};


//  ~~~~~~~~~ M_TT_CAN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if __LITTLE_ENDIAN
/* Type Definition for register TTTMC - TT Trigger Memory Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
    VOLATILE unsigned int   TMSA    : 14; //!< 15..02: Trigger Memory Start Address
    VOLATILE unsigned int   TME     :  7; //!< 22..16: Trigger Memory Elements
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTTMC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTTMC - TT Trigger Memory Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  9; //!< 31..23: reserved
    VOLATILE unsigned int   TME     :  7; //!< 22..16: Trigger Memory Elements
    VOLATILE unsigned int   TMSA    : 14; //!< 15..02: Trigger Memory Start Address
    VOLATILE unsigned int   res0    :  2; //!< 01..00: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTTMC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTRMC - TT Reference Message Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RID     : 29; //!< 28..00: Reference Identifier
    VOLATILE unsigned int   res0    :  1; //!<     29: reserved
    VOLATILE unsigned int   XTD     :  1; //!<     30: Extended Identifier
    VOLATILE unsigned int   RMPS    :  1; //!<     31: Reference Message Payload Select
  } bits;                                 //!< Bit representation
} M_CAN_TTRMC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTRMC - TT Reference Message Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   RMPS    :  1; //!<     31: Reference Message Payload Select
    VOLATILE unsigned int   XTD     :  1; //!<     30: Extended Identifier
    VOLATILE unsigned int   res0    :  1; //!<     29: reserved
    VOLATILE unsigned int   RID     : 29; //!< 28..00: Reference Identifier
  } bits;                                 //!< Bit representation
} M_CAN_TTRMC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTOCF - TT Operation Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   OM      :  2; //!< 01..00: Operation Mode
    VOLATILE unsigned int   res0    :  1; //!<     02: reserved
    VOLATILE unsigned int   GEN     :  1; //!<     03: Gap Enable
    VOLATILE unsigned int   TM      :  1; //!<     04: Time Master
    VOLATILE unsigned int   LDSDL   :  3; //!< 07..05: LD of Synchronization Deviation Limit
    VOLATILE unsigned int   IRTO    :  7; //!< 14..08: Initial Reference Trigger Offset
    VOLATILE unsigned int   EECS    :  1; //!<     15: Enable External Clock Synchronization
    VOLATILE unsigned int   AWL     :  8; //!< 23..16: Application Watchdog Limit
    VOLATILE unsigned int   EGTF    :  1; //!<     24: Enable Global Time Filtering
    VOLATILE unsigned int   ECC     :  1; //!<     25: Enable Clock Calibration
    VOLATILE unsigned int   EVTP    :  1; //!<     26: Event Trigger Polarity
    VOLATILE unsigned int   res1    :  5; //!< 31..27: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTOCF_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTOCF - TT Operation Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  5; //!< 31..27: reserved
    VOLATILE unsigned int   EVTP    :  1; //!<     26: Event Trigger Polarity
    VOLATILE unsigned int   ECC     :  1; //!<     25: Enable Clock Calibration
    VOLATILE unsigned int   EGTF    :  1; //!<     24: Enable Global Time Filtering
    VOLATILE unsigned int   AWL     :  8; //!< 23..16: Application Watchdog Limit
    VOLATILE unsigned int   EECS    :  1; //!<     15: Enable External Clock Synchronization
    VOLATILE unsigned int   IRTO    :  7; //!< 14..08: Initial Reference Trigger Offset
    VOLATILE unsigned int   LDSDL   :  3; //!< 07..05: LD of Synchronization Deviation Limit
    VOLATILE unsigned int   TM      :  1; //!<     04: Time Master
    VOLATILE unsigned int   GEN     :  1; //!<     03: Gap Enable
    VOLATILE unsigned int   res0    :  1; //!<     02: reserved
    VOLATILE unsigned int   OM      :  2; //!< 01..00: Operation Mode
  } bits;                                 //!< Bit representation
} M_CAN_TTOCF_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTMLM - TT Matrix Limits */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CCM     :  6; //!< 05..00: Cycle Count Max
    VOLATILE unsigned int   CSS     :  2; //!< 07..06: Cycle Start Synchronization
    VOLATILE unsigned int   TXEW    :  4; //!< 11..08: Tx Enable Window
    VOLATILE unsigned int   res0    :  4; //!< 15..12: reserved
    VOLATILE unsigned int   ENTT    : 12; //!< 27..16: Expected Number of Tx Triggers
    VOLATILE unsigned int   res1    :  4; //!< 31..28: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTMLM_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTMLM - TT Matrix Limits */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    :  4; //!< 31..28: reserved
    VOLATILE unsigned int   ENTT    : 12; //!< 27..16: Expected Number of Tx Triggers
    VOLATILE unsigned int   res0    :  4; //!< 15..12: reserved
    VOLATILE unsigned int   TXEW    :  4; //!< 11..08: Tx Enable Window
    VOLATILE unsigned int   CSS     :  2; //!< 07..06: Cycle Start Synchronization
    VOLATILE unsigned int   CCM     :  6; //!< 05..00: Cycle Count Max
  } bits;                                 //!< Bit representation
} M_CAN_TTMLM_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TURCF - TUR Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   NCL     : 16; //!< 15..00: Numerator Configuration Low
    VOLATILE unsigned int   DC      : 14; //!< 29..16: Denominator Configuration
    VOLATILE unsigned int   res0    :  1; //!<     30: reserved
    VOLATILE unsigned int   ELT     :  1; //!<     31: Enable Local Time
  } bits;                                 //!< Bit representation
} M_CAN_TURCF_union;

#elif __BIG_ENDIAN
/* Type Definition for register TURCF - TUR Configuration */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   ELT     :  1; //!<     31: Enable Local Time
    VOLATILE unsigned int   res0    :  1; //!<     30: reserved
    VOLATILE unsigned int   DC      : 14; //!< 29..16: Denominator Configuration
    VOLATILE unsigned int   NCL     : 16; //!< 15..00: Numerator Configuration Low
  } bits;                                 //!< Bit representation
} M_CAN_TURCF_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTOCN - TT Operation Control */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SGT     :  1; //!<     00: Set Global time
    VOLATILE unsigned int   ECS     :  1; //!<     01: External Clock Synchronization
    VOLATILE unsigned int   SWP     :  1; //!<     02: Stop Watch Edge Select
    VOLATILE unsigned int   SWS     :  2; //!< 04..03: Stop Watch Source
    VOLATILE unsigned int   RTIE    :  1; //!<     05: Register Time Mark Interrupt Pulse Enable
    VOLATILE unsigned int   TMC     :  2; //!< 07..06: Register Time Mark Compare
    VOLATILE unsigned int   TTIE    :  1; //!<     08: Trigger Time Mark Interrupt Pulse Enable
    VOLATILE unsigned int   GCS     :  1; //!<     09: Gap Control Select
    VOLATILE unsigned int   FGP     :  1; //!<     10: Gap Finished Indicator
    VOLATILE unsigned int   TMG     :  1; //!<     11: Time Mark Gap
    VOLATILE unsigned int   NIG     :  1; //!<     12: Next is Gap
    VOLATILE unsigned int   ESCN    :  1; //!<     13: External Synchronization Control
    VOLATILE unsigned int   res0    :  1; //!<     14: reserved
    VOLATILE unsigned int   LCKC    :  1; //!<     15: TT Operation Control Register Locked
    VOLATILE unsigned int   res1    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTOCN_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTOCN - TT Operation Control */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res1    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   LCKC    :  1; //!<     15: TT Operation Control Register Locked
    VOLATILE unsigned int   res0    :  1; //!<     14: reserved
    VOLATILE unsigned int   ESCN    :  1; //!<     13: External Synchronization Control
    VOLATILE unsigned int   NIG     :  1; //!<     12: Next is Gap
    VOLATILE unsigned int   TMG     :  1; //!<     11: Time Mark Gap
    VOLATILE unsigned int   FGP     :  1; //!<     10: Gap Finished Indicator
    VOLATILE unsigned int   GCS     :  1; //!<     09: Gap Control Select
    VOLATILE unsigned int   TTIE    :  1; //!<     08: Trigger Time Mark Interrupt Pulse Enable
    VOLATILE unsigned int   TMC     :  2; //!< 07..06: Register Time Mark Compare
    VOLATILE unsigned int   RTIE    :  1; //!<     05: Register Time Mark Interrupt Pulse Enable
    VOLATILE unsigned int   SWS     :  2; //!< 04..03: Stop Watch Source
    VOLATILE unsigned int   SWP     :  1; //!<     02: Stop Watch Edge Select
    VOLATILE unsigned int   ECS     :  1; //!<     01: External Clock Synchronization
    VOLATILE unsigned int   SGT     :  1; //!<     00: Set Global time
  } bits;                                 //!< Bit representation
} M_CAN_TTOCN_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTGTP - TT Global Time Preset */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TP      : 16; //!< 15..00: Time Preset
    VOLATILE unsigned int   CTP     : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTGTP_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTGTP - TT Global Time Preset */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CTP     : 16; //!< 31..16: reserved
    VOLATILE unsigned int   TP      : 16; //!< 15..00: Time Preset
  } bits;                                 //!< Bit representation
} M_CAN_TTGTP_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTTMK - TT Register Time Mark */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   TM      : 16; //!< 15..00: Time Mark
    VOLATILE unsigned int   TICC    :  7; //!< 22..16: Time Mark Cycle Code
    VOLATILE unsigned int   res0    :  8; //!< 30..23: reserved
    VOLATILE unsigned int   LCKM    :  1; //!<     31: TT Time Mark Register Locked
  } bits;                                 //!< Bit representation
} M_CAN_TTTMK_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTTMK - TT Register Time Mark */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   LCKM    :  1; //!<     31: TT Time Mark Register Locked
    VOLATILE unsigned int   res0    :  8; //!< 30..23: reserved
    VOLATILE unsigned int   TICC    :  7; //!< 22..16: Time Mark Cycle Code
    VOLATILE unsigned int   TM      : 16; //!< 15..00: Time Mark
  } bits;                                 //!< Bit representation
} M_CAN_TTTMK_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTIR - TT Interrupt Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SBC     :  1; //!<     00: Start of Basic Cycle
    VOLATILE unsigned int   SMC     :  1; //!<     01: Start of Matrix Cycle
    VOLATILE unsigned int   CSM     :  1; //!<     02: Change of Synchronization Mode
    VOLATILE unsigned int   SOG     :  1; //!<     03: Start of Gap
    VOLATILE unsigned int   RTMI    :  1; //!<     04: Register Time Mark Interrupt
    VOLATILE unsigned int   TTMI    :  1; //!<     05: Trigger Time Mark Interrupt
    VOLATILE unsigned int   SWE     :  1; //!<     06: Stop Watch Event
    VOLATILE unsigned int   GTW     :  1; //!<     07: Global Time Wrap
    VOLATILE unsigned int   GTD     :  1; //!<     08: Global Time Discontinuity
    VOLATILE unsigned int   GTE     :  1; //!<     09: Global Time Error
    VOLATILE unsigned int   TXU     :  1; //!<     10: Tx Count Underflow
    VOLATILE unsigned int   TXO     :  1; //!<     11: Tx Count Overflow
    VOLATILE unsigned int   SE1     :  1; //!<     12: Scheduling Error 1
    VOLATILE unsigned int   SE2     :  1; //!<     13: Scheduling Error 2
    VOLATILE unsigned int   ELC     :  1; //!<     14: Error Level Changed
    VOLATILE unsigned int   IWT     :  1; //!<     15: Initialization Watch Trigger
    VOLATILE unsigned int   WT      :  1; //!<     16: Watch Trigger
    VOLATILE unsigned int   AW      :  1; //!<     17: Application Watchdog
    VOLATILE unsigned int   CER     :  1; //!<     18: Configuration Error
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTIR_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTIR - TT Interrupt Register */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
    VOLATILE unsigned int   CER     :  1; //!<     18: Configuration Error
    VOLATILE unsigned int   AW      :  1; //!<     17: Application Watchdog
    VOLATILE unsigned int   WT      :  1; //!<     16: Watch Trigger
    VOLATILE unsigned int   IWT     :  1; //!<     15: Initialization Watch Trigger
    VOLATILE unsigned int   ELC     :  1; //!<     14: Error Level Changed
    VOLATILE unsigned int   SE2     :  1; //!<     13: Scheduling Error 2
    VOLATILE unsigned int   SE1     :  1; //!<     12: Scheduling Error 1
    VOLATILE unsigned int   TXO     :  1; //!<     11: Tx Count Overflow
    VOLATILE unsigned int   TXU     :  1; //!<     10: Tx Count Underflow
    VOLATILE unsigned int   GTE     :  1; //!<     09: Global Time Error
    VOLATILE unsigned int   GTD     :  1; //!<     08: Global Time Discontinuity
    VOLATILE unsigned int   GTW     :  1; //!<     07: Global Time Wrap
    VOLATILE unsigned int   SWE     :  1; //!<     06: Stop Watch Event
    VOLATILE unsigned int   TTMI    :  1; //!<     05: Trigger Time Mark Interrupt
    VOLATILE unsigned int   RTMI    :  1; //!<     04: Register Time Mark Interrupt
    VOLATILE unsigned int   SOG     :  1; //!<     03: Start of Gap
    VOLATILE unsigned int   CSM     :  1; //!<     02: Change of Synchronization Mode
    VOLATILE unsigned int   SMC     :  1; //!<     01: Start of Matrix Cycle
    VOLATILE unsigned int   SBC     :  1; //!<     00: Start of Basic Cycle
  } bits;                                 //!< Bit representation
} M_CAN_TTIR_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTIE - TT Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SBCE    :  1; //!<     00: Start of Basic Cycle Interrupt Enable
    VOLATILE unsigned int   SMCE    :  1; //!<     01: Start of Matrix Cycle Interrupt Enable
    VOLATILE unsigned int   CSME    :  1; //!<     02: Change of Synchronization Mode Interrupt Enable
    VOLATILE unsigned int   SOGE    :  1; //!<     03: Start of Gap Interrupt Enable
    VOLATILE unsigned int   RTMIE   :  1; //!<     04: Register Time Mark Interrupt Interrupt Enable
    VOLATILE unsigned int   TTMIE   :  1; //!<     05: Trigger Time Mark Interrupt Interrupt Enable
    VOLATILE unsigned int   SWEE    :  1; //!<     06: Stop Watch Event Interrupt Enable
    VOLATILE unsigned int   GTWE    :  1; //!<     07: Global Time Wrap Interrupt Enable
    VOLATILE unsigned int   GTDE    :  1; //!<     08: Global Time Discontinuity Interrupt Enable
    VOLATILE unsigned int   GTEE    :  1; //!<     09: Global Time Error Interrupt Enable
    VOLATILE unsigned int   TXUE    :  1; //!<     10: Tx Count Underflow Interrupt Enable
    VOLATILE unsigned int   TXOE    :  1; //!<     11: Tx Count Overflow Interrupt Enable
    VOLATILE unsigned int   SE1E    :  1; //!<     12: Scheduling Error 1 Interrupt Enable
    VOLATILE unsigned int   SE2E    :  1; //!<     13: Scheduling Error 2 Interrupt Enable
    VOLATILE unsigned int   ELCE    :  1; //!<     14: Error Level Changed Interrupt Enable
    VOLATILE unsigned int   IWTE    :  1; //!<     15: Initialization Watch Trigger Interrupt Enable
    VOLATILE unsigned int   WTE     :  1; //!<     16: Watch Trigger Interrupt Enable
    VOLATILE unsigned int   AWE     :  1; //!<     17: Application Watchdog Interrupt Enable
    VOLATILE unsigned int   CERE    :  1; //!<     18: Configuration Error Interrupt Enable
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTIE_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTIE - TT Interrupt Enable */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
    VOLATILE unsigned int   CERE    :  1; //!<     18: Configuration Error Interrupt Enable
    VOLATILE unsigned int   AWE     :  1; //!<     17: Application Watchdog Interrupt Enable
    VOLATILE unsigned int   WTE     :  1; //!<     16: Watch Trigger Interrupt Enable
    VOLATILE unsigned int   IWTE    :  1; //!<     15: Initialization Watch Trigger Interrupt Enable
    VOLATILE unsigned int   ELCE    :  1; //!<     14: Error Level Changed Interrupt Enable
    VOLATILE unsigned int   SE2E    :  1; //!<     13: Scheduling Error 2 Interrupt Enable
    VOLATILE unsigned int   SE1E    :  1; //!<     12: Scheduling Error 1 Interrupt Enable
    VOLATILE unsigned int   TXOE    :  1; //!<     11: Tx Count Overflow Interrupt Enable
    VOLATILE unsigned int   TXUE    :  1; //!<     10: Tx Count Underflow Interrupt Enable
    VOLATILE unsigned int   GTEE    :  1; //!<     09: Global Time Error Interrupt Enable
    VOLATILE unsigned int   GTDE    :  1; //!<     08: Global Time Discontinuity Interrupt Enable
    VOLATILE unsigned int   GTWE    :  1; //!<     07: Global Time Wrap Interrupt Enable
    VOLATILE unsigned int   SWEE    :  1; //!<     06: Stop Watch Event Interrupt Enable
    VOLATILE unsigned int   TTMIE   :  1; //!<     05: Trigger Time Mark Interrupt Interrupt Enable
    VOLATILE unsigned int   RTMIE   :  1; //!<     04: Register Time Mark Interrupt Interrupt Enable
    VOLATILE unsigned int   SOGE    :  1; //!<     03: Start of Gap Interrupt Enable
    VOLATILE unsigned int   CSME    :  1; //!<     02: Change of Synchronization Mode Interrupt Enable
    VOLATILE unsigned int   SMCE    :  1; //!<     01: Start of Matrix Cycle Interrupt Enable
    VOLATILE unsigned int   SBCE    :  1; //!<     00: Start of Basic Cycle Interrupt Enable
  } bits;                                 //!< Bit representation
} M_CAN_TTIE_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTILS - TT Interrupt Line Select */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SBCL    :  1; //!<     00: Start of Basic Cycle Interrupt Line
    VOLATILE unsigned int   SMCL    :  1; //!<     01: Start of Matrix Cycle Interrupt Line
    VOLATILE unsigned int   CSML    :  1; //!<     02: Change of Synchronization Mode Interrupt Line
    VOLATILE unsigned int   SOGL    :  1; //!<     03: Start of Gap Interrupt Line
    VOLATILE unsigned int   RTMIL   :  1; //!<     04: Register Time Mark Interrupt Interrupt Line
    VOLATILE unsigned int   TTMIL   :  1; //!<     05: Trigger Time Mark Interrupt Interrupt Line
    VOLATILE unsigned int   SWEL    :  1; //!<     06: Stop Watch Event Interrupt Line
    VOLATILE unsigned int   GTWL    :  1; //!<     07: Global Time Wrap Interrupt Line
    VOLATILE unsigned int   GTDL    :  1; //!<     08: Global Time Discontinuity Interrupt Line
    VOLATILE unsigned int   GTEL    :  1; //!<     09: Global Time Error Interrupt Line
    VOLATILE unsigned int   TXUL    :  1; //!<     10: Tx Count Underflow Interrupt Line
    VOLATILE unsigned int   TXOL    :  1; //!<     11: Tx Count Overflow Interrupt Line
    VOLATILE unsigned int   SE1L    :  1; //!<     12: Scheduling Error 1 Interrupt Line
    VOLATILE unsigned int   SE2L    :  1; //!<     13: Scheduling Error 2 Interrupt Line
    VOLATILE unsigned int   ELCL    :  1; //!<     14: Error Level Changed Interrupt Line
    VOLATILE unsigned int   IWTL    :  1; //!<     15: Initialization Watch Trigger Interrupt Line
    VOLATILE unsigned int   WTL     :  1; //!<     16: Watch Trigger Interrupt Line
    VOLATILE unsigned int   AWL     :  1; //!<     17: Application Watchdog Interrupt Line
    VOLATILE unsigned int   CERL    :  1; //!<     18: Configuration Error Interrupt Line
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTILS_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTILS - TT Interrupt Line Select */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 13; //!< 31..19: reserved
    VOLATILE unsigned int   CERL    :  1; //!<     18: Configuration Error Interrupt Line
    VOLATILE unsigned int   AWL     :  1; //!<     17: Application Watchdog Interrupt Line
    VOLATILE unsigned int   WTL     :  1; //!<     16: Watch Trigger Interrupt Line
    VOLATILE unsigned int   IWTL    :  1; //!<     15: Initialization Watch Trigger Interrupt Line
    VOLATILE unsigned int   ELCL    :  1; //!<     14: Error Level Changed Interrupt Line
    VOLATILE unsigned int   SE2L    :  1; //!<     13: Scheduling Error 2 Interrupt Line
    VOLATILE unsigned int   SE1L    :  1; //!<     12: Scheduling Error 1 Interrupt Line
    VOLATILE unsigned int   TXOL    :  1; //!<     11: Tx Count Overflow Interrupt Line
    VOLATILE unsigned int   TXUL    :  1; //!<     10: Tx Count Underflow Interrupt Line
    VOLATILE unsigned int   GTEL    :  1; //!<     09: Global Time Error Interrupt Line
    VOLATILE unsigned int   GTDL    :  1; //!<     08: Global Time Discontinuity Interrupt Line
    VOLATILE unsigned int   GTWL    :  1; //!<     07: Global Time Wrap Interrupt Line
    VOLATILE unsigned int   SWEL    :  1; //!<     06: Stop Watch Event Interrupt Line
    VOLATILE unsigned int   TTMIL   :  1; //!<     05: Trigger Time Mark Interrupt Interrupt Line
    VOLATILE unsigned int   RTMIL   :  1; //!<     04: Register Time Mark Interrupt Interrupt Line
    VOLATILE unsigned int   SOGL    :  1; //!<     03: Start of Gap Interrupt Line
    VOLATILE unsigned int   CSML    :  1; //!<     02: Change of Synchronization Mode Interrupt Line
    VOLATILE unsigned int   SMCL    :  1; //!<     01: Start of Matrix Cycle Interrupt Line
    VOLATILE unsigned int   SBCL    :  1; //!<     00: Start of Basic Cycle Interrupt Line
  } bits;                                 //!< Bit representation
} M_CAN_TTILS_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTOST - TT Operation Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   EL      :  2; //!< 01..00: error level
    VOLATILE unsigned int   MS      :  2; //!< 03..02: master state
    VOLATILE unsigned int   SYS     :  2; //!< 05..04: synchronization state
    VOLATILE unsigned int   QGTP    :  1; //!<     06: quality of global time phase
    VOLATILE unsigned int   QCS     :  1; //!<     07: quality of clock speed
    VOLATILE unsigned int   RTO     :  8; //!< 15..08: reference trigger offset
    VOLATILE unsigned int   res0    :  6; //!< 21..16: reserved
    VOLATILE unsigned int   WGTD    :  1; //!<     22: wait for global time discontinuity
    VOLATILE unsigned int   GFI     :  1; //!<     23: gap finished indicator
    VOLATILE unsigned int   TMP     :  3; //!< 26..24: time master priority
    VOLATILE unsigned int   GSI     :  1; //!<     27: gap started indicator
    VOLATILE unsigned int   WFE     :  1; //!<     28: wait for event
    VOLATILE unsigned int   AWE     :  1; //!<     29: application watchdog event
    VOLATILE unsigned int   WECS    :  1; //!<     30: wait for external clock synchronization
    VOLATILE unsigned int   SPL     :  1; //!<     31: Schedule Phase Lock
  } bits;                                 //!< Bit representation
} M_CAN_TTOST_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTOST - TT Operation Status */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SPL     :  1; //!<     31: Schedule Phase Lock
    VOLATILE unsigned int   WECS    :  1; //!<     30: wait for external clock synchronization
    VOLATILE unsigned int   AWE     :  1; //!<     29: application watchdog event
    VOLATILE unsigned int   WFE     :  1; //!<     28: wait for event
    VOLATILE unsigned int   GSI     :  1; //!<     27: gap started indicator
    VOLATILE unsigned int   TMP     :  3; //!< 26..24: time master priority
    VOLATILE unsigned int   GFI     :  1; //!<     23: gap finished indicator
    VOLATILE unsigned int   WGTD    :  1; //!<     22: wait for global time discontinuity
    VOLATILE unsigned int   res0    :  6; //!< 21..16: reserved
    VOLATILE unsigned int   RTO     :  8; //!< 15..08: reference trigger offset
    VOLATILE unsigned int   QCS     :  1; //!<     07: quality of clock speed
    VOLATILE unsigned int   QGTP    :  1; //!<     06: quality of global time phase
    VOLATILE unsigned int   SYS     :  2; //!< 05..04: synchronization state
    VOLATILE unsigned int   MS      :  2; //!< 03..02: master state
    VOLATILE unsigned int   EL      :  2; //!< 01..00: error level
  } bits;                                 //!< Bit representation
} M_CAN_TTOST_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TURNA - TUR Numerator Actual */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   NAV     : 18; //!< 17..00: Numerator Actual Value
    VOLATILE unsigned int   res0    : 14; //!< 31..18: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TURNA_union;

#elif __BIG_ENDIAN
/* Type Definition for register TURNA - TUR Numerator Actual */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 14; //!< 31..18: reserved
    VOLATILE unsigned int   NAV     : 18; //!< 17..00: Numerator Actual Value
  } bits;                                 //!< Bit representation
} M_CAN_TURNA_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTLGT - TT Local & Global Time */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   LT      : 16; //!< 15..00: LT[15:0]: Local Time
    VOLATILE unsigned int   GT      : 16; //!< 31..16: GT[15:0]: Global Time
  } bits;                                 //!< Bit representation
} M_CAN_TTLGT_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTLGT - TT Local & Global Time */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   GT      : 16; //!< 31..16: GT[15:0]: Global Time
    VOLATILE unsigned int   LT      : 16; //!< 15..00: LT[15:0]: Local Time
  } bits;                                 //!< Bit representation
} M_CAN_TTLGT_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTCTC - TT Cycle Time & Count */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CT      : 16; //!< 15..00: cycle time
    VOLATILE unsigned int   CC      :  6; //!< 21..16: cycle count
    VOLATILE unsigned int   res0    : 10; //!< 31..17: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTCTC_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTCTC - TT Cycle Time & Count */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 10; //!< 31..17: reserved
    VOLATILE unsigned int   CC      :  6; //!< 21..16: cycle count
    VOLATILE unsigned int   CT      : 16; //!< 15..00: cycle time
  } bits;                                 //!< Bit representation
} M_CAN_TTCTC_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTCPT - TT Capture Time */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CCV     :  6; //!< 05..00: Cycle Count Value
    VOLATILE unsigned int   res0    : 10; //!< 15..06: reserved
    VOLATILE unsigned int   SWV     : 16; //!< 31..16: Stop Watch Value
  } bits;                                 //!< Bit representation
} M_CAN_TTCPT_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTCPT - TT Capture Time */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   SWV     : 16; //!< 31..16: Stop Watch Value
    VOLATILE unsigned int   res0    : 10; //!< 15..06: reserved
    VOLATILE unsigned int   CCV     :  6; //!< 05..00: Cycle Count Value
  } bits;                                 //!< Bit representation
} M_CAN_TTCPT_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for register TTCSM - TT Cycle Sync Mark */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   CSM     : 16; //!< 15..00: Cycle Sync Mark
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
  } bits;                                 //!< Bit representation
} M_CAN_TTCSM_union;

#elif __BIG_ENDIAN
/* Type Definition for register TTCSM - TT Cycle Sync Mark */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
    VOLATILE unsigned int   res0    : 16; //!< 31..16: reserved
    VOLATILE unsigned int   CSM     : 16; //!< 15..00: Cycle Sync Mark
  } bits;                                 //!< Bit representation
} M_CAN_TTCSM_union;
#endif


// === Type Definition for RAM ELEMENTSs =====================================

#if __LITTLE_ENDIAN
/* Type Definition for RAM Element - Standard Message ID Filter */
typedef union {
    VOLATILE unsigned int value;            //!< Integer representation
    VOLATILE struct {
      VOLATILE unsigned int   SFID2   : 11; //!< 10..00: Standard Filter ID 2
      VOLATILE unsigned int   res0    :  5; //!< 15..11: reserved
      VOLATILE unsigned int   SFID1   : 11; //!< 26..16: Standard Filter ID 1
      VOLATILE unsigned int   SFEC    :  3; //!< 29..27: Standard Filter Element Configuration
      VOLATILE unsigned int   SFT     :  2; //!< 31..30: Standard Filter Type
  } bits;                                   //!< Bit representation
} M_CAN_STD_ID_FILT_ELEMENT_union;

#elif __BIG_ENDIAN
/* Type Definition for RAM Element - Standard Message ID Filter */
typedef union {
  VOLATILE unsigned int value;            //!< Integer representation
  VOLATILE struct {
	    VOLATILE unsigned int   SFT     :  2; //!< 31..30: Standard Filter Type
	    VOLATILE unsigned int   SFEC    :  3; //!< 29..27: Standard Filter Element Configuration
	    VOLATILE unsigned int   SFID1   : 11; //!< 26..16: Standard Filter ID 1
	    VOLATILE unsigned int   res0    :  5; //!< 15..11: reserved
	    VOLATILE unsigned int   SFID2   : 11; //!< 10..00: Standard Filter ID 2
	} bits;                                   //!< Bit representation
} M_CAN_STD_ID_FILT_ELEMENT_union;
#endif

/* Standard ID Filter Element Type */
typedef enum {
  STD_FILTER_TYPE_RANGE     = 0x0,
  STD_FILTER_TYPE_DUAL      = 0x1,
  STD_FILTER_TYPE_CLASSIC   = 0x2,
  STD_FILTER_TYPE_DISABLE   = 0x3
} SFT_Standard_Filter_Type_enum;

/* Filter Element Configuration - Can be used for SFEC(Standard Id filter configuration) and EFEC(Extended Id filter configuration) */
typedef enum {
  FILTER_ELEMENT_DISBALBE       = 0x0,
  FILTER_ELEMENT_STORE_IN_FIFO0 = 0x1,
  FILTER_ELEMENT_STORE_IN_FIFO1 = 0x2,
  FILTER_ELEMENT_REJECT_ID      = 0x3,
  FILTER_ELEMENT_SET_PRIORITY   = 0x4,
  FILTER_ELEMENT_SET_PRIORITY_AND_STORE_IN_FIFO0 = 0x5,
  FILTER_ELEMENT_SET_PRIORITY_AND_STORE_IN_FIFO1 = 0x6,
  FILTER_ELEMENT_STORE_IN_RX_BUFFER_OR_DEBUG_MSG = 0x7
} Filter_Configuration_enum;

#if __LITTLE_ENDIAN
/* Type Definition for RAM Element - Extended Message ID Filter Element - Word 1*/
typedef union {
	VOLATILE unsigned int value;              //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   EFID1   : 29; //!< 28..00: Extended Filter ID 1
		VOLATILE unsigned int   EFEC    :  3; //!< 31..29: Extended Filter Element Configuration
	} bits;                                   //!< Bit representation
} M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_1_union;

#elif __BIG_ENDIAN
/* Type Definition for RAM Element - Extended Message ID Filter Element - Word 1*/
typedef union {
	VOLATILE unsigned int value;            //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   EFEC    :  3; //!< 31..29: Extended Filter Element Configuration
		VOLATILE unsigned int   EFID1   : 29; //!< 28..00: Extended Filter ID 1
	} bits;                                   //!< Bit representation
} M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_1_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for RAM Element - Extended Message ID Filter Element - Word 2*/
typedef union {
	VOLATILE unsigned int value;              //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   EFID2   : 29; //!< 28..00: Extended Filter ID 2
		VOLATILE unsigned int   res0    :  1; //!< 29    : reserved
		VOLATILE unsigned int   EFT     :  2; //!< 31..30: Extended Filter Type
	} bits;                                   //!< Bit representation
} M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_2_union;

#elif __BIG_ENDIAN
/* Type Definition for RAM Element - Extended Message ID Filter Element - Word 2*/
typedef union {
	VOLATILE unsigned int value;            //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   EFT     :  2; //!< 31..30: Extended Filter Type
		VOLATILE unsigned int   res0    :  1; //!< 29    : reserved
		VOLATILE unsigned int   EFID2   : 29; //!< 28..00: Extended Filter ID 2
	} bits;                                   //!< Bit representation
} M_CAN_EXTENDED_ID_FILT_ELEMENT_WORD_2_union;
#endif

/* Extended ID Filter Element Type */
typedef enum {
  EXTD_FILTER_TYPE_RANGE                         = 0x0,
  EXTD_FILTER_TYPE_DUAL                          = 0x1,
  EXTD_FILTER_TYPE_CLASSIC                       = 0x2,
  EXTD_FILTER_TYPE_RANGE_XIDAM_MASK_NOT_APPLIED  = 0x3
} EFT_Extended_Filter_Type_enum;

/* Rx Buffer and FIFO Element Word 1*/
// Word T0
#define RX_BUF_ESI          BIT(31)
#define RX_BUF_XTD          BIT(30)
#define RX_BUF_RTR          BIT(29)
#define RX_BUF_STDID_SHIFT  18
#define RX_BUF_STDID_MASK   (0x7FF << RX_BUF_STDID_SHIFT)
#define RX_BUF_EXTID_MASK   0x1FFFFFFF
// Word T1
#define RX_BUF_ANMF         BIT(31)
#define RX_BUF_FIDX_SHIFT   24
#define RX_BUF_FIDX_MASK    (0x7F << RX_BUF_FIDX_SHIFT)
#define RX_BUF_FDF          BIT(21)
#define RX_BUF_BRS          BIT(20)
#define RX_BUF_DLC_SHIFT    16
#define RX_BUF_DLC_MASK     (0xF << RX_BUF_DLC_SHIFT)
#define RX_BUF_RXTS_MASK    0xFFFF

/* Tx Buffer Element (in Msg. RAM) */
// Word T0
#define TX_BUF_ESI          BIT(31)    // Error State Indicator (is transmitted with this value)
#define TX_BUF_XTD          BIT(30)    // Extended Identifier (0= standard ID, 1= extended ID)
#define TX_BUF_RTR          BIT(29)    // Remote Transmission Request (0= transmit data frame, 1= transmitt remote frame)
#define TX_BUF_STDID_SHIFT  18
#define TX_BUF_STDID_MASK   (0x7FF << TX_BUF_STDID_SHIFT)
#define TX_BUF_EXTID_MASK   0x1FFFFFFF
// Word T1
#define TX_BUF_MM_SHIFT     24           // Message Marker
#define TX_BUF_MM_MASK      (0xFF << TX_BUF_MM_SHIFT)
#define TX_BUF_EFC          BIT(23)        // Event FIFO control (0= store TX events, 1= don't)
#define TX_BUF_FDF          BIT(21)
#define TX_BUF_BRS          BIT(20)
#define TX_BUF_DLC_SHIFT    16
#define TX_BUF_DLC_MASK     (0xF << TX_BUF_DLC_SHIFT)

// TX Event FIFO Element
#if __LITTLE_ENDIAN
/* Type Definition for RAM Element - TX Event FIFO Element - Word 1*/
typedef union {
	VOLATILE unsigned int value;              //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   ID      : 29; //!< 28..00: Identifier
		VOLATILE unsigned int   RTR     :  1; //!< 29    : Remote Transmission Request
		VOLATILE unsigned int   XTD     :  1; //!< 30    : Extended Identifier
		VOLATILE unsigned int   ESI     :  1; //!< 31    : Error State Indicator
	} bits;                                   //!< Bit representation
} M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_1_union;

#elif __BIG_ENDIAN
/* Type Definition for RAM Element - TX Event FIFO Element - Word 1*/
typedef union {
	VOLATILE unsigned int value;            //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   ESI     :  1; //!< 31    : Error State Indicator
		VOLATILE unsigned int   XTD     :  1; //!< 30    : Extended Identifier
		VOLATILE unsigned int   RTR     :  1; //!< 29    : Remote Transmission Request
		VOLATILE unsigned int   ID      : 29; //!< 28..00: Identifier
	} bits;                                   //!< Bit representation
} M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_1_union;
#endif

#if __LITTLE_ENDIAN
/* Type Definition for RAM Element - TX Event FIFO Element - Word 2*/
typedef union {
	VOLATILE unsigned int value;              //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   TXTS   : 16; //!< 15..00: Tx Timestamp
		VOLATILE unsigned int   DLC    :  4; //!< 19..16: Data Length Code
		VOLATILE unsigned int   BRS    :  1; //!< 20    : Bit Rate Switch
		VOLATILE unsigned int   FDF    :  1; //!< 21    : FD Format
		VOLATILE unsigned int   ET     :  2; //!< 23..22: Event Type
		VOLATILE unsigned int   MM     :  8; //!< 31..24: Message marker
	} bits;                                   //!< Bit representation
} M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_2_union;

#elif __BIG_ENDIAN
/* Type Definition for RAM Element - TX Event FIFO Element - Word 2*/
typedef union {
	VOLATILE unsigned int value;            //!< Integer representation
	VOLATILE struct {
		VOLATILE unsigned int   MM     :  8; //!< 31..24: Message marker
		VOLATILE unsigned int   ET     :  2; //!< 23..22: Event Type
		VOLATILE unsigned int   FDF    :  1; //!< 21    : FD Format
		VOLATILE unsigned int   BRS    :  1; //!< 20    : Bit Rate Switch
		VOLATILE unsigned int   DLC    :  4; //!< 19..16: Data Length Code
		VOLATILE unsigned int   TXTS   : 16; //!< 15..00: Tx Timestamp
	} bits;                                   //!< Bit representation
} M_CAN_TX_EVENT_FIFO_ELEMENT_WORD_2_union;
#endif

// TX Event FIFO Element Type
typedef enum {
	reserved = 0,
	tx_event = 1,
	transmission_in_spite_of_cancellation = 2
}tx_event_fifo_elem_event_type_enum;

/* Tx Event FIFO Element (in Msg. RAM) */
// Word T0
#define TX_EVENT_FIFO_ESI			BIT(31)
#define TX_EVENT_FIFO_XTD			BIT(30)
#define TX_EVENT_FIFO_RTR			BIT(29)
#define TX_EVENT_FIFO_STDID_SHIFT   18
#define TX_EVENT_FIFO_STDID_MASK    (0x7FF << TX_EVENT_FIFO_STDID_SHIFT)
#define TX_EVENT_FIFO_EXTID_MASK    0x1FFFFFFF
// Word T1
#define TX_EVENT_FIFO_MM_SHIFT		24
#define TX_EVENT_FIFO_MM_MASK		(0xFF << TX_EVENT_FIFO_MM_SHIFT)
#define TX_EVENT_FIFO_ET_SHIFT		22
#define TX_EVENT_FIFO_ET_MASK		(0x3 << TX_EVENT_FIFO_ET_SHIFT)
#define TX_EVENT_FIFO_FDF			BIT(21)
#define TX_EVENT_FIFO_BRS			BIT(22)
#define TX_EVENT_FIFO_DLC_SHIFT 	16
#define TX_EVENT_FIFO_DLC_MASK  	(0xF << TX_EVENT_FIFO_DLC_SHIFT)
#define TX_EVENT_FIFO_TXTS_MASK		0xFF

#endif /*M_CAN_REGDEF_H_*/
