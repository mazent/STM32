#define STAMPA_DBG
#include "utili.h"

#include "ff.h"
#include "diskio.h"
#include "mmc/mmc.h"

#define MMC_DIM_SEC      512
#if 0
#define MMC_GPP_SEC      17408
#define MMC_BYTES        7818182656
#else
#define MMC_GPP_SEC      0
#define MMC_BYTES        (1024 * 1024 * 1024)
#endif
#define MMC_NUM_SEC      (MMC_BYTES / MMC_DIM_SEC)

//#define STAMPA_ROBA

DSTATUS MMC_initialize(void)
{
    DBG_FUN ;
    return 0 ;
}

DSTATUS MMC_status(void)
{
    DBG_FUN ;
    return 0 ;
}

DRESULT MMC_read(
    BYTE * buff,
    LBA_t sector,
    UINT count)
{
    UINT i ;
    uint8_t * dst = buff ;

    DBG_PRINTF("%s(%u,%u)", __func__, sector, count) ;

    for ( i = 0 ; i < count ; i++ ) {
        if ( !MMC_leggi(sector, dst) ) {
            DBG_ERR ;
            break ;
        }

        dst += MMC_BLOCKSIZE ;
        sector++ ;
    }

#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("\t", buff, DIM) ;
#endif

    return i == count ? RES_OK : RES_ERROR ;
}

DRESULT MMC_write(
    const BYTE * buff,
    LBA_t sector,
    UINT count)
{
    UINT i ;
    uint8_t * srg = CONST_CAST(buff) ;

    DBG_PRINTF("%s(%u,%u)", __func__, sector, count) ;

    for ( i = 0 ; i < count ; i++ ) {
        if ( !MMC_scrivi(sector, srg) ) {
            DBG_ERR ;
            break ;
        }

        srg += MMC_BLOCKSIZE ;
        sector++ ;
    }

#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("\t", buff, DIM) ;
#endif
    return i == count ? RES_OK : RES_ERROR ;
}

DRESULT MMC_ioctl(
    BYTE cmd,
    void * buff)
{
    DRESULT res = RES_ERROR ;

    switch ( cmd ) {
    /* Make sure that no pending write process */
    case CTRL_SYNC:
        DBG_PRINTF("%s(CTRL_SYNC)", __func__) ;
        res = RES_OK ;
        break ;

    /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT:
        DBG_PRINTF("%s(GET_SECTOR_COUNT)", __func__) ;
        *(DWORD *) buff = MMC_NUM_SEC ;
        res = RES_OK ;
        break ;

    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE:
        DBG_PRINTF("%s(GET_SECTOR_SIZE)", __func__) ;
        *(WORD *) buff = MMC_DIM_SEC ;
        res = RES_OK ;
        break ;

    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE:
        DBG_PRINTF("%s(GET_BLOCK_SIZE)", __func__) ;
        *(DWORD *) buff = 1 ;
        res = RES_OK ;
        break ;

    case CTRL_TRIM: {
            LBA_t sec[2] ;

            memcpy( sec, buff, sizeof(sec) ) ;

            DBG_PRINTF("%s(CTRL_TRIM, %u, %u)", __func__, sec[0], sec[1]) ;

            if ( MMC_trim(sec[0], sec[1]) ) {
                res = RES_OK ;
            }
        }
        break ;

    default:
        DBG_PRINTF("%s(%u)", __func__, (unsigned) cmd) ;
        res = RES_PARERR ;
        break ;
    }

    return res ;
}
