#ifndef APP_FPGA_H_
#define APP_FPGA_H_

#include <stdbool.h>
#include <stdint.h>

// Caso 1:
// ==============================
// 1: trasferire l'immagine
bool FPGA_copia(
    uint32_t pos,
    uint32_t dim,
    const void *) ;

// 2: calcolare sha (opzionale)
bool FPGA_sha(
    uint32_t dim,
    void *) ;

// 3: installare l'immagine
bool FPGA_config1(void) ;

// Caso 2: XC7S6
// ==============================
bool FPGA_config2(const uint8_t *,uint32_t) ;

// **************************
// SPI
bool fpga_spi_tx(
    const void * v,
    uint16_t dim) ;
bool fpga_spi_tx_rx(
    const void * tx,
    void * rx,
    uint16_t dim) ;

#else
#   warning fpga.h incluso
#endif
