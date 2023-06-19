#ifndef DISPI_SDRAM_SDRAM_H_
#define DISPI_SDRAM_SDRAM_H_

#include "stm32h7xx_hal.h"

#ifdef HAL_SDRAM_MODULE_ENABLED

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef * /*hsdram*/) ;

#endif

#else
#   warning sdram.h incluso
#endif
