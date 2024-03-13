#ifndef BQ27427_H_
#define BQ27427_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// TI recommends that the host save Qmax and Ra table in the system NVM
// once the [QMAX_UP] and [RES_UP] bits have been set
//     mAh = Qmax x Design Capacity / 2^14
// Ricavati dal "Learning Cycle"

#define BLOCKDATA_DIM      32

typedef struct {
    int16_t Qmax ;

    uint8_t Ra[BLOCKDATA_DIM] ;
} BQ27427_RA ;

// Upon device reset, the contents of ROM are copied to associated volatile
// RAM-based data memory blocks. For proper operation, all parameters in RAM-based
// data memory require initialization.
// Se non sono disponibili i dati passare NULL
bool BQ27427_iniz(const BQ27427_RA * p) ;

// Alla fine del "Learning Cycle" recuperare i dati
// e salvarli da qualche parte
bool BQ27427_ra(BQ27427_RA * p) ;

// in .1 kelvin
bool BQ27427_temperature(void * p_uint16_t) ;

// in mV
bool BQ27427_voltage(void * p_uint16_t) ;

// in mA
bool BQ27427_averageCurrent(void * p_int16_t) ;

// in percento
bool BQ27427_stateOfCharge(void * p_uint16_t) ;

#define BQ27427_DEVICE_TYPE     0x0427

bool BQ27427_device_type(void * p_uint16_t) ;

#define BQ27427_CHEM_ID_4_35_V  3230
#define BQ27427_CHEM_ID_4_2_V   1202
#define BQ27427_CHEM_ID_4_4_V   3142

bool BQ27427_chem_id(void * p_uint16_t) ;

bool BQ27427_reset(void) ;

// Interfaccia verso I2C
// ================================
extern bool bsp_gg_scrivi(
    void * dati,
    size_t dim) ;

extern bool bsp_gg_leggi(
    void * dati,
    size_t dim) ;
// the host must not issue any standard command more than two times per second
extern void bsp_millipausa(uint32_t) ;

#else
#   warning bq27427.h incluso
#endif
