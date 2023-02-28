/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"         /* Obtains integer types */
#include "diskio.h"     /* Declarations of disk functions */
#include "ram_disk.h"
#include "mmc_disk.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM     0   /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv       /* Physical drive nmuber to identify the drive */
                    )
{
    switch ( pdrv ) {
#ifdef DEV_RAM
    case DEV_RAM:
        return RD_status() ;
#endif
#ifdef DEV_MMC
    case DEV_MMC:
        return MMC_status() ;
#endif
#ifdef DEV_USB
    case DEV_USB:
        result = USB_disk_status() ;

        // translate the reslut code here

        return stat ;
#endif
    }
    return STA_NODISK ;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv               /* Physical drive nmuber to identify the drive */
                        )
{
    switch ( pdrv ) {
#ifdef DEV_RAM
    case DEV_RAM:
        return RD_initialize() ;
#endif
#ifdef DEV_MMC
        case DEV_MMC :
		return MMC_initialize();
#endif
//	case DEV_USB :
//		result = USB_disk_initialize();
//
//		// translate the reslut code here
//
//		return stat;
    }
    return STA_NOINIT ;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE * buff,    /* Data buffer to store read data */
    LBA_t sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
    )
{
    switch ( pdrv ) {
#ifdef DEV_RAM
    case DEV_RAM:
        return RD_read(buff, sector, count) ;
#endif
#ifdef DEV_MMC
	case DEV_MMC :
		return MMC_read(buff, sector, count);
#endif
//	case DEV_USB :
//		// translate the arguments here
//
//		result = USB_disk_read(buff, sector, count);
//
//		// translate the reslut code here
//
//		return res;
    }

    return RES_PARERR ;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE * buff,  /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
    )
{
    switch ( pdrv ) {
#ifdef DEV_RAM
    case DEV_RAM:
        return RD_write(buff, sector, count) ;
#endif
#ifdef DEV_MMC
	case DEV_MMC :
		return MMC_write(buff, sector, count);
#endif
//	case DEV_USB :
//		// translate the arguments here
//
//		result = USB_disk_write(buff, sector, count);
//
//		// translate the reslut code here
//
//		return res;
    }

    return RES_PARERR ;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void * buff      /* Buffer to send/receive control data */
    )
{
    switch ( pdrv ) {
#ifdef DEV_RAM
    case DEV_RAM:
        return RD_ioctl(cmd, buff) ;
#endif
#ifdef DEV_MMC
	case DEV_MMC :
		return MMC_ioctl(cmd, buff) ;;
#endif
//	case DEV_USB :
//
//		// Process of the command the USB drive
//
//		return res;
    }

    return RES_PARERR ;
}
