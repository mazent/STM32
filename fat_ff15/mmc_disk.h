#ifndef MIDDLEWARES_FF15_MMC_DISK_H_
#define MIDDLEWARES_FF15_MMC_DISK_H_

DSTATUS MMC_initialize(void) ;
DSTATUS MMC_status(void) ;
DRESULT MMC_read(
    BYTE * buff,
    LBA_t sector,
    UINT count) ;
DRESULT MMC_write(
    const BYTE * buff,
    LBA_t sector,
    UINT count) ;
DRESULT MMC_ioctl(
    BYTE cmd,
    void * buff) ;

#else
#   warning ram_disk.h incluso
#endif
