#define STAMPA_DBG
#include "utili.h"
#include "cybt_u.h"
#include "stm32h7xx_hal.h"
#include "bsp.h"

//#define STAMPA_ROBA		1

UART_HandleTypeDef huart6 = {
    .Instance = USART6,
    .Init = {
        // Costanti
        .WordLength = UART_WORDLENGTH_8B,
        .StopBits = UART_STOPBITS_1,
        .Parity = UART_PARITY_NONE,
        .Mode = UART_MODE_TX_RX,
        .OverSampling = UART_OVERSAMPLING_16,
        .OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE,
        .ClockPrescaler = UART_PRESCALER_DIV1,
        // Variabili
        .BaudRate = 115200,
        .HwFlowCtl = UART_HWCONTROL_NONE,
    },
    .AdvancedInit = {
        .AdvFeatureInit = UART_ADVFEATURE_NO_INIT,
    }
} ;

DMA_HandleTypeDef hdma_usart6_rx ;

#ifdef USA_CACHE
#define IN_RIGHE        ( (CYBTU_DIM_RX + __SCB_DCACHE_LINE_SIZE - 1) \
                          / __SCB_DCACHE_LINE_SIZE )
__attribute__( ( aligned(__SCB_DCACHE_LINE_SIZE) ) )
__attribute__( ( section(".sram12") ) )
static uint8_t dmacRx[__SCB_DCACHE_LINE_SIZE * IN_RIGHE] ;
#else
__attribute__( ( section(".sram12") ) )
static uint8_t dmacRx[CYBTU_DIM_RX] ;
#endif

static uint32_t preNDTR = sizeof(dmacRx) ;

// Nella prima fase viene ignorato
#define RTS         rts_6
#define RTS_LIV     rts_6_liv

static bool ricevi(void)
{
    preNDTR = sizeof(dmacRx) ;

    bool esito = HAL_OK == HAL_UART_Receive_DMA( &huart6, dmacRx, sizeof(dmacRx) ) ;
    if ( esito ) {
        SET_BIT(huart6.Instance->CR1, USART_CR1_IDLEIE) ;
        RTS(true) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool BTU_iniz(
    uint32_t baud,
    bool cfhw)
{
    DBG_PRINTF("%s(%u,%s)", __func__, baud, cfhw ? "CFHW" : "cfhw") ;

    huart6.Init.BaudRate = baud ;
    huart6.Init.HwFlowCtl = cfhw ? UART_HWCONTROL_CTS : UART_HWCONTROL_NONE ;
    bool esito = false ;
    do {
        if ( HAL_UART_Init(&huart6) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }
        if ( HAL_UARTEx_SetTxFifoThreshold(&huart6,
                                           UART_TXFIFO_THRESHOLD_1_8) !=
             HAL_OK ) {
            DBG_ERR ;
            break ;
        }
        if ( HAL_UARTEx_SetRxFifoThreshold(&huart6,
                                           UART_RXFIFO_THRESHOLD_1_8) !=
             HAL_OK ) {
            DBG_ERR ;
            break ;
        }
        if ( HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK ) {
            DBG_ERR ;
            break ;
        }

        esito = ricevi() ;
    } while ( false ) ;
    return esito ;
}

void BTU_fine(void)
{
    if ( HAL_UART_STATE_RESET != huart6.gState ) {
        RTS(false) ;
        HAL_DMA_Abort(&hdma_usart6_rx) ;

        HAL_UART_DeInit(&huart6) ;
    }
}

bool BTU_tx(
    const void * tx,
    uint32_t dim)
{
    if ( HAL_OK == HAL_UART_Transmit(&huart6, tx, dim, 1000) ) {
#ifdef STAMPA_ROBA
        DBG_PRINT_HEX("-> BTU", tx, dim) ;
#else
        //DBG_PRINTF("[%d] -> BTU", (int) dim) ;
#endif
        return true ;
    }

    DBG_ERR ;

    return false ;
}

// Il contatore del dma decrementa ad ogni carattere ricevuto
// per cui se si ricevono k byte ci sono due possibilita':
//
//                dmacRx NDTR
//              +-------+-----+
//              |  0    | N-1 |  | .
//              +-------+-----+  | .
//              |       |     |  V k-2
//   preNDTR -->|       |     |<-- valNDTR
//         0 |  |       |     |
//         1 |  +-------+-----+
//         . |  | N-1-x |  x  |
//         . |  +-------+-----+
//       k-2 V  |       |     |
//   valNDTR -->|       |     |<-- preNDTR
//              |       |     |  | 0
//              +-------+-----+  | 1
//              |  N-1  |  0  |  V .
//              +-------+-----+

uint32_t BTU_rx(
    void * rx,
    uint32_t dim)
{
    if ( HAL_UART_STATE_RESET == huart6.RxState ) {
        DBG_ERR ;
        return 0 ;
    }

    if ( !RTS_LIV() ) {
        // RTS attivo: invocata senza che sia scattata idle
        DBG_QUA ;
        return 0 ;
    }

    uint32_t letti = 0 ;
    uint8_t * dati = (uint8_t *) rx ;

    DMA_Stream_TypeDef * inst =
        ( (DMA_Stream_TypeDef *) hdma_usart6_rx.Instance ) ;
    uint32_t valNDTR = inst->NDTR ;

    // Quanti byte ci sono?
    if ( valNDTR <= preNDTR ) {
        letti = preNDTR - valNDTR ;
    }
    else {
        // Ricircolo !
        letti = sizeof(dmacRx) - (valNDTR - preNDTR) ;
    }

    if ( 0 == letti ) {
        // La IDLE e' scattata ma non e' arrivato niente
        DBG_QUA ;
    }
    else {
#ifdef USA_CACHE
        SCB_InvalidateDCache_by_Addr( dmacRx, sizeof(dmacRx) ) ;
#endif
        letti = MINI(dim, letti) ;

        // Vogliono leggere la roba
        uint32_t pre = sizeof(dmacRx) - preNDTR ;
#if 0
        for ( dim = 0 ; dim < letti ; dim++ ) {
            dati[dim] = dmacRx[pre] ;
            pre++ ;
            if ( sizeof(dmacRx) == pre ) {
                pre = 0 ;
            }
        }
#else
        dim = letti ;
        uint32_t da_leggere = letti ;

        while ( true ) {
            // In un colpo posso leggere o fino alla fine:
            //                l   U
            //     ****.......****
            // o fino al totale:
            //         l      U
            //     ....*******....
            const uint16_t ULTIMO = MINI(sizeof(dmacRx), pre + da_leggere) ;
            const uint16_t DIM = MINI(dim, ULTIMO - pre) ;
            if ( 1 == DIM ) {
                dati[0] = dmacRx[pre] ;
            }
            else {
                memcpy_(dati, dmacRx + pre, DIM) ;
            }

            pre += DIM ;
            if ( pre >= sizeof(dmacRx) ) {
                pre -= sizeof(dmacRx) ;
            }

            dim -= DIM ;
            if ( 0 == dim ) {
                break ;
            }

            dati += DIM ;
            da_leggere -= DIM ;
        }
#endif
#ifdef STAMPA_ROBA
        DBG_PRINT_HEX("<- BTU", rx, letti) ;
#else
        //DBG_PRINTF("[%d] <- BTU", (int) letti) ;
#endif

        // Aggiorno
        preNDTR = sizeof(dmacRx) - pre ;
    }

    // Riabilito
    RTS(true) ;

    return letti ;
}

void USART6_IRQHandler(void)
{
    uint32_t isrflags = READ_REG(huart6.Instance->ISR) ;
    if ( isrflags & USART_ISR_IDLE ) {
        RTS(false) ;

        __HAL_UART_CLEAR_IDLEFLAG(&huart6) ;

        btu_nuovidati() ;
    }

    HAL_UART_IRQHandler(&huart6) ;
}

void DMA1_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart6_rx) ;
}

__weak void btu_nuovidati(void)
{
    DBG_PRINTF("IMPLEMENTA: %s", __func__) ;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * h)
{
    INUTILE(h) ;

    RTS(false) ;
#if 0
    // Non serve: scatta l'idle
    pfCB(cbARG) ;
#endif
//    DBG_FUN ;
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef * h)
{
    INUTILE(h) ;

    RTS(false) ;
#if 0
    // Non serve: scatta l'idle
    pfCB(cbARG) ;
#endif
//    DBG_FUN ;
}

void btu_errorcallback(void)
{
//#ifdef STAMPA_DBG
//    if ( hdma_usart6_tx.ErrorCode != HAL_DMA_ERROR_NONE ) {
//        // Questo errore interessa gli strati superiori, che ritenteranno
//        if ( HAL_DMA_ERROR_TE & hdma_usart6_tx.ErrorCode ) {
//            DBG_PUTS("BTU ERR TX: TE           ") ;
//        }
//        if ( HAL_DMA_ERROR_TIMEOUT & hdma_usart6_tx.ErrorCode ) {
//            DBG_PUTS("BTU ERR TX: TIMEOUT      ") ;
//        }
//        if ( HAL_DMA_ERROR_NO_XFER & hdma_usart6_tx.ErrorCode ) {
//            DBG_PUTS("BTU ERR TX: NO_XFER      ") ;
//        }
//        if ( HAL_DMA_ERROR_NOT_SUPPORTED & hdma_usart6_tx.ErrorCode ) {
//            DBG_PUTS("BTU ERR TX: NOT_SUPPORTED") ;
//        }
//    }
//#endif

#ifdef STAMPA_DBG
    if ( hdma_usart6_rx.ErrorCode != HAL_DMA_ERROR_NONE ) {
        if ( HAL_DMA_ERROR_TE & hdma_usart6_rx.ErrorCode ) {
            DBG_PUTS("BTU ERR RX: TE           ") ;
        }
        if ( HAL_DMA_ERROR_TIMEOUT & hdma_usart6_rx.ErrorCode ) {
            DBG_PUTS("BTU ERR RX: TIMEOUT      ") ;
        }
        if ( HAL_DMA_ERROR_NO_XFER & hdma_usart6_rx.ErrorCode ) {
            DBG_PUTS("BTU ERR RX: NO_XFER      ") ;
        }
        if ( HAL_DMA_ERROR_NOT_SUPPORTED & hdma_usart6_rx.ErrorCode ) {
            DBG_PUTS("BTU ERR RX: NOT_SUPPORTED") ;
        }
    }
#endif
    if ( hdma_usart6_rx.State != HAL_DMA_STATE_BUSY ) {
        // Lo faccio ripartire
        CONTROLLA( ricevi() ) ;
    }
}
