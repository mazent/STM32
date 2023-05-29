#define STAMPA_DBG
#include "utili.h"
#include "fpga.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"
#include "i2c.h"
#include "hash.h"

extern SPI_HandleTypeDef hspi4 ;

// Qui c'e' la roba
#define DIM_BIT     (32 * 1024)

__attribute__( ( section(".fpga") ) )
static uint8_t bit[DIM_BIT] ;

static uint32_t dimbit ;

static bool fine_tx ;

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef * hspi)
{
    UNUSED(hspi) ;
    fine_tx = true ;
    DBG_ERR ;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi)
{
    UNUSED(hspi) ;
    fine_tx = true ;
}

bool FPGA_copia(
    uint32_t pos,
    uint32_t dim,
    const void * srg)
{
    if ( pos + dim < DIM_BIT ) {
        memcpy_(bit + pos, srg, dim) ;
        dimbit = pos + dim ;
        return true ;
    }

    return false ;
}

bool FPGA_sha(
    uint32_t dim,
    void * mem)
{
    if ( dim > DIM_BIT ) {
        DBG_ERR ;
        return false ;
    }

    return HASH_calcola(bit, dim, mem) ;
}

bool FPGA_config(void)
{
    // robe interne in reset
    areset(false) ;

    // Minimum CRESET_B Low	pulse width required to	restart configuration 200 ns
    ss(false) ;
    creset(false) ;
    HAL_Delay(2) ;

    // If the SPI_SS pin is sampled as a logic ‘0’ (Low),
    // then the device waits to be configured

    // POR/CRESET_B to Device I/O Active
    // iCE40LP/HX1K - Low Frequency (Default) 53 ms
    creset(true) ;
    HAL_Delay(100) ;

    // toggle SPI_SS to high, send 8 dummy clocks, then toggle it back to low
    ss(true) ;
#if 0
    // Non serve ...
    fine_tx = false ;
    uint8_t x = 0 ;
    if ( HAL_OK != HAL_SPI_Transmit_DMA(&hspi4, &x, 1) ) {
        DBG_ERR ;
        return false ;
    }
    while ( !fine_tx ) {
        HAL_Delay(1) ;
    }
#else
    // ... basta una pausa
    HAL_Delay(2) ;
#endif
    ss(false) ;

    // start sending the FPGA bitmap to	the iCE40 device
    fine_tx = false ;
    if ( HAL_OK != HAL_SPI_Transmit_DMA(&hspi4, CONST_CAST(bit), dimbit) ) {
        DBG_ERR ;
        return false ;
    }
    while ( !fine_tx ) {
        HAL_Delay(1) ;
    }

    ss(true) ;

    // Wait for 100 clocks
#if 0
    // Non serve ...
    fine_tx = false ;
    uint8_t y[(100 + 7) / 8] ;
    if ( HAL_OK != HAL_SPI_Transmit_DMA( &hspi4, y, sizeof(y) ) ) {
        DBG_ERR ;
        return false ;
    }
    while ( !fine_tx ) {
        HAL_Delay(1) ;
    }
#else
    // ... basta una pausa
    HAL_Delay(10) ;
#endif
    // Monitor the CDONE pin. It should go to high
    for ( int i = 0 ; i < 100 && !cdone() ; ++i ) {
        HAL_Delay(1) ;
    }

    if ( cdone() ) {
        // After the CDONE output pin goes High, send at least 49 additional
        // dummy bits, effectively 49 additional SPI_SCK clock cycles measured
        // from rising-edge to rising-edge
        fine_tx = false ;
        if ( HAL_OK !=
             HAL_SPI_Transmit_DMA(&hspi4, CONST_CAST(bit),
                                  (49 + 7) / 8) ) {
            DBG_ERR ;
            return false ;
        }
        while ( !fine_tx ) {
            HAL_Delay(1) ;
        }

        // robe interne
        HAL_Delay(10) ;
        areset(true) ;
        HAL_Delay(10) ;
        areset(false) ;
        HAL_Delay(10) ;
        areset(true) ;
    }

    return cdone() ;
}

bool FPGA_leggi(
    uint8_t ind,
    uint16_t * p)
{
    bool esito = false ;

    if ( NULL == p ) {
        DBG_ERR ;
    }
    else {
        esito = I2C_leggi(
            ind,
            p,
            sizeof(uint16_t) ) ;
    }

    return esito ;
}

bool FPGA_scrivi(
    uint8_t ind,
    uint16_t val)
{
    return I2C_scrivi(
               ind,
               &val,
               sizeof(uint16_t) ) ;
}
