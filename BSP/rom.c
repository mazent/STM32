#define STAMPA_DBG
#include "utili.h"
#include "rom.h"
#include "stm32h7xx_hal.h"

#ifdef __ICCARM__
#pragma segment=".intvec"
#pragma segment="ALTRO"
static uint8_t * INIZIO_IO = __section_begin(".intvec") ;
static uint8_t * INIZIO_ALTRO = __section_begin("ALTRO") ;
static uint8_t * FINE_ALTRO = __section_end("ALTRO") ;
static uint32_t DIM_ALTRO = __section_size("ALTRO") ;
#else
static uint8_t * INIZIO_IO = NULL ;
static uint8_t * INIZIO_ALTRO = NULL ;
static uint8_t * FINE_ALTRO = NULL ;
static uint32_t DIM_ALTRO = 0 ;
#endif

static const uint32_t MAX_SETTORE = FLASH_SECTOR_7 ;
#define DIM_RIGA        ( FLASH_NB_32BITWORD_IN_FLASHWORD * sizeof(uint32_t) )

uint32_t ROM_indirizzo(uint32_t cosa)
{
    uint32_t indir = 0 ;

    const uint8_t * INIZIO = MINI(INIZIO_IO, INIZIO_ALTRO) ;
    if ( cosa <= MAX_SETTORE ) {
        // Hanno passato il settore
        indir = UINTEGER(INIZIO + cosa * DIM_SETTORE_ROM_B) ;
    }
    else {
        // Hanno passato l'indirizzo ...
        if ( cosa < UINTEGER(INIZIO) ) {
            DBG_ERR ;
        }
        else {
            uint32_t settore = cosa - UINTEGER(INIZIO) ;
            settore /= DIM_SETTORE_ROM_B ;
            if ( settore > MAX_SETTORE ) {
                DBG_ERR ;
            }
            else {
                indir = cosa ;
            }
        }
    }

    return indir ;
}

bool ROM_cancella(const uint32_t cosa)
{
    bool esito = false ;
    const uint8_t * INIZIO = MINI(INIZIO_IO, INIZIO_ALTRO) ;

    HAL_FLASH_Unlock() ;

    do {
        // Trovo questi valori
        uint32_t indir ;
        uint32_t settore ;
        if ( cosa <= MAX_SETTORE ) {
            // Hanno passato il settore ...
            settore = cosa ;
            // ... calcolo indirizzo
            indir = UINTEGER(INIZIO + settore * DIM_SETTORE_ROM_B) ;
        }
        else {
            // Hanno passato un indirizzo ...
            indir = cosa ;
            // ... calcolo il settore
            if ( cosa < UINTEGER(INIZIO) ) {
                DBG_ERR ;
                break ;
            }
            settore = indir - UINTEGER(INIZIO) ;
            settore /= DIM_SETTORE_ROM_B ;
            if ( settore > MAX_SETTORE ) {
                DBG_ERR ;
                break ;
            }
        }

        // Cancello l'altro
        if ( indir < UINTEGER(INIZIO_ALTRO) ) {
            DBG_ERR ;
            break ;
        }

        // Procedo
        uint32_t SectorError ;
        FLASH_EraseInitTypeDef ei = {
            .TypeErase = FLASH_TYPEERASE_SECTORS,
            .Banks = FLASH_BANK_1,
            .Sector = settore,
            .NbSectors = 1,
            .VoltageRange = FLASH_VOLTAGE_RANGE_4
        } ;
        if ( HAL_OK != HAL_FLASHEx_Erase(&ei, &SectorError) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    HAL_FLASH_Lock() ;

    return esito ;
}

bool ROM_scrivi(
    const void * rom,
    const uint32_t * otto)
{
    bool esito = false ;

    static_assert(DIM_RIGA_ROM_B == DIM_RIGA, "OKKIO") ;

    HAL_FLASH_Unlock() ;

    do {
        // Scrivo l'altro
        if ( UINTEGER(rom) < UINTEGER(INIZIO_ALTRO) ) {
            DBG_ERR ;
            break ;
        }
        if ( UINTEGER(rom) > (UINTEGER(FINE_ALTRO) - DIM_RIGA) ) {
            DBG_ERR ;
            break ;
        }

        // Allineato?
        if ( UINTEGER(rom) & (DIM_RIGA - 1) ) {
            DBG_ERR ;
            break ;
        }

        // Procedo
        if ( HAL_OK !=
             HAL_FLASH_Program( FLASH_TYPEPROGRAM_FLASHWORD, UINTEGER(rom),
                                UINTEGER(otto) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    HAL_FLASH_Lock() ;

    return esito ;
}

void * ROM_altro_ind(void)
{
    return INIZIO_ALTRO ;
}

uint32_t ROM_altro_dim(void)
{
    return DIM_ALTRO ;
}
