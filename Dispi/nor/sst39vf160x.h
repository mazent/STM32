#ifndef DISPI_SST39VF160X_SST39VF160X_H_
#define DISPI_SST39VF160X_SST39VF160X_H_

// Organized as 1M x16
#define NOR_SIZE            (1024 * 1024)

// Uniform 2 KWord sectors
#define NOR_SECTOR_SIZE     2048

#define NOR_SECTORS         (NOR_SIZE / NOR_SECTOR_SIZE)

// Durata massima
// Tbp  Word-Program    10 µs
// Tse  Sector-Erase    25 ms
// Tbe  Block-Erase     25 ms
// Tsce Chip-Erase      50 ms

#define NOR_SECTOR_ERASE_MS    25
#define NOR_BLOCK_ERASE_MS     25
#define NOR_CHIP_ERASE_MS      50

bool nor_erase_sector(
    NOR_HandleTypeDef * hnor,
    uint32_t sector) ;

bool nor_program(
    NOR_HandleTypeDef * hnor,
    uint32_t ofsW,
    uint16_t data) ;

uint16_t * nor_addr(
    NOR_HandleTypeDef * hnor,
    uint32_t ofsW) ;

#else
#   warning sst39vf160x.h incluso
#endif
