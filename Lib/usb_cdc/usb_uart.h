#ifndef USB_DEVICE_USB_UART_H_
#define USB_DEVICE_USB_UART_H_

#include <stdbool.h>
#include <stdint.h>
#include "usb_cdc_cfg.h"

typedef void (*USBU_CALLBACK)(void *) ;

// Callback invocata quando c'e' qualcosa da leggere
bool USBU_iniz(
    USBU_CALLBACK rx,
    void * arg) ;

// Quando la cb rx viene invocata, disaccoppiare e usare questa per recuperare i dati
#ifdef USB_UART_USA_BUF_CIRC
uint32_t USBU_rx(
    void * rx,
    uint32_t dim) ;
#else
void * USBU_rx(uint32_t * dim) ;
#endif

// Usare questa per trasmettere
bool USBU_tx(const void *, uint32_t) ;

#else
#   warning usb_uart.h incluso
#endif
