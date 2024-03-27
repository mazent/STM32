#define STAMPA_DBG
#include "utili.h"
#include "stm32h7xx_hal.h"

extern CRC_HandleTypeDef hcrc ;

static const CRC_HandleTypeDef itt_crc = {
    .State = HAL_CRC_STATE_RESET,
    .Instance = CRC,
    .Init = {
        .DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE,
        .DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE,
        .InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE,
        .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
        .GeneratingPolynomial = 0x1021,
        .CRCLength = CRC_POLYLENGTH_16B,
        .InitValue = crc_ini,
    },
    .InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES,
} ;

uint16_t CRC_1021_hw(
    uint16_t crc_ini,
    const void * v,
    int dim)
{
    uint32_t crc ;

    if ( HAL_CRC_STATE_RESET == hcrc.State ) {
        // Ottimo
    }
    else if ( hcrc.Init != itt_crc.Init ) {
        HAL_CRC_MspDeInit(&hcrc) ;
    }

    hcrc = itt_crc ;

    if ( HAL_OK == HAL_CRC_Init(&hcrc) ) {
        crc = HAL_CRC_Calculate(&hcrc, CONST_CAST(v), dim) ;
    }

    return crc ;
}

#if 0
extern RNG_HandleTypeDef hrng ;

extern uint16_t CRC_1021_v(
    uint16_t crc,
    const void * vett,
    int dim) ;

void app_iniz(void)
{
    uint16_t crc_ini = 0xABCD ;
    while ( true ) {
        uint32_t val ;

        if ( HAL_OK ==
             HAL_RNG_GenerateRandomNumber(&hrng, &val) ) {
            uint16_t hw = CRC_1021_hw( crc_ini, &val, sizeof(val) ) ;
            uint16_t sw = CRC_1021_v( crc_ini, &val, sizeof(val) ) ;

            DBG_PRINTF("hw %04X, sw %04X", hw, sw) ;
        }
    }
}
#endif
