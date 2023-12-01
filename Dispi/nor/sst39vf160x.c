#define STAMPA_DBG
#include "utili.h"
#include "sst39vf160x.h"
#include "bsp.h"

#define NOR_AMD_FUJITSU_COMMAND_SET           ( (uint16_t) 0x0002 )

#define NOR_CMD_ADDRESS_FIRST                 ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_SECOND                ( (uint16_t) 0x02AA )
#define NOR_CMD_ADDRESS_THIRD                 ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_FOURTH                ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_FIFTH                 ( (uint16_t) 0x02AA )
#define NOR_CMD_ADDRESS_SIXTH                 ( (uint16_t) 0x0555 )

#define NOR_CMD_DATA_FIRST                    ( (uint16_t) 0x00AA )
#define NOR_CMD_DATA_SECOND                   ( (uint16_t) 0x0055 )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_THIRD   ( (uint16_t) 0x0080 )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_FOURTH  ( (uint16_t) 0x00AA )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_FIFTH   ( (uint16_t) 0x0055 )
#define NOR_CMD_DATA_PROGRAM                  ( (uint16_t) 0x00A0 )
#define NOR_CMD_DATA_CHIP_ERASE               ( (uint16_t) 0x0010 )

#define CMD_SECTOR_ERASE                      ( (uint16_t) 0x0050 )

static uint16_t leggi_mem(uint32_t memad)
{
    // La lettura si sblocca quando RY/BY# torna alto
    volatile uint16_t * qua = POINTER(memad) ;
    return *qua ;
}

bool nor_erase_chip(void)
{
    uint32_t memad = NORF_ADDR ;

    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_FIRST),
              NOR_CMD_DATA_FIRST) ;
    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_SECOND),
              NOR_CMD_DATA_SECOND) ;
    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_THIRD),
              NOR_CMD_DATA_CHIP_BLOCK_ERASE_THIRD) ;
    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_FOURTH),
              NOR_CMD_DATA_CHIP_BLOCK_ERASE_FOURTH) ;
    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_FIFTH),
              NOR_CMD_DATA_CHIP_BLOCK_ERASE_FIFTH) ;
    NOR_WRITE(NOR_ADDR_SHIFT(memad,
                             NOR_MEMORY_16B,
                             NOR_CMD_ADDRESS_SIXTH),
              NOR_CMD_DATA_CHIP_ERASE) ;

    // Erase porta tutto a 1
    return 0xFFFF == leggi_mem(memad) ;
}

bool nor_erase_sector(uint32_t sector)
{
    bool esito = false ;

    do {
        if ( sector >= NOR_SECTORS ) {
            DBG_ERR ;
            break ;
        }

        uint32_t memad = NORF_ADDR ;

        // indirizzo in word
        sector *= NOR_SECTOR_SIZE ;
        // ... in byte
        sector <<= 1 ;
        // ... per il micro
        sector += memad ;

        /* Send block erase command sequence */

        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_FIRST),
                  NOR_CMD_DATA_FIRST) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_SECOND),
                  NOR_CMD_DATA_SECOND) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_THIRD),
                  NOR_CMD_DATA_CHIP_BLOCK_ERASE_THIRD) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_FOURTH),
                  NOR_CMD_DATA_CHIP_BLOCK_ERASE_FOURTH) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_FIFTH),
                  NOR_CMD_DATA_CHIP_BLOCK_ERASE_FIFTH) ;
        NOR_WRITE(sector, CMD_SECTOR_ERASE) ;

        // Erase porta tutto a 1
        esito = 0xFFFF == leggi_mem(sector) ;
    } while ( false ) ;

    return esito ;
}

bool nor_program(
    uint32_t ofsW,
    uint16_t data)
{
    bool esito = false ;

    do {
        if ( ofsW >= NOR_SIZE ) {
            DBG_ERR ;
            break ;
        }

        uint32_t memad = NORF_ADDR ;

        // in byte
        uint32_t adr = ofsW << 1 ;
        // visto dal micro
        adr += memad ;

        /* Send program data command */
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_FIRST),
                  NOR_CMD_DATA_FIRST) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_SECOND),
                  NOR_CMD_DATA_SECOND) ;
        NOR_WRITE(NOR_ADDR_SHIFT(memad, NOR_MEMORY_16B, NOR_CMD_ADDRESS_THIRD),
                  NOR_CMD_DATA_PROGRAM) ;
        /* Write the data */
        NOR_WRITE(adr, data) ;

        // Uguali?
        esito = data == leggi_mem(adr) ;
    } while ( false ) ;

    return esito ;
}

uint16_t * nor_addr(uint32_t ofsW)
{
    uint32_t memad = NORF_ADDR ;

    memad += ofsW << 1 ;

    return POINTER(memad) ;
}

#if 0
static const uint16_t roba[] = {
    0x6A38, 0xFE46, 0x7BE5, 0xA5FC, 0x986E, 0x7AA3, 0xE236, 0x3899,
    0xBCDA, 0x6295, 0x1B90, 0x7D69, 0x2190, 0xA511, 0xDA2E, 0x69B6,
    0xBBB7, 0x2C57, 0xC956, 0x1EF7, 0x4BB2, 0x315F, 0xE555, 0x9855,
    0x0AC1
} ;

void test(void)
{
    int ciclo = 0 ;
    const size_t ELEM_ROBA = DIM_VETT(roba) ;
    const uint32_t INC_POS = 409 ;
    uint32_t pos = 0 ;

    DBG_PUTS("elimino tutto") ;
    ASSERT( nor_erase_chip() ) ;

    while ( true ) {
        ciclo++ ;

        pos += INC_POS ;
        if ( pos + ELEM_ROBA > NOR_SECTOR_SIZE ) {
            // il -1 contribuisce a spostare la zona scritta
            pos = pos + ELEM_ROBA - NOR_SECTOR_SIZE - 1 ;
        }

        DBG_PRINTF("ciclo %d", ciclo) ;

        for ( uint32_t sector = 0 ;
              sector < NOR_SECTORS ;
              sector++ ) {
            uint32_t ofs = sector * NOR_SECTOR_SIZE + pos ;

            DBG_PRINTF("\t ofs %08X", ofs) ;

            // scrivo
            for ( size_t i = 0 ; i < ELEM_ROBA ; i++ ) {
                ASSERT( nor_program(ofs + i, roba[i]) ) ;
            }

            // leggo e confronto
            uint16_t * mem = nor_addr(ofs) ;
            for ( size_t i = 0 ; i < ELEM_ROBA ; i++ ) {
                ASSERT(roba[i] == mem[i]) ;
            }

            // elimino
            ASSERT( nor_erase_sector(sector) ) ;

            // sbiancata?
            for ( size_t i = 0 ; i < ELEM_ROBA ; i++ ) {
                ASSERT(0xFFFF == mem[i]) ;
            }
        }
    }
}

#endif
