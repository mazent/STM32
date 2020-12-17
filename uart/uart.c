//#define STAMPA_DBG
#include "utili.h"
#include "bsp.h"
#include "bsp.uart.pmz.h"

//#define STAMPA_LA_ROBA

/*
 * La seriale viene gestita con un dma circolare + l'interruzione di idle + rts manuale
 * (l'errata riporta un baco se la controparte non rispetta il controllo di flusso)
 *
 * Al contrario di quanto descritto nel datasheet, l'interruzione di idle capita quando
 * la linea rx e' inattiva per un certo numero di bit, permettendo di realizzare
 * il timeout di ricezione. L'HAL deve essere modificata in questo modo:

		void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
			...
			extern void MZ_UART_IdleCallback(UART_HandleTypeDef *huart) ;
			if (isrflags & USART_SR_IDLE) {
				(void) huart->Instance->DR ;

				huart->Instance->CR1 &= ~(USART_CR1_IDLEIE) ;

				MZ_UART_IdleCallback(huart) ;
			}
		}

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

#define RTS(abil)   (abil) ? PIN_BAS(GPIOG, GPIO_PIN_12) : PIN_ALT(GPIOG, GPIO_PIN_12)

static UART_HandleTypeDef huartX = {
    .gState = HAL_UART_STATE_RESET,
    .RxState = HAL_UART_STATE_RESET,

    .Instance = USART6,

    .Init = {
		.WordLength = UART_WORDLENGTH_8B,
		.StopBits = UART_STOPBITS_1,
		.Parity = UART_PARITY_NONE,
		.Mode = UART_MODE_TX_RX,
		.HwFlowCtl = UART_HWCONTROL_CTS,
		.OverSampling = UART_OVERSAMPLING_16
    }
} ;

static DMA_HandleTypeDef hdma_rx = {
    .State = HAL_DMA_STATE_RESET,

    .Instance = DMA2_Stream1,

    .Init = {
		.Channel = DMA_CHANNEL_5,
		.Direction = DMA_PERIPH_TO_MEMORY,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment = DMA_MDATAALIGN_BYTE,
		.Mode = DMA_CIRCULAR,
		.Priority = DMA_PRIORITY_LOW,
		.FIFOMode = DMA_FIFOMODE_DISABLE
    }
} ;

static DMA_HandleTypeDef hdma_tx = {
    .State = HAL_DMA_STATE_RESET,

    .Instance = DMA2_Stream6,

    .Init = {
		.Channel = DMA_CHANNEL_5,
		.Direction = DMA_MEMORY_TO_PERIPH,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment = DMA_MDATAALIGN_BYTE,
		.Mode = DMA_NORMAL,
		.Priority = DMA_PRIORITY_LOW,
		.FIFOMode = DMA_FIFOMODE_DISABLE
    }
} ;

static UPMZ_CALLBACK pfIrq = NULL ;
static UPMZ_CALLBACK pfTx = NULL ;

#ifndef UPMZ_DIM_FIFO
#   error MANCA UPMZ_DIM_FIFO
#elif UPMZ_DIM_FIFO == 0
#   error FIFO DI DIMENSIONE NULLA
#elif UPMZ_DIM_FIFO > 0xFFFF
#   error FIFO TROPPO GRANDE
#else
#   define UART_DIM_DMA    UPMZ_DIM_FIFO
static uint32_t preNDTR = UART_DIM_DMA ;
static uint8_t dmacRx[UART_DIM_DMA] ;
#endif

static bool ricevi(void)
{
	preNDTR = UART_DIM_DMA ;

	bool esito = HAL_OK == HAL_UART_Receive_DMA(&huartX, dmacRx, UART_DIM_DMA) ;
	if (esito) {
    	huartX.Instance->CR1 |= USART_CR1_IDLEIE ;
    	RTS(true) ;
	}
	else {
		DBG_ERR ;
	}

    return esito ;
}

static bool iniz(void)
{
	bool esito = false ;

	do {
	    if (HAL_OK != HAL_UART_Init(&huartX)) {
	    	DBG_ERR ;
	    	break ;
	    }

	    esito = ricevi() ;

	} while (false) ;

	return esito ;
}

/*************************************
    Parte esportata
*************************************/

bool UPMZ_Iniz(uint32_t baud, UPMZ_CALLBACK pf)
{
    huartX.Init.BaudRate = baud ;
    pfIrq = pf ;

    return iniz() ;
}

void UPMZ_ReIniz(void)
{
    iniz() ;
}

void UPMZ_Fine(void)
{
    HAL_DMA_Abort(&hdma_rx) ;
    HAL_DMA_Abort(&hdma_tx) ;

    HAL_UART_DeInit(&huartX) ;
}

bool UPMZ_Tx(const void * tx, uint32_t dim)
{
    if (HAL_OK == HAL_UART_Transmit_DMA(&huartX, (const uint8_t *) tx, dim)) {
#ifdef STAMPA_LA_ROBA
    	const size_t DIM = min(dim, 16) ;
    	const uint8_t * Dati = (const uint8_t *) tx ;

    	PRINTF("UPMZ -> [%u] %02X ", dim, Dati[0]) ;
    	for (size_t i=1 ; i<DIM ; i++) {
    		PRINTF("%02X ", Dati[i]) ;
    	}
    	if (dim <= DIM)
    		PUTS("") ;
    	else
    		PUTS("...") ;
#endif

    	return true ;
    }
    else {
    	BPOINT ;

    	return false ;
    }
}

bool UPMZ_TxCB(const void * tx, uint32_t dim, UPMZ_CALLBACK pf)
{
	assert(NULL == pfTx) ;

	pfTx = pf ;

	return UPMZ_Tx(tx, dim) ;
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

uint32_t UPMZ_Rx(void * rx, uint32_t dim)
{
    uint32_t letti = 0 ;
    uint8_t * dati = (uint8_t *) rx ;

    do {
        uint32_t valNDTR = hdma_rx.Instance->NDTR ;

        // Quanti byte ci sono?
        if (valNDTR <= preNDTR)
            letti = preNDTR - valNDTR ;
        else {
            // Ricircolo !
            letti = UART_DIM_DMA - (valNDTR - preNDTR) ;
        }

        if ( (0 == dati) || (0 == dim) ) {
            // Vogliono sapere quanta roba c'e'
            break ;
        }

        if (0 == letti) {
        	DBG_ERR ;
        	if (HAL_OK != HAL_UART_DMAStop(&huartX)) {
        		DBG_ERR ;
        		break ;
        	}
        	else if (!ricevi())
        		break ;
        }

        letti = min(dim, letti) ;

        // Vogliono leggere la roba
        uint32_t pre = UART_DIM_DMA - preNDTR ;
#if 1
        for (dim = 0 ; dim < letti ; dim++) {
            dati[dim] = dmacRx[pre] ;
            pre++ ;
            if (UART_DIM_DMA == pre)
                pre = 0 ;
        }
#else
        dim = letti ;
        uint32_t da_leggere = letti ;

		while (true) {
			// In un colpo posso leggere o fino alla fine:
			//                l   U
			//     ****.......****
			// o fino al totale:
			//         l      U
			//     ....*******....
			const uint16_t ULTIMO = min(UART_DIM_DMA, pre + da_leggere) ;
			const uint16_t DIM = min(dim, ULTIMO - pre) ;
			if (1 == DIM)
				dati[0] = dmacRx[pre] ;
			else
				memcpy(dati, dmacRx + pre, DIM) ;

			pre += DIM ;
			if (pre >= UART_DIM_DMA)
				pre -= UART_DIM_DMA ;

			dim -= DIM ;
			if (0 == dim)
				break ;

			dati += DIM ;
			da_leggere -= DIM ;
		}
#endif
        preNDTR = UART_DIM_DMA - pre ;

#ifdef STAMPA_LA_ROBA
		{
			const size_t DIM = min(dim, 16) ;
			PRINTF("PMZ <- [%u] ", dim) ;
			for (size_t i=0 ; i<DIM ; i++) {
				PRINTF("%02X ", dati[i]) ;
			}
			if (dim <= DIM)
				PUTS("") ;
			else
				PUTS("...") ;
		}
#endif

	    // Riabilito
		huartX.Instance->CR1 |= USART_CR1_IDLEIE ;
	    RTS(true) ;

    } while (false) ;

    return letti ;
}


// Irq

#pragma call_graph_root="interruzione"
void USART6_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huartX);
}

#pragma call_graph_root="interruzione"
void DMA2_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_rx) ;
}

#pragma call_graph_root="interruzione"
void DMA2_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_tx) ;
}

// MSP

void UPMZ_MSP_Iniz(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF8_USART6
    } ;

    /** USART6 GPIO Configuration
        PG12  -----> USART6_RTS no-pull
        PG13  -----> USART6_CTS no-pull
        PG9   -----> USART6_RX  pull-up
        PC6   -----> USART6_TX  pull-up
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9 ;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct) ;

    GPIO_InitStruct.Pin = GPIO_PIN_6 ;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct) ;

    GPIO_InitStruct.Pull = GPIO_NOPULL ;
    GPIO_InitStruct.Pin = GPIO_PIN_13 ;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct) ;

    // RTS a mano
    RTS(false) ;
    GPIO_InitStruct.Pin = GPIO_PIN_12 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP ;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct) ;

    __HAL_RCC_USART6_CLK_ENABLE() ;

    /* Peripheral DMA init*/

    if (HAL_DMA_Init(&hdma_rx) != HAL_OK) {
        DBG_ERR ;
    }
    __HAL_LINKDMA(&huartX, hdmarx, hdma_rx) ;

    if (HAL_DMA_Init(&hdma_tx) != HAL_OK) {
        DBG_ERR ;
    }
    __HAL_LINKDMA(&huartX, hdmatx, hdma_tx) ;

    // Irq
    HAL_NVIC_SetPriority(USART6_IRQn, UPMZ_IRQ_PRI, 0) ;
    HAL_NVIC_EnableIRQ(USART6_IRQn) ;

    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, UPMZ_IRQ_PRI, 0) ;
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn) ;

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, UPMZ_IRQ_PRI, 0) ;
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn) ;
}

void UPMZ_MSP_Fine(void)
{
    NVIC_DisableIRQ(DMA2_Stream1_IRQn) ;
    NVIC_DisableIRQ(DMA2_Stream6_IRQn) ;
    NVIC_DisableIRQ(USART6_IRQn) ;

    __HAL_RCC_USART6_CLK_DISABLE() ;

    MZ_GPIO_DeInit(GPIOG, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_9) ;
    MZ_GPIO_DeInit(GPIOC, GPIO_PIN_6) ;

    HAL_DMA_DeInit(&hdma_rx) ;
    HAL_DMA_DeInit(&hdma_tx) ;
}

// Callback (p.e HAL_UART_TxCpltCallback)

void UPMZ_IdleCallback(void)
{
	pfIrq() ;
}

void UPMZ_TxCpltCallback(void)
{
	if (pfTx) {
		pfTx() ;
		pfTx = NULL ;
	}
}

void UPMZ_TxHalfCpltCallback(void)
{
}

void UPMZ_RxCpltCallback(void)
{
    RTS(false) ;
#if 0
    // Non serve: scatta l'idle
    pfIrq() ;
#endif
}

void UPMZ_RxHalfCpltCallback(void)
{
    RTS(false) ;
#if 0
    // Non serve: scatta l'idle
    pfIrq() ;
#endif
}

void UPMZ_ErrorCallback(void)
{
#ifdef STAMPA_DBG
	if (hdma_tx.ErrorCode != HAL_DMA_ERROR_NONE) {
		// Questo errore interessa gli strati superiori, che ritenteranno
		if (HAL_DMA_ERROR_TE            & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: TE           ") ;
		if (HAL_DMA_ERROR_FE            & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: FE           ") ;
		if (HAL_DMA_ERROR_DME           & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: DME          ") ;
		if (HAL_DMA_ERROR_TIMEOUT       & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: TIMEOUT      ") ;
		if (HAL_DMA_ERROR_PARAM         & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: PARAM        ") ;
		if (HAL_DMA_ERROR_NO_XFER       & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: NO_XFER      ") ;
		if (HAL_DMA_ERROR_NOT_SUPPORTED & hdma_tx.ErrorCode) DBG_PUTS("UPMZ ERR TX: NOT_SUPPORTED") ;
	}
#endif

	if (hdma_rx.ErrorCode != HAL_DMA_ERROR_NONE) {
		// Lo faccio ripartire
	    preNDTR = UART_DIM_DMA ;

	    if ( HAL_OK != HAL_UART_Receive_DMA(&huartX, dmacRx, UART_DIM_DMA) ) {
	        BPOINT ;
	    }
	    else {
	    	RTS(true) ;
	    }
#ifdef STAMPA_DBG
		if (HAL_DMA_ERROR_TE            & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: TE           ") ;
		if (HAL_DMA_ERROR_FE            & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: FE           ") ;
		if (HAL_DMA_ERROR_DME           & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: DME          ") ;
		if (HAL_DMA_ERROR_TIMEOUT       & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: TIMEOUT      ") ;
		if (HAL_DMA_ERROR_PARAM         & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: PARAM        ") ;
		if (HAL_DMA_ERROR_NO_XFER       & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: NO_XFER      ") ;
		if (HAL_DMA_ERROR_NOT_SUPPORTED & hdma_rx.ErrorCode) DBG_PUTS("UPMZ ERR RX: NOT_SUPPORTED") ;
#endif
	}
}
