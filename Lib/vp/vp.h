#ifndef LIB_UTILI_VP_H_
#define LIB_UTILI_VP_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Associa ad un valore un crc rendendo sicuro il
 * salvataggio nelle sezioni non inizializzate
 */

typedef struct {
    uint32_t val ;
    uint16_t crc ;
    /*
     * Vedi AN5342 - Rev 3 - pag 5
     *
     * Su H7 la ram ha ecc per cui:
     *     The ECC is computed on data word
     *     ...
     *     On an incomplete access, the ECC does not write the value immediately
     *     ...
     *     a write operation is not to be completed in case of reset
     *     ...
     *     The workaround for this limitation is to write a dummy incomplete
     *     word write after each regular one
     */
    uint16_t _ ;
} S_VP ;

// Se non valido imposta 0 e torna falso
bool VP_ini(S_VP * /*vp*/) ;

// Se non valido torna 0
uint32_t VP_leggi(S_VP * /*vp*/) ;

// Aggiunge 1
void VP_incrm(S_VP * /*vp*/) ;

// Assegna un nuovo valore
void VP_nuovo(S_VP * /*vp*/, uint32_t /*val*/) ;

#else
#   warning vp.h incluso
#endif
