#define STAMPA_DBG
#include "utili.h"
#include "fpga.h"

/*
 * Xilinx XC7S6
 */

#define SIZE_BLOCK_BIN_FPGA         10240
#define TIMEOUT_MS                  2000

// Servono pin per la programmazione
extern void fpga_program_b(bool) ;
extern bool fpga_init_b(void) ;
extern bool fpga_done(void) ;

static bool iniz = false ;

static bool prog_blocco(
    const uint8_t * dati,
    uint32_t dim)
{
    bool esito = false ;

    if ( !fpga_spi_tx(CONST_CAST(dati), dim) ) {
        DBG_ERR ;
    }
    else {
        esito = true ;
    }

    return esito ;
}

// UG470, Figure 2-3: Serial Configuration Clocking Sequence
static bool programma(
    const uint8_t * dati,
    uint32_t dim)
{
#ifdef DBG_ABIL
    uint32_t durata = HAL_GetTick() ;
#endif
    // Clear Configuration Memory (Step 2, Initialization) (TPROGRAM = 250 ns)
    fpga_program_b(false) ;
    HAL_Delay(1 + 1) ;
    fpga_program_b(true) ;

    // Attesa che INIT_B vada alto (TPL = 5.00 ms)
    for ( int i = 0 ; i < TIMEOUT_MS ; i++ ) {
        HAL_Delay(1) ;
        if ( fpga_init_b() ) {
            break ;
        }
    }

    if ( !fpga_init_b() ) {
        DBG_ERR ;
        return false ;
    }

    // Sample Mode Pins (Step 3): ora fpga e' in ascolto

    // Synchronization (Step 4): inutile con slave serial

    // Check Device ID (Step 5): nel binario?

    // Load Configuration Data Frames (Step 6)
    do {
        uint32_t dim_corr = MINI(dim, SIZE_BLOCK_BIN_FPGA) ;
        if ( !prog_blocco(dati, dim_corr) ) {
            break ;
        }

        dati += dim_corr ;
        dim -= dim_corr ;
    } while ( dim ) ;

    // Startup (Step 8) + attendo DONE alto
    for ( int i = 0 ; i < TIMEOUT_MS ; i++ ) {
        uint8_t clk ;
        fpga_spi_tx( &clk, sizeof(clk) ) ;
        if ( fpga_done() ) {
            break ;
        }
    }

    if ( !fpga_done() ) {
        DBG_ERR ;
        return false ;
    }

    // A conservative number for the clock cycles required after DONE is 24
    uint8_t clk[3] = {
        0
    } ;
    fpga_spi_tx( clk, sizeof(clk) ) ;

#ifdef DBG_ABIL
    durata = HAL_GetTick() - durata ;

    DBG_PRINTF("FPGA ok %u\n", durata) ;
#endif
    return true ;
}

bool FPGA_iniz(
    const uint8_t * dati,
    uint32_t dim)
{
    iniz = programma(dati, dim) ;

    return iniz ;
}

bool FPGA_leggi(
    uint8_t ind,
    uint16_t * p)
{
    bool esito = false ;

    if ( !iniz ) {
        DBG_ERR ;
    }
    else if ( NULL == p ) {
        DBG_ERR ;
    }
    else {
        esito = fpga_i2c_leggi(ind, p) ;
    }

    return esito ;
}

bool FPGA_scrivi(
    uint8_t ind,
    uint16_t val)
{
    bool esito = false ;

    if ( !iniz ) {
        DBG_ERR ;
    }
    else {
        esito = fpga_i2c_scrivi(ind, val) ;
    }

    return esito ;
}
