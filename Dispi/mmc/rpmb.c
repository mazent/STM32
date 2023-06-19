#define STAMPA_DBG
#include "utili.h"
#include "hash.h"
#include "stm32h7xx_hal.h"

/*
 * Tentativo di accesso alla rpmb della mmc
 *
 * Vedi: https://community.st.com/s/question/0D53W00002IeEdQSAV
 */

extern RNG_HandleTypeDef hrng ;

extern void mmc_iniz_io(void) ;
extern bool mmc_fine_io(void) ;

#define REQ_AUTH_KEY_PROG       0x0001
#define REQ_READ_WRITE_COUNTER  0x0002
#define REQ_DATA_WRITE          0x0003
#define REQ_DATA_READ           0x0004
#define REQ_RESULT              0x0005

#define RSP_AUTH_KEY_PROG       0x0100
#define RSP_READ_WRITE_COUNTER  0x0200
#define RSP_DATA_WRITE          0x0300
#define RSP_DATA_READ           0x0400

// se alto, wc = 0xFFFFFFFF e non incrementa piu'
#define RES_WC_STATUS_BIT       (1 << 7)

#define RES_OPERATION_OK        0x0000
#define RES_GENERAL_FAILURE     0x0001
#define RES_AUTH_FAILURE        0x0002
#define RES_COUNTER_FAILURE     0x0003
#define RES_ADDRESS_FAILURE     0x0004
#define RES_WRITE_FAILURE       0x0005
#define RES_READ_FAILURE        0x0006
#define RES_AUTH_KEY_NOT_PROG   0x0007

typedef struct {
    uint8_t stuff[196] ;
    uint8_t mac[HASH256_DIM] ;
    uint8_t data[256] ;
    uint32_t nonce[4] ;
    uint32_t w_counter ;
    uint16_t address ;
    uint16_t block_count ;
    uint16_t result ;
    uint16_t req_resp ;
} RPMB_DATA_FRAME ;

static RPMB_DATA_FRAME rpmb_req, rpmb_rsp ;

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

static bool mmc_cmd23(
    MMC_HandleTypeDef * h,
    bool reliable,
    bool packed,
    uint32_t num_blocchi)
{
//    {
//        SDMMC_DataInitTypeDef config ;
//
//        /* Configure the MMC DPSM (Data Path State Machine) */
//        config.DataTimeOut = SDMMC_DATATIMEOUT ;
//        config.DataLength = MMC_BLOCKSIZE  ;
//        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B ;
//        config.TransferDir = SDMMC_TRANSFER_DIR_TO_CARD ;
//        config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK ;
//        config.DPSM = SDMMC_DPSM_DISABLE ;
//        (void) SDMMC_ConfigData(h->Instance, &config) ;
//
//        __SDMMC_CMDTRANS_ENABLE(h->Instance) ;
//    }

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
    (void) SDMMC_SendCommand(h->Instance, &cmdinit) ;

    /* Check for error conditions */
    return SDMMC_ERROR_NONE == SDMMC_GetCmdResp1(h->Instance,
                                                 SDMMC_CMD_SET_BLOCK_COUNT,
                                                 SDMMC_CMDTIMEOUT) ;
}

static bool mmc_writeblocks_dma(
    MMC_HandleTypeDef * h,
    void * pData,
    uint32_t BlockAdd,
    uint32_t NumberOfBlocks)
{
    uint32_t errorstate ;
    uint32_t add = BlockAdd ;

    if ( h->State == HAL_MMC_STATE_READY ) {
        h->ErrorCode = HAL_MMC_ERROR_NONE ;

        h->State = HAL_MMC_STATE_BUSY ;

        /* Initialize data control register */
        h->Instance->DCTRL = 0U ;

        h->pTxBuffPtr = pData ;
        h->TxXferSize = MMC_BLOCKSIZE * NumberOfBlocks ;

        if ( (h->MmcCard.CardType) != MMC_HIGH_CAPACITY_CARD ) {
            add *= 512U ;
        }

        /* Configure the MMC DPSM (Data Path State Machine) */
        SDMMC_DataInitTypeDef config ;
        config.DataTimeOut = SDMMC_DATATIMEOUT ;
        config.DataLength = MMC_BLOCKSIZE * NumberOfBlocks ;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B ;
        config.TransferDir = SDMMC_TRANSFER_DIR_TO_CARD ;
        config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK ;
        config.DPSM = SDMMC_DPSM_DISABLE ;
        (void) SDMMC_ConfigData(h->Instance, &config) ;

        __SDMMC_CMDTRANS_ENABLE(h->Instance) ;

        h->Instance->IDMABASE0 = (uint32_t) pData ;
        h->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_SINGLE_BUFF ;

        /* Write Blocks in Polling mode */
        h->Context = (MMC_CONTEXT_WRITE_MULTIPLE_BLOCK | MMC_CONTEXT_DMA) ;

        /* Write Multi Block command */
        errorstate = SDMMC_CmdWriteMultiBlock(h->Instance, add) ;

        if ( errorstate != HAL_MMC_ERROR_NONE ) {
            /* Clear all the static flags */
            __HAL_MMC_CLEAR_FLAG(h, SDMMC_STATIC_FLAGS) ;
            h->ErrorCode |= errorstate ;
            h->State = HAL_MMC_STATE_READY ;
            DBG_ERR ;
            return false ;
        }

        /* Enable transfer interrupts */
        __HAL_MMC_ENABLE_IT( h,
                             (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT
                              | SDMMC_IT_TXUNDERR
                              | SDMMC_IT_DATAEND) ) ;

        return true ;
    }

        DBG_ERR ;
        return false ;
}

static bool mmc_readblocks_dma(
    MMC_HandleTypeDef * h,
    void * pData,
    uint32_t BlockAdd,
    uint32_t NumberOfBlocks)
{
    uint32_t errorstate ;
    uint32_t add = BlockAdd ;

    if ( h->State == HAL_MMC_STATE_READY ) {
        h->ErrorCode = HAL_DMA_ERROR_NONE ;

        h->State = HAL_MMC_STATE_BUSY ;

        /* Initialize data control register */
        h->Instance->DCTRL = 0U ;

        h->pRxBuffPtr = pData ;
        h->RxXferSize = MMC_BLOCKSIZE * NumberOfBlocks ;

        if ( (h->MmcCard.CardType) != MMC_HIGH_CAPACITY_CARD ) {
            add *= 512U ;
        }

        /* Configure the MMC DPSM (Data Path State Machine) */
        SDMMC_DataInitTypeDef config ;
        config.DataTimeOut = SDMMC_DATATIMEOUT ;
        config.DataLength = MMC_BLOCKSIZE * NumberOfBlocks ;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B ;
        config.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC ;
        config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK ;
        config.DPSM = SDMMC_DPSM_DISABLE ;
        (void) SDMMC_ConfigData(h->Instance, &config) ;

        __SDMMC_CMDTRANS_ENABLE(h->Instance) ;
        h->Instance->IDMABASE0 = (uint32_t) pData ;
        h->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_SINGLE_BUFF ;

        /* Read Blocks in DMA mode */
        h->Context = (MMC_CONTEXT_READ_MULTIPLE_BLOCK | MMC_CONTEXT_DMA) ;

        /* Read Multi Block command */
        errorstate = SDMMC_CmdReadMultiBlock(h->Instance, add) ;

        if ( errorstate != HAL_MMC_ERROR_NONE ) {
            /* Clear all the static flags */
            __HAL_MMC_CLEAR_FLAG(h, SDMMC_STATIC_FLAGS) ;
            h->ErrorCode = errorstate ;
            h->State = HAL_MMC_STATE_READY ;
            DBG_ERR ;
            return false ;
        }

        /* Enable transfer interrupts */
        __HAL_MMC_ENABLE_IT( h,
                             (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT
                              | SDMMC_IT_RXOVERR
                              | SDMMC_IT_DATAEND) ) ;

        return true ;
    }

    DBG_ERR ;
        return false ;
}

static uint8_t kiave[HASH256_DIM] ;

void rpmb_iniz(void)
{
    uint8_t tmp[HASH256_DIM] ;
    uint32_t uid[3] = {
        HAL_GetUIDw1(),
        HAL_GetUIDw2(),
        HAL_GetUIDw0()
    } ;

    static_assert(512 == sizeof(RPMB_DATA_FRAME), "OKKIO") ;

    // Primo giro
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if ( !HASH_cal_256(uid, sizeof(uid), kiave) ) {
        DBG_ERR ;
    }
    // Secondo giro
    else if ( !HASH_cal_256(kiave, HASH256_DIM, tmp) ) {
        DBG_ERR ;
    }
    // Terzo giro
    else if ( !HASH_cal_256(tmp, HASH256_DIM, kiave) ) {
        DBG_ERR ;
    }
    else {
        // Ok
    }
}

static bool nuovo_nonce(void)
{
    uint32_t * nonce = rpmb_req.nonce ;
    int i = 0 ;
    for ( ; i < 4 ; ++i, nonce++ ) {
        if ( HAL_OK !=
             HAL_RNG_GenerateRandomNumber(&hrng, nonce) ) {
            DBG_ERR ;
            break ;
        }
    }

    return i == 4 ;
}

void rpmb_read_wc(MMC_HandleTypeDef * h)
{
    do {
        if ( !nuovo_nonce() ) {
            DBG_ERR ;
            break ;
        }

        // Prior to CMD25, the block count is set to 1 by CMD23
        if ( !mmc_cmd23(h, false, false, 1) ) {
            DBG_ERR ;
            break ;
        }

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;
        rpmb_req.req_resp = __REV16(REQ_READ_WRITE_COUNTER) ;

        // The counter read sequence is initiated by Write Multiple Block command, CMD25
        mmc_iniz_io() ;
        if ( !mmc_writeblocks_dma(h, &rpmb_req, 0, 1) ) {
            DBG_ERR ;
            break ;
        }

        if ( !mmc_fine_io() ) {
            DBG_ERR ;
            break ;
        }

        // Prior to the command CMD18, the block count is set to 1 by CMD23
        if ( !mmc_cmd23(h, false, false, 1) ) {
            DBG_ERR ;
            break ;
        }

        memset( &rpmb_rsp, 0, sizeof(RPMB_DATA_FRAME) ) ;

        // The counter value itself is read out with the Read Multiple Block command, CMD18
        mmc_iniz_io() ;
        if ( !mmc_readblocks_dma(h, &rpmb_rsp, 0, 1) ) {
            DBG_ERR ;
            break ;
        }

        if ( !mmc_fine_io() ) {
            DBG_ERR ;
            break ;
        }

        DBG_PRINT_HEX( "rsp", &rpmb_rsp, sizeof(RPMB_DATA_FRAME) ) ;
    } while ( false ) ;
}
