#define STAMPA_DBG
#include "utili.h"
#include "sst39vf160x.h"
#include "stm32h7xx_hal.h"

#define NOR_AMD_FUJITSU_COMMAND_SET           ( (uint16_t) 0x0002 )

#define NOR_CMD_ADDRESS_FIRST                 ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_SECOND                ( (uint16_t) 0x02AA )
#define NOR_CMD_ADDRESS_THIRD                 ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_FOURTH                ( (uint16_t) 0x0555 )
#define NOR_CMD_ADDRESS_FIFTH                 ( (uint16_t) 0x02AA )

#define NOR_CMD_DATA_FIRST                    ( (uint16_t) 0x00AA )
#define NOR_CMD_DATA_SECOND                   ( (uint16_t) 0x0055 )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_THIRD   ( (uint16_t) 0x0080 )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_FOURTH  ( (uint16_t) 0x00AA )
#define NOR_CMD_DATA_CHIP_BLOCK_ERASE_FIFTH   ( (uint16_t) 0x0055 )
#define NOR_CMD_DATA_PROGRAM                  ( (uint16_t) 0x00A0 )

#define CMD_SECTOR_ERASE                      ( (uint16_t) 0x0050 )

bool nor_erase_sector(
    NOR_HandleTypeDef * hnor,
    uint32_t sector)
{
    bool esito = false ;

    do {
        /* Check the NOR controller state */
        if ( hnor->State == HAL_NOR_STATE_BUSY ) {
            DBG_ERR ;
            break ;
        }

        if ( hnor->State != HAL_NOR_STATE_READY ) {
            DBG_ERR ;
            break ;
        }

        if ( hnor->CommandSet != NOR_AMD_FUJITSU_COMMAND_SET ) {
            DBG_ERR ;
            break ;
        }

        /* Process Locked */
        __HAL_LOCK(hnor) ;

        /* Update the NOR controller state */
        hnor->State = HAL_NOR_STATE_BUSY ;

        /* Select the NOR device address */
        uint32_t memad ;
        if ( hnor->Init.NSBank == FMC_NORSRAM_BANK1 ) {
            memad = NOR_MEMORY_ADRESS1 ;
        }
        else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK2 ) {
            memad = NOR_MEMORY_ADRESS2 ;
        }
        else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK3 ) {
            memad = NOR_MEMORY_ADRESS3 ;
        }
        else { /* FMC_NORSRAM_BANK4 */
            memad = NOR_MEMORY_ADRESS4 ;
        }

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

        /* Check the NOR memory status and update the controller state */
        hnor->State = HAL_NOR_STATE_READY ;

        /* Process unlocked */
        __HAL_UNLOCK(hnor) ;

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool nor_program(
    NOR_HandleTypeDef * hnor,
    uint32_t ofsW,
    uint16_t data)
{
    bool esito = false ;

    do {
        /* Check the NOR controller state */
        if ( hnor->State == HAL_NOR_STATE_BUSY ) {
            DBG_ERR ;
            break ;
        }

        if ( hnor->State != HAL_NOR_STATE_READY ) {
            DBG_ERR ;
            break ;
        }

        if ( hnor->CommandSet != NOR_AMD_FUJITSU_COMMAND_SET ) {
            DBG_ERR ;
            break ;
        }

        if ( ofsW >= NOR_SIZE ) {
            DBG_ERR ;
            break ;
        }

        /* Process Locked */
        __HAL_LOCK(hnor) ;

        /* Update the NOR controller state */
        hnor->State = HAL_NOR_STATE_BUSY ;

        /* Select the NOR device address */
        uint32_t memad ;
        if ( hnor->Init.NSBank == FMC_NORSRAM_BANK1 ) {
            memad = NOR_MEMORY_ADRESS1 ;
        }
        else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK2 ) {
            memad = NOR_MEMORY_ADRESS2 ;
        }
        else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK3 ) {
            memad = NOR_MEMORY_ADRESS3 ;
        }
        else { /* FMC_NORSRAM_BANK4 */
            memad = NOR_MEMORY_ADRESS4 ;
        }

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

        /* Check the NOR controller state */
        hnor->State = HAL_NOR_STATE_READY ;

        /* Process unlocked */
        __HAL_UNLOCK(hnor) ;

        esito = true ;
    } while ( false ) ;

    return esito ;
}

uint16_t * nor_addr(
    NOR_HandleTypeDef * hnor,
    uint32_t ofsW)
{
    uint32_t memad ;
    if ( hnor->Init.NSBank == FMC_NORSRAM_BANK1 ) {
        memad = NOR_MEMORY_ADRESS1 ;
    }
    else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK2 ) {
        memad = NOR_MEMORY_ADRESS2 ;
    }
    else if ( hnor->Init.NSBank == FMC_NORSRAM_BANK3 ) {
        memad = NOR_MEMORY_ADRESS3 ;
    }
    else { /* FMC_NORSRAM_BANK4 */
        memad = NOR_MEMORY_ADRESS4 ;
    }

    memad += ofsW << 1 ;

    return POINTER(memad) ;
}
