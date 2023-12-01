#ifndef DISPI_MCP23S17_MCP23S17_H_
#define DISPI_MCP23S17_MCP23S17_H_

#include <stdbool.h>
#include <stdint.h>

#define MCP_BASE_IND     0x40

// Registri
#define MCP_IODIRA      0x00
#define MCP_IODIRB      0x01
#define MCP_IPOLA       0x02
#define MCP_IPOLB       0x03
#define MCP_GPINTENA    0x04
#define MCP_GPINTENB    0x05
#define MCP_DEFVALA     0x06
#define MCP_DEFVALB     0x07
#define MCP_INTCONA     0x08
#define MCP_INTCONB     0x09
#define MCP_IOCON       0x0A
#define MCP_IOCON_      0x0B
#define MCP_GPPUA       0x0C
#define MCP_GPPUB       0x0D
#define MCP_INTFA       0x0E
#define MCP_INTFB       0x0F
#define MCP_INTCAPA     0x10
#define MCP_INTCAPB     0x11
#define MCP_GPIOA       0x12
#define MCP_GPIOB       0x13
#define MCP_OLATA       0x14
#define MCP_OLATB       0x15

#define MCP_IOCON_BANK      (1 << 7)
#define MCP_IOCON_MIRROR    (1 << 6)
#define MCP_IOCON_SEQOP     (1 << 5)
#define MCP_IOCON_DISSLW    (1 << 4)
// Non mettere 0!
#define MCP_IOCON_HAEN      (1 << 3)
#define MCP_IOCON_ODR       (1 << 2)
#define MCP_IOCON_INTPOL    (1 << 1)

// Invocare ogni volta che si resetta
bool MCP_iniz(void) ;

// per entrambe, dim = 1 | 2
bool MCP_leggi(
    uint8_t ind,
    uint8_t reg,
    void * buf,
    uint8_t dim) ;
bool MCP_scrivi(
    uint8_t ind,
    uint8_t reg,
    const void * buf,
    uint8_t dim) ;

// Interfaccia verso spi
// Mode 0,0 (o 1,1)
// 10 MHz (maximum)
bool mcp_spi_tx(void *, uint16_t) ;
bool mcp_spi_txrx(void */*tx*/, void */*rx*/, uint16_t) ;

#else
#   warning mcp23s17.h incluso
#endif
