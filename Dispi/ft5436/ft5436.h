#ifndef DISPI_FT5436_FT5436_H_
#define DISPI_FT5436_FT5436_H_

#include <stdbool.h>
#include <stdint.h>

#define NUM_PUNTI       5

#define EVN_PUT_DOWN   0
#define EVN_PUT_UP     1
#define EVN_CONTACT    2
#define EVN_RESERVED   3

typedef struct {
    uint16_t
        x : 12,
        ris12 : 2,
        evn : 2 ;
    uint16_t
        y : 12,
        id : 4 ;
} FT5436_TOUCH ;

bool FT5436_iniz(void) ;
void FT5436_fine(void) ;

// Quando arriva l'irq# differite la chiamata di ...
void FT5436_isr(void) ;

size_t FT5436_punti(void /*FT5436_TOUCH*/ *) ;

// Interfaccia
// ===================================
// I2C
extern bool bsp_touch_scrivi(
    void * dati,
    size_t dim) ;
extern bool bsp_touch_leggi(
    void * dati,
    size_t dim) ;

extern void bsp_touch_reset(bool alto) ;
extern void bsp_millipausa(uint32_t) ;

#else
#   warning ft5436.h incluso
#endif
