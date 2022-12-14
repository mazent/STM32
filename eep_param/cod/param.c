#define STAMPA_DBG
#include "utili.h"
#include "../eeprom.h"
#include "../param.h"
#include "stm32h7xx_hal.h"

extern HASH_HandleTypeDef hhash ;
extern CRYP_HandleTypeDef hcryp ;
extern RNG_HandleTypeDef hrng ;

#define SHA_DIM     20

#define DIM_IV          16

#define DIM_DATI_32     ( DIM_DATI / sizeof(uint32_t) )

typedef struct {
    // Initialization vector
    uint32_t iv[DIM_IV / sizeof(uint32_t)] ;

    // Dati
    union {
        uint32_t dati[DIM_DATI_32] ;
        uint8_t bytes[DIM_DATI] ;
    } ;
} PAGINA ;

static uint32_t plain[DIM_DATI_32] ;

static union {
    uint8_t bpag[EEP_DIM_PAGINA] ;
    PAGINA pagina ;
} ;

static uint32_t kiave[4] ;

static void chiave(void)
{
    uint8_t tmp[SHA_DIM] ;
    uint32_t uid[3] = {
        HAL_GetUIDw0(),
        HAL_GetUIDw1(),
        HAL_GetUIDw2()
    } ;

    // Primo giro
    if ( HAL_OK !=
         HAL_HASH_SHA1_Start(&hhash, (uint8_t *) uid, sizeof(uid), tmp,
                             1000) ) {
        DBG_ERR ;
    }
    // Secondo giro
    else if ( HAL_OK !=
              HAL_HASH_SHA1_Start(&hhash, tmp, SHA_DIM, bpag, 1000) ) {
        DBG_ERR ;
    }
    // Terzo giro
    else if ( HAL_OK !=
              HAL_HASH_SHA1_Start(&hhash, bpag, SHA_DIM, tmp, 1000) ) {
        DBG_ERR ;
    }
    else {
        // Ok
        memcpy_( kiave, tmp, sizeof(kiave) ) ;
    }
}

const void * prm_leggi(uint16_t fis)
{
    const void * prm = NULL ;

    do {
        if ( !EEP_leggi(fis, bpag, EEP_DIM_PAGINA) ) {
            DBG_ERR ;
            break ;
        }

        // Non si sa mai
        (void) HAL_CRYP_DeInit(&hcryp) ;

        hcryp.Instance = CRYP ;
        hcryp.Init.DataType = CRYP_DATATYPE_32B ;
        hcryp.Init.KeySize = CRYP_KEYSIZE_128B ;
        hcryp.Init.pKey = kiave ;
        hcryp.Init.pInitVect = pagina.iv ;
        hcryp.Init.Algorithm = CRYP_AES_CBC ;
        if ( HAL_CRYP_Init(&hcryp) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_OK !=
             HAL_CRYP_Decrypt(&hcryp, (uint32_t *) &pagina.dati, DIM_DATI_32,
                              plain, 1000) ) {
            DBG_ERR ;
            break ;
        }

        // Ottimo
        prm = plain ;
    } while ( false ) ;

    (void) HAL_CRYP_DeInit(&hcryp) ;

    return prm ;
}

static bool nuovo_iv(void)
{
    for ( int i = 0 ; i < 4 ; i++ ) {
        if ( HAL_OK !=
             HAL_RNG_GenerateRandomNumber(&hrng, pagina.iv + i) ) {
            DBG_ERR ;
            return false ;
        }
    }

    return true ;
}

bool prm_scrivi(
    uint16_t fis,
    const void * prm,
    uint8_t dim)
{
    bool esito = false ;

    do {
        if ( !nuovo_iv() ) {
            break ;
        }

        // Copio
        {
            // riutilizzo plain
            uint8_t * bplain = (uint8_t *) plain ;
            memcpy_(bplain, prm, dim) ;
            if ( dim < DIM_DATI ) {
                // azzero la roba che manca
                memset_(bplain + dim, 0, DIM_DATI - dim) ;
            }
        }

        // cifro
        (void) HAL_CRYP_DeInit(&hcryp) ;

        hcryp.Instance = CRYP ;
        hcryp.Init.DataType = CRYP_DATATYPE_32B ;
        hcryp.Init.KeySize = CRYP_KEYSIZE_128B ;
        hcryp.Init.pKey = kiave ;
        hcryp.Init.pInitVect = pagina.iv ;
        hcryp.Init.Algorithm = CRYP_AES_CBC ;
        if ( HAL_CRYP_Init(&hcryp) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_OK !=
             HAL_CRYP_Encrypt(&hcryp, plain, DIM_DATI_32, pagina.dati,
                              1000) ) {
            DBG_ERR ;
            break ;
        }

        // salvo
        if ( !EEP_scrivi(fis, &pagina, EEP_DIM_PAGINA) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

void PRM_iniz(void)
{
    static_assert(EEP_DIM_PAGINA == sizeof(PAGINA), "OKKIO") ;

    chiave() ;
}

