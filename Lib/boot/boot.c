#define STAMPA_DBG
#include "utili.h"

#ifdef DESCRITTORE_MIO

#include "boot.h"
#include "stm32h7xx_hal.h"

static CRC_HandleTypeDef hcrc = {
    .Instance = CRC,
    .Init = {
        .DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE,
        .DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE,
        .InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE,
        .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
    },
    .InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS,
} ;

#pragma section=".intvec"

bool app_valida(const uint32_t dove)
{
    bool esito = false ;

    do {
        const uint32_t desc = dove + __section_size(".intvec") ;

        const S_DESCRITTORE * pD = CPOINTER(desc) ;
        if ( S_DESCRITTORE_FIRMA != pD->Firma ) {
            DBG_ERR ;
            break ;
        }

        if ( 0 == pD->Dim32 ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_CRC_Init(&hcrc) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        uint32_t crc = HAL_CRC_Calculate(&hcrc, POINTER(dove), pD->Dim32) ;
        if ( crc ) {
            DBG_ERR ;
            DBG_PRINTF("\tcrc = %08X", crc) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    CONTROLLA( HAL_OK == HAL_CRC_DeInit(&hcrc) ) ;

    return esito ;
}

#endif
