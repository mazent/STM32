#define STAMPA_DBG
#include "utili.h"
#include "bsp.h"

extern void attesa_clock(uint32_t clock) ;

// colpi di clock per microsecondo
static uint32_t clk_us = 0 ;

void attesa_us(uint32_t micro)
{
	if (0 == clk_us) {
		// Ipotesi: il clock non cambia
		clk_us = HAL_RCC_GetSysClockFreq() / 1000000 ;
	}

	attesa_clock(micro * clk_us) ;
}

