// Tratto da
// http://www.bosch-semiconductors.com/ip-modules/can-ip-modules/m-can/
#include "../bosch/global_includes.h"
#include "../mcan.h"
#include "bsp.h"
#include "tcan4550.h"

// Costanti
#define SID_FILTER_DIM      4
#define XID_FILTER_DIM      8
#define RX_FIFO_H_DIM       8
#define TX_EVN_H_DIM        8
#define TX_FIFO_H_DIM       8

// Se mancano ...
#ifndef TCAN_TXRX_MAX_DIM
#   define TCAN_TXRX_MAX_DIM    64
#endif

#ifndef TCAN_SID_ELEMS
#   define TCAN_SID_ELEMS       2
#endif

#ifndef TCAN_XID_ELEMS
#   define TCAN_XID_ELEMS       1
#endif

#ifndef TCAN_RX0_ELEMS
#   define TCAN_RX0_ELEMS       4
#endif

#ifndef TCAN_RX1_ELEMS
#   define TCAN_RX1_ELEMS       5
#endif

#ifndef TCAN_TXEVN_ELEMS
#   define TCAN_TXEVN_ELEMS     3
#endif

#ifndef TCAN_TX_ELEMS
#   define TCAN_TX_ELEMS        10
#endif

// Allocazione MRAM
#define SID_START       0
#define XID_START       ( SID_START + (TCAN_SID_ELEMS * SID_FILTER_DIM) )
#define RXF0_START      ( XID_START + (TCAN_XID_ELEMS * XID_FILTER_DIM) )
#define RXF1_START      ( RXF0_START   \
                          + ( TCAN_RX0_ELEMS \
                              * (RX_FIFO_H_DIM + TCAN_TXRX_MAX_DIM) ) )
#define TXEVN_START     ( RXF1_START   \
                          + ( TCAN_RX1_ELEMS \
                              * (RX_FIFO_H_DIM + TCAN_TXRX_MAX_DIM) ) )
#define TX_START        ( TXEVN_START + (TCAN_TXEVN_ELEMS * TX_EVN_H_DIM) )
#define FINE_RAM        ( TX_START   \
                          + ( TCAN_TX_ELEMS \
                              * (TX_FIFO_H_DIM + TCAN_TXRX_MAX_DIM) ) )

// Interruzioni
// Divido fra le due linee
// Trasmissione
#define MCAN_INT_1      (IR_TEFL_TX_EVENT_FIFO_EVENT_LOST               \
                         | IR_TEFF_TX_EVENT_FIFO_FULL                   \
                         | IR_TEFW_TX_EVENT_FIFO_WATERMARK_REACHED      \
                         | IR_TEFN_TX_EVENT_FIFO_NEW_ENTRY              \
                         | IR_TFE_TX_FIFO_EMPTY                         \
                         | IR_TCF_TRANSMISSION_CANCELLATION_FINISHED    \
                         | IR_TC_TRANSMISSION_COMPLETED)
// Resto
#define MCAN_INT_0      (INTERRUPT_ALL_SIGNALS & ~MCAN_INT_1)

// TCAN4550
// Register 0x0000 through 0x000C are Device ID and SPI Registers
// Register 0x0800 through 0x083C are device configuration registers and
// Interrupt Flags
// Register 0x1000 through 0x10FC are for M_CAN
// Register 0x8000 through 0x87FF is for MRAM

#define TCAN_OSC          40000000
#define TCAN_MRAM_REG     0x8000
#define TCAN_MRAM_DIM     2048

#define TCAN_STATUS_REG     0x000C
// Modes of Operation and Pin Configuration Registers
#define TCAN_MOPC_REG       0x0800

#define MCAN_CCCR_REG       (REG_BASE + ADR_M_CAN_CCCR)
// Nominal Bit Timing & Prescaler Register (address = h101C)
#define MCAN_NBTP_REG       (REG_BASE + ADR_M_CAN_NBTP)
// Data Bit Timing & Prescaler
#define MCAN_DBTP_REG       (REG_BASE + ADR_M_CAN_DBTP)
// Transmitter Delay Compensation Register
#define MCAN_TDC_REG        (REG_BASE + ADR_M_CAN_TDCR)
// Standard ID Filter Configuration
#define MCAN_SIDFC_REG      (REG_BASE + ADR_M_CAN_SIDFC)
// Extended ID Filter Configuration
#define MCAN_XIDFC_REG      (REG_BASE + ADR_M_CAN_XIDFC)
// Rx FIFO 0 Configuration
#define MCAN_RXF0C_REG      (REG_BASE + ADR_M_CAN_RXF0C)
// Rx FIFO 1 Configuration
#define MCAN_RXF1C_REG      (REG_BASE + ADR_M_CAN_RXF1C)
// Rx Buffer Configuration
#define MCAN_RXBC_REG       (REG_BASE + ADR_M_CAN_RXBC)
// Rx Buffer/FIFO Element Size Configuration
#define MCAN_RXESC_REG      (REG_BASE + ADR_M_CAN_RXESC)
// Tx Event FIFO Configuration
#define MCAN_TXEFC_REG      (REG_BASE + ADR_M_CAN_TXEFC)
// Tx Buffer Configuration
#define MCAN_TXBC_REG       (REG_BASE + ADR_M_CAN_TXBC)
// Tx Buffer Element Size Configuration
#define MCAN_TXESC_REG      (REG_BASE + ADR_M_CAN_TXESC)
// Interrupt Enable
#define MCAN_IE_REG         (REG_BASE + ADR_M_CAN_IE)
// Interrupt Line Select
#define MCAN_ILS_REG        (REG_BASE + ADR_M_CAN_ILS)
// Interrupt Register
#define TCAN_IF_REG         0x0820
// MCAN Interrupts
#define TCAN_MCANI_REG      0x0824
// Interrupt Enables
#define TCAN_IE_REG         0x0830
// Interrupt Register
#define MCAN_IF_REG         (REG_BASE + ADR_M_CAN_IR)
// Interrupt Enable
#define MCAN_IE_REG         (REG_BASE + ADR_M_CAN_IE)
// Tx FIFO/Queue Status
#define MCAN_TXFQS_REG      (REG_BASE + ADR_M_CAN_TXFQS)
// Tx Buffer Add Request
#define MCAN_TXBAR_REG      (REG_BASE + ADR_M_CAN_TXBAR)

static UN_MCAN * un_mcan = NULL ;

static int dlc_da_dim(uint16_t dim)
{
    if ( dim <= 8 ) {
        return dim ;
    }
    else {
        switch ( dim ) {
        case 12:
            return 9 ;
        case 16:
            return 10 ;
        case 20:
            return 11 ;
        case 24:
            return 12 ;
        case 32:
            return 13 ;
        case 48:
            return 14 ;
        case 64:
            return 15 ;
        default:
            DBG_ERR ;
            if ( dim < 12 ) {
                return 9 ;
            }
            if ( dim < 16 ) {
                return 10 ;
            }
            if ( dim < 20 ) {
                return 11 ;
            }
            if ( dim < 24 ) {
                return 12 ;
            }
            if ( dim < 32 ) {
                return 13 ;
            }
            if ( dim < 48 ) {
                return 14 ;
            }
            return 15 ;
        }
    }
}

static uint32_t reg_leggi(uint16_t reg)
{
    uint32_t * pR = TCAN_reg_leggi(un_mcan->spi_txrx, 1, reg) ;
    if ( pR ) {
        uint32_t val = *pR ;
#ifdef STAMPA_REGISTRI
        DBG_PRINTF("reg[%04X] -> %08X", reg, val) ;
#endif
        return val ;
    }
    else {
        DBG_ERR ;
        return 0 ;
    }
}

static void reg_scrivi(
    uint16_t reg,
    uint32_t val)
{
    if ( !TCAN_reg_scrivi(un_mcan->spi_tx, 1, reg, &val) ) {
        DBG_ERR ;
    }
    else {
#ifdef STAMPA_REGISTRI
        DBG_PRINTF("reg[%04X] <- %08X", reg, val) ;
#endif
    }
}

typedef struct {
    uint32_t
        TEST_MODE_CONFIG : 1,
        SWE_DIS : 1,
        DEVICE_RESET : 1,
        WD_EN : 1,
        rsvd4 : 2,
        MODE_SEL : 2,
        nWKRQ_CONFIG : 1,
        INH_DIS : 1,
        GPIO1_GPO_CONFIG : 2,
        rsvd12 : 1,
        FAIL_SAFE_EN : 1,
        GPIO1_CONFIG : 2,
        WD_ACTION : 2,
        WD_BIT_SET : 1,
        nWKRQ_VOLTAGE : 1,
        rsvd20 : 1,
        TEST_MODE_EN : 1,
        GPO2_CONFIG : 2,
        rsvd24 : 3,
        CLK_REF : 1,
        WD_TIMER : 2,
        WAKE_CONFIG : 2 ;
} REG_MOPC ;

// Copiata togliendo i volatili
typedef struct {
    uint32_t INIT : 1 ;
    uint32_t CCE  : 1 ;
    uint32_t ASM  : 1 ;
    uint32_t CSA  : 1 ;
    uint32_t CSR  : 1 ;
    uint32_t MON  : 1 ;
    uint32_t DAR  : 1 ;
    uint32_t TEST : 1 ;
    uint32_t FDOE : 1 ;
    uint32_t BRSE : 1 ;
    uint32_t res0 : 2 ;
    uint32_t PXHD : 1 ;
    uint32_t EFBI : 1 ;
    uint32_t TXP  : 1 ;
    uint32_t NISO : 1 ;
    uint32_t res1 : 16 ;
} REG_CCCR ;

typedef struct {
    uint32_t TFFL    :  6 ;
    uint32_t res0    :  2 ;
    uint32_t TFGI    :  5 ;
    uint32_t res1    :  3 ;
    uint32_t TFQPI   :  5 ;
    uint32_t TFQF    :  1 ;
    uint32_t res2    : 10 ;
} REG_TXFQS ;

can_global_struct global = {
    .board_id = 0,

    .can = {
        // TCAN3
        {
            // TRUE = node enabled/aktive,
            // FALSE = node disabled/does  not TX or RX frames
            .ena = true,

            // base address of this CAN module
            .base = 0,

            // local  CAN-Node identifier, e.g. 0 or 2
            .id = 0,

            // global CAN-Node identifier, used to separate Nodes in a
            // multi-board network
            // uint8_t  id_global;

            // Absolute Byte Base Address of this M_CAN
            .mram_base = TCAN_MRAM_REG,

            // Size of the Message RAM: number of words
            .mram_size_words = TCAN_MRAM_DIM / sizeof(uint32_t),

            // Absolute Byte Start Addresses for Element Types in Message RAM
            .mram_sa = {
                .SIDFC_FLSSA = TCAN_MRAM_REG + SID_START,
                .XIDFC_FLESA = TCAN_MRAM_REG + XID_START,
                .RXF0C_F0SA = TCAN_MRAM_REG + RXF0_START,
                .RXF1C_F1SA = TCAN_MRAM_REG + RXF1_START,
                .RXBC_RBSA = TCAN_MRAM_REG + 0,
                .TXEFC_EFSA = TCAN_MRAM_REG + TXEVN_START,
                .TXBC_TBSA = TCAN_MRAM_REG + TX_START,
            },

            // Size of Elements in Message RAM (RX Elem. in FIFO0, in FIFO1, TX
            // Buffer) given in words
            .elem_size_word = CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME
                              / sizeof(uint32_t),

            // Statistics, any kind
            // can_statistics_struct stat;

            // Bit Timing Configuration
            // bt_config_canfd_struct bt_config;

            // TX Buffer Configuration
            // tx_buff_config_struct tx_config;

            // FALSE: M_CAN Node, TRUE: M_TTCAN Node
            .is_m_ttcan_node = false,

            // internal_test_struct internal_test;

            // MZ
            .autotx = FALSE,
            .lback_abil = TRUE,
            .lback_intrnl = TRUE
        },
        // TCAN4
        {
            .id = 1,
            .ena = true,

            .base = 0,

            .mram_base = TCAN_MRAM_REG,

            .mram_size_words = TCAN_MRAM_DIM / sizeof(uint32_t),

            .mram_sa = {
                .SIDFC_FLSSA = TCAN_MRAM_REG + SID_START,
                .XIDFC_FLESA = TCAN_MRAM_REG + XID_START,
                .RXF0C_F0SA = TCAN_MRAM_REG + RXF0_START,
                .RXF1C_F1SA = TCAN_MRAM_REG + RXF1_START,
                .RXBC_RBSA = TCAN_MRAM_REG + 0,
                .TXEFC_EFSA = TCAN_MRAM_REG + TXEVN_START,
                .TXBC_TBSA = TCAN_MRAM_REG + TX_START,
            },

            .elem_size_word = CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME
                              / sizeof(uint32_t),

            .is_m_ttcan_node = false,

            // MZ
            .autotx = FALSE,
            .lback_abil = TRUE,
            .lback_intrnl = TRUE
        }
    }
} ;

can_struct * get_can_struct(void)
{
    return global.can + un_mcan->indice ;
}

static bt_config_struct cfg_nmnl = {
    // Variabili
    // baudrate prescaler
    //uint16_t brp;
    // phase1 segment
    //uint16_t phase_seg1;
    // phase2 segment
    //uint16_t phase_seg2;
    // (re) synchronization jumpwidth
    //uint16_t sjw;

    // propagation segment (Inutile)
    .prop_seg = 0,

    // Come l'esempio
    // transceiver delay compensation (1:yes, 0:no)
    .tdc = 0,
    // transceiver delay compensation offset
    //uint16_t tdc_offset;
    // transceiver delay compensation filter window length
    //uint16_t tdc_filter_window;
} ;

static bt_config_struct cfg_data = {
    .prop_seg = 0,
    .tdc = 0,
} ;

static void prispolo(UN_MCAN * pC)
{
    pC->reset_pin(false) ; //PIN_BASS(FD_CAN_RESET) ;
    pC->attesa_us(100) ;
    pC->reset_pin(true) ; //PIN_ALTO(FD_CAN_RESET) ;
    // PULSE_WIDTH >= 30 us
    pC->attesa_us(50) ;
    pC->reset_pin(false) ; //PIN_BASS(FD_CAN_RESET) ;

    // >= 700 us
    pC->attesa_us(800) ;
}

// SLLSF91 – DECEMBER 2018 / 8.5 Programming / pag 41
// To avoid ECC errors right after initialization the MRAM should be zeroed out
// during the initialization, power up, power on reset and wake events

static bool azzera_mram(void)
{
    const uint16_t MRAM_NUM_REG = TCAN_MRAM_DIM / sizeof(uint32_t) ;
    uint16_t primo = 0 ;

    while ( primo < MRAM_NUM_REG ) {
        uint16_t nr = MINI(MRAM_NUM_REG - primo, TCAN_MAX_REG) ;
        if ( !TCAN_reg_scrivi(un_mcan->spi_tx, nr, TCAN_MRAM_REG + primo
                              * sizeof(uint32_t),
                              NULL) ) {
            DBG_ERR ;
            return false ;
        }
        primo += nr ;
    }

    DBG_PUTS("MRAM azzerata") ;
    return true ;
}

#ifdef DBG_ABIL

#define TCAN_INTERNAL_READ_ERROR      (1 << 29) // W1C 0  Internal read received
                                                // an error response
#define TCAN_INTERNAL_WRITE_ERROR     (1 << 28) // W1C 0  Internal write
                                                // received an error response
#define TCAN_INTERNAL_ERROR_LOG_WRITE (1 << 27) // W1C 0  Entry written to the
                                                // Internal error log
#define TCAN_READ_FIFO_UNDERFLOW      (1 << 26) // W1C 0  Read FIFO underflow
                                                // after 1 or more read data
                                                // words returned
#define TCAN_READ_FIFO_EMPTY          (1 << 25) // W1C 0  Read FIFO empty for
                                                // first read data word to
                                                // return
#define TCAN_WRITE_FIFO_OVERFLOW      (1 << 24) // W1C 0  Write/command FIFO
                                                // overflow
#define TCAN_SPI_END_ERROR            (1 << 21) // W1C 0  SPI transfer did not
                                                // end on a byte boundary
#define TCAN_INVALID_COMMAND          (1 << 20) // W1C 0  Invalid SPI command
                                                // received
#define TCAN_WRITE_OVERFLOW           (1 << 19) // W1C 0  SPI write sequence had
                                                // continue requests after the
                                                // data transfer was completed
#define TCAN_WRITE_UNDERFLOW          (1 << 18) // W1C 0  SPI write sequence
                                                // ended with less data
                                                // transferred then requested
#define TCAN_READ_OVERFLOW            (1 << 17) // W1C 0  SPI read sequence had
                                                // continue requests after the
                                                // data transfer was completed
#define TCAN_READ_UNDERFLOW           (1 << 16) // W1C 0  SPI read sequence
                                                // ended with less data
                                                // transferred then requested
#define TCAN_WRITE_FIFO_AVAILABLE     (1 << 5)  // RO  0  write fifo empty
                                                // entries is greater than or
                                                // equal to the
                                                // write_fifo_threshold
#define TCAN_READ_FIFO_AVAILABLE      (1 << 4)  // RO  0  Read fifo entries is
                                                // greater than or equal to the
                                                // read_fifo_threshold
#define TCAN_INTERNAL_ACCESS_ACTIVE   (1 << 3)  // RO  U  Internal Multiple
                                                // transfer mode access in
                                                // progress
#define TCAN_INTERNAL_ERROR_INTERRUPT (1 << 2)  // RO  0  Unmasked Internal
                                                // error set
#define TCAN_SPI_ERROR_INTERRUPT      (1 << 1)  // RO  0  Unmasked SPIerror set
#define TCAN_INTERRUPT                (1 << 0)  // RO  U  Value of interrupt
                                                // input level (active high)

typedef struct {
    uint32_t msk ;
    const char * msg ;
} S_DBG_REG ;

static const S_DBG_REG dbg_stt_reg[] = {
    { .msk = TCAN_INTERNAL_READ_ERROR, .msg =
          "Internal read received an error response" },
    { .msk = TCAN_INTERNAL_WRITE_ERROR, .msg =
          "Internal write received an error response" },
    { .msk = TCAN_INTERNAL_ERROR_LOG_WRITE, .msg =
          "Entry written to the Internal error log" },
    { .msk = TCAN_READ_FIFO_UNDERFLOW, .msg =
          "Read FIFO underflow after 1 or more read data words returned" },
    { .msk = TCAN_READ_FIFO_EMPTY, .msg =
          "Read FIFO empty for first read data word to return" },
    { .msk = TCAN_WRITE_FIFO_OVERFLOW, .msg = "Write/command FIFO overflow" },
    { .msk = TCAN_SPI_END_ERROR, .msg =
          "SPI transfer did not end on a byte boundary" },
    { .msk = TCAN_INVALID_COMMAND, .msg = "Invalid SPI command received" },
    { .msk = TCAN_WRITE_OVERFLOW, .msg =
          "SPI write sequence had continue requests after the data transfer was completed" },
    { .msk = TCAN_WRITE_UNDERFLOW, .msg =
          "SPI write sequence ended with less data transferred then requested" },
    { .msk = TCAN_READ_OVERFLOW, .msg =
          "SPI read sequence had continue requests after the data transfer was completed" },
    { .msk = TCAN_READ_UNDERFLOW, .msg =
          "SPI read sequence ended with less data transferred then requested" },
    { .msk = TCAN_WRITE_FIFO_AVAILABLE, .msg =
          "write fifo empty entries is greater than or equal to the write_fifo_threshold" },
    { .msk = TCAN_READ_FIFO_AVAILABLE, .msg =
          "Read fifo entries is greater than or equal to the read_fifo_threshold" },
    { .msk = TCAN_INTERNAL_ACCESS_ACTIVE, .msg =
          "Internal Multiple transfer mode access in progress" },
    { .msk = TCAN_INTERNAL_ERROR_INTERRUPT, .msg =
          "Unmasked Internal error set" },
    { .msk = TCAN_SPI_ERROR_INTERRUPT, .msg = "Unmasked SPIerror set" },
    { .msk = TCAN_INTERRUPT, .msg =
          "Value of interrupt input level (active high)" },
    { .msk = 0 }
} ;

static void stampa_reg(
    const char * tit,
    uint32_t val,
    const S_DBG_REG * dbg)
{
    DBG_PRINTF("%s = %08X", tit, val) ;

    while ( true ) {
        if ( 0 == dbg->msk ) {
            break ;
        }

        if ( val & dbg->msk ) {
            DBG_PRINTF("\t%s", dbg->msg) ;
        }

        dbg += 1 ;
    }
}

static void status_reg(UN_MCAN * pC)
{
    uint32_t * pS = TCAN_reg_leggi(pC->spi_txrx, 1, TCAN_STATUS_REG) ;

    stampa_reg("status", *pS, dbg_stt_reg) ;
}

#else

static void status_reg(UN_MCAN * _)
{
    INUTILE(_) ;
}

#endif

bool MCAN_iniz(
    UN_MCAN * pC,
    const MCAN_CFG * nominal,
    const MCAN_CFG * data)
{
    bool esito = false ;

    static_assert(FINE_RAM <= TCAN_MRAM_DIM, "OKKIO alla mram") ;

    cfg_nmnl.phase_seg1 = nominal->bs1 ;
    cfg_nmnl.phase_seg2 = nominal->bs2 ;
    cfg_nmnl.sjw = nominal->sjw ;
    cfg_nmnl.brp = TCAN_OSC / nominal->freqnt ;

    // Le funzioni bosch sottraggono 1 a tutto!
    ++cfg_nmnl.phase_seg1 ;
    ++cfg_nmnl.phase_seg2 ;
    ++cfg_nmnl.sjw ;

    if ( data ) {
        cfg_data.phase_seg1 = data->bs1 ;
        cfg_data.phase_seg2 = data->bs2 ;
        cfg_data.sjw = data->sjw ;
        cfg_data.brp = TCAN_OSC / data->freqnt ;

        // Le funzioni bosch sottraggono 1 a tutto!
        ++cfg_data.phase_seg1 ;
        ++cfg_data.phase_seg2 ;
        ++cfg_data.sjw ;

        pC->fd = true ;
    }
    else {
        cfg_data = cfg_nmnl ;

        pC->fd = false ;
    }

    un_mcan = pC ;
    if ( TCAN_iniz() ) {
        DBG_PRINTF("++++ tcan iniz %d", pC->indice) ;

        prispolo(pC) ;

        status_reg(pC) ;

        // Test M_CAN/M_TTCAN Registers
        //m_can_register_test(global.can[pC->indice], FALSE);

        do {
            if ( !azzera_mram() ) {
                break ;
            }

            union {
                uint32_t u ;
                REG_MOPC mopc ;
            } tmp ;

            //Ensure that TCAN in is standby mode...
            tmp.u = reg_leggi(TCAN_MOPC_REG) ;
            if ( tmp.mopc.MODE_SEL != 1 ) {
                DBG_ERR ;
                break ;
            }

            // MCAN_INT 0 interrupt (Active low)
            tmp.mopc.GPO2_CONFIG = 1 ;
            reg_scrivi(TCAN_MOPC_REG, tmp.u) ;
#ifndef NDEBUG
            // debug
            tmp.u = reg_leggi(TCAN_MOPC_REG) ;
#endif
            m_can_init_msg_ram_partitioning(&global.can[pC->indice]) ;
            // lungo se stampe abilitate
            //ram_check_reset_value(global.can[0].id, global.can[0].mram_base,
            // global.can[0].mram_size_words, M_CAN_RAM_WORD_WIDTH_IN_BYTE);

            m_can_set_config_change_enable(&global.can[pC->indice]) ;

            global.can[pC->indice].bt_config.fd_ena = pC->fd ? TRUE : FALSE ;
            global.can[pC->indice].bt_config.brs_ena = pC->fd ? TRUE : FALSE ;
            global.can[pC->indice].bt_config.nominal = cfg_nmnl ;
            global.can[pC->indice].bt_config.data = cfg_data ;

            global.can[pC->indice].lback_abil = pC->lback ? TRUE : FALSE ;
            global.can[pC->indice].lback_intrnl = pC->lb_intrn ? TRUE : FALSE ;

            m_can_set_bit_timing(&global.can[pC->indice]) ;

            m_can_interrupt_init(&global.can[pC->indice],
                                 MCAN_INT_0,
                                 MCAN_INT_1,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF) ;

            // Configure TX Buffers
            // Parameters:       M_CAN node    , FIFO_true_QUEUE_false, no. FIFO
            // elements  , no. ded Buffers, datafield size
            m_can_tx_buffer_init(&global.can[pC->indice],
                                 TRUE,
                                 MAX_TX_BUFFER_ELEMS,
                                 0,
                                 BYTE64) ;

            // set Global filter parameters in Rx Node to accept all messages in
            // Rx FIFO0
            m_can_global_filter_configuration(
                &global.can[pC->indice],
                ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0,
                ACCEPT_NON_MATCHING_FRAMES_IN_RX_FIFO0,
                TRUE,
                TRUE) ;

            /* RX FIFO Configuration
             * Parameters:     M_CAN node    , FIFO 0/1, FIFO_size          , watermark, element Size */
            m_can_rx_fifo_init(&global.can[pC->indice],
                               0,
                               MAX_RX_FIFO_0_ELEMS,
                               10,
                               BYTE64) ;

//            /* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
//          // reset CCE and INIT: M_CANs will participate on the CAN bus
#   if 0
            // con questo si entra in ciclo infinito
            m_can_reset_config_change_enable_and_reset_init(&global.can[pC->
                                                                        indice]) ;
#   else
            // con questo si passa in normal mode e si esce da INIT
            // SLLSF91 – DECEMBER 2018 - pag 52
            //     When the device is changing the device to normal mode a write
            // of 0 to CCCR.INIT is
            //     automatically issued and when changing from normal mode to
            // standby or sleep modes
            //     a write of 1 to CCCR.INIT is automatically issued.
            {
                union {
                    uint32_t u ;
                    REG_MOPC mopc ;
                } umopc ;

                umopc.u = reg_leggi(TCAN_MOPC_REG) ;

                umopc.mopc.MODE_SEL = 2 ;
                reg_scrivi(TCAN_MOPC_REG, umopc.u) ;
#       ifndef NDEBUG
                // debug
                umopc.u = reg_leggi(TCAN_MOPC_REG) ;
#       endif
            }
#   endif
            esito = true ;
        } while ( false ) ;

        status_reg(pC) ;

        DBG_PUTS("---- tcan iniz") ;
    }
    else {
        MCAN_fine(pC) ;
    }
    un_mcan = NULL ;

    return esito ;
}

void MCAN_fine(UN_MCAN * pC)
{
    pC->reset_pin(true) ;

//    un_mcan = pC ;
//    TCAN_fine(pC) ;
//    un_mcan = NULL ;
}

bool MCAN_rx(
    UN_MCAN * pC,
    MCAN_RX * pRx)
{
    un_mcan = pC ;
    bool esito = msg_list_is_empty(&global.can[pC->indice]) == FALSE ;
    if ( esito ) {
        can_msg_struct * msg = msg_list_get_head_msg(&global.can[pC->indice]) ;

        pRx->extended = extended_id == msg->idtype ;
        pRx->id = msg->id ;
        pRx->dim = convert_DLC_to_data_length(msg->dlc) ;
        if ( pRx->dim ) {
            memcpy(pRx->dati, msg->data, pRx->dim) ;
        }

        msg_list_remove_head_msg(&global.can[pC->indice]) ;
    }
    un_mcan = NULL ;

    return esito ;
}

bool MCAN_tx(
    UN_MCAN * pC,
    const MCAN_TX * mtx,
    const void * v,
    uint8_t dim)
{
    bool esito = false ;

    DBG_PUTS("++++ tcan tx") ;

    can_msg_struct msg = {
        // TRUE = Remote Frame, FALSE = Data Frame
        .remote = FALSE,

        .idtype = mtx->extended ? extended_id : standard_id,

        // ID (11) or IDext (29)
        .id = mtx->id,

        // FD Format (TRUE = FD Foramt)
        .fdf = pC->fd ? TRUE : FALSE,

        // Bit Rate Switch (TRUE = with Bit Rate Switch)
        .brs = pC->fd ? TRUE : FALSE,

        // Error State Indicator
        //boolean esi;

        // Data Length Code used in the frame on the bus: current CAN FD IP
        // impl. delivers 8 data bytes to the
        // software, but transmits up to 64 on the CAN bus
        // to calculate the data length of a frame based on DLC use the function
        // convert_DLC_to_length(*)
        // this should be removed, when CAN FD IP can deliver DLC-number of
        // bytes to the software
        .dlc = dlc_da_dim(dim),

        // uint8_t data[CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME];

        // Message Direction: RX or TX
        .direction = tx_dir,

        // Information regarding the reception of the frame
        //rx_info_struct rx_info;

        // Event FIFO Control (TRUE = Store TX Event FIFO element after
        // transmission)
        //boolean efc;

        // Message marker (will be copied to TX Event FIFO element)
        //uint8_t mm;
    } ;

    if ( v ) {
        const uint8_t mdim = MINI(dim, CAN_FD_MAX_NO_OF_DATABYTE_PER_FRAME) ;
        memcpy(msg.data, v, mdim) ;
        const uint8_t dlcdim = convert_DLC_to_data_length(msg.dlc) ;
        if ( mdim != dlcdim ) {
            const uint8_t val = dlcdim - mdim ;
            memset(msg.data + mdim, val, val) ;
        }
    }

    // Gia' fatti quando si inizializza
    /* ======== Step - 1: Configure the M_CANs ======== */
    // set CCE: enables configuration of M_CANs, disconnects M_CANs from CAN bus
//    m_can_set_config_change_enable(&global.can[0]);

//    /* ===== Configure Tx Node => M_CAN 0 ===== */
//    global.can[0].ena = TRUE; // enable node (software representation)
//
//    // define the bit timings for Tx node
//    global.can[0].bt_config.fd_ena  = mtx->fd;
//    global.can[0].bt_config.brs_ena = FALSE;
//    global.can[0].bt_config.nominal = cfg;
//    global.can[0].bt_config.data    = cfg;
//
//    // set the bit timings for Tx node
//    m_can_set_bit_timing(&global.can[0]);

//    // initialize the interrupt registers for Tx node
//    m_can_interrupt_init(&global.can[0],
//            INTERRUPT_ALL_SIGNALS & ~IR_TSW_TIMESTAMP_WRAPAROUND,       //
// Assign to INT_LINE_0: all except the TimeStampWrapArround
//            0x0,                    // Assign to INT_LINE_1: NO Interrupts
//            0xFFFFFFFF,             // Enable TX Buffer Transmission Interrupt
//            0xFFFFFFFF);            // Enable TX Buffer Cancellation Interrupt

//    // Configure TX Buffers:
//    // Parameters:       M_CAN node    , FIFO_true_QUEUE_false, no. FIFO
// elements  , no. ded Buffers, datafield size
//    m_can_tx_buffer_init(&global.can[0], TRUE                 , 16
//               ,  0             , BYTE64); // apply configuration in M_CAN

    /* ======== Step - 3: Connecting M_CANs to the CAN Bus ======== */
//    // reset CCE and INIT: M_CANs will participate on the CAN bus
//    m_can_reset_config_change_enable_and_reset_init(&global.can[0]);

    /* ======== STEP - 4: Transmit messages via Tx FIFO ======== */
    un_mcan = pC ;
    esito = 1 == m_can_tx_fifo_queue_msg_transmit(&global.can[pC->indice], &msg) ;
    un_mcan = NULL ;

    DBG_PUTS("---- tcan tx") ;

    return esito ;
}

void m_can_disable_interrupt(int can_id)
{
    INUTILE(can_id) ;
    HAL_NVIC_DisableIRQ(EXTI2_IRQn) ;
    HAL_NVIC_DisableIRQ(EXTI4_IRQn) ;
}

void m_can_enable_interrupt(int can_id)
{
    INUTILE(can_id) ;
    HAL_NVIC_EnableIRQ(EXTI2_IRQn) ;
    HAL_NVIC_EnableIRQ(EXTI4_IRQn) ;
}

uint32_t * questo_reg_leggi(
    uint16_t numreg,
    uint16_t primo)
{
    if ( un_mcan ) {
        return TCAN_reg_leggi(un_mcan->spi_txrx, numreg, primo) ;
    }

    return NULL ;
}

bool questo_reg_scrivi(
    uint16_t numreg,
    uint16_t primo,
    const uint32_t * val)
{
    if ( un_mcan ) {
        return TCAN_reg_scrivi(un_mcan->spi_tx, numreg, primo, val) ;
    }

    return false ;
}

void MCAN_isr(UN_MCAN * pC)
{
    DBG_PUTS("! tcan irq !") ;
    un_mcan = pC ;
#ifndef NDEBUG
    (void) reg_leggi(TCAN_IF_REG) ;
    (void) reg_leggi(TCAN_MCANI_REG) ;
    (void) reg_leggi(MCAN_IF_REG) ;
    (void) reg_leggi(TCAN_STATUS_REG) ;
#endif
    m_can_process_IRQ(global.can + pC->indice) ;
    un_mcan = NULL ;
}

void mcan_tx_fifo_empty_cb(void)
{
    DBG_FUN ;
}

void mcan_rx_fifo_msg_cb(void)
{
    DBG_FUN ;
}

uint32_t * MCAN_reg_leggi(
    UN_MCAN * pC,
    uint16_t a,
    uint16_t b)
{
    return TCAN_reg_leggi(pC->spi_txrx, a, b) ;
}

bool MCAN_reg_scrivi(
    UN_MCAN * pC,
    uint16_t a,
    uint16_t b,
    const uint32_t * c)
{
    return TCAN_reg_scrivi(pC->spi_tx, a, b, c) ;
}

#ifdef MRAM_OTTIMIZZATA

void mcan_copia_in_mram(
    uint32_t indir,
    const void * v,
    int dim)
{
    uint16_t numreg = dim >> 2 ;

    union {
        const void * v ;
        uint32_t u ;
    } u ;
    u.v = v ;
    assert( 0 == (u.u & 3) ) ;

    DBG_PRINTF("mram[%04X] <- [%d]", indir, numreg) ;

    CONTROLLA( TCAN_reg_scrivi(un_mcan->spi_tx, numreg, indir,
                               (const uint32_t *) v) ) ;
}

#endif
