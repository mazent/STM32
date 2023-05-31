#ifndef LIB_BOOT_BOOT_H_
#define LIB_BOOT_BOOT_H_

#ifdef DESCRITTORE_MIO

/*
    Una applicazione e' valida se:
        *) la firma e' valida
        *) il crc e' valido (cioe' 0)
*/

#include <stdint.h>
#include <stdbool.h>

// Indirizzi
// ------------------------
// BL+CLD abita il blocco 0
// SA occupa i blocchi 1 e 2
#define SRVC_APP_BASE       0x8020000
#define SRVC_APP_DIMB       (256 * 1024)
// MA occupa i restanti 3..7
#define MAIN_APP_BASE       0x8060000
#define MAIN_APP_DIMB       (640 * 1024)

#define S_DESCRITTORE_FIRMA        0x82AE4DC8

typedef struct {
    uint32_t Firma ;
    uint32_t Ident ;
    uint32_t Versione ;
    uint32_t Dim32 ;
} S_DESCRITTORE ;

// Vera se in dove c'e' una app valida
bool app_valida(const uint32_t dove) ;


#endif

#else
#   warning boot.h incluso
#endif
