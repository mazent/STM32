#ifndef BSP_EEPROM_H_
#define BSP_EEPROM_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Interfaccia verso eeprom (per param.h)
 */

#ifdef HW_B0
	// Vera
#	define EEP_NUM_PAGINE      512
#else
	// Simulata
#	define EEP_NUM_PAGINE      128
#endif

#define EEP_DIM_PAGINA      128


// Max una pagina
bool EEP_leggi(
    uint16_t pag,
    void * buf,
    uint16_t dim) ;

// Max una pagina
bool EEP_scrivi(
    uint16_t pag,
    const void * buf,
    uint16_t dim) ;

#else
#   warning eeprom.h incluso
#endif
