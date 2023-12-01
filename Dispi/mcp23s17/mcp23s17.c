#define STAMPA_DBG
#include "utili.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"
#include "mcp23s17.h"

#define MAX_DIM     (1 + 1 + 2)

static uint8_t tx[MAX_DIM] ;
static uint8_t rx[MAX_DIM] ;

bool MCP_iniz(void)
{
    // al reset HAEN=0 -> the device’s hardware address	is A2 = A1 = A0 = 0
    tx[0] = MCP_BASE_IND ;
    tx[1] = MCP_IOCON ;
    tx[2] = MCP_IOCON_HAEN ;

    return mcp_spi_tx(tx, sizeof(tx)) ;
}

bool MCP_leggi(
    uint8_t ind,
    uint8_t reg,
    void * buf,
    uint8_t dim)
{
    bool esito = false ;

    do {
        if ( dim > 2 ) {
            DBG_ERR ;
            break ;
        }

        tx[0] = ind | 1 ;
        tx[1] = reg ;
        memset_(tx + 2, 0, dim) ;

        esito = mcp_spi_txrx(tx, rx, 2 + dim) ;
        if ( esito ) {
            memcpy_(buf, rx + 2, dim) ;
        }
    } while ( false ) ;

    return esito ;
}

bool MCP_scrivi(
    uint8_t ind,
    uint8_t reg,
    const void * buf,
    uint8_t dim)
{
    bool esito = false ;

    do {
        if ( dim > 2 ) {
            DBG_ERR ;
            break ;
        }

        tx[0] = ind ;
        tx[1] = reg ;
        memcpy_(tx + 2, buf, dim) ;

        esito = mcp_spi_tx(tx, 2 + dim) ;
    } while ( false ) ;

    return esito ;
}
