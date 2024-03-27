#define STAMPA_DBG
#include "utili.h"
#include "stm32h7xx_hal.h"

extern void HAL_TIM_MspPostInit(TIM_HandleTypeDef *);

#define FREQ_MAX     1000000
#define FREQ_COR        1000

static TIM_HandleTypeDef htim = {
    .State = HAL_TIM_STATE_RESET,

    .Instance = TIM13,

    .Init = {
        .CounterMode = TIM_COUNTERMODE_UP,
        .ClockDivision = TIM_CLOCKDIVISION_DIV1,
    }
} ;

static bool iniz = false ;

static void pwmUscita(
    TIM_HandleTypeDef * timer,
    uint32_t canale,
    bool abil)
{
    if ( abil ) {
        __HAL_TIM_SET_COUNTER(timer, 0) ;
        HAL_TIM_PWM_Start(timer, canale) ;
    }
    else {
        HAL_TIM_PWM_Stop(timer, canale) ;
    }
}

static void pwmFreq(
    TIM_HandleTypeDef * timer,
    uint32_t fmax,
	uint16_t hz)
{
    uint32_t tmp = (fmax / hz) - 1 ;

    timer->Instance->ARR = tmp ;

    // Duty-cycle del 50%
    timer->Instance->CCR1 = tmp >> 1 ;
}

//static void pwmDutyCycle(
//    TIM_HandleTypeDef * timer,
//    uint32_t perc)
//{
//    uint32_t tmp = timer->Instance->ARR + 1 ;
//
//    if ( perc >= 100 ) {
//        // 100 %
//    }
//    else {
//        tmp *= perc ;
//        tmp /= 100 ;
//        tmp -= 1 ;
//    }
//
//    timer->Instance->CCR1 = tmp ;
//}

bool BIP_iniz(void)
{
    if ( !iniz ) {
        uint32_t hclk = HAL_RCC_GetHCLKFreq() ;
        uint32_t clock = HAL_RCC_GetPCLK1Freq() ;
        uint32_t presc = hclk / clock ;
        if ( presc != 1 ) {
            clock *= 2 ;
        }

        htim.Init.Prescaler = (clock / FREQ_MAX) - 1 ;
        htim.Init.Period = (FREQ_MAX / FREQ_COR) - 1 ;

        do {
            if ( HAL_TIM_Base_Init(&htim) != HAL_OK ) {
                DBG_ERR ;
                break ;
            }

            if ( HAL_TIM_PWM_Init(&htim) != HAL_OK ) {
                DBG_ERR ;
                break ;
            }

            TIM_OC_InitTypeDef sConfigOC = {
                .OCMode = TIM_OCMODE_PWM1,
                .Pulse = htim.Init.Period >> 1,
                .OCPolarity = TIM_OCPOLARITY_HIGH,
                .OCFastMode = TIM_OCFAST_DISABLE,
            } ;
            if ( HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC,
                                           TIM_CHANNEL_1) != HAL_OK ) {
                DBG_ERR ;
                break ;
            }

            HAL_TIM_MspPostInit(&htim) ;

            iniz = true ;
        } while ( false ) ;
    }

    return iniz ;
}

void BIP_fine(void)
{
    if ( iniz ) {
        HAL_TIM_PWM_DeInit(&htim) ;

        iniz = false ;
    }
}

void BIP_freq(uint16_t hz)
{
    if ( htim.State != HAL_TIM_STATE_RESET ) {
        pwmUscita(&htim, TIM_CHANNEL_1, false) ;

        if ( hz ) {
            pwmFreq(&htim, FREQ_MAX, hz) ;
            pwmUscita(&htim, TIM_CHANNEL_1, true) ;
        }
    }
}
