#ifndef APP_ROM_H_
#define APP_ROM_H_

#include <stdbool.h>
#include <stdint.h>

#define DIM_RIGA_ROM_B      32

#define DIM_SETTORE_ROM_B       (128 * 1024)

// Settore o indirizzo
bool ROM_cancella(uint32_t /*cosa*/) ;

// Scrive una riga, cioe' 32 byte == 8 uint32_t
bool ROM_scrivi(
    const void * rom,
    const uint32_t * otto) ;

uint32_t ROM_indirizzo(uint32_t /*cosa*/) ;

// Per aggiornamento
void * ROM_altro_ind(void) ;
uint32_t ROM_altro_dim(void) ;

#else
#   warning rom.h incluso
#endif
