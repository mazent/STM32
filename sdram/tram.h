#ifndef TRAM_H_
#define TRAM_H_

/******************************************

    Test della ram

******************************************/

#ifdef __cplusplus
extern "C" {
#endif

DWORD TRAM_AddrWalk1(DWORD Base, DWORD numByte) ;
DWORD TRAM_AddrWalk0(DWORD Base, DWORD numByte) ;
DWORD TRAM_DataWalk1e0_16bit(DWORD Base) ;

#ifdef __cplusplus
}
#endif


#endif
