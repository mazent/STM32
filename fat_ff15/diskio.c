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


void stampa_fr(
    const char * t,
    FRESULT fr)
{
    const char * cr = "???" ;

    switch ( fr ) {
    case FR_OK:
        cr = "Succeeded" ;
        break ;
    case FR_DISK_ERR:
        cr = "A hard error occurred in the low level disk I/O layer " ;
        break ;
    case FR_INT_ERR:
        cr = "Assertion failed" ;
        break ;
    case FR_NOT_READY:
        cr = "The physical drive cannot work" ;
        break ;
    case FR_NO_FILE:
        cr = "Could not find the file" ;
        break ;
    case FR_NO_PATH:
        cr = "Could not find the path" ;
        break ;
    case FR_INVALID_NAME:
        cr = "The path name format is invalid" ;
        break ;
    case FR_DENIED:
        cr = "Access denied due to prohibited access or directory full" ;
        break ;
    case FR_EXIST:
        cr = "Access denied due to prohibited access" ;
        break ;
    case FR_INVALID_OBJECT:
        cr = "The file/directory object is invalid" ;
        break ;
    case FR_WRITE_PROTECTED:
        cr = "The physical drive is write protected" ;
        break ;
    case FR_INVALID_DRIVE:
        cr = "The logical drive number is invalid" ;
        break ;
    case FR_NOT_ENABLED:
        cr = "The volume has no work area" ;
        break ;
    case FR_NO_FILESYSTEM:
        cr = "There is no valid FAT volume" ;
        break ;
    case FR_MKFS_ABORTED:
        cr = "The f_mkfs() aborted due to any problem" ;
        break ;
    case FR_TIMEOUT:
        cr =
            "Could not get a grant to access the volume within defined period " ;
        break ;
    case FR_LOCKED:
        cr = "The operation is rejected according to the file sharing policy" ;
        break ;
    case FR_NOT_ENOUGH_CORE:
        cr = "LFN working buffer could not be allocated" ;
        break ;
    case FR_TOO_MANY_OPEN_FILES:
        cr = "Number of open files > FF_FS_LOCK " ;
        break ;
    case FR_INVALID_PARAMETER:
        cr = "Given parameter is invalid" ;
        break ;
    }

    DBG_PRINTF("%s -> %d == %s", t, fr, cr) ;
}

#if 0

// Un test semplice

#include "ff.h"     
static FATFS FatFs ;
static FIL Fil ;    


static void test(void * v)
{
    INUTILE(v) ;

    DBG_FUN ;

    FRESULT fr ;
    MKFS_PARM fsp = {
    	// quasi come quella predefinita
        .fmt = FM_FAT32 | FM_FAT,
    } ;
    static uint8_t wb[1000] ;

    int ciclo = 0 ;

    while ( true ) {
        char scritti[20] ;
        int nums ;
        char letti[20] ;

        fr = f_mount(&FatFs, "", 1) ;
        stampa_fr("f_mount", fr) ;
        if ( FR_OK != fr ) {
            fr = f_mkfs( "", &fsp, wb, sizeof(wb) ) ;
            stampa_fr("f_mkfs", fr) ;
        }

        {
            fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) ;
            stampa_fr("f_open s", fr) ;

            if ( fr == FR_OK ) {
                ciclo += 1 ;
                nums = sprintf(scritti, "ciclo %d", ciclo) ;

                UINT bw ;
                fr = f_write(&Fil, scritti, nums, &bw) ;
                stampa_fr("f_write", fr) ;

                CONTROLLA(nums == bw) ;

                fr = f_close(&Fil) ;
                stampa_fr("f_close", fr) ;
            }
        }

        {
            fr = f_open(&Fil, "newfile.txt", FA_READ | FA_OPEN_EXISTING) ;
            stampa_fr("f_open l", fr) ;

            if ( fr == FR_OK ) {
                UINT br ;
                fr = f_read(&Fil, letti, sizeof(letti), &br) ;
                stampa_fr("f_read", fr) ;

                if ( br != nums ) {
                    DBG_ERR ;
                }
                else if ( memcmp(letti, scritti, br) ) {
                    DBG_ERR ;
                }

                fr = f_close(&Fil) ;
                stampa_fr("f_close", fr) ;
            }
            else {
                DBG_ERR ;
            }
        }

        fr = f_mount(NULL, "", 1) ;
        stampa_fr("f_[un]mount", fr) ;
    }
}



#endif