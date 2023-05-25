#ifndef TRAM_H_
#define TRAM_H_

/******************************************
    Test della ram
******************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    RDB_8_BIT,
    RDB_16_BIT,
    RDB_32_BIT
} RAM_DATA_BUS ;

// Se il bus dati e' a posto ...
// Torna 0 se ok, altrimenti il primo walking uno fallito
uint32_t TRAM_DataWalk1(uint32_t Base, RAM_DATA_BUS) ;
// Torna 0 se ok, altrimenti il primo walking zero fallito
uint32_t TRAM_DataWalk0(uint32_t Base, RAM_DATA_BUS) ;
// ... e il bus indirizzi non ha linee "stecche"
// Torna 0 se ok, altrimenti il primo walking uno fallito
uint32_t TRAM_AddrWalk1(uint32_t Base, uint32_t numByte, RAM_DATA_BUS) ;
// Torna 0 se ok, altrimenti il primo walking zero fallito
uint32_t TRAM_AddrWalk0(uint32_t Base, uint32_t numByte, RAM_DATA_BUS) ;
// ... allora si controlla l'accesso al dispositivo scrivendolo e rileggendo
bool TRAM_accedi(uint32_t Base, uint32_t numByte, RAM_DATA_BUS) ;

#ifdef __cplusplus
}
#endif

#endif
