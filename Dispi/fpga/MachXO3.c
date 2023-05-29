#define STAMPA_DBG
#include "utili.h"
#include "fpga.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"
#include "hash.h"

extern void fpga_programn(bool) ;


// Qui c'e' la roba
#define DIM_BIT     (60 * 1024)

__attribute__( ( section(".fpga") ) )
static uint8_t bit[DIM_BIT] ;

static uint32_t dimbit ;

// Temporanei
#define DIM_TX_RX       100

__attribute__( ( section(".fpga") ) )
static uint8_t tx[DIM_TX_RX] ;

__attribute__( ( section(".fpga") ) )
static uint8_t rx[DIM_TX_RX] ;

// Comandi
static const uint8_t IDCODE_PUB[] = {
    0xE0, 0, 0, 0
} ;
static const uint8_t ISC_ENABLE[] = {
    0xC6, 0, 0, 0
} ;
static const uint8_t ISC_ERASE[] = {
    0x0E, 0x01, 0, 0
} ;
static const uint8_t LSC_READ_STATUS[] = {
    0x3C, 0, 0, 0
} ;
static const uint8_t LSC_BITSTREAM_BURST[] = {
    0x7A, 0, 0, 0
} ;
static const uint8_t ISC_DISABLE[] = {
    0x26, 0, 0
} ;
static const uint8_t ISC_NOOP[] = {
    0xFF, 0xFF, 0xFF, 0xFF
} ;
// Inutili
//static const uint8_t LSC_REFRESH[] = {
//    0x79, 0, 0
//} ;
//static const uint8_t USERCODE[] = {
//    0xC0, 0, 0, 0
//} ;

#define SR_DONE       (1 << 8)
#define SR_BUSY       (1 << 12)
#define SR_FAILFLAG   (1 << 13)

// cfr Table 15.39. Device ID - FPGA-TN-02064-1.8
// static const uint32_t MACHXO3L_1300E_MG256 = 0xC12B3043 ;
// static const uint32_t MACHXO3L_2100 = 0x412B3043 ;
__WEAK bool fpga_id_valido(uint32_t did)
{
    INUTILE(did) ;
    return true ;
}

// sarebbe static, ma e' usata anche fuori per debug
void fpga_config(void)
{
    fpga_programn(false) ;
    HAL_Delay(1 + 1) ;
    fpga_programn(true) ;
}

// sarebbe static, ma e' usata anche fuori per debug
bool fpga_read(
    const uint8_t * cmd,
    size_t dimcmd,
    void * ris,
    size_t dim)
{
    bool esito = false ;
    uint16_t dimtx = 0 ;

    memcpy_(tx, cmd, dimcmd) ;
    dimtx += dimcmd ;

    memset_(tx + dimtx, 0, sizeof(tx) - dimtx) ;

    do {
        if ( !fpga_spi_tx_rx(tx, rx, dimtx + dim) ) {
            DBG_ERR ;
            break ;
        }

        memcpy_(ris, rx + dimtx, dim) ;

        esito = true ;
    } while ( false ) ;

    return esito ;
}

// sarebbe static, ma e' usata anche fuori per debug
bool fpga_cmd(
    const uint8_t * cmd,
    size_t dim)
{
    bool esito = false ;

    if ( !fpga_spi_tx(cmd, dim) ) {
        DBG_ERR ;
    }
    else {
        esito = true ;
    }

    return esito ;
}

static bool read_dev_id(uint32_t * pid)
{
    bool esito = false ;
    uint32_t did ;

    if ( fpga_read( IDCODE_PUB, sizeof(IDCODE_PUB), &did,
                    sizeof(did) ) ) {
        *pid = __REV(did) ;
        esito = true ;
    }

    return esito ;
}

static bool read_status(uint32_t * psr)
{
    bool esito = false ;
    uint32_t sr ;

    if ( fpga_read( LSC_READ_STATUS, sizeof(LSC_READ_STATUS), &sr,
                    sizeof(sr) ) ) {
        *psr = __REV(sr) ;
        esito = true ;
    }

    return esito ;
}

bool FPGA_copia(
    uint32_t pos,
    uint32_t dim,
    const void * srg)
{
    // I primi byte sono per il comando
    pos += sizeof(LSC_BITSTREAM_BURST) ;

    if ( pos + dim < DIM_BIT ) {
        memcpy_(bit + pos, srg, dim) ;
        dimbit = pos + dim ;
        return true ;
    }

    return false ;
}

bool FPGA_sha(
    uint32_t dim,
    void * mem)
{
    if ( dim > DIM_BIT ) {
        DBG_ERR ;
        return false ;
    }

    return HASH_calcola(bit + sizeof(LSC_BITSTREAM_BURST),
                        dim,
                        mem) ;
}

bool FPGA_config(void)
{
    bool esito = false ;

    // Usa:
    // FPGA-TN-02055-2.7
    // 9.4. MachXO3 Slave SPI/I2C SRAM Configuration Flow
    fpga_config() ;

    do {
        // Ho la roba?
        if ( 0 == dimbit ) {
            DBG_ERR ;
            break ;
        }
        else if ( dimbit > DIM_BIT ) {
            DBG_ERR ;
            break ;
        }

        // Start
        // Check Device ID? (optional)
        uint32_t did ;
        if ( !read_dev_id(&did) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("device id %08X", did) ;

        // ID Match?
        if ( fpga_id_valido(did) ) {
            // Ok
        }
        else {
            DBG_ERR ;
            break ;
        }

        // --------------- 1 ---------------
        // Transmit REFRESH Command to reset the device
        // gia' fatto da fpga_config()
//        if ( !fpga_cmd( LSC_REFRESH, sizeof(LSC_REFRESH) ) ) {
//            DBG_ERR ;
//            break ;
//        }

        // Transmit ENABLE Configuration Interface (Offline Mode) Command to enable SRAM Programming
        if ( !fpga_cmd( ISC_ENABLE, sizeof(ISC_ENABLE) ) ) {
            DBG_ERR ;
            break ;
        }

        // Transmit ERASE Command to Erase SRAM
        if ( !fpga_cmd( ISC_ERASE, sizeof(ISC_ERASE) ) ) {
            DBG_ERR ;
            break ;
        }

        // Wait 200 µs
        HAL_Delay(1 + 1) ;

        // --------------- 2 ---------------
        // Read SRAM status Register (optional)
        // Transmit READ_STATUS Command to read status register
        uint32_t sr ;
read_st:
        if ( !read_status(&sr) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("status %08X", sr) ;
        if ( sr & (SR_DONE | SR_FAILFLAG) ) {
            DBG_ERR ;
            break ;
        }
        if ( sr & SR_BUSY ) {
            DBG_QUA ;
            // questa e' roba mia
            HAL_Delay(1) ;
            goto read_st ;
        }

        // Transmit LSC_BITSTREAM_BURST Command to configure SRAM
        // manca il comando: lo aggiungo
        memcpy_( bit, LSC_BITSTREAM_BURST, sizeof(LSC_BITSTREAM_BURST) ) ;

        // eseguo
        {
            if ( !fpga_spi_tx(bit, dimbit) ) {
                DBG_ERR ;
                break ;
            }
        }

        // --------------- 3 ---------------
        // Check SRAM Status Register? (Optional)
read_st1:
        if ( !read_status(&sr) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("status %08X", sr) ;
        if ( sr & SR_FAILFLAG ) {
            DBG_ERR ;
            break ;
        }
        if ( sr & SR_BUSY ) {
            DBG_QUA ;
            // questa e' roba mia
            HAL_Delay(1) ;
            goto read_st1 ;
        }
        if ( 0 == (sr & SR_DONE) ) {
            DBG_ERR ;
            break ;
        }

        // --------------- 4 ---------------
        // Verify USERCODE? (optional)
        // Transmit READ USERCODE Command
//        uint32_t uc ;
//        if ( !fpga_read( USERCODE, sizeof(USERCODE), &uc, sizeof(uc) ) ) {
//            DBG_ERR ;
//            break ;
//        }
//        DBG_PRINTF("usercode %08X", uc) ;

        // Transmit DISABLE Command
        if ( !fpga_cmd( ISC_DISABLE, sizeof(ISC_DISABLE) ) ) {
            DBG_ERR ;
            break ;
        }

        // Transmit NO-OP Command to exit programming mode
        if ( !fpga_cmd( ISC_NOOP, sizeof(ISC_NOOP) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}
