#ifndef TCAN4550_H_
#define TCAN4550_H_

#include <stdbool.h>
#include <stdint.h>

typedef bool (*PF_TCAN_SPI_TX)(void *, uint16_t) ;
typedef bool (*PF_TCAN_SPI_TXRX)(void *, void *, uint16_t) ;

#define TCAN_MAX_REG     256

bool TCAN_iniz(void) ;
void TCAN_fine(void) ;

// Legge numreg registri (max 256) a partire da primo
// Torna NULL se errore
uint32_t * TCAN_reg_leggi(PF_TCAN_SPI_TXRX, uint16_t numreg, uint16_t primo) ;

// Scrive su numreg registri (max 256) a partire da primo il contenuto di val
// se val == NULL, si scrive 0
bool TCAN_reg_scrivi(PF_TCAN_SPI_TX,
                     uint16_t numreg,
                     uint16_t primo,
                     const uint32_t * val) ;

#endif
