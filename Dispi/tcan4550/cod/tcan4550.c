/*
 * Si usa la spi per parlare col dispositivo
 */

#define STAMPA_DBG
#include "utili.h"
#include "tcan4550.h"
#include "bsp.h"

#define ATTESA_MS       100

#define READ_B_FL       0x41
#define WRITE_B_FL      0x61

#define TCAN_CMD_POS    0
#define TCAN_IND_POS    1
#define TCAN_DIM_POS    3
#define TCAN_VAL_POS    4
typedef struct {
    // cmd | ind | dim | val[]
    uint8_t blob[1 + 2 + 1 + sizeof(uint32_t) * TCAN_MAX_REG] ;
} S_TCAN ;

static S_TCAN leggi, scrivi ;
static uint32_t letti[TCAN_MAX_REG] ;

bool TCAN_iniz(void)
{
    return true ;
}

void TCAN_fine(void)
{}

// +-----------+-----------+----------+-----+
// | READ_B_FL | ind[15/8] | ind[7/0] | len |------
// +-----------+-----------+----------+-----+
//
//   ignorato                                 restituito
// +-----------+                            +-------------+-------------+------------+-----------+-------------+--
// | 0820[7/0] |----------------------------| reg0[31/24] | reg0[23/16] | reg0[15/8] | reg0[7/0] | reg1[31/24] | ...
// +-----------+                            +-------------+-------------+------------+-----------+-------------+--

uint32_t * TCAN_reg_leggi(
    PF_TCAN_SPI_TXRX spi_txrx,
    uint16_t numreg,
    uint16_t primo)
{
    uint32_t * val = NULL ;

    do {
        if ( numreg > TCAN_MAX_REG ) {
            DBG_ERR ;
            break ;
        }

        if ( primo & 3 ) {
            DBG_ERR ;
            break ;
        }
        primo = __REV16(primo) ;

        scrivi.blob[TCAN_CMD_POS] = READ_B_FL ;
        memcpy( scrivi.blob + TCAN_IND_POS, &primo, sizeof(primo) ) ;
        scrivi.blob[TCAN_DIM_POS] = numreg & (TCAN_MAX_REG - 1) ;
        memset( scrivi.blob + TCAN_VAL_POS, 0, numreg * sizeof(uint32_t) ) ;

        if ( spi_txrx( scrivi.blob, leggi.blob, TCAN_VAL_POS + numreg
                       * sizeof(uint32_t) ) ) {
            memcpy( &letti, leggi.blob + TCAN_VAL_POS, numreg * sizeof(uint32_t) ) ;
            for ( uint16_t i = 0 ; i < numreg ; ++i ) {
                letti[i] = __REV(letti[i]) ;
            }

            val = letti ;
        }
    } while ( false ) ;

    return val ;
}

// +------------+-----------+----------+-----+-------------+-------------+------------+-----------+--
// | WRITE_B_FL | ind[15/8] | ind[7/0] | len | reg0[31/24] | reg0[23/16] | reg0[15/8] | reg0[7/0] | ...
// +------------+-----------+----------+-----+-------------+-------------+------------+-----------+--
//
//    ignorato
// +-----------+
// | 0820[7/0] |----
// +-----------+

bool TCAN_reg_scrivi(
    PF_TCAN_SPI_TX spi_tx,
    uint16_t numreg,
    uint16_t primo,
    const uint32_t * val)
{
    bool esito = false ;

    do {
        if ( numreg > TCAN_MAX_REG ) {
            DBG_ERR ;
            break ;
        }

        if ( primo & 3 ) {
            DBG_ERR ;
            break ;
        }
        primo = __REV16(primo) ;

        scrivi.blob[TCAN_CMD_POS] = WRITE_B_FL ;
        memcpy( scrivi.blob + TCAN_IND_POS, &primo, sizeof(primo) ) ;
        scrivi.blob[TCAN_DIM_POS] = numreg & (TCAN_MAX_REG - 1) ;
        if ( val ) {
            for ( uint16_t i = 0 ; i < numreg ; ++i ) {
                letti[i] = __REV(val[i]) ;
            }
            memcpy( scrivi.blob + TCAN_VAL_POS, &letti, numreg
                    * sizeof(uint32_t) ) ;
        }
        else {
            memset( scrivi.blob + TCAN_VAL_POS, 0, numreg * sizeof(uint32_t) ) ;
        }

        esito =
            spi_tx( (uint8_t *) &scrivi, TCAN_VAL_POS + numreg
                    * sizeof(uint32_t) ) ;
    } while ( false ) ;

    return esito ;
}
