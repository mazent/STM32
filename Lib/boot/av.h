#ifndef LIB_BOOT_BOOT_H_
#define LIB_BOOT_BOOT_H_

/*
    Una applicazione e' valida se:
        *) la firma e' valida
        *) il crc e' valido (cioe' 0)
*/

#include <stdint.h>
#include <stdbool.h>

#define S_DESCRITTORE_FIRMA        0x82AE4DC8

typedef struct {
    uint32_t Firma ;
    uint32_t Ident ;
    uint32_t Versione ;
    uint32_t Dim32 ;
} S_DESCRITTORE ;

/**
 * app_valida
 *
 * @param dove[in] indirizzo da controllare
 * @param vi[out]  vettore delle interruzioni
 * @return		   torna true se all'indirizzo indicato c'e' una
 *                 immagine valida (*vi contiene l'indirizzo del vettore
 *                 delle interruzioni)
 * @note           la funzione dipende dall'intestazione per cui occorre
 *                 compilare solo uno dei avX.c:
 *                 av0.c   descrittore mio (S_DESCRITTORE)
 *                 av1.c   descrittore Badanai
 *                 av2.c   altro descrittore Badanai
 */

bool app_valida(
    uint32_t dove,
    uint32_t * vi) ;

#else
#   warning boot.h incluso
#endif
