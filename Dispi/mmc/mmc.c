#define STAMPA_DBG
#include "utili.h"
#include "mmc.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"

extern void rpmb_iniz(void) ;
extern HAL_StatusTypeDef mmc_writeblocks_dma(
    MMC_HandleTypeDef * hmmc,
    uint8_t * pData,
    uint32_t BlockAdd,
    uint32_t NumberOfBlocks) ;
extern HAL_StatusTypeDef mmc_readblocks_dma(
    MMC_HandleTypeDef * hmmc,
    uint8_t * pData,
    uint32_t BlockAdd,
    uint32_t NumberOfBlocks) ;

extern void stampa_cid(void) ;
extern void stampa_csd(void) ;
extern void stampa_ecsd(void) ;

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

#define USA_RPMB            0

//#define STAMPA_ROBA         1

typedef enum {
    OCCUPATO,
    FINE,
    ERRORE
} STATO ;

static STATO stt ;

void MMC_cid(S_MMC_CID * cid)
{
    uint8_t * p = (uint8_t *) cid ;
    memcpy_( p, &sdmmc.CID[3], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[2], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[1], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CID[0], sizeof(uint32_t) ) ;
}

void MMC_csd(S_MMC_CSD * csd)
{
    uint8_t * p = (uint8_t *) csd ;
    memcpy_( p, &sdmmc.CSD[3], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[2], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[1], sizeof(uint32_t) ) ;
    p += sizeof(uint32_t) ;
    memcpy_( p, &sdmmc.CSD[0], sizeof(uint32_t) ) ;
}

void MMC_ext_csd(S_MMC_EXT_CSD * csd)
{
    static_assert(512 == sizeof(S_MMC_EXT_CSD), "OKKIO") ;
    void * p = csd ;
    memcpy_( p, sdmmc.Ext_CSD, sizeof(S_MMC_EXT_CSD) ) ;
}

bool MMC_iniz(void)
{
    if ( HAL_MMC_Init(&sdmmc) != HAL_OK ) {
        return false ;
    }

    // se abilitate
    stampa_cid() ;
    stampa_csd() ;
    stampa_ecsd() ;

#if USA_RPMB
    rpmb_iniz() ;
#endif

    return true ;
}

void MMC_fine(void)
{
    HAL_MMC_DeInit(&sdmmc) ;
}

void mmc_iniz_io(void)
{
    stt = OCCUPATO ;
}

bool mmc_fine_io(void)
{
    for ( int i = 0 ; i < 1000 ; ++i ) {
        if ( FINE == stt ) {
            return true ;
        }
        if ( ERRORE == stt ) {
            DBG_ERR ;
            return false ;
        }
        HAL_Delay(1) ;
    }
    return false ;
}

bool mmc_wait(void)
{
    bool esito = false ;

    while ( true ) {
        HAL_MMC_CardStateTypeDef cs = HAL_MMC_GetCardState(&sdmmc) ;

        if ( sdmmc.ErrorCode ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_MMC_CARD_PROGRAMMING == cs ) {
            HAL_Delay(10) ;
        }
        else {
            DBG_PRINTF("%s -> %08X", __func__, cs) ;
            esito = true ;
            break ;
        }
    }

    return esito ;
}

/*
    Argument:
        [31]    Reliable Write Request
        [30]    0: non-packed
        [29]    tag request
        [28:25] context ID
        [24]    forced programming
        [23:16] set to 0
        [15:0]  number of blocks
    oppure:
        [31]    set to 0
        [30]    1 packed
        [29:16] set to 0
        [15:0]  number of blocks
*/

bool mmc_cmd23(
    bool reliable,
    bool packed,
    uint32_t num_blocchi)
{
    uint32_t arg = num_blocchi ;
    if ( packed ) {
        arg |= 1 << 30 ;
    }
    else if ( reliable ) {
        arg |= (uint32_t) (1 << 31) ;
    }

    SDMMC_CmdInitTypeDef cmdinit ;

    cmdinit.Argument = arg ;
    cmdinit.CmdIndex = SDMMC_CMD_SET_BLOCK_COUNT ;
    cmdinit.Response = SDMMC_RESPONSE_SHORT ;
    cmdinit.WaitForInterrupt = SDMMC_WAIT_NO ;
    cmdinit.CPSM = SDMMC_CPSM_ENABLE ;
    (void) SDMMC_SendCommand(sdmmc.Instance, &cmdinit) ;

    /* Check for error conditions */
    return SDMMC_ERROR_NONE == SDMMC_GetCmdResp1(sdmmc.Instance,
                                                 SDMMC_CMD_SET_BLOCK_COUNT,
                                                 SDMMC_CMDTIMEOUT) ;
}

bool MMC_leggi(
    uint32_t blocco,
    uint32_t num_blocchi,
    void * dati)
{
    bool esito = false ;

    do {
        mmc_iniz_io() ;
#if USA_RPMB == 0
        if ( HAL_MMC_ReadBlocks_DMA(&sdmmc,
                                    dati, blocco, num_blocchi) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }
#else
        // Alternativa (usata con rpmb)
        if ( !mmc_cmd23(false, false, num_blocchi) ) {
            DBG_ERR ;
            break ;
        }

        if ( mmc_readblocks_dma(&sdmmc, dati, blocco,
                                num_blocchi) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }
#endif
        esito = mmc_fine_io() ;
#ifdef STAMPA_ROBA
        if ( esito ) {
            DBG_PRINT_HEX("mmc -> ", dati, MMC_BLOCKSIZE) ;
        }
#endif
    } while ( false ) ;

    return esito ;
}

bool mmc_leggi(
    uint32_t blocco,
    void * dati)
{
    bool esito = false ;

    do {
        mmc_iniz_io() ;
        if ( mmc_readblocks_dma(&sdmmc, dati, blocco, 1) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        esito = mmc_fine_io() ;
    } while ( false ) ;

    return esito ;
}

bool MMC_scrivi(
    uint32_t blocco,
    uint32_t num_blocchi,
    const void * dati)
{
    bool esito = false ;

    do {
        mmc_iniz_io() ;
        if ( HAL_MMC_WriteBlocks_DMA(&sdmmc,
                                     CONST_CAST(dati), blocco,
                                     num_blocchi) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        esito = mmc_fine_io() ;
#ifdef STAMPA_ROBA
        if ( esito ) {
            DBG_PRINT_HEX("mmc <- ", dati, MMC_BLOCKSIZE) ;
        }
#endif
    } while ( false ) ;

    return esito ;
}

bool mmc_scrivi(
    uint32_t blocco,
    const void * dati)
{
    bool esito = false ;

    do {
        mmc_iniz_io() ;
        if ( mmc_writeblocks_dma(&sdmmc, CONST_CAST(dati), blocco,
                                 1) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        esito = mmc_fine_io() ;
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
            else if ( HAL_MMC_CARD_PROGRAMMING == cs ) {
                HAL_Delay(10) ;
            }
            else {
                DBG_PRINTF("%s -> %08X", __func__, cs) ;
                break ;
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

/*
    JEDEC Standard No. 84-B451

    6.2.5 Access partitions
        Each time the host wants to access a partition the following flow shall
        be executed:
            1. Set PARTITION_ACCESS bits in the PARTITION_CONFIG field of the
               Extended CSD register in order to address one of the partitions
            2. Issue commands referred to the selected partition
            3. Restore default access to the User Data Area or re-direction
               the access to another partition

    6.3.5 Access to boot partition
        After putting a slave into transfer state, master sends CMD6 (SWITCH)
        to set the PARTITION_ACCESS bits in the EXT_CSD register, byte [179].
        After that, master can use normal MMC commands to access a boot partition.

        Master can program boot data on DAT line(s) using CMD24 (WRITE_BLOCK)
        or CMD25 (WRITE_MULTIPLE_BLOCK) with slave supported addressing mode
        i.e. byte addressing or sector addressing.
        If the master uses CMD25 (WRITE_MULTIPLE_BLOCK) and the writes past the
        selected partition boundary, the slave will report an
        ADDRESS_OUT_OF_RANGE error. Data that is within the partition boundary
        will be written to the selected boot partition.

        Master can read boot data on DAT line(s) using CMD17 (READ_SINGLE_BLOCK)
        or CMD18 (READ_MULTIPLE_BLOCK) with slave supported addressing mode i.e.
        byte addressing or sector addressing. If the master page uses CMD18
        (READ_MULTIPLE_BLOCK) and then reads past the selected partition
        boundary, the slave will report an ADDRESS_OUT_OF_RANGE error.
*/

// Table 41 — Basic commands (class 0 and class 1)
typedef struct {
    uint32_t
        cmd_set : 3,
        ris3 : 5,
        value : 8,
        index : 8,
        access : 2,
        ris26 : 6 ;
} CMD6_ARG ;

// 6.6.1 Command sets and extended settings
// The command set is changed according to the Cmd Set field of the argument
#define ACCESS_CSET     0
// The bits in the pointed byte are set, according to the ‘1’ bits in the Value field.
#define ACCESS_SET      1
// The bits in the pointed byte are cleared, according to the ‘1’ bits in the Value field.
#define ACCESS_CLEAR    2
// The Value field is written into the pointed byte.
#define ACCESS_WRITE    3

#define PARTITION_ACCESS_NO      0x0 // No access to boot partition (default)
#define PARTITION_ACCESS_B_1     0x1 // R/W boot partition 1
#define PARTITION_ACCESS_B_2     0x2 // R/W boot partition 2
#define PARTITION_ACCESS_RPMB    0x3 // R/W Replay Protected Memory Block (RPMB)
#define PARTITION_ACCESS_GP_1    0x4 // Access to General Purpose partition 1
#define PARTITION_ACCESS_GP_2    0x5 // Access to General Purpose partition 2
#define PARTITION_ACCESS_GP_3    0x6 // Access to General Purpose partition 3
#define PARTITION_ACCESS_GP_4    0x7 // Access to General Purpose partition 4

#define PARTITION_CONFIG_IDX    179

bool MMC_access(MMC_PART part)
{
    bool esito = false ;
    union {
        CMD6_ARG arg ;
        uint32_t x ;
    } u = {
        .arg = {
            .index = PARTITION_CONFIG_IDX,
            .access = ACCESS_WRITE
        }
    } ;

    switch ( part ) {
    case MMC_PART_RPMB:
        u.arg.value = PARTITION_ACCESS_RPMB ;
        break ;
    case MMC_PART_BOOT_1:
        u.arg.value = PARTITION_ACCESS_B_1 ;
        break ;
    case MMC_PART_BOOT_2:
        u.arg.value = PARTITION_ACCESS_B_2 ;
        break ;
    case MMC_PART_USER:
        u.arg.value = PARTITION_ACCESS_NO ;
        break ;
    default:
        goto esci ;
    }
    {
        uint32_t errorstate = SDMMC_CmdSwitch(sdmmc.Instance, u.x) ;
        if ( HAL_MMC_ERROR_NONE == errorstate ) {
            // The SWITCH command response is of type R1b, therefore, the host
            // should read the Device status, using SEND_STATUS command, after the
            // busy signal is de-asserted, to check the result of the SWITCH operation.
            esito = mmc_wait() ;
        }
    }
esci:
    return esito ;
}
