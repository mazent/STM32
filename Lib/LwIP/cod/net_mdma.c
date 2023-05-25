#define STAMPA_DBG
#include "utili.h"
#include "stm32h7xx_hal.h"
#include "cmsis_rtos/cmsis_os.h"
#include "net_mdma.h"

/*
 * DA RIVEDERE
 */

#ifdef USA_MDMA

static bool rx = true ;
static int rx_id ;
static const void * rx_srg ;
static void * rx_dst ;
static int rx_dim ;

static MDMA_HandleTypeDef hrx = {
    .Instance = MDMA_Channel0,
    .Init = {
        .Request = MDMA_REQUEST_SW,
        .TransferTriggerMode = MDMA_FULL_TRANSFER,
        .Priority = MDMA_PRIORITY_LOW,
        .Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE,

        .SourceInc = MDMA_SRC_INC_BYTE,
        .SourceDataSize = MDMA_SRC_DATASIZE_BYTE,

        .DestinationInc = MDMA_DEST_INC_BYTE,
        .DestDataSize = MDMA_DEST_DATASIZE_BYTE,

        .DataAlignment = MDMA_DATAALIGN_PACKENABLE,

        .BufferTransferLength = 1,

        .SourceBurst = MDMA_SOURCE_BURST_SINGLE,
        .DestBurst = MDMA_DEST_BURST_SINGLE,

        .SourceBlockAddressOffset = 0,
        .DestBlockAddressOffset = 0,
    }
} ;

#define NUM_NODI_RX     4

static
__attribute__( ( section(".no_cache") ) )
MDMA_LinkNodeTypeDef vLnRx[NUM_NODI_RX] ;

static void rx_fine(MDMA_HandleTypeDef * h)
{
    DBG_FUN ;
    INUTILE(h) ;
    rx = true ;
    net_mdma_rx_esito(true) ;
}

static void rx_errore(MDMA_HandleTypeDef * h)
{
    DBG_FUN ;
    INUTILE(h) ;
    rx = true ;
    net_mdma_rx_esito(false) ;
}

__WEAK void net_mdma_iniz(void)
{
    __HAL_RCC_MDMA_CLK_ENABLE() ;

    HAL_NVIC_SetPriority(MDMA_IRQn, 5, 0) ;
    HAL_NVIC_EnableIRQ(MDMA_IRQn) ;
}

bool net_mdma_rx_iniz(void)
{
    if ( rx ) {
        rx = false ;
        rx_id = 0 ;
        return true ;
    }
    else {
        DBG_ERR ;
    }

    return false ;
}

bool net_mdma_rx_buf(
    void * d,
    const void * s,
    int dim)
{
    bool esito = false ;
    if ( 0 == rx_id ) {
        // primo nodo
        (void) HAL_MDMA_DeInit(&hrx) ;

        esito = HAL_OK == HAL_MDMA_Init(&hrx) ;
        if ( esito ) {
            rx_srg = s ;
            rx_dst = d ;
            rx_dim = dim ;
            rx_id = 1 ;
        }
        else {
            DBG_ERR ;
        }
    }
    else if ( rx_id > NUM_NODI_RX ) {
        DBG_ERR ;
    }
    else {
        // nodo successivo
        MDMA_LinkNodeConfTypeDef nodeConfig = {
            .Init = hrx.Init,
            .PostRequestMaskAddress = 0,
            .PostRequestMaskData = 0,
            .SrcAddress = UINTEGER(s),
            .DstAddress = UINTEGER(d),
            .BlockDataLength = 1,
            .BlockCount = dim,
        } ;
        if ( HAL_OK ==
             HAL_MDMA_LinkedList_CreateNode(&vLnRx[rx_id - 1], &nodeConfig) ) {
            MDMA_LinkNodeTypeDef * prec = 1 == rx_id ? NULL
                                          : &vLnRx[rx_id - 2] ;
            if ( HAL_OK ==
                 HAL_MDMA_LinkedList_AddNode(&hrx,
                                             &vLnRx[rx_id - 1],
                                             prec) ) {
                rx_id += 1 ;
                esito = true ;
            }
            else {
                DBG_ERR ;
            }
        }
        else {
            DBG_ERR ;
        }
    }

    if ( !esito ) {
        rx = true ;
    }
    return esito ;
}

bool net_mdma_rx(void)
{
    HAL_MDMA_RegisterCallback(&hrx,
                              HAL_MDMA_XFER_CPLT_CB_ID,
                              rx_fine) ;
    HAL_MDMA_RegisterCallback(&hrx,
                              HAL_MDMA_XFER_ERROR_CB_ID,
                              rx_errore) ;
    bool esito = HAL_OK == HAL_MDMA_Start_IT(&hrx,
                                             UINTEGER(rx_srg),
                                             UINTEGER(rx_dst),
                                             rx_dim,
                                             1) ;
    if ( !esito ) {
        DBG_ERR ;
        rx = true ;
    }
    return esito ;
}

void net_mdma_irq(void)
{
    HAL_MDMA_IRQHandler(&hrx) ;
//    HAL_MDMA_IRQHandler(&hmdma_mdma_channel41_sw_0) ;
}

#else

void net_mdma_iniz(void){}
void net_mdma_irq(void){}

#endif
