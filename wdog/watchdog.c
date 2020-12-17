//#define STAMPA_DBG
#include "includimi.h"

#include "watchdog.h"

#include "tabint.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_wwdg.h"
#include "tick.h"
#include "stm32f10x_dbgmcu.h"

#include "gpio.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>

/*
 * Usa il wdog "a finestra" perche' si spegne assieme al processore
 *
 * Occorre resettare il wdog (calcio) entro una finestra di tempo, altrimenti
 * ci si resetta
 *
 * Un task ad alta priorita' fornisce agli altri task un wdog software:
 *     x) con tempi svincolati dalla finestra hw
 *     x) che puo' essere sospeso
 *
 * Il wdog viene calciato da un timer hw con isr che gira in ram, in modo
 * che il calcio avvenga anche quando la cpu e' bloccata per operazioni
 * in flash; una variabile permette alla isr di capire se il task del
 * wdog gira periodicamente
 *
 */

#if 1
	// Questo task e' l'unico alla priorita' piu' alta
	// per cui, anche se il tick non fosse sufficiente
	// a concludere le operazioni, lo scheduler continua
	// a farlo girare
#	define ENTER_CS
#	define LEAVE_CS
#else
	// Il task condivide la priorita' piu' alta con
	// altri e il tick potrebbe non essere sufficiente
	// a concludere le operazioni per cui bisogna
	// sospendere la schedulazione
#	define ENTER_CS     os_KernelSuspend()
#	define LEAVE_CS		os_KernelResume()
#endif

// Parametri del watchdog sw
#define CONTROLLO_MS		200

// Parametri del watchdog hw
	// (8 MHz / 4096) / 8 = 244,14 Hz -> 4,096 ms
#define PRESCALER				WWDG_Prescaler_8
	// Refresh permesso per 70 - 63 => 28,67 ms > tick di sistema
#define WDOG_CFR_W			70
	// Refresh disabilitato per 127 - 70 => 233 ms
#define WDOG_CR_T			127

// 240 - 233 = 7 ms dentro la finestra
#define ATTESA_FINESTRA_MS  240

static osThreadId hWDog = NULL ;

#define CANE_SOSPESO		((uint32_t) -1)

typedef struct {
    // Ogni task ha un cane e sceglie una durata in ms,
    // convertita in periodi da CONTROLLO_MS
	// Modificata dai task
    uint32_t Durata ;
    // Il wd la incrementa ogni CONTROLLO_MS: quando
    // diventa uguale a Durata controlla Incrementami
    uint32_t Conta ;

    // Ogni calcio aumenta questo valore
    // Modificata dai task
    uint32_t Incrementami ;

} UN_CANE ;

static UN_CANE canile[WDOG_MAX] ;

// 8 MHz / 32768 = 244,14 Hz = 4,096 ms come wdog
#define UN_COLPO_US		4096

#ifdef USA_WDOG

// Durata massima senza che il task del wdog abbia azzerato
// la variabile (segno che gira)
#define T_MAX_SOSP_MS	5000
#define MAX_CONTA_T6 	((T_MAX_SOSP_MS + ATTESA_FINESTRA_MS - 1) / ATTESA_FINESTRA_MS)
static uint32_t conta_t6 = 0 ;

static void hw_timer_iniz(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE) ;

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure = {
		.TIM_Prescaler = 32767,
		.TIM_Period = (ATTESA_FINESTRA_MS * 1000 - 1) / UN_COLPO_US,
	} ;

	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure) ;
}

static __ramfunc void t6irq(void)
{
	//TIM_ClearFlag(TIM6, TIM_IT_Update) ;
	TIM6->SR = (uint16_t) NEGA(TIM_IT_Update) ;

	conta_t6++ ;

	if (conta_t6 < MAX_CONTA_T6) {
		//WWDG_SetCounter(WDOG_CR_T) ;
		WWDG->CR = WDOG_CR_T & ((uint8_t)0x7F) ;

		// DbgB26Commuta()
		DWORD x = GPIOE->IDR ;
		x ^= GPIO_Pin_12 ;
		GPIOE->ODR = x ;
	}
	else {
		// Mi resetto (fra poco)
	}
}
#endif

static void TaskWDog(void * p)
{
	INUTILE(p) ;

	while (true) {
		osDelay(CONTROLLO_MS) ;
#ifdef USA_WDOG
		conta_t6 = 0 ;
#endif
        for (int i=0 ; i<WDOG_MAX ; i++) {
            if (CANE_SOSPESO == canile[i].Conta) {
                // Ottimo
            }
            else {
            	canile[i].Conta++ ;

                if (canile[i].Conta != canile[i].Durata) {
                	// Alla prossima
                }
                	// Deve avere calciato almeno una volta
                else if (0 == canile[i].Incrementami) {
                    // Problemi
                    BPOINT ;
                    DBG_PRINTF("!!!! WDOG: reset %d !!!!\n", i) ;
#ifdef NDEBUG
                    // Uno di questi mi resetta
                    while (true)
                    	WWDG_SetCounter(WDOG_CR_T) ;
#else
                    canile[i].Conta = 0 ;
                    canile[i].Incrementami = 0 ;
#endif
                }
                else {
                    canile[i].Conta = 0 ;
                    canile[i].Incrementami = 0 ;
                }
            }
        }
	}
}

void WDOG_Iniz(void)
{
    assert(NULL == hWDog) ;

    if (NULL == hWDog) {
    	for (int i=0 ; i<WDOG_MAX ; i++) {
    		canile[i].Conta = CANE_SOSPESO ;
    	}

#ifdef USA_WDOG
    	DBG_PRINTF("MAX_CONTA_T6 = %d\n", MAX_CONTA_T6) ;

    	ISR_Installa(TIM6_IRQn, t6irq) ;
    	ISR_Abilita(TIM6_IRQn) ;

    	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE) ;
    	WWDG_SetPrescaler(PRESCALER) ;
    	WWDG_SetWindowValue(WDOG_CFR_W) ;

    	hw_timer_iniz() ;
    	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE) ;

#	ifndef NDEBUG
    	// Non voglio che wd giri se mi fermo su un bp
    	DBGMCU_Config(DBGMCU_WWDG_STOP, ENABLE) ;
    	DBGMCU_Config(DBGMCU_TIM6_STOP, ENABLE) ;
#	endif

    	WWDG_Enable(WDOG_CR_T) ;
    	TIM_Cmd(TIM6, ENABLE) ;
#endif
		osThreadDef(TaskWDog, osPriorityRealtime, 1, 0) ;
		hWDog = osThreadCreate(osThread(TaskWDog), NULL) ;
    }
}

void WDOG_Imposta(int cane, uint32_t milli)
{
	assert(NULL != hWDog) ;
    assert(cane < WDOG_MAX) ;
    assert(milli != 0) ;

    if (NULL == hWDog) {
    }
    else if (cane >= WDOG_MAX) {
    }
    else if (0 == milli) {
    }
    else {
    	ENTER_CS ;

    	DBG_PRINTF("WDOG imposta %d %u\n", cane, milli) ;

        // Prima lo sospendo ...
    	canile[cane].Conta = CANE_SOSPESO ;
        // ... poi lo alloco
        canile[cane].Durata = 1 + (milli - 1) / CONTROLLO_MS ;

        LEAVE_CS ;
    }
}

void WDOG_Calcia(int quale)
{
	assert(NULL != hWDog) ;
	assert(quale < WDOG_MAX) ;

	if (NULL == hWDog) {
    }
    else if (quale >= WDOG_MAX) {
    }
    else {
    	ENTER_CS ;

    	DBG_PRINTF("WDOG calcia %d\n", quale) ;

        if (CANE_SOSPESO == canile[quale].Conta) {
            // Se il task lo trova 0 mi resetta, per cui ...
                // ... prima lo metto a 1
            canile[quale].Incrementami = 1 ;
                // ... e poi abilito
            canile[quale].Conta = 0 ;
        }
        else
        	canile[quale].Incrementami++ ;

        LEAVE_CS ;
    }
}

void WDOG_Sospendi(int quale)
{
	assert(NULL != hWDog) ;
	assert(quale < WDOG_MAX) ;

	if (NULL == hWDog) {
    }
    else if (quale >= WDOG_MAX) {
    }
    else {
        ENTER_CS ;

        DBG_PRINTF("WDOG sospendi %d\n", quale) ;

        // Se il task lo trova 0 mi resetta, per cui ...
            // ... prima lo metto a 1
        canile[quale].Incrementami = 1 ;
            // ... e poi sospendo
        canile[quale].Conta = CANE_SOSPESO ;

        LEAVE_CS ;
    }
}

void WDOG_ferma(void)
{
#ifdef USA_WDOG
	os_ThreadSuspend(hWDog) ;
	DBG_PUTS("WDOG sospeso!") ;
#endif
}

void WDOG_reset(void)
{
#ifdef USA_WDOG
	if (NULL == hWDog) {
		// non iniz.to: reset normale
		NVIC_SystemReset() ;
    }
	else {
        // Uno di questi mi resetta
        while (true)
        	WWDG_SetCounter(WDOG_CR_T) ;
	}
#else
	NVIC_SystemReset() ;
#endif
}
