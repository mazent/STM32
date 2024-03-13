#ifndef DISPI_MAX17055_MAX17055_H_
#define DISPI_MAX17055_MAX17055_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool MAX17055_iniz(void) ;

// in .1 kelvin
bool MAX17055_temperature(void * p_uint16_t) ;

// in mV
bool MAX17055_voltage(void * p_uint16_t) ;

// in mA
bool MAX17055_current(void * p_int16_t) ;

// in percento
bool MAX17055_stateOfCharge(void * p_uint16_t) ;

// time count since last POR (in s)
bool MAX17055_timer(void * p_uint32_t) ;

// cycles with a 1% LSb (0 to 655.35 cycles)
bool MAX17055_cicli(void * p_uint16_t) ;

// Interfaccia verso I2C
// ================================
extern bool bsp_gg_scrivi(
    void * dati,
    size_t dim) ;

extern bool bsp_gg_leggi(
    void * dati,
    size_t dim) ;
extern void bsp_millipausa(uint32_t) ;

#else
#   warning max17055.h incluso
#endif
