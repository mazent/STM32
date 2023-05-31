//#define STAMPA_DBG
#include "utili.h"
#include "mmc.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"

/*
 * https://linux.codingbelief.com/zh/storage/flash_memory/emmc/emmc_commands.html
 */

static MMC_HandleTypeDef sdmmc = {
    .Instance = SDMMC1,
    .Init = {
        .ClockEdge = SDMMC_CLOCK_EDGE_RISING,
        .ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE,
        .BusWide = SDMMC_BUS_WIDE_8B,
        .HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE,
        .ClockDiv = 1,
    }
} ;

//#define STAMPA_ROBA         1

typedef enum {
    OCCUPATO,
    FINE,
    ERRORE
} STATO ;

static STATO stt ;

void MMC_cid(uint8_t * cid16)
{
	uint8_t * p = cid16 ;
    uint32_t tmp = __REV(sdmmc.CID[0]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CID[1]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CID[2]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CID[3]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
}

void MMC_csd(uint8_t * csd16)
{
    uint8_t * p = csd16 ;
    uint32_t tmp = __REV(sdmmc.CSD[0]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CSD[1]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CSD[2]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    tmp = __REV(sdmmc.CSD[3]) ;
    memcpy_( p, &tmp, sizeof(uint32_t) ) ;
}

#ifdef DBG_ABIL

static int meseanno(
    uint8_t md,
    char * * cmese)
{
    uint8_t mese = md >> 4 ;
    *cmese = "?" ;
    switch ( mese ) {
    case 1:
        *cmese = "gen" ;
        break ;
    case 2:
        *cmese = "feb" ;
        break ;
    case 3:
        *cmese = "mar" ;
        break ;
    case 4:
        *cmese = "apr" ;
        break ;
    case 5:
        *cmese = "mag" ;
        break ;
    case 6:
        *cmese = "giu" ;
        break ;
    case 7:
        *cmese = "lug" ;
        break ;
    case 8:
        *cmese = "ago" ;
        break ;
    case 9:
        *cmese = "set" ;
        break ;
    case 10:
        *cmese = "ott" ;
        break ;
    case 11:
        *cmese = "nov" ;
        break ;
    case 12:
        *cmese = "dic" ;
        break ;
    }
    return 2013 + (md & 0x0F) ;
}

#endif

bool MMC_iniz(void)
{
    if ( HAL_MMC_Init(&sdmmc) != HAL_OK ) {
        return false ;
    }
#ifdef DBG_ABIL
    {
        uint8_t cid[16] ;

        MMC_cid(cid);

        DBG_PRINT_HEX("CID", cid, 16) ;
        // [127:120]
        DBG_PRINTF("\t Manufacturer ID:       %02X", cid[0]) ;
        // [111:104]
        DBG_PRINTF("\t OEM/Application ID:    %02X", cid[2]) ;
        // [103:56]
        char pn[6 + 1] ;
        memcpy(pn, cid + 3, 6) ;
        pn[6] = 0 ;
        DBG_PRINTF("\t Product Name part:     %s", pn) ;
        // [55:48]
        DBG_PRINTF("\t Product Revision:      %02X", cid[9]) ;
        // [47:16]
        uint32_t sn ;
        memcpy(&sn, cid + 10, 4) ;
        DBG_PRINTF("\t Product Serial Number: %08X", sn) ;
        // [15:8]
        char * cmese ;
        int anno = meseanno(cid[14], &cmese) ;
        DBG_PRINTF("\t Manufacturing Date:    %02X -> %s %d",
                   cid[14],
                   cmese,
                   anno) ;
    }

    DBG_PUTS("CSD") ;
    DBG_PRINTF("\t %08X", sdmmc.CSD[0]) ;
    DBG_PRINTF("\t %08X", sdmmc.CSD[1]) ;
    DBG_PRINTF("\t %08X", sdmmc.CSD[2]) ;
    DBG_PRINTF("\t %08X", sdmmc.CSD[3]) ;
    {
        uint8_t csd[16] ;
        uint8_t * p = csd ;
        memcpy_( p, &sdmmc.CSD[3], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CSD[2], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CSD[1], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CSD[0], sizeof(uint32_t) ) ;
        DBG_PRINT_HEX("\t", csd, 16) ;
    }
#endif
    return true ;
}

bool MMC_leggi(
    uint32_t blocco,
    uint8_t * dati)
{
    bool esito = false ;

    do {
        stt = OCCUPATO ;
        if ( HAL_MMC_ReadBlocks_DMA(&sdmmc, dati, blocco, 1) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        for ( int i = 0 ; i < 1000 ; ++i ) {
            if ( FINE == stt ) {
                esito = true ;
#ifdef STAMPA_ROBA
                DBG_PRINT_HEX("mmc -> ", dati, MMC_BLOCKSIZE) ;
#endif
                break ;
            }
            else if ( ERRORE == stt ) {
                DBG_ERR ;
                break ;
            }
            HAL_Delay(1) ;
        }
    } while ( false ) ;

    return esito ;
}

bool MMC_scrivi(
    uint32_t blocco,
    uint8_t * dati)
{
    bool esito = false ;

    do {
        stt = OCCUPATO ;
        if ( HAL_MMC_WriteBlocks_DMA(&sdmmc, dati, blocco, 1) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        for ( int i = 0 ; i < 1000 ; ++i ) {
            if ( FINE == stt ) {
                esito = true ;
#ifdef STAMPA_ROBA
                DBG_PRINT_HEX("mmc <- ", dati, MMC_BLOCKSIZE) ;
#endif
                break ;
            }
            else if ( ERRORE == stt ) {
                DBG_ERR ;
                break ;
            }
            HAL_Delay(1) ;
        }
    } while ( false ) ;

    return esito ;
}

bool MMC_trim(
    uint32_t da,
    uint32_t a)
{
    bool esito = false ;

    do {
        if ( HAL_OK != HAL_MMC_EraseSequence(&sdmmc, HAL_MMC_TRIM, da, a) ) {
            DBG_ERR ;
            break ;
        }

        while ( true ) {
            HAL_MMC_CardStateTypeDef cs = HAL_MMC_GetCardState(&sdmmc) ;
            if ( sdmmc.ErrorCode ) {
                DBG_ERR ;
            }
            else {
                if ( HAL_MMC_CARD_PROGRAMMING == cs ) {
                    HAL_Delay(10) ;
                }
                else {
                    DBG_PRINTF("%s -> %08X", __func__, cs) ;
                    break ;
                }
            }
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef * hmmc)
{
    INUTILE(hmmc) ;
    stt = FINE ;
}

void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef * hmmc)
{
    INUTILE(hmmc) ;
    stt = FINE ;
}

void HAL_MMC_ErrorCallback(MMC_HandleTypeDef * hmmc)
{
    INUTILE(hmmc) ;
    stt = ERRORE ;
}

void SDMMC1_IRQHandler(void)
{
    HAL_MMC_IRQHandler(&sdmmc) ;
}
