/*
 * Cfr http://www.ganssle.com/watchdogs.htm
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <stdint.h>

// Mettete qui la lista
#include "watchdog_cfg.h"

void WDOG_Iniz(void) ;

// Ogni task deve avere un cane di una certa durata
// Il cane viene sospeso
void WDOG_Imposta(int cane, uint32_t milli) ;

// Periodicamente tirarne uno (se sospeso lo riattiva)
void WDOG_Calcia(int cane) ;

// Se il task dorme a lungo, puo' evitare
// di svegliarsi solo per tirare un calcio
void WDOG_Sospendi(int cane) ;

// se qualcosa va male (hard fault, ecc)
void WDOG_reset(void) ;

#endif
