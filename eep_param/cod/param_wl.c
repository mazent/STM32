#define STAMPA_DBG
#include "utili.h"
#include "../eeprom.h"
#include "../param.h"
#include "stm32h7xx_hal.h"

extern HASH_HandleTypeDef hhash ;
extern CRYP_HandleTypeDef hcryp ;
extern RNG_HandleTypeDef hrng ;

#define SHA_DIM     20

#define DIM_IV          11
#define DIM_DATI_32     ( DIM_DATI / sizeof(uint32_t) )

#define MAX_DIM_NS      ( DIM_DATI - sizeof(uint16_t) )

#define VER_NONVALE     0

typedef struct {
    // Progressivo
    uint32_t ver ;
    // Pagina logica
    uint8_t logica ;
    // Initialization vector (ultimi byte)
    uint8_t iv[DIM_IV] ;
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

static uint32_t verMax = VER_NONVALE ;

#define FIS_NONVALE     EEP_NUM_PAGINE

// Associa alla pagina logica la fisica con la versione piu' recente
static uint16_t log2fis[PLOG_MAX] ;

// Pagine fisiche riutilizzabili
static bool fisLib[EEP_NUM_PAGINE] ;

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

const void * prm_leggi(uint16_t plog)
{
    const void * prm = NULL ;

    //DBG_PRINTF("%s(%d)", __func__, plog) ;

    do {
        uint16_t fis = log2fis[plog] ;
        if ( FIS_NONVALE == fis ) {
            DBG_ERR ;
            break ;
        }

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
        hcryp.Init.pInitVect = &pagina.ver ;
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
        //DBG_PRINTF("\t-> %d", fis) ;
        prm = plain ;
    } while ( false ) ;

    (void) HAL_CRYP_DeInit(&hcryp) ;

    return prm ;
}

static bool nuovo_iv(void)
{
    for ( int i = 0 ; i < 3 ; i++ ) {
        if ( HAL_OK !=
             HAL_RNG_GenerateRandomNumber(&hrng, plain + i) ) {
            DBG_ERR ;
            return false ;
        }
    }

    memcpy_(pagina.iv, plain, DIM_IV) ;
    return true ;
}

bool prm_scrivi(
    uint16_t plog,
    const void * prm,
    uint8_t dim)
{
    bool esito = false ;

    //DBG_PRINTF("%s(%d)", __func__, plog) ;

    do {
        if ( !nuovo_iv() ) {
            break ;
        }

        // Aggiorno
        verMax++ ;
        if ( VER_NONVALE == verMax ) {
            verMax++ ;
        }
        pagina.ver = verMax ;
        pagina.logica = plog ;

        // cerco pagina libera partendo a caso (-> plain)
        static_assert(POTENZA_DI_2(EEP_NUM_PAGINE), "OKKIO") ;
        uint16_t fis ;
        memcpy_( &fis, plain, sizeof(fis) ) ;
        fis &= EEP_NUM_PAGINE - 1 ;
        uint16_t i = 0 ;
        for ( ; i < EEP_NUM_PAGINE ; i++ ) {
            if ( fisLib[fis] ) {
                break ;
            }
            fis++ ;
            fis &= EEP_NUM_PAGINE - 1 ;
        }
        if ( i == EEP_NUM_PAGINE ) {
            DBG_ERR ;
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
        hcryp.Init.pInitVect = &pagina.ver ;
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

        // aggiorno
        uint16_t vfis = log2fis[plog] ;
        if ( FIS_NONVALE != vfis ) {
            fisLib[vfis] = true ;
        }
        log2fis[plog] = fis ;
        fisLib[fis] = false ;

        //DBG_PRINTF("\t-> %d", fis) ;
        esito = true ;
    } while ( false ) ;

    return esito ;
}

void PRM_iniz(void)
{
    uint32_t ver[PLOG_MAX] = {
        VER_NONVALE
    } ;

    static_assert(EEP_DIM_PAGINA == sizeof(PAGINA), "OKKIO") ;

    chiave() ;

    // Niente di valido
    for ( uint16_t log = 0 ; log < PLOG_MAX ; log++ ) {
        log2fis[log] = FIS_NONVALE ;
    }

    // Tutto libero
    for ( uint16_t fis = 0 ; fis < EEP_NUM_PAGINE ; fis++ ) {
        fisLib[fis] = true ;
    }

    // Leggo il primo pezzo di ogni pagina
    for ( uint16_t pag = 0 ; pag < EEP_NUM_PAGINE ; pag++ ) {
        if ( !EEP_leggi( pag, bpag, sizeof(uint32_t) + sizeof(uint8_t) ) ) {
            DBG_ERR ;
            continue ;
        }

        if ( pagina.logica >= PLOG_MAX ) {
            // Vuota == FF > LOG_MAX
            continue ;
        }

        if ( verMax < pagina.ver ) {
            verMax = pagina.ver ;
        }

        if ( VER_NONVALE == ver[pagina.logica] ) {
            //DBG_PRINTF("OK: (%d %08X) -> %d", pagina.logica, pagina.ver, pag) ;

            // prima volta che la vedo
            log2fis[pagina.logica] = pag ;
            ver[pagina.logica] = pagina.ver ;
            fisLib[pag] = false ;
        }
        else if ( ver[pagina.logica] < pagina.ver ) {
            //DBG_PRINTF("NUOVA: (%d %08X -> %08X) -> %d",
            //           pagina.logica,
            //           pagina.ver,
            //           ver[pagina.logica],
            //           pag) ;

            // posso riutilizzare quella precedente
            fisLib[log2fis[pagina.logica]] = true ;

            // aggiorno
            log2fis[pagina.logica] = pag ;
            ver[pagina.logica] = pagina.ver ;
            fisLib[pag] = false ;
        }
        else {
            // Vecchia
            //DBG_PRINTF("VECCHIA: (%d %08X <- %08X)",
            //           pagina.logica,
            //           pagina.ver,
            //           ver[pagina.logica]) ;
        }
    }
}
