#define STAMPA_DBG
#include "utili.h"
#include "fpga.h"
#include "bsp.h"
#include "hash.h"

/*
 * Xilinx XC7S6
 *
 * L'immagine e' molto grande, per cui si usa la
 * ram esterna (bsp.h deve definire indirizzo e dimensione)
 *
 * SPI Mode 0,0 senza SS# (finta spi)
 */

#define SIZE_BLOCK_BIN_FPGA         10240
#define TIMEOUT_MS                  2000

// Servono pin per la programmazione
extern void fpga_program_b(bool) ;
extern bool fpga_init_b(void) ;
extern bool fpga_done(void) ;

static uint32_t dimimg ;

bool FPGA_copia(
    uint32_t pos,
    uint32_t dim,
    const void * srg)
{
    if ( pos + dim < SDRAM_BYTES ) {
        uint8_t * img = (uint8_t *) (SDRAM_ADDR + pos) ;
        memcpy(img, srg, dim) ;

        dimimg = pos + dim ;

        return true ;
    }

    return false ;
}

static bool calcola_sha1(
    const void * v,
    uint32_t dim,
    void * ris)
{
    bool esito = false ;

    if ( HASH_iniz() ) {
        esito = HASH_calcola(v, dim, ris) ;
        HASH_fine() ;
    }

    return esito ;
}

bool FPGA_sha(
    uint32_t dim,
    void * sha)
{
    bool esito = false ;

    if ( dim > SDRAM_BYTES ) {
        DBG_ERR ;
    }
    else {
        uint8_t * img = (uint8_t *) SDRAM_ADDR ;

        esito = calcola_sha1(img, dim, sha) ;
    }

    return esito ;
}

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
bool FPGA_config(void)
{
    bool esito = false ;
    uint8_t * dati = (uint8_t *) SDRAM_ADDR ;
#ifdef DBG_ABIL
    uint32_t durata = HAL_GetTick() ;
#endif
    // Clear Configuration Memory (Step 2, Initialization) (TPROGRAM = 250 ns)
    fpga_program_b(false) ;
    HAL_Delay(1 + 1) ;
    fpga_program_b(true) ;

    do {
        // Attesa che INIT_B vada alto (TPL = 5.00 ms)
        for ( int i = 0 ; i < TIMEOUT_MS ; i++ ) {
            HAL_Delay(1) ;
            if ( fpga_init_b() ) {
                break ;
            }
        }

        // Ancora una volta
        if ( !fpga_init_b() ) {
            DBG_ERR ;
            break ;
        }

        // Sample Mode Pins (Step 3): ora fpga e' in ascolto

        // Synchronization (Step 4): inutile con slave serial

        // Check Device ID (Step 5): nel binario?

        // Load Configuration Data Frames (Step 6)
        do {
            uint32_t dim_corr = MINI(dimimg, SIZE_BLOCK_BIN_FPGA) ;
            if ( !prog_blocco(dati, dim_corr) ) {
                break ;
            }

            dati += dim_corr ;
            dimimg -= dim_corr ;
        } while ( dimimg ) ;

        if ( dimimg ) {
            break ;
        }

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
            break ;
        }

        // A conservative number for the clock cycles required after DONE is 24
        uint8_t clk[3] = {
            0
        } ;
        fpga_spi_tx( clk, sizeof(clk) ) ;

        esito = true ;
    } while ( false ) ;
#ifdef DBG_ABIL
    durata = HAL_GetTick() - durata ;

    DBG_PRINTF("FPGA %s %u\n", esito ? "OK" : "ERR", durata) ;
#endif
    return esito ;
}
