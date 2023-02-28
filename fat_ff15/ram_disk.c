#define STAMPA_DBG
#include "utili.h"

#include "ff.h"
#include "diskio.h"
#include "bsp.h"

#define RAM_DIM_SEC      512
#define RAM_NUM_SEC      (SDRAM_BYTES / RAM_DIM_SEC)

//#define STAMPA_ROBA

DSTATUS RD_initialize(void)
{
    DBG_FUN ;
    return 0 ;
}

DSTATUS RD_status(void)
{
    DBG_FUN ;
    return 0 ;
}

DRESULT RD_read(
    BYTE * buff,
    LBA_t sector,
    UINT count)
{
    DBG_PRINTF("%s(%u,%u)", __func__, sector, count) ;

    if ( sector + count >= RAM_NUM_SEC ) {
        DBG_ERR ;
        return RES_PARERR ;
    }
    BYTE * srg = POINTER(SDRAM_ADDR + sector * RAM_DIM_SEC) ;
    const size_t DIM = RAM_DIM_SEC * count ;
    memcpy(buff, srg, DIM) ;
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("\t", buff, DIM) ;
#endif
    return RES_OK ;
}

DRESULT RD_write(
    const BYTE * buff,
    LBA_t sector,
    UINT count)
{
    DBG_PRINTF("%s(%u,%u)", __func__, sector, count) ;

    if ( sector + count >= RAM_NUM_SEC ) {
        DBG_ERR ;
        return RES_PARERR ;
    }
    BYTE * dst = POINTER(SDRAM_ADDR + sector * RAM_DIM_SEC) ;
    const size_t DIM = RAM_DIM_SEC * count ;
    memcpy(dst, buff, DIM) ;
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("\t", buff, DIM) ;
#endif
    return RES_OK ;
}

DRESULT RD_ioctl(
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
        *(DWORD *) buff = RAM_NUM_SEC ;
        res = RES_OK ;
        break ;

    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE:
        DBG_PRINTF("%s(GET_SECTOR_SIZE)", __func__) ;
        *(WORD *) buff = RAM_DIM_SEC ;
        res = RES_OK ;
        break ;

    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE:
        *(DWORD *) buff = 1 ;
        res = RES_OK ;
        break ;

    case CTRL_TRIM: {
            LBA_t sec[2] ;

            memcpy( sec, buff, sizeof(sec) ) ;

            DBG_PRINTF("%s(CTRL_TRIM, %u, %u)", __func__, sec[0], sec[1]) ;

                res = RES_OK ;
        }
        break ;

    default:
        DBG_PRINTF("%s(%u)", __func__, (unsigned) cmd) ;
        res = RES_PARERR ;
        break ;
    }

    return res ;
}
