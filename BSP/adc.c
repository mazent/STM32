//#define STAMPA_DBG
#include "adc.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"

extern ADC_HandleTypeDef hadc1 ;
extern ADC_HandleTypeDef hadc3 ;

static bool campiona(
    ADC_HandleTypeDef * h,
    uint16_t * p1,
    uint16_t * p2)
{
    bool esito = false ;

    do {
        if ( HAL_OK != HAL_ADC_Start(h) ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_OK != HAL_ADC_PollForConversion(h, 1000) ) {
            DBG_ERR ;
            break ;
        }
        uint32_t c1 = HAL_ADC_GetValue(h) ;
        DBG_PRINTF("c1 %08X", c1) ;

        if ( HAL_OK != HAL_ADC_PollForConversion(h, 1000) ) {
            DBG_ERR ;
            break ;
        }
        uint32_t c2 = HAL_ADC_GetValue(h) ;
        DBG_PRINTF("c2 %08X", c2) ;

        memcpy_( p1, &c1, sizeof(uint16_t) ) ;
        memcpy_( p2, &c2, sizeof(uint16_t) ) ;
        esito = true ;
    } while ( false ) ;

    CONTROLLA( HAL_OK == HAL_ADC_Stop(h) ) ;

    return esito ;
}

bool ADC_dammi(S_ADC * p)
{
    return campiona(&hadc1, &p->ch1, &p->ch2) &&
           campiona(&hadc3, &p->vbatt, &p->vkey) ;
}

/*
 * Rapporto di partizione:
 *     2k7 / (2k7 + 66k) = 0.0393
 *
 * Vref = 2.5V
 * 12 bit allineati a sx = 16 bit finti
 * ADCmax = 2^16 - 1 = 65535
 */

// 6 V = 0.2358 = 6181 -> 0x1820 = 6176 = 5.99 V
#define VBAT_MIN    6176

// 7 V = 0.2751 = 7211 -> 0x1C20 = 7200 = 6.99 V
#define VBAT_MAX    7200

bool batteria_bassa(void)
{
    bool bassa = false ;
    uint16_t vbatt, _ ;

    if ( campiona(&hadc3, &vbatt, &_) ) {
        bassa = (VBAT_MIN < vbatt) && (vbatt < VBAT_MAX) ;
    }

    return bassa ;
}
