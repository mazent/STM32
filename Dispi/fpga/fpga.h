#ifndef FPGA_H_
#define FPGA_H_

#include <stdbool.h>
#include <stdint.h>

bool FPGA_iniz(
    const uint8_t * dati,
    uint32_t dim) ;

bool FPGA_leggi(uint8_t, uint16_t *) ;
bool FPGA_scrivi(uint8_t, uint16_t) ;

// Interfaccia verso spi
// ------------------------
// Mode 0,0
// ??? MHz (maximum)
bool fpga_spi_tx(void *, uint16_t) ;
bool fpga_spi_txrx(void */*tx*/, void */*rx*/, uint16_t) ;

// Interfaccia verso i2c
// ------------------------
bool fpga_i2c_scrivi(
    uint8_t ind,
    uint16_t val) ;
bool fpga_i2c_leggi(
    uint8_t ind,
    uint16_t * val) ;

#endif
