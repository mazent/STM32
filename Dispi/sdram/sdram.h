#ifndef DISPI_SDRAM_SDRAM_H_
#define DISPI_SDRAM_SDRAM_H_

#include "stm32h7xx_hal.h"

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *) ;

#else
#   warning sdram.h incluso
#endif
