#ifndef CYBT_U_H_
#define CYBT_U_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Interfaccia verso la seriale
 */

typedef void (*PF_BTU_CB)(void) ;

// Apre la seriale
bool BTU_iniz(uint32_t /*baud*/,bool);

// Chiude la seriale
void BTU_fine(void) ;

// Trasmette la roba
bool BTU_tx(const void * /*tx*/, uint32_t /*dim*/) ;

// Recupera la roba ricevuta
uint32_t BTU_rx(void * /*rx*/, uint32_t /*dim*/) ;

// Disaccoppiare e prendere la roba
extern void btu_nuovidati(void) ;

// Invocare in caso di errore
extern void btu_errorcallback(void) ;

#else
#   warning cybt_u.h incluso
#endif
