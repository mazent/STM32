#ifndef TRAM_H_
#define TRAM_H_

/******************************************

    Test della ram

******************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t TRAM_AddrWalk1(uint32_t Base, uint32_t numByte) ;
uint32_t TRAM_AddrWalk0(uint32_t Base, uint32_t numByte) ;
uint32_t TRAM_DataWalk1e0_16bit(uint32_t Base) ;

#ifdef __cplusplus
}
#endif


#endif
