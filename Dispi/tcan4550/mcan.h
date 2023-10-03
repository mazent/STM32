#ifndef MCAN_H_
#define MCAN_H_

/*
 * TCAN4550 viene gestito col codice bosch di mcan, per cui
 * uso questo nome
 */

#include <stdint.h>
#include <stdbool.h>

typedef void (*PF_MCAN_RESET_PIN)(bool) ;
typedef void (*PF_MCAN_ATTESA_US)(uint32_t) ;
typedef bool (*PF_MCAN_SPI_TX)(void *, uint16_t) ;
typedef bool (*PF_MCAN_SPI_TXRX)(void */*tx*/, void */*rx*/, uint16_t) ;

typedef enum {
    TCM_NORMALE,
    // TX -> RX <- pin rx, pin tx = 1
    TCM_BUS_MON,
    // TX -> RX, pin tx = 1
    TCM_INT_LBACK,
    // TX -> RX, TX -> pin tx
    TCM_EXT_LBACK,
    // riceve e da ack ma non trasmette
    TCM_RESTR_OP
} TCAN_MODO ;

typedef struct {
    int indice ;

    bool fd ;

    TCAN_MODO modo ;

    // Pilota il pin collegato a RST (vero->alto)
    PF_MCAN_RESET_PIN reset_pin ;

    // Attende per un tot di microsecondi
    PF_MCAN_ATTESA_US attesa_us ;

    // SPI
    // MODE 0 (CPOL = 0, CPHA = 0)
    // Max 18 MHz
    PF_MCAN_SPI_TX spi_tx ;
    PF_MCAN_SPI_TXRX spi_txrx ;
} UN_MCAN ;

typedef struct {
    uint8_t bs1 ;
    uint8_t bs2 ;
    uint8_t sjw ;
    uint32_t freqnt ;
} MCAN_CFG ;

// se data == NULL, non si usa fd
bool MCAN_iniz(
    UN_MCAN *,
    const MCAN_CFG * nominal,
    const MCAN_CFG * data) ;
void MCAN_fine(UN_MCAN *) ;

typedef struct {
    bool extended ;
    uint32_t id ;
} MCAN_TX ;

bool MCAN_tx(UN_MCAN *, const MCAN_TX *, const void *, uint8_t) ;

typedef struct {
    bool extended ;
    uint32_t id ;

    bool fd_format ;
    bool esi ;

    uint8_t dim ;
    uint8_t dati[64] ;
} MCAN_RX ;

// Preleva un pacchetto ricevuto
bool MCAN_rx(
    UN_MCAN *,
    MCAN_RX *) ;

// Invocare (in un task) quando capita interruzione
void MCAN_isr(UN_MCAN *) ;

// Le mie sono weak
void mcan_tx_fifo_empty_cb(uint8_t id);
void mcan_rx_fifo_msg_cb(uint8_t id);


// Debug
uint32_t * MCAN_reg_leggi(UN_MCAN *, uint16_t, uint16_t) ;
bool MCAN_reg_scrivi(UN_MCAN *, uint16_t, uint16_t, const uint32_t *) ;

#endif
