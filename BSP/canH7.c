#define STAMPA_DBG
#include "utili.h"
#include "can.h"
#include "bsp.h"
#include "stm32h7xx_hal.h"

//#define STAMPA_ROBA         1
//#define STAMPA_ERRORI		1
//#define STAMPA_COMP

extern void cfg_std(
    FDCAN_HandleTypeDef * uc,
    BASE_CAN * nmnl) ;
extern void cfg_fd(
    FDCAN_HandleTypeDef * uc,
    BASE_CAN * nmnl,
    BASE_CAN * data) ;

// cfr FDCAN_MESSAGE_RAM_SIZE
#define RAM_CIASCUNO    ( (0x2800 / NUM_CAN) / sizeof(uint32_t) )
#define RAM_OFS_1       0
#define RAM_OFS_3       (RAM_OFS_1 + RAM_CIASCUNO)

#define INTERRUZIONI    (FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_TX_COMPLETE)
#define BUFFER_TX       (FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2)

// Dimensioni ammesse
#define DIM_0       0
#define DIM_1       1
#define DIM_2       2
#define DIM_3       3
#define DIM_4       4
#define DIM_5       5
#define DIM_6       6
#define DIM_7       7
#define DIM_8       8
#define DIM_12      12
#define DIM_16      16
#define DIM_20      20
#define DIM_24      24
#define DIM_32      32
#define DIM_48      48
#define DIM_64      64

// Parte costante dei parametri can

static const FDCAN_InitTypeDef fdcan_init = {
    //.FrameFormat = FDCAN_FRAME_FD_BRS,
    .Mode = FDCAN_MODE_NORMAL,
    .AutoRetransmission = DISABLE,
    .TransmitPause = DISABLE,
    .ProtocolException = DISABLE,
    //.NominalPrescaler = 1,
    .NominalSyncJumpWidth = 1,
    //.NominalTimeSeg1 = 2,
    //.NominalTimeSeg2 = 2,
    //.DataPrescaler = 1,
    .DataSyncJumpWidth = 1,
    //.DataTimeSeg1 = 1,
    //.DataTimeSeg2 = 1,
    .StdFiltersNbr = 0,
    .ExtFiltersNbr = 0,
    .RxFifo0ElmtsNbr = 3,
    .RxFifo0ElmtSize = FDCAN_DATA_BYTES_64,
    .TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION,
    .TxFifoQueueElmtsNbr = 3,
    .TxElmtSize = FDCAN_DATA_BYTES_64
} ;

/*
 * Esempio
 * =================================
 *
 * PCLK1 = 170 MHz
 *
 * CK = PCLK1 / ClockDivider = 85 MHz
 *
 *                         CK                         85 M
 * Fbit = ---------------------------------------- = ------ = 1 M
 *        CAN1_PRESCALER (1 + CAN1_BS1 + CAN1_BS2)   5 17
 *
 *
 *          1 + CAN1_BS1          13
 * % = ----------------------- = ---- = 76
 *     1 + CAN1_BS1 + CAN1_BS2    17
 */

const int CK_KBS = 64000 ;

FDCAN_HandleTypeDef hfdcan1 = {
    .Instance = FDCAN1,
    .State = HAL_FDCAN_STATE_RESET
} ;

FDCAN_HandleTypeDef hfdcan3 = {
    .Instance = FDCAN3,
    .State = HAL_FDCAN_STATE_RESET
} ;

static uint8_t MessageMarker1 = 0 ;
static uint8_t MessageMarker3 = 0 ;

static const S_CAN_PRM * pC1 = NULL ;
static const S_CAN_PRM * pC3 = NULL ;

void CAN_riavvia(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    CONTROLLA( HAL_OK == HAL_FDCAN_Stop(pC) ) ;

    CONTROLLA( HAL_OK == HAL_FDCAN_Start(pC) ) ;
}

CAN_HANDLE CAN_auto(int h)
{
    if ( 1 == h ) {
        return &hfdcan1 ;
    }

    return &hfdcan3 ;
}

void CAN_base_std(
    CAN_HANDLE ch,
    BASE_CAN * prm)
{
    FDCAN_HandleTypeDef * pC = ch ;

    pC->Init = fdcan_init ;

    if ( FDCAN1 == pC->Instance ) {
        pC->Init.MessageRAMOffset = RAM_OFS_1 ;
    }
    else {
        pC->Init.MessageRAMOffset = RAM_OFS_3 ;
    }

    pC->Init.FrameFormat = FDCAN_FRAME_CLASSIC ;

    pC->Init.NominalSyncJumpWidth = prm->sjw ;

    cfg_std(pC, prm) ;
}

void CAN_base_std_x(
    CAN_HANDLE ch,
    BASE_CAN_X * prm)
{
    FDCAN_HandleTypeDef * pC = ch ;

    pC->Init = fdcan_init ;

    if ( FDCAN1 == pC->Instance ) {
        pC->Init.MessageRAMOffset = RAM_OFS_1 ;
    }
    else {
        pC->Init.MessageRAMOffset = RAM_OFS_3 ;
    }

    pC->Init.FrameFormat = FDCAN_FRAME_CLASSIC ;

    pC->Init.NominalSyncJumpWidth = prm->sjw ;
    pC->Init.NominalPrescaler = prm->prescaler ;
    pC->Init.NominalTimeSeg1 = prm->bs1 ;
    pC->Init.NominalTimeSeg2 = prm->bs2 ;
}

void CAN_base_fd(
    CAN_HANDLE ch,
    BASE_CAN * nmnl,
    BASE_CAN * data)
{
    FDCAN_HandleTypeDef * pC = ch ;

    pC->Init = fdcan_init ;

    if ( FDCAN1 == pC->Instance ) {
        pC->Init.MessageRAMOffset = RAM_OFS_1 ;
    }
    else {
        pC->Init.MessageRAMOffset = RAM_OFS_3 ;
    }

    pC->Init.FrameFormat = FDCAN_FRAME_FD_BRS ;
    pC->Init.NominalSyncJumpWidth = nmnl->sjw ;
    pC->Init.DataSyncJumpWidth = data->sjw ;

    cfg_fd(pC, nmnl, data) ;

    /*
     * Compensazione
     *
     * Quando il punto di campionamento (sp) cade nel ritardo causato dal
     * transceiver si genera un errore:
     *     Tsp = % Tbit < Ttrans
     *     Tbit < Ttrans / %
     *     Fbit > % / Ttrans
     *
     * Il TJA1044T riporta un "delay time from TXD LOW to RXD LOW"
     * di max 230 ns
     *
     * L'sp minimo del 50% implica  Fbit > 2100
     */
    data->tdc = data->Kbs >= 2100 ;
    if ( data->tdc ) {
        // Campiono in mezzo ...
        uint32_t ssp = 1 + pC->Init.DataTimeSeg1 + pC->Init.DataTimeSeg2 ;
        // ... ma la base dei tempi e' diversa
        ssp *= pC->Init.DataPrescaler ;
        ssp >>= 1 ;
        data->tdco = (uint8_t) MINI(0x7F, ssp) ;
#ifdef STAMPA_COMP
        DBG_PRINTF("tdco %d", data->tdco) ;
#endif
    }
}

void CAN_base_fd_x(
    CAN_HANDLE ch,
    BASE_CAN_X * nmnl,
    BASE_CAN_X * data)
{
    FDCAN_HandleTypeDef * pC = ch ;

    pC->Init = fdcan_init ;

    if ( FDCAN1 == pC->Instance ) {
        pC->Init.MessageRAMOffset = RAM_OFS_1 ;
    }
    else {
        pC->Init.MessageRAMOffset = RAM_OFS_3 ;
    }

    pC->Init.FrameFormat = FDCAN_FRAME_FD_BRS ;

    pC->Init.NominalSyncJumpWidth = nmnl->sjw ;
    pC->Init.NominalPrescaler = nmnl->prescaler ;
    pC->Init.NominalTimeSeg1 = nmnl->bs1 ;
    pC->Init.NominalTimeSeg2 = nmnl->bs2 ;

    pC->Init.DataSyncJumpWidth = data->sjw ;
    pC->Init.DataPrescaler = data->prescaler ;
    pC->Init.DataTimeSeg1 = data->bs1 ;
    pC->Init.DataTimeSeg2 = data->bs2 ;
}

void CAN_leggi_x(
    CAN_HANDLE ch,
    BASE_CAN_X * nmnl,
    BASE_CAN_X * data)
{
    FDCAN_HandleTypeDef * pC = ch ;

    nmnl->sjw = pC->Init.NominalSyncJumpWidth ;
    nmnl->prescaler = pC->Init.NominalPrescaler ;
    nmnl->bs1 = pC->Init.NominalTimeSeg1 ;
    nmnl->bs2 = pC->Init.NominalTimeSeg2 ;

    data->sjw = pC->Init.DataSyncJumpWidth ;
    data->prescaler = pC->Init.DataPrescaler ;
    data->bs1 = pC->Init.DataTimeSeg1 ;
    data->bs2 = pC->Init.DataTimeSeg2 ;
}

uint16_t CAN_iniz(
    CAN_HANDLE ch,
    const S_CAN_PRM * cp)
{
    FDCAN_HandleTypeDef * pC = ch ;

#ifndef NDEBUG
    uint32_t clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_FDCAN) ;
    ASSERT(clk == CK_KBS * 1000) ;
#endif

    switch ( cp->modo ) {
    default:
    case CAN_MODO_NORM:
        pC->Init.Mode = FDCAN_MODE_NORMAL ;
        break ;
    case CAN_MODO_BUSMON:
        pC->Init.Mode = FDCAN_MODE_BUS_MONITORING ;
        break ;
    case CAN_MODO_INTLBK:
        pC->Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK ;
        break ;
    case CAN_MODO_EXTLBK:
        pC->Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK ;
        break ;
    case CAN_MODO_RESTOP:
        pC->Init.Mode = FDCAN_MODE_RESTRICTED_OPERATION ;
        break ;
    }

    pC->Init.AutoRetransmission = cp->tx_auto ? ENABLE : DISABLE ;

    return HAL_FDCAN_Init(pC) ;
}

static HAL_StatusTypeDef imposta_filtro(
    bool std,
    size_t id,
    const UN_FILTRO * uf,
    FDCAN_HandleTypeDef * pC)
{
    FDCAN_FilterTypeDef sFilterConfig = {
        .IdType = std ? FDCAN_STANDARD_ID : FDCAN_EXTENDED_ID,
        .FilterIndex = id,
        .FilterID1 = uf->id1,
        .FilterID2 = uf->id2,
        .FilterConfig =
            uf->scarta ? FDCAN_FILTER_REJECT : FDCAN_FILTER_TO_RXFIFO0
    } ;

    switch ( uf->tipo ) {
    case CAN_UF_TIPO_RANGE:
        sFilterConfig.FilterType = FDCAN_FILTER_RANGE ;
        break ;
    case CAN_UF_TIPO_DUAL:
        sFilterConfig.FilterType = FDCAN_FILTER_DUAL ;
        break ;
    case CAN_UF_TIPO_MASK:
        sFilterConfig.FilterType = FDCAN_FILTER_MASK ;
        break ;
    case CAN_UF_DISAB:
        sFilterConfig.FilterConfig = FDCAN_FILTER_DISABLE ;
        break ;
    }

    //stampa_FDCAN_FilterTypeDef(&sFilterConfig) ;

    return HAL_FDCAN_ConfigFilter(pC, &sFilterConfig) ;
}

uint16_t CAN_filtri(
    CAN_HANDLE ch,
    const S_CAN_PRM * cp)
{
    FDCAN_HandleTypeDef * pC = ch ;

    for ( size_t i = 0 ; i < MAX_FLT_STD ; ++i ) {
        uint16_t stt = imposta_filtro(true, i, cp->fStd + i, pC) ;
        if ( stt != HAL_OK ) {
            return stt ;
        }
    }

    for ( size_t i = 0 ; i < MAX_FLT_EST ; ++i ) {
        uint16_t stt = imposta_filtro(false, i, cp->fEst + i, pC) ;
        if ( stt != HAL_OK ) {
            return stt ;
        }
    }

    return HAL_OK ;
}

uint16_t CAN_glob_filt(
    CAN_HANDLE ch,
    bool rejNoMatch,
    bool rejRem)
{
    FDCAN_HandleTypeDef * pC = ch ;

    uint32_t NonMatching = FDCAN_ACCEPT_IN_RX_FIFO0 ;
    if ( rejNoMatch ) {
        NonMatching = FDCAN_REJECT ;
    }
    uint32_t RejectRemote = FDCAN_FILTER_REMOTE ;
    if ( rejRem ) {
        RejectRemote = FDCAN_REJECT_REMOTE ;
    }

    return HAL_FDCAN_ConfigGlobalFilter(pC, NonMatching, NonMatching,
                                        RejectRemote, RejectRemote) ;
}

uint16_t CAN_compensa(
    CAN_HANDLE ch,
    bool tdc,
    uint8_t tdco)
{
    FDCAN_HandleTypeDef * pC = ch ;

    if ( tdc ) {
        //DBG_PRINTF("tdco <- %d", tdco) ;

        HAL_StatusTypeDef e = HAL_FDCAN_ConfigTxDelayCompensation(pC,
                                                                  tdco,
                                                                  0) ;
        if ( HAL_OK == e ) {
            e = HAL_FDCAN_EnableTxDelayCompensation(pC) ;
        }

        return e ;
    }

    return HAL_FDCAN_DisableTxDelayCompensation(pC) ;
}

uint16_t CAN_cfg(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    return HAL_FDCAN_Start(pC) ;
}

static HAL_StatusTypeDef attiva_notifiche(
    FDCAN_HandleTypeDef * pC,
    uint32_t irq,
    uint32_t buf)
{
    // Cancello pendenti
    uint32_t attive = pC->Instance->IR ;
    if ( attive != 0 ) {
        DBG_PRINTF("attive %08X", attive) ;
        __HAL_FDCAN_CLEAR_FLAG(pC, attive) ;
    }

    return HAL_FDCAN_ActivateNotification(pC, irq, buf) ;
}

#if 0
static void stampa_ram(FDCAN_MsgRamAddressTypeDef * ram)
{
    DBG_PUTS("ram") ;
    DBG_PRINTF("\t StandardFilterSA = %08X", ram->StandardFilterSA) ;
    DBG_PRINTF("\t ExtendedFilterSA = %08X", ram->ExtendedFilterSA) ;
    DBG_PRINTF("\t RxFIFO0SA        = %08X", ram->RxFIFO0SA) ;
    DBG_PRINTF("\t RxFIFO1SA        = %08X", ram->RxFIFO1SA) ;
    DBG_PRINTF("\t RxBufferSA       = %08X", ram->RxBufferSA) ;
    DBG_PRINTF("\t TxEventFIFOSA    = %08X", ram->TxEventFIFOSA) ;
    DBG_PRINTF("\t TxBufferSA       = %08X", ram->TxBufferSA) ;
    DBG_PRINTF("\t TxFIFOQSA        = %08X", ram->TxFIFOQSA) ;
    DBG_PRINTF("\t TTMemorySA       = %08X", ram->TTMemorySA) ;
    DBG_PRINTF("\t EndAddress       = %08X", ram->EndAddress) ;
}

#else
#define stampa_ram(a)
#endif

uint16_t CAN_attiva(
    CAN_HANDLE ch,
    const S_CAN_PRM * prm)
{
    FDCAN_HandleTypeDef * pC = ch ;

    // Ottimismo
    if ( pC == &hfdcan1 ) {
        pC1 = prm ;
    }
    else {
        pC3 = prm ;
    }

    stampa_ram(&pC->msgRam) ;

    return attiva_notifiche(pC, INTERRUZIONI, BUFFER_TX) ;
}

bool CAN_tx(
    CAN_HANDLE ch,
    const CAN_DATI * sd)
{
    FDCAN_HandleTypeDef * pC = ch ;
    FDCAN_TxHeaderTypeDef txh = {
        .TxFrameType = FDCAN_DATA_FRAME,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .IdType = FDCAN_STANDARD_ID
    } ;

//    DBG_FUN ;

    if ( FDCAN_FRAME_CLASSIC == pC->Init.FrameFormat ) {
        if ( sd->dim > MAX_DATI_CAN ) {
            DBG_ERR ;
            return false ;
        }
        txh.FDFormat = FDCAN_CLASSIC_CAN ;
        txh.BitRateSwitch = FDCAN_BRS_OFF ;
    }
    else {
        txh.FDFormat = sd->fd ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN ;
        txh.BitRateSwitch = sd->brs ? FDCAN_BRS_ON : FDCAN_BRS_OFF ;
    }
//    DBG_PRINTF("\tformato: %s", txh.FDFormat == FDCAN_FD_CAN ? "fd" : "classic") ;
//    DBG_PRINTF("\tbrs: %s", txh.BitRateSwitch == FDCAN_BRS_ON ? "on" : "off") ;

    if ( pC == &hfdcan1 ) {
        txh.MessageMarker = MessageMarker1 ;
        MessageMarker1++ ;
    }
    else {
        txh.MessageMarker = MessageMarker3 ;
        MessageMarker3++ ;
    }
//    DBG_PRINTF("\tmarker: %d", txh.MessageMarker) ;

    switch ( sd->dim ) {
    case DIM_0:
        txh.TxFrameType = FDCAN_REMOTE_FRAME ;
        txh.DataLength = FDCAN_DLC_BYTES_0 ;
        break ;
    case DIM_1:
        txh.DataLength = FDCAN_DLC_BYTES_1 ;
        break ;
    case DIM_2:
        txh.DataLength = FDCAN_DLC_BYTES_2 ;
        break ;
    case DIM_3:
        txh.DataLength = FDCAN_DLC_BYTES_3 ;
        break ;
    case DIM_4:
        txh.DataLength = FDCAN_DLC_BYTES_4 ;
        break ;
    case DIM_5:
        txh.DataLength = FDCAN_DLC_BYTES_5 ;
        break ;
    case DIM_6:
        txh.DataLength = FDCAN_DLC_BYTES_6 ;
        break ;
    case DIM_7:
        txh.DataLength = FDCAN_DLC_BYTES_7 ;
        break ;
    case DIM_8:
        txh.DataLength = FDCAN_DLC_BYTES_8 ;
        break ;
    case DIM_12:
        txh.DataLength = FDCAN_DLC_BYTES_12 ;
        break ;
    case DIM_16:
        txh.DataLength = FDCAN_DLC_BYTES_16 ;
        break ;
    case DIM_20:
        txh.DataLength = FDCAN_DLC_BYTES_20 ;
        break ;
    case DIM_24:
        txh.DataLength = FDCAN_DLC_BYTES_24 ;
        break ;
    case DIM_32:
        txh.DataLength = FDCAN_DLC_BYTES_32 ;
        break ;
    case DIM_48:
        txh.DataLength = FDCAN_DLC_BYTES_48 ;
        break ;
    case DIM_64:
        txh.DataLength = FDCAN_DLC_BYTES_64 ;
        break ;
    default:
        DBG_ERR ;
        return false ;
    }

    if ( sd->ide ) {
        txh.IdType = FDCAN_EXTENDED_ID ;
    }
    txh.Identifier = sd->id ;

#if defined (STAMPA_ROBA)
#warning OKKIO
    char testa[50] ;
    const char * titolo ;
    if ( pC == &hfdcan1 ) {
        titolo = "-> CAN1" ;
    }
    else {
        titolo = "-> CAN3" ;
    }
    snprintf(testa, sizeof(testa), "%08X %s", (unsigned int) sd->id, titolo) ;
    DBG_PRINT_HEX(testa, sd->dati, sd->dim) ;
#endif

    return HAL_OK == HAL_FDCAN_AddMessageToTxFifoQ(pC,
                                                   &txh,
                                                   sd->dati) ;
}

bool CAN_txabile(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    return 0 == (pC->Instance->TXFQS & FDCAN_TXFQS_TFQF) ;
}

bool CAN_attivo(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    return HAL_FDCAN_STATE_RESET != pC->State ;
}

void CAN_disab(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    if ( HAL_FDCAN_STATE_RESET != pC->State ) {
        HAL_FDCAN_DeInit(pC) ;

        if ( pC == &hfdcan1 ) {
            pC1 = NULL ;
        }
        else {
            pC3 = NULL ;
        }
    }
}

void CAN_canc_tx(CAN_HANDLE ch)
{
    FDCAN_HandleTypeDef * pC = ch ;

    if ( HAL_FDCAN_IsTxBufferMessagePending(pC, FDCAN_TX_BUFFER0) ) {
        HAL_FDCAN_AbortTxRequest(pC, FDCAN_TX_BUFFER0) ;
    }

    if ( HAL_FDCAN_IsTxBufferMessagePending(pC, FDCAN_TX_BUFFER1) ) {
        HAL_FDCAN_AbortTxRequest(pC, FDCAN_TX_BUFFER1) ;
    }

    if ( HAL_FDCAN_IsTxBufferMessagePending(pC, FDCAN_TX_BUFFER2) ) {
        HAL_FDCAN_AbortTxRequest(pC, FDCAN_TX_BUFFER2) ;
    }
}

void CAN_err(
    CAN_HANDLE ch,
    S_CAN_ERR * pCE)
{
    FDCAN_HandleTypeDef * pC = ch ;

    static_assert(sizeof(S_CAN_ERR) == 4, "OKKIO") ;

    /*
     *  A me (MB) interessano i seguenti errori:
     *      Last error code
     *      Error passive status
     *      Bus off status
     *      Error warning status
     *      Receive error counter
     *      Transmit error counter
     */

    FDCAN_ProtocolStatusTypeDef ps ;
    (void) HAL_FDCAN_GetProtocolStatus(pC, &ps) ;
    pCE->lec = ps.LastErrorCode ;
    pCE->ep = ps.ErrorPassive ;
    pCE->bo = ps.BusOff ;
    pCE->ew = ps.Warning ;

    FDCAN_ErrorCountersTypeDef ec ;
    (void) HAL_FDCAN_GetErrorCounters(pC, &ec) ;
    pCE->rec = ec.RxErrorCnt ;
    pCE->tec = ec.TxErrorCnt ;

    // Queste le aggiungo io
    pCE->dlec = ps.DataLastErrorCode ;
    switch ( ps.Activity ) {
    case FDCAN_COM_STATE_SYNC:
        pCE->act = CAN_ACT_SYNCHRONIZING ;
        break ;
    case FDCAN_COM_STATE_IDLE:
        pCE->act = CAN_ACT_IDLE ;
        break ;
    case FDCAN_COM_STATE_RX:
        pCE->act = CAN_ACT_RECEIVER ;
        break ;
    case FDCAN_COM_STATE_TX:
        pCE->act = CAN_ACT_TRANSMITTER ;
        break ;
    }
}

CAN_ERR CAN_ctrl_rx(
    CAN_HANDLE ch,
    bool abil)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef stt ;

    if ( abil ) {
        stt = HAL_FDCAN_ActivateNotification(pC,
                                             FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                             0) ;
    }
    else {
        stt = HAL_FDCAN_DeactivateNotification(pC,
                                               FDCAN_IT_RX_FIFO0_NEW_MESSAGE) ;
    }
    return stt ;
}

CAN_ERR CAN_err_irq_abil(
    CAN_HANDLE ch,
    bool warning,
    bool passive,
    bool busoff,
    bool arbi,
    bool data)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef stt = HAL_OK ;
    uint32_t spie = 0 ;

    if ( warning ) {
        spie |= FDCAN_IT_ERROR_WARNING ;
    }
    if ( passive ) {
        spie |= FDCAN_IT_ERROR_PASSIVE ;
    }
    if ( busoff ) {
        spie |= FDCAN_IT_BUS_OFF ;
    }
    if ( arbi ) {
        spie |= FDCAN_IT_ARB_PROTOCOL_ERROR ;
    }
    if ( data ) {
        spie |= FDCAN_IT_DATA_PROTOCOL_ERROR ;
    }

    if ( spie ) {
        stt = attiva_notifiche(pC, spie, 0) ;
    }

    return stt ;
}

CAN_ERR CAN_err_irq_disa(
    CAN_HANDLE ch,
    bool warning,
    bool passive,
    bool busoff,
    bool arbi,
    bool data)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef stt = HAL_OK ;
    uint32_t spie = 0 ;

    if ( warning ) {
        spie |= FDCAN_IT_ERROR_WARNING ;
    }
    if ( passive ) {
        spie |= FDCAN_IT_ERROR_PASSIVE ;
    }
    if ( busoff ) {
        spie |= FDCAN_IT_BUS_OFF ;
    }
    if ( arbi ) {
        spie |= FDCAN_IT_ARB_PROTOCOL_ERROR ;
    }
    if ( data ) {
        spie |= FDCAN_IT_DATA_PROTOCOL_ERROR ;
    }

    if ( spie ) {
        stt = HAL_FDCAN_DeactivateNotification(pC, spie) ;
    }

    return stt ;
}

CAN_ERR CAN_ctrl_ta(
    CAN_HANDLE ch,
    bool abil)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef stt = HAL_OK ;
    bool abilitata = ENABLE == pC->Init.AutoRetransmission ;

    do {
        if ( abil && abilitata ) {
            // Ottimo
            break ;
        }

        if ( !abil && !abilitata ) {
            // Ottimo
            break ;
        }

        pC->Init.AutoRetransmission = abil ? ENABLE : DISABLE ;

        HAL_FDCAN_StateTypeDef x = HAL_FDCAN_GetState(pC) ;
        if ( HAL_FDCAN_STATE_BUSY != x ) {
            // Buono
            break ;
        }

        // Il can e' attivo
        (void) HAL_FDCAN_Stop(pC) ;

        if ( abil ) {
            CLEAR_BIT(pC->Instance->CCCR, FDCAN_CCCR_DAR) ;
        }
        else {
            SET_BIT(pC->Instance->CCCR, FDCAN_CCCR_DAR) ;
        }

        (void) HAL_FDCAN_Start(pC) ;
    } while ( false ) ;

    return stt ;
}

CAN_ERR CAN_modo(
    CAN_HANDLE ch,
    uint8_t modo)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef stt = HAL_OK ;
    uint8_t corr ;

    switch ( pC->Init.Mode ) {
    default:
    case FDCAN_MODE_NORMAL:
        corr = CAN_MODO_NORM ;
        break ;
    case FDCAN_MODE_RESTRICTED_OPERATION:
        corr = CAN_MODO_RESTOP ;
        break ;
    case FDCAN_MODE_BUS_MONITORING:
        corr = CAN_MODO_BUSMON ;
        break ;
    case FDCAN_MODE_INTERNAL_LOOPBACK:
        corr = CAN_MODO_INTLBK ;
        break ;
    case FDCAN_MODE_EXTERNAL_LOOPBACK:
        corr = CAN_MODO_EXTLBK ;
        break ;
    }

    do {
        if ( modo == corr ) {
            // Ottimo
            break ;
        }

        switch ( modo ) {
        default:
        case CAN_MODO_NORM:
            pC->Init.Mode = FDCAN_MODE_NORMAL ;
            break ;
        case CAN_MODO_BUSMON:
            pC->Init.Mode = FDCAN_MODE_BUS_MONITORING ;
            break ;
        case CAN_MODO_INTLBK:
            pC->Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK ;
            break ;
        case CAN_MODO_EXTLBK:
            pC->Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK ;
            break ;
        case CAN_MODO_RESTOP:
            pC->Init.Mode = FDCAN_MODE_RESTRICTED_OPERATION ;
            break ;
        }

        HAL_FDCAN_StateTypeDef x = HAL_FDCAN_GetState(pC) ;
        if ( HAL_FDCAN_STATE_BUSY != x ) {
            // Buono
            break ;
        }

        // Il can e' attivo
        (void) HAL_FDCAN_Stop(pC) ;

        /* Set FDCAN Operating Mode:
                     | Normal | Restricted |    Bus     | Internal | External
                     |        | Operation  | Monitoring | LoopBack | LoopBack
           CCCR.TEST |   0    |     0      |     0      |    1     |    1
           CCCR.MON  |   0    |     0      |     1      |    1     |    0
           TEST.LBCK |   0    |     0      |     0      |    1     |    1
           CCCR.ASM  |   0    |     1      |     0      |    0     |    0
        */
        CLEAR_BIT( pC->Instance->CCCR,
                   (FDCAN_CCCR_TEST | FDCAN_CCCR_MON | FDCAN_CCCR_ASM) ) ;
        CLEAR_BIT(pC->Instance->TEST, FDCAN_TEST_LBCK) ;

        switch ( modo ) {
        default:
        case CAN_MODO_NORM:
            break ;
        case CAN_MODO_RESTOP:
            SET_BIT(pC->Instance->CCCR, FDCAN_CCCR_ASM) ;
            break ;
        case CAN_MODO_BUSMON:
            SET_BIT(pC->Instance->CCCR, FDCAN_CCCR_MON) ;
            break ;
        case CAN_MODO_INTLBK:
        case CAN_MODO_EXTLBK:
            SET_BIT(pC->Instance->CCCR, FDCAN_CCCR_TEST) ;
            SET_BIT(pC->Instance->TEST, FDCAN_TEST_LBCK) ;
            if ( CAN_MODO_INTLBK == modo ) {
                SET_BIT(pC->Instance->CCCR, FDCAN_CCCR_MON) ;
            }
            break ;
        }

        (void) HAL_FDCAN_Start(pC) ;
    } while ( false ) ;

    return stt ;
}

CAN_ERR CAN_agg_filtro(
    CAN_HANDLE ch,
    bool std,
    size_t quale,
    S_CAN_PRM * cp)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef ris = HAL_OK ;

    do {
        HAL_FDCAN_StateTypeDef stt = HAL_FDCAN_GetState(pC) ;
        if ( HAL_FDCAN_STATE_BUSY != stt ) {
            // Ottimo
            break ;
        }

        // Il can e' attivo
        UN_FILTRO * base = std ? cp->fStd : cp->fEst ;

        (void) HAL_FDCAN_Stop(pC) ;

        ris = imposta_filtro(std, quale, base + quale, pC) ;

        (void) HAL_FDCAN_Start(pC) ;
    } while ( false ) ;

    return ris ;
}

CAN_ERR CAN_dsb_filtri(
    CAN_HANDLE ch,
    bool std,
    S_CAN_PRM * cp)
{
    FDCAN_HandleTypeDef * pC = ch ;
    HAL_StatusTypeDef ris = HAL_OK ;
    const size_t NUM_FLT = std ? MAX_FLT_STD : MAX_FLT_EST ;
    UN_FILTRO * base = std ? cp->fStd : cp->fEst ;

    for ( size_t i = 0 ; i < NUM_FLT ; ++i ) {
        base[i].tipo = CAN_UF_DISAB ;
    }

    do {
        HAL_FDCAN_StateTypeDef stt = HAL_FDCAN_GetState(pC) ;
        if ( HAL_FDCAN_STATE_BUSY != stt ) {
            // Ottimo
            break ;
        }

        // Il can e' attivo
        (void) HAL_FDCAN_Stop(pC) ;

        for ( size_t i = 0 ; i < NUM_FLT && (HAL_OK == ris) ; ++i ) {
            ris = imposta_filtro(std, i, base + i, pC) ;
        }

        (void) HAL_FDCAN_Start(pC) ;
    } while ( false ) ;

    return ris ;
}

/*
 * Callback
 */

void HAL_FDCAN_TxBufferCompleteCallback(
    FDCAN_HandleTypeDef * hfdcan,
    uint32_t BufferIndexes)
{
#ifdef STAMPA_COMP
    if ( FDCAN_DBTP_TDC == (hfdcan->Instance->DBTP & FDCAN_DBTP_TDC) ) {
        uint32_t ps = hfdcan->Instance->PSR ;

        DBG_PRINTF( "tdcv %d", ( (ps & FDCAN_PSR_TDCV) >> FDCAN_PSR_TDCV_Pos ) ) ;
    }
#endif
    INUTILE(BufferIndexes) ;

    if ( hfdcan == &hfdcan1 ) {
        if ( pC1 ) {
            pC1->tx_cb() ;
        }
    }
    else {
        if ( pC3 ) {
            pC3->tx_cb() ;
        }
    }
}

void HAL_FDCAN_RxFifo0Callback(
    FDCAN_HandleTypeDef * hfdcan,
    uint32_t RxFifo0ITs)
{
    if ( (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET ) {
        FDCAN_RxHeaderTypeDef h ;
        uint8_t dati[MAX_DATI_CAN_FD] ;

        if ( HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &h,
                                    dati) != HAL_OK ) {
            DBG_ERR ;
        }
        else {
            CAN_RX_H rx = {
                .ext = FDCAN_EXTENDED_ID == h.IdType,
                .id = h.Identifier,
                .FDcan = FDCAN_FD_CAN == h.FDFormat,
                .ESIpassive = FDCAN_ESI_PASSIVE == h.ErrorStateIndicator
            } ;

//            DBG_PRINTF("No match=%d, FilterIndex=%d",
//                       h.IsFilterMatchingFrame,
//                       h.FilterIndex) ;

            switch ( h.DataLength ) {
            case FDCAN_DLC_BYTES_0:
                rx.dim = DIM_0 ;
                break ;
            case FDCAN_DLC_BYTES_1:
                rx.dim = DIM_1 ;
                break ;
            case FDCAN_DLC_BYTES_2:
                rx.dim = DIM_2 ;
                break ;
            case FDCAN_DLC_BYTES_3:
                rx.dim = DIM_3 ;
                break ;
            case FDCAN_DLC_BYTES_4:
                rx.dim = DIM_4 ;
                break ;
            case FDCAN_DLC_BYTES_5:
                rx.dim = DIM_5 ;
                break ;
            case FDCAN_DLC_BYTES_6:
                rx.dim = DIM_6 ;
                break ;
            case FDCAN_DLC_BYTES_7:
                rx.dim = DIM_7 ;
                break ;
            case FDCAN_DLC_BYTES_8:
                rx.dim = DIM_8 ;
                break ;
            case FDCAN_DLC_BYTES_12:
                rx.dim = DIM_12 ;
                break ;
            case FDCAN_DLC_BYTES_16:
                rx.dim = DIM_16 ;
                break ;
            case FDCAN_DLC_BYTES_20:
                rx.dim = DIM_20 ;
                break ;
            case FDCAN_DLC_BYTES_24:
                rx.dim = DIM_24 ;
                break ;
            case FDCAN_DLC_BYTES_32:
                rx.dim = DIM_32 ;
                break ;
            case FDCAN_DLC_BYTES_48:
                rx.dim = DIM_48 ;
                break ;
            case FDCAN_DLC_BYTES_64:
                rx.dim = DIM_64 ;
                break ;
            }

#if defined (STAMPA_ROBA)
            char testa[50] ;
            const char * titolo ;
            if ( hfdcan == &hfdcan1 ) {
                titolo = "<- CAN1" ;
            }
            else {
                titolo = "<- CAN3" ;
            }
            snprintf(testa,
                     sizeof(testa),
                     "%s %08X",
                     titolo,
                     (unsigned int) rx.id) ;
            DBG_PRINT_HEX(testa, dati, rx.dim) ;
#endif

            if ( hfdcan == &hfdcan1 ) {
                if ( pC1 ) {
                    pC1->rx_cb(&rx, dati) ;
                }
            }
            else {
                if ( pC3 ) {
                    pC3->rx_cb(&rx, dati) ;
                }
            }
        }
    }
}

#ifdef VCI_BUS_OFF

#if defined (STAMPA_ERRORI)
#warning OKKIO

static const char * slec(uint32_t lec)
{
    switch ( lec ) {
    case FDCAN_PROTOCOL_ERROR_NONE:
        return "No error occurred" ;
    case FDCAN_PROTOCOL_ERROR_STUFF:
        return "Stuff error" ;
    case FDCAN_PROTOCOL_ERROR_FORM:
        return "Form error" ;
    case FDCAN_PROTOCOL_ERROR_ACK:
        return "Acknowledge error" ;
    case FDCAN_PROTOCOL_ERROR_BIT1:
        return "Bit 1 (recessive) error" ;
    case FDCAN_PROTOCOL_ERROR_BIT0:
        return "Bit 0 (dominant) error" ;
    case FDCAN_PROTOCOL_ERROR_CRC:
        return "CRC check sum error" ;
    case FDCAN_PROTOCOL_ERROR_NO_CHANGE:
        return "No change since last read" ;
    }
    return "?" ;
}

static const char * sact(uint32_t act)
{
    switch ( act ) {
    case FDCAN_COM_STATE_SYNC:
        return "Node is synchronizing on CAN communication" ;
    case FDCAN_COM_STATE_IDLE:
        return "Node is neither receiver nor transmitter" ;
    case FDCAN_COM_STATE_RX:
        return "Node is operating as receiver" ;
    case FDCAN_COM_STATE_TX:
        return "Node is operating as transmitter " ;
    }

    return "?" ;
}

static void stampa(FDCAN_HandleTypeDef * hfdcan)
{
    FDCAN_ProtocolStatusTypeDef ps ;
    HAL_FDCAN_GetProtocolStatus(hfdcan, &ps) ;

    DBG_PRINTF( "\t\tLastErrorCode = %s", slec(ps.LastErrorCode) ) ;
    DBG_PRINTF( "\t\tDataLastErrorCode = %s", slec(ps.DataLastErrorCode) ) ;
    DBG_PRINTF( "\t\tActivity = %s", sact(ps.Activity) ) ;
    DBG_PRINTF("\t\tErrorPassive = %d", ps.ErrorPassive) ;
    DBG_PRINTF("\t\tWarning = %d", ps.Warning) ;
    DBG_PRINTF("\t\tBusOff = %d", ps.BusOff) ;
    DBG_PRINTF("\t\tRxESIflag = %d", ps.RxESIflag) ;
    DBG_PRINTF("\t\tRxBRSflag = %d", ps.RxBRSflag) ;
    DBG_PRINTF("\t\tRxFDFflag = %d", ps.RxFDFflag) ;
    DBG_PRINTF("\t\tProtocolException = %d", ps.ProtocolException) ;
    DBG_PRINTF("\t\tTDCvalue = %d", ps.TDCvalue) ;

    FDCAN_ErrorCountersTypeDef ec ;
    HAL_FDCAN_GetErrorCounters(hfdcan, &ec) ;

    DBG_PRINTF("\t\tTxErrorCnt = %d", ec.TxErrorCnt) ;
    DBG_PRINTF("\t\tRxErrorCnt = %d", ec.RxErrorCnt) ;
    DBG_PRINTF("\t\tRxErrorPassive = %d", ec.RxErrorPassive) ;
    DBG_PRINTF("\t\tErrorLogging = %d", ec.ErrorLogging) ;
}

#else
#define stampa(x)
#endif

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef * hfdcan)
{
    DBG_PRINTF("%s(%08X, %08X)", __func__, hfdcan, hfdcan->ErrorCode) ;

    bool arbi = FDCAN_IR_PEA == (FDCAN_IR_PEA & hfdcan->ErrorCode) ;
    bool data = FDCAN_IR_PED == (FDCAN_IR_PED & hfdcan->ErrorCode) ;

    stampa(hfdcan) ;

    // Con la trasm. autom. viene generata una valanga di interruzioni
    HAL_FDCAN_DeactivateNotification(
        hfdcan,
        FDCAN_IT_ARB_PROTOCOL_ERROR
        | FDCAN_IT_DATA_PROTOCOL_ERROR) ;
    uint32_t attive = hfdcan->Instance->IR & (FDCAN_IR_PEA | FDCAN_IR_PED) ;
    if ( attive ) {
        __HAL_FDCAN_CLEAR_FLAG(hfdcan, attive) ;
    }

    if ( hfdcan == &hfdcan1 ) {
        ASSERT(pC1) ;
        if ( pC1 ) {
            pC1->err_cb(arbi, data) ;
        }
    }
    else {
        ASSERT(pC3) ;
        if ( pC3 ) {
            pC3->err_cb(arbi, data) ;
        }
    }
}

void HAL_FDCAN_ErrorStatusCallback(
    FDCAN_HandleTypeDef * hfdcan,
    uint32_t ErrorStatusITs)
{
    DBG_PRINTF("%s(%08X, %08X)", __func__, hfdcan, ErrorStatusITs) ;

    bool warn = FDCAN_IR_EW == (FDCAN_IR_EW & ErrorStatusITs) ;
    bool pass = FDCAN_IR_EP == (FDCAN_IR_EP & ErrorStatusITs) ;
    bool busoff = FDCAN_IR_BO == (FDCAN_IR_BO & ErrorStatusITs) ;

    stampa(hfdcan) ;

    if ( hfdcan == &hfdcan1 ) {
        ASSERT(pC1) ;
        if ( pC1 ) {
            pC1->stt_cb(warn, pass, busoff) ;
        }
    }
    else {
        ASSERT(pC3) ;
        if ( pC3 ) {
            pC3->stt_cb(warn, pass, busoff) ;
        }
    }
}

#endif
