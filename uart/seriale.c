#define STAMPA_DBG
#include "dbgp.h"
#include "utili.h"
#include "seriale.h"
#include "main.h"
#include "bsp.h"

#include <string.h>

/*
 * La seriale viene gestita con un dma circolare + l'interruzione di idle + rts manuale
 * (l'errata riporta un baco se la controparte non rispetta il controllo di flusso)
 *
 * Al contrario di quanto descritto nel datasheet, l'interruzione di idle capita quando
 * la linea rx e' inattiva per un certo numero di bit, permettendo di realizzare
 * il timeout di ricezione. L'HAL deve essere modificata in questo modo:

        void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
            ...
            if (isrflags & USART_SR_IDLE) {
                extern void SER_IdleCallback(void);

                __HAL_UART_CLEAR_IDLEFLAG(huart);

                SER_IdleCallback() ;
            }
            ...
        }
   Inutilizzabile HAL_UART_RECEPTION_TOIDLE
 *
 * Complessivamente:
 *    1) le due interruzioni di half/full dma attivano l'rts sospendendo la trasmissione
 *    2) scatta l'interruzione idle e viene invocata la callback passata alla X_Iniz
 *    3) la X_Rx disattiva l'rts permettendo al trasmettitore di riprendere (conviene
 *       che venga invocata con un buffer di dimensione uguale a quella della fifo)
 *
 * La fifo deve essere dimensionata in modo che non vengano persi caratteri durante
 * il periodo di latenza dell'interruzione + quello di latenza dell'rts del trasmettitore
 *
 * Se manca il controllo di flusso hw, UART_DIM_DMA deve essere in grado di contenere
 * un pacchetto completo
 *
 * NOTA
 *    Conviene anche questa modifica:

        HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size)
        {
            uint32_t *tmp;

            // MZ  if(huart->gState == HAL_UART_STATE_READY)
            if (HAL_DMA_STATE_READY == huart->hdmatx->State)
            ...
 */

//#define STAMPA_ROBA         1

static inline void RTS(bool attivo)
{
    if ( attivo ) {
        // Inviami la roba
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET) ;
    }
    else {
        // Fermati!
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET) ;
    }
}

static UART_HandleTypeDef huart = {
    .gState = HAL_UART_STATE_RESET,
    .RxState = HAL_UART_STATE_RESET,

    .Instance = USART3,

    .Init = {
        .BaudRate = 460800,
        .WordLength = UART_WORDLENGTH_8B,
        .StopBits = UART_STOPBITS_1,
        .Parity = UART_PARITY_NONE,
        .Mode = UART_MODE_TX_RX,
        .HwFlowCtl = UART_HWCONTROL_CTS,
        .OverSampling = UART_OVERSAMPLING_16
    }
} ;

static DMA_HandleTypeDef hdma_tx = {
    .State = HAL_DMA_STATE_RESET,

    .Instance = DMA1_Channel2,

    .Init = {
        .Direction = DMA_MEMORY_TO_PERIPH,
        .PeriphInc = DMA_PINC_DISABLE,
        .MemInc = DMA_MINC_ENABLE,
        .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
        .MemDataAlignment = DMA_MDATAALIGN_BYTE,
        .Mode = DMA_NORMAL,
        .Priority = DMA_PRIORITY_LOW,
    }
} ;

static DMA_HandleTypeDef hdma_rx = {
    .State = HAL_DMA_STATE_RESET,

    .Instance = DMA1_Channel3,

    .Init = {
        .Direction = DMA_PERIPH_TO_MEMORY,
        .PeriphInc = DMA_PINC_DISABLE,
        .MemInc = DMA_MINC_ENABLE,
        .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
        .MemDataAlignment = DMA_MDATAALIGN_BYTE,
        .Mode = DMA_CIRCULAR,
        .Priority = DMA_PRIORITY_LOW,
    }
} ;

static SER_CALLBACK pfCB = NULL ;
static void * cbARG = NULL ;

static SER_CALLBACK pfTx = NULL ;

#ifndef SER_DIM_FIFO
#   error MANCA SER_DIM_FIFO
#elif SER_DIM_FIFO == 0
#   error FIFO DI DIMENSIONE NULLA
#elif SER_DIM_FIFO > 0xFFFF
#   error FIFO TROPPO GRANDE
#else
#   define UART_DIM_DMA    SER_DIM_FIFO
static uint32_t preNDTR = UART_DIM_DMA ;
static uint8_t dmacRx[UART_DIM_DMA] ;
#endif

static bool ricevi(void)
{
    preNDTR = UART_DIM_DMA ;

    bool esito = HAL_OK == HAL_UART_Receive_DMA(&huart, dmacRx, UART_DIM_DMA) ;
    if ( esito ) {
        SET_BIT(huart.Instance->CR1, USART_CR1_IDLEIE) ;
        RTS(true) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

static bool iniz(uint32_t baud)
{
    bool esito = false ;

    do {
        huart.Init.BaudRate = baud ;
        if ( HAL_OK != HAL_UART_Init(&huart) ) {
            DBG_ERR ;
            break ;
        }

        esito = ricevi() ;
    } while ( false ) ;

    return esito ;
}

/*************************************
    Parte esportata
*************************************/

bool SER_Iniz(
    uint32_t baud,
    SER_CALLBACK cb,
    void * arg)
{
    do {
        if ( NULL == cb ) {
            break ;
        }

        if ( pfCB ) {
            break ;
        }

        pfCB = cb ;
        cbARG = arg ;
    } while ( false ) ;

    return iniz(baud) ;
}

void SER_Fine(void)
{
    HAL_DMA_Abort(&hdma_rx) ;
    HAL_DMA_Abort(&hdma_tx) ;

    HAL_UART_DeInit(&huart) ;

    pfCB = NULL ;
}

bool SER_Tx(
    const void * tx,
    uint32_t dim)
{
    if ( HAL_OK == HAL_UART_Transmit_DMA(&huart, CONST_CAST(tx), dim) ) {
#ifdef STAMPA_ROBA
        DBG_PRINT_HEX("-> SER", tx, dim) ;
#else
        //DBG_PRINTF("[%d] -> SER", (int) dim) ;
#endif
        return true ;
    }
    else {
        DBG_ERR ;

        return false ;
    }
}

bool SER_TxCB(
    const void * tx,
    uint32_t dim,
    SER_CALLBACK pf)
{
    pfTx = pf ;

    return SER_Tx(tx, dim) ;
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

uint32_t SER_Rx(
    void * rx,
    uint32_t dim)
{
    uint32_t letti = 0 ;
    uint8_t * dati = (uint8_t *) rx ;

    do {
        uint32_t valNDTR = hdma_rx.Instance->CNDTR ;

        // Quanti byte ci sono?
        if ( valNDTR <= preNDTR ) {
            letti = preNDTR - valNDTR ;
        }
        else {
            // Ricircolo !
            letti = UART_DIM_DMA - (valNDTR - preNDTR) ;
        }

        if ( (NULL == dati) || (0 == dim) ) {
            // Vogliono sapere quanta roba c'e'
            DBG_QUA ;
            break ;
        }

        if ( 0 == letti ) {
            DBG_QUA ;
            break ;
        }

        letti = MINI(dim, letti) ;

        // Vogliono leggere la roba
        uint32_t pre = UART_DIM_DMA - preNDTR ;
#if 0
        for ( dim = 0 ; dim < letti ; dim++ ) {
            dati[dim] = dmacRx[pre] ;
            pre++ ;
            if ( UART_DIM_DMA == pre ) {
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
            const uint16_t ULTIMO = MINI(UART_DIM_DMA, pre + da_leggere) ;
            const uint16_t DIM = MINI(dim, ULTIMO - pre) ;
            if ( 1 == DIM ) {
                dati[0] = dmacRx[pre] ;
            }
            else {
                memcpy(dati, dmacRx + pre, DIM) ;
            }

            pre += DIM ;
            if ( pre >= UART_DIM_DMA ) {
                pre -= UART_DIM_DMA ;
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
        DBG_PRINT_HEX("<- SER", rx, letti) ;
#else
        //DBG_PRINTF("[%d] <- SER", (int) letti) ;
#endif

        // Aggiorno
        preNDTR = UART_DIM_DMA - pre ;

        // Riabilito
        RTS(true) ;
    } while ( false ) ;

    return letti ;
}

// Irq

#pragma call_graph_root="interruzione"
void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart) ;
}

#pragma call_graph_root="interruzione"
void DMA1_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_rx) ;
}

#pragma call_graph_root="interruzione"
void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_tx) ;
}

// MSP

void HAL_UART_MspInit(UART_HandleTypeDef * uh)
{
    GPIO_InitTypeDef GPIO_InitStruct = {
        0
    } ;

    if ( uh->Instance == USART3 ) {
        /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE() ;

        /**USART3 GPIO Configuration
            PB10 --> USART3_TX
            PB11 --> USART3_RX
            PB13 --> USART3_CTS
            PB14 --> USART3_RTS
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10 ;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP ;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH ;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct) ;

        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13 ;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT ;
        GPIO_InitStruct.Pull = GPIO_NOPULL ;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct) ;

        // RTS a mano
        RTS(false) ;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP ;
        GPIO_InitStruct.Pin = GPIO_PIN_14 ;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct) ;

        // DMA
        if ( HAL_DMA_Init(&hdma_rx) != HAL_OK ) {
            DBG_ERR ;
        }

        __HAL_LINKDMA(uh, hdmarx, hdma_rx) ;

        /* USART3_TX Init */
        if ( HAL_DMA_Init(&hdma_tx) != HAL_OK ) {
            DBG_ERR ;
        }

        __HAL_LINKDMA(uh, hdmatx, hdma_tx) ;

        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART3_IRQn, 0, 0) ;
        HAL_NVIC_EnableIRQ(USART3_IRQn) ;

        HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0) ;
        HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn) ;

        HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0) ;
        HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn) ;
    }
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef * uh)
{
    if ( uh->Instance == USART3 ) {
        HAL_NVIC_DisableIRQ(USART3_IRQn) ;
        HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn) ;
        HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn) ;

        /* Peripheral clock disable */
        __HAL_RCC_USART3_CLK_DISABLE() ;

        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_14 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_13) ;

        /* USART3 DMA DeInit */
        HAL_DMA_DeInit(uh->hdmarx) ;
        HAL_DMA_DeInit(uh->hdmatx) ;
    }
}

// Callback (p.e HAL_UART_TxCpltCallback)

void SER_IdleCallback(void)
{
    //DBG_FUN ;

    pfCB(cbARG) ;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef * h)
{
    INUTILE(h) ;

    if ( pfTx ) {
        SER_CALLBACK cb = pfTx ;
        pfTx = NULL ;
        cb(cbARG) ;
    }
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

void HAL_UART_ErrorCallback(UART_HandleTypeDef * h)
{
    INUTILE(h) ;

#ifdef STAMPA_DBG
    if ( hdma_tx.ErrorCode != HAL_DMA_ERROR_NONE ) {
        // Questo errore interessa gli strati superiori, che ritenteranno
        if ( HAL_DMA_ERROR_TE & hdma_tx.ErrorCode ) {
            DBG_PUTS("SER ERR TX: TE           ") ;
        }
        if ( HAL_DMA_ERROR_TIMEOUT & hdma_tx.ErrorCode ) {
            DBG_PUTS("SER ERR TX: TIMEOUT      ") ;
        }
        if ( HAL_DMA_ERROR_NO_XFER & hdma_tx.ErrorCode ) {
            DBG_PUTS("SER ERR TX: NO_XFER      ") ;
        }
        if ( HAL_DMA_ERROR_NOT_SUPPORTED & hdma_tx.ErrorCode ) {
            DBG_PUTS("SER ERR TX: NOT_SUPPORTED") ;
        }
    }
#endif

    if ( hdma_rx.ErrorCode != HAL_DMA_ERROR_NONE ) {
        // Lo faccio ripartire
        CHECK( ricevi() ) ;

#ifdef STAMPA_DBG
        if ( HAL_DMA_ERROR_TE & hdma_rx.ErrorCode ) {
            DBG_PUTS("SER ERR RX: TE           ") ;
        }
        if ( HAL_DMA_ERROR_TIMEOUT & hdma_rx.ErrorCode ) {
            DBG_PUTS("SER ERR RX: TIMEOUT      ") ;
        }
        if ( HAL_DMA_ERROR_NO_XFER & hdma_rx.ErrorCode ) {
            DBG_PUTS("SER ERR RX: NO_XFER      ") ;
        }
        if ( HAL_DMA_ERROR_NOT_SUPPORTED & hdma_rx.ErrorCode ) {
            DBG_PUTS("SER ERR RX: NOT_SUPPORTED") ;
        }
#endif
    }
}
