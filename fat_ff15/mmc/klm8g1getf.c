#define STAMPA_DBG
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

#define CID_MID_POS   (120 / 8)       // DIM 8  //Manufacturer ID  [127:120]
//#define CID_CBX_POS           // DIM 2    //Device/BGA  [113:112]
//#define CID_OID_POS   104     // DIM 8	//OEM/Application ID   [111:104]
#define CID_PNM_POS    (56 / 8)       // DIM 48	//Product name   [103:56]
#define CID_PNM_DIM    (48 / 8)
//#define CID_PRV_POS           // DIM 8	//Product revision   [55:48]
#define CID_PSN_POS     (16 / 8)      // DIM 32	//Product serial number   [47:16]
#define CID_PSN_DIM     (32 / 8)
#define CID_MDT_POS     (8 / 8)       // DIM 8	//Manufacturing date   [15:8]

void MMC_cid(uint8_t * cid16)
{
    uint8_t * p = cid16 ;
    memcpy_( p, &sdmmc.CID[3], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[2], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[1], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[0], sizeof(uint32_t) ) ;
}

void MMC_csd(uint8_t * csd16)
{
    uint8_t * p = csd16 ;
    memcpy_( p, &sdmmc.CSD[3], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[2], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[1], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[0], sizeof(uint32_t) ) ;
}

bool MMC_iniz(void)
{
    if ( HAL_MMC_Init(&sdmmc) != HAL_OK ) {
        return false ;
    }

#ifdef DBG_ABIL
    DBG_PUTS("CID") ;
    DBG_PRINTF("\t %08X", sdmmc.CID[0]) ;
    DBG_PRINTF("\t %08X", sdmmc.CID[1]) ;
    DBG_PRINTF("\t %08X", sdmmc.CID[2]) ;
    DBG_PRINTF("\t %08X", sdmmc.CID[3]) ;
    {
        uint8_t cid[16] ;
        uint8_t * p = cid ;
        memcpy_( p, &sdmmc.CID[3], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CID[2], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CID[1], sizeof(uint32_t) ) ;
        p += sizeof(uint32_t) ;
        memcpy_( p, &sdmmc.CID[0], sizeof(uint32_t) ) ;
        DBG_PRINT_HEX("\t", cid, 16) ;
        DBG_PRINTF("\t Manufacturer ID: %02X", cid[CID_MID_POS]) ;
        DBG_PRINT_HEX("\t Product name: ", &cid[CID_PNM_POS], CID_PNM_DIM) ;
        DBG_PRINT_HEX("\t Product serial number: ",
                      &cid[CID_PSN_POS],
                      CID_PSN_DIM) ;
        uint8_t md = cid[CID_MDT_POS] ;
        uint8_t mese = md >> 4 ;
        char * cmese = "?" ;
        switch ( mese ) {
        case 1:
            cmese = "gen" ;
            break ;
        case 2:
            cmese = "feb" ;
            break ;
        case 3:
            cmese = "mar" ;
            break ;
        case 4:
            cmese = "apr" ;
            break ;
        case 5:
            cmese = "mag" ;
            break ;
        case 6:
            cmese = "giu" ;
            break ;
        case 7:
            cmese = "lug" ;
            break ;
        case 8:
            cmese = "ago" ;
            break ;
        case 9:
            cmese = "set" ;
            break ;
        case 10:
            cmese = "ott" ;
            break ;
        case 11:
            cmese = "nov" ;
            break ;
        case 12:
            cmese = "dic" ;
            break ;
        }
        uint16_t anno = 2013 + (md & 0x0F) ;
        DBG_PRINTF("\t Manufacturing date: %s %d", cmese, anno) ;
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
