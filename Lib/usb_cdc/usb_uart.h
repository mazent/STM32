#ifndef USB_DEVICE_USB_UART_H_
#define USB_DEVICE_USB_UART_H_

#include <stdbool.h>
#include <stdint.h>

#include "usb_cdc_cfg.h"

#ifdef USB_UART_NO_CIRCO
// Ecco cosa ho ricevuto
typedef void (*USBU_CALLBACK)(void *, void *, uint32_t) ;
#else
// I dati ricevuti vengono salvati in un buffer circolare:
// leggerli con USBU_rx
typedef void (*USBU_CALLBACK)(void *) ;
#endif

// Callback invocata quando c'e' qualcosa da leggere
// (ricevono arg per primo)
bool USBU_iniz(
    USBU_CALLBACK rx,
    void * arg) ;

#ifndef USB_UART_NO_CIRCO
// Quando la cb rx viene invocata, usare questa per recuperare i dati
uint32_t USBU_rx(
    void * rx,
    uint32_t dim) ;
#endif

// usare questa per trasmettere
bool USBU_tx(const void *, uint32_t) ;

#else
#   warning usb_uart.h incluso
#endif
