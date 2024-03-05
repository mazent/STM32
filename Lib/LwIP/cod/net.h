#ifndef CM7_CORE_SRC_NET_H_
#define CM7_CORE_SRC_NET_H_

#include <stdbool.h>
#include <stdint.h>
#include "net_if.h"

extern void PHY_iniz(NET_MODO, NET_MDI) ;
extern void PHY_iniz_loopback(NET_MDI) ;
extern void PHY_isr(void) ;
extern bool PHY_link(void) ;

// i registri sono a 15 bit
#define ERRORE_REG_L        NEGA(0)

uint32_t reg_leggi(uint32_t reg) ;
bool reg_scrivi(
    uint32_t reg,
    uint32_t val) ;

extern void MAC_iniz(
    bool fullduplex,
    bool centomega) ;
extern void MAC_fine(void) ;

extern void netop_iniz(uint32_t) ;
extern void netop_fine(void) ;
extern void netop_ric(void) ;

#else
#   warning net.h incluso
#endif
