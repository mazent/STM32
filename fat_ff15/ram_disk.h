#ifndef MIDDLEWARES_FF15_RAM_DISK_H_
#define MIDDLEWARES_FF15_RAM_DISK_H_

DSTATUS RD_initialize(void) ;
DSTATUS RD_status(void) ;
DRESULT RD_read(
    BYTE * buff,
    LBA_t sector,
    UINT count) ;
DRESULT RD_write(
    const BYTE * buff,
    LBA_t sector,
    UINT count) ;
DRESULT RD_ioctl(
    BYTE cmd,
    void * buff) ;

#else
#   warning ram_disk.h incluso
#endif
