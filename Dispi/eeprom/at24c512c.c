#define STAMPA_DBG
#include "utili.h"
#include "eeprom.h"
#include "i2c.h"

// 1010.000r
#define AT24C512C       0xA0

#define OFS_PAG         7

bool EEP_leggi(
    uint16_t pag,
    void * buf,
    uint16_t dim)
{
    bool esito = false ;

    do {
        if ( pag >= EEP_NUM_PAGINE ) {
            DBG_ERR ;
            break ;
        }

        if ( NULL == buf ) {
            DBG_ERR ;
            break ;
        }

        if ( 0 == dim ) {
            DBG_ERR ;
            break ;
        }
        else if ( dim > EEP_DIM_PAGINA ) {
            DBG_ERR ;
            break ;
        }

        pag <<= OFS_PAG ;
        pag = __REV16(pag) ;

        if ( !I2C_scrivi( AT24C512C, &pag, sizeof(pag) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = I2C_leggi(AT24C512C, buf, dim) ;
    } while ( false ) ;

    return esito ;
}

static uint8_t tmp[sizeof(uint16_t) + EEP_DIM_PAGINA] ;

bool EEP_scrivi(
    uint16_t pag,
    const void * buf,
    uint16_t dim)
{
    bool esito = false ;

    do {
        if ( pag >= EEP_NUM_PAGINE ) {
            DBG_ERR ;
            break ;
        }

        if ( NULL == buf ) {
            DBG_ERR ;
            break ;
        }

        if ( 0 == dim ) {
            DBG_ERR ;
            break ;
        }
        else if ( dim > EEP_DIM_PAGINA ) {
            DBG_ERR ;
            break ;
        }

        pag <<= OFS_PAG ;
        pag = __REV16(pag) ;

        memcpy_( tmp, &pag, sizeof(pag) ) ;
        memcpy_(tmp + sizeof(pag), buf, dim) ;

        esito = I2C_scrivi( AT24C512C, tmp, dim + sizeof(pag) ) ;
    } while ( false ) ;

    return esito ;
}
