#ifndef BSP_BIP_H_
#define BSP_BIP_H_

bool BIP_iniz(void) ;
void BIP_fine(void) ;

// 0 spegne
void BIP_freq(uint16_t hz) ;

#else
#   warning bip.h incluso
#endif
