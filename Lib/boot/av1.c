#define STAMPA_DBG
#include "utili.h"
#include "priv_av1.h"

extern CRC_HandleTypeDef hcrc ;


static bool h_valido(const commonHeader * pD)
{
    uint32_t cks = 0 ;

    // salto cs attuale
    const uint8_t * pB = (uint8_t *) pD ;
    pB += sizeof(uint32_t) ;

    for ( size_t i = 0 ; i < sizeof(commonHeader) - sizeof(uint32_t) ;
          i++, pB++ ) {
        cks += *pB ;
    }

    return cks == pD->headerChecksum32 ;
}

static uint32_t fw_crc32(
    void * /*bin*/,
    uint32_t /*dim*/) ;

/**
 *
 * @param dove[in] indirizzo da controllare
 * @param vi[out]  vettore delle interruzioni
 * @return		   torna true se all'indirizzo indicato c'e' una
 *                 immagine valida (*vi contiene l'indirizzo del vettore
 *                 delle interruzioni)
 */

bool app_valida(
    uint32_t dove,
    uint32_t * vi)
{
    bool esito = false ;

    DBG_PRINTF("%s(%08X)", __func__, dove) ;

    do {
        const uint32_t desc = dove + DIM_VI ;

        const commonHeader * pD = CPOINTER(desc) ;
        if ( HEADER_FILE_SIGNATURE != pD->headerSignature ) {
            DBG_ERR ;
            break ;
        }

        if ( HEADER_FILE_VERSION != pD->headerVersion ) {
            DBG_ERR ;
            break ;
        }

        if ( !h_valido(pD) ) {
            DBG_ERR ;
            break ;
        }

        if ( pD->fileSize < DIM_VI + sizeof(commonHeader) ) {
            DBG_ERR ;
            break ;
        }

        uint32_t crc = fw_crc32(POINTER(dove), pD->fileSize) ;
        if ( crc != pD->fwCrc32 ) {
            DBG_ERR ;
            DBG_PRINTF("\tcrc = %08X != %08X", crc, pD->fwCrc32) ;
            break ;
        }

        esito = true ;
        *vi = dove ;
    } while ( false ) ;

    return esito ;
}

#if 0
// Calcolo sw (vedi ^/SW/PostBuildUtility/HeaderCreator)

// prese da crc32_STM32.cpp
static void STM32_CRC32_Init(uint32_t * crc)
{
    *crc = 0xFFFFFFFF ;
}

static void calcCRC32_STM(
    uint32_t * crc,
    uint32_t data)
{
    uint32_t Crc = *crc ^ data ;

    for ( int i = 0 ; i < 32 ; i++ ) {
        if ( Crc & 0x80000000 ) {
            Crc = (Crc << 1) ^ 0x04C11DB7 ; // Polynomial used in STM32
        }
        else {
            Crc = (Crc << 1) ;
        }
    }

    *crc = Crc ;
}

static uint32_t STM32_CRC32(
    uint32_t * pBuffer,
    uint32_t sizeBufferWord)
{
    uint32_t crc32 = 0 ;

    STM32_CRC32_Init(&crc32) ;
    for ( uint32_t n = 0 ; n < sizeBufferWord ; n++ ) {
        calcCRC32_STM(&crc32, *pBuffer++) ;
    }

    return crc32 ;
}

static uint32_t STM32_CRC32_Update(
    uint32_t prevCrc,
    uint32_t * pBuffer,
    uint32_t sizeBufferWord)
{
    uint32_t crc32 = prevCrc ;

    STM32_CRC32_Init(&crc32) ;
    calcCRC32_STM(&crc32, prevCrc) ;

    for ( uint32_t n = 0 ; n < sizeBufferWord ; n++ ) {
        calcCRC32_STM(&crc32, *pBuffer++) ;
    }

    return crc32 ;
}

#define THE_APPLICATION_INTVECT_OFFSET              (0x00000000)
#define THE_APPLICATION_PRIVATE_HEADERFW_OFFSET     (0x00000480)
#define THE_APPLICATION_CODE_OFFSET                 (0x00000500)
#define THE_APPLICATION_INTERRUPT_VECTOR_SIZE       (0x00000400)
#define THE_APPLICATION_PRIVATE_HEADERFW_SIZE       (0x00000080)

// ex Calc_Crc32_STM32 in HeaderCreator.cpp
static uint32_t fw_crc32(
    uint8_t * pBinaryFile,
    uint32_t sizeBuffer)
{
    uint32_t crc32 = 0 ;

    STM32_CRC32_Init(&crc32) ;

    //Calculate the CRC32 of interrupt vector table
    uint32_t * pBuf =
        (uint32_t *) (pBinaryFile + THE_APPLICATION_INTVECT_OFFSET) ;
    crc32 = STM32_CRC32(pBuf, THE_APPLICATION_INTERRUPT_VECTOR_SIZE / 4) ;
    DBG_PRINTF("IV %08X", crc32) ;

    //Calculate the CRC32 of private header
    pBuf = (uint32_t *) (pBinaryFile + THE_APPLICATION_PRIVATE_HEADERFW_OFFSET) ;
    crc32 = STM32_CRC32_Update(crc32,
                               pBuf,
                               THE_APPLICATION_PRIVATE_HEADERFW_SIZE / 4) ;
    DBG_PRINTF("PH %08X", crc32) ;

    //Calculate the CRC32 of main firmware
    pBuf = (uint32_t *) (pBinaryFile + THE_APPLICATION_CODE_OFFSET) ;
    crc32 =
        STM32_CRC32_Update(crc32,
                           pBuf,
                           (sizeBuffer - THE_APPLICATION_CODE_OFFSET) / 4) ;
    DBG_PRINTF("AP %08X", crc32) ;

    return crc32 ;
}

#else

// Uso la periferica

#include "stm32h7xx_hal.h"

static const CRC_HandleTypeDef fw_crc = {
    .State = HAL_CRC_STATE_RESET,
    .Instance = CRC,
    .Init = {
        .DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE,
        .DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE,
        .InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE,
        .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
    },
    .InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS,
} ;

#define DIM_PRIVATE_H       128

static uint32_t fw_crc32(
    void * bin,
    uint32_t dim)
{
    uint32_t crc = 0 ;
    // Il fw e' sempre allineato all'inizio dei blocchi in flash
    uint32_t * dove = (uint32_t *) bin ;

    CONTROLLA( HAL_OK == HAL_CRC_DeInit(&hcrc) ) ;

    do {
        hcrc = fw_crc ;
        if ( HAL_CRC_Init(&hcrc) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        // Prima parte: interruzioni
        uint32_t dimcrc = DIM_VI / sizeof(uint32_t) ;
        uint32_t tmp = HAL_CRC_Calculate(&hcrc,
                                         dove,
                                         dimcrc) ;
        dove += dimcrc ;
        DBG_PRINTF("IV %08X", tmp) ;

        // Seconda parte: header privato
        (void) HAL_CRC_Calculate( &hcrc,
                                  &tmp,
                                  sizeof(tmp) / sizeof(uint32_t) ) ;
        dove += sizeof(commonHeader) / sizeof(uint32_t) ;
        dimcrc = DIM_PRIVATE_H / sizeof(uint32_t) ;
        tmp = HAL_CRC_Accumulate(&hcrc,
                                 dove,
                                 dimcrc) ;
        dove += dimcrc ;
        DBG_PRINTF("PH %08X", tmp) ;

        // Terza parte: dopo header privato
        (void) HAL_CRC_Calculate( &hcrc,
                                  &tmp,
                                  sizeof(tmp) / sizeof(uint32_t) ) ;
        dimcrc = (dim                     // totale
                  - DIM_VI                // fatto (prima parte)
                  - sizeof(commonHeader)  // da saltare
                  - DIM_PRIVATE_H)        // fatto (seconda parte)
                 / sizeof(uint32_t) ;
        crc = HAL_CRC_Accumulate(&hcrc,
                                 dove,
                                 dimcrc) ;
        DBG_PRINTF("AP %08X", crc) ;
    } while ( false ) ;

    CONTROLLA( HAL_OK == HAL_CRC_DeInit(&hcrc) ) ;

    return crc ;
}

#endif

#if 0
void app_iniz(void)
{
    // settore 2: falso
    uint32_t dove = 0x8040000 ;
    DBG_PRINTF("%08X: %s / FALSO", dove, app_valida(dove) ? "VERO" : "FALSO") ;

    // settore 3: vero
    dove = 0x8060000 ;
    DBG_PRINTF("%08X: %s / VERO", dove, app_valida(dove) ? "VERO" : "FALSO") ;

    BPOINT ;
}

#endif


