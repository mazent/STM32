#ifndef MIDDLEWARES_THIRD_PARTY_LWIP_PORT_COD_NET_MDMA_H_
#define MIDDLEWARES_THIRD_PARTY_LWIP_PORT_COD_NET_MDMA_H_

#include <stdbool.h>

// weak, cosi' inizializzate irq
void net_mdma_iniz(void) ;

bool net_mdma_rx_iniz(void) ;
bool net_mdma_rx_buf(
    void *,
    const void *,
    int) ;
bool net_mdma_rx(void) ;

bool net_mdma_tx_iniz(void) ;
bool net_mdma_tx_buf(
    void *,
    const void *,
    int) ;
bool net_mdma_tx(void) ;

// callback --------------------

// invocare dentro MDMA_IRQHandler
void net_mdma_irq(void) ;

void net_mdma_rx_esito(bool) ;
void net_mdma_tx_esito(bool) ;

#else
#   warning net_mdma.h incluso
#endif
