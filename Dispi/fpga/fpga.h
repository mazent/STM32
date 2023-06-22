#ifndef APP_FPGA_H_
#define APP_FPGA_H_

#include <stdbool.h>
#include <stdint.h>

// 1: trasferire l'immagine
bool FPGA_copia(
    uint32_t pos,
    uint32_t dim,
    const void * /*srg*/) ;

// 2: calcolare sha (opzionale)
#define FPGA_SHA_DIM         20
bool FPGA_sha(
    uint32_t dim,
    void * /*mem*/) ;

// 3: installare l'immagine
bool FPGA_config(void) ;

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
