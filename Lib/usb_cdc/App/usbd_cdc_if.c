#define STAMPA_DBG
#include "utili.h"
#include "diario/diario.h"
#include "usbd_cdc_if.h"
#include "../usb_uart.h"

#ifndef USB_UART_NO_CIRCO
#include "circo/circo.h"
#define MAX_BUFF        APP_RX_DATA_SIZE

static union {
    S_CIRCO c ;
    uint8_t b[sizeof(S_CIRCO) - 1 + MAX_BUFF] ;
} u ;
#endif

#ifndef USB_UART_NO_CIRCO
static void cb_vuota(void * v)
{
    UNUSED(v) ;
}

#else
static void cb_vuota(
    void * v,
    void * r,
    uint32_t d)
{
    UNUSED(v) ;
    UNUSED(r) ;
    UNUSED(d) ;
}

#endif

static USBU_CALLBACK rx_cb = cb_vuota ;
static void * cb_arg = NULL ;
static bool iniz = false ;

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
static uint8_t UserRxBufferHS[CDC_DATA_HS_IN_PACKET_SIZE] ;
#ifdef USB_UART_NO_CIRCO
static uint8_t UserRxBufferHS2[CDC_DATA_HS_IN_PACKET_SIZE] ;
static uint8_t * rx_buf = NULL ;
static uint32_t rx_flip = 0 ;
#endif

static USBD_HandleTypeDef hUsbDeviceHS ;
extern USBD_DescriptorsTypeDef HS_Desc ;

/**
  * @brief  Initializes the CDC media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_HS(void)
{
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, UserRxBufferHS) ;
#ifdef USB_UART_NO_CIRCO
    rx_buf = UserRxBufferHS ;
    rx_flip = UINTEGER(UserRxBufferHS) ^ UINTEGER(UserRxBufferHS2) ;
#endif
    return USBD_OK ;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_HS(void)
{
    return USBD_OK ;
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

#define S_LINE_CODING_FORMAT_1___STOP_BIT        0
#define S_LINE_CODING_FORMAT_1_5_STOP_BITS       1
#define S_LINE_CODING_FORMAT_2___STOP_BITS       2

#define S_LINE_CODING_PARITYTYPE_NONE         0
#define S_LINE_CODING_PARITYTYPE_ODD          1
#define S_LINE_CODING_PARITYTYPE_EVEN         2
#define S_LINE_CODING_PARITYTYPE_MARK         3
#define S_LINE_CODING_PARITYTYPE_SPACE        4

#pragma pack(1)
// Come USBD_CDC_LineCodingTypeDef ma con la dimensione giusta
typedef struct {
    uint32_t bitrate ;
    uint8_t format ;
    uint8_t paritytype ;
    uint8_t datatype ;
} S_LINE_CODING ;

#pragma pack()

static S_LINE_CODING lineCoding = {
    .bitrate = 115200,
    .format = S_LINE_CODING_FORMAT_1___STOP_BIT,
    .paritytype = S_LINE_CODING_PARITYTYPE_NONE,
    .datatype = 8
} ;

static int8_t CDC_Control_HS(
    uint8_t cmd,
    uint8_t * pbuf,
    uint16_t length)
{
    switch ( cmd ) {
    case CDC_SEND_ENCAPSULATED_COMMAND:
        DDB_DEBUG("CDC_SEND_ENCAPSULATED_COMMAND") ;
        break ;

    case CDC_GET_ENCAPSULATED_RESPONSE:
        DDB_DEBUG("CDC_GET_ENCAPSULATED_RESPONSE") ;
        break ;

    case CDC_SET_COMM_FEATURE:
        DDB_DEBUG("CDC_SET_COMM_FEATURE") ;
        break ;

    case CDC_GET_COMM_FEATURE:
        DDB_DEBUG("CDC_GET_COMM_FEATURE") ;
        break ;

    case CDC_CLEAR_COMM_FEATURE:
        DDB_DEBUG("CDC_CLEAR_COMM_FEATURE") ;
        break ;

    /*******************************************************************************/
    /* Line Coding Structure                                                       */
    /*-----------------------------------------------------------------------------*/
    /* Offset | Field       | Size | Value  | Description                          */
    /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
    /* 4      | bCharFormat |   1  | Number | Stop bits                            */
    /*                                        0 - 1 Stop bit                       */
    /*                                        1 - 1.5 Stop bits                    */
    /*                                        2 - 2 Stop bits                      */
    /* 5      | bParityType |  1   | Number | Parity                               */
    /*                                        0 - None                             */
    /*                                        1 - Odd                              */
    /*                                        2 - Even                             */
    /*                                        3 - Mark                             */
    /*                                        4 - Space                            */
    /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
    /*******************************************************************************/
    case CDC_SET_LINE_CODING:
        DDB_DEBUG("CDC_SET_LINE_CODING") ;
        DDB_PRINT_HEX( "\t", pbuf, sizeof(lineCoding) ) ;
        if ( sizeof(lineCoding) == length ) {
            memcpy_( &lineCoding, pbuf, sizeof(lineCoding) ) ;
        }
        break ;

    case CDC_GET_LINE_CODING:
        DDB_DEBUG("CDC_GET_LINE_CODING") ;
        if ( sizeof(lineCoding) == length ) {
            memcpy_( pbuf, &lineCoding, sizeof(lineCoding) ) ;
        }
        else {
            DDB_ERR ;
        }
        break ;

    case CDC_SET_CONTROL_LINE_STATE:
        DDB_DEBUG("CDC_SET_CONTROL_LINE_STATE") ;
        break ;

    case CDC_SEND_BREAK:
        DDB_DEBUG("CDC_SEND_BREAK") ;
        break ;

    default:
        DDB_ERR ;
        DDB_DEBUG("\t ? cmd = %02X ?", cmd) ;
        break ;
    }

    return USBD_OK ;
}

/**
  * @brief  Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_HS(
    uint8_t * Buf,
    uint16_t Len)
{
    uint8_t result = USBD_OK ;

    USBD_CDC_HandleTypeDef * hcdc =
        (USBD_CDC_HandleTypeDef *) hUsbDeviceHS.pClassData ;
    if ( hcdc->TxState != 0 ) {
        return USBD_BUSY ;
    }
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len) ;
    result = USBD_CDC_TransmitPacket(&hUsbDeviceHS) ;

    return result ;
}

/**
  * @brief  CDC_TransmitCplt_HS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_HS(
    uint8_t * Buf,
    uint32_t * Len,
    uint8_t epnum)
{
    uint8_t result = USBD_OK ;

    UNUSED(Buf) ;
    UNUSED(Len) ;
    UNUSED(epnum) ;

    DDB_DEBUG("%s(,%d)", __func__, (int) *Len) ;

    return result ;
}

/**
  * @brief Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAILL
  */
static int8_t CDC_Receive_HS(
    uint8_t * Buf,
    uint32_t * Len)
{
    DDB_DEBUG("%s(,%d)", __func__, (int) *Len) ;
#ifndef USB_UART_NO_CIRCO
    (void) CIRCO_ins(&u.c, Buf, *Len) ;

    rx_cb(cb_arg) ;
#else
    rx_cb(cb_arg, Buf, *Len) ;
    rx_buf = POINTER( rx_flip ^ UINTEGER(rx_buf) ) ;
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, rx_buf) ;
#endif
    if ( USBD_FAIL == USBD_CDC_ReceivePacket(&hUsbDeviceHS) ) {
        DDB_ERR ;
    }
    return USBD_OK ;
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS = {
    CDC_Init_HS,
    CDC_DeInit_HS,
    CDC_Control_HS,
    CDC_Receive_HS,
    CDC_TransmitCplt_HS
} ;

/***************************************************
                Interfaccia verso uart
***************************************************/

bool USBU_iniz(
    USBU_CALLBACK rx,
    void * arg)
{
    if ( !iniz ) {
#ifndef USB_UART_NO_CIRCO
        CIRCO_iniz(&u.c, MAX_BUFF) ;
#endif
        cb_arg = arg ;

        if ( NULL == rx ) {
            rx = cb_vuota ;
        }
        rx_cb = rx ;

        // MX_USB_DEVICE_Init
        do {
            if ( USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK ) {
                DDB_ERR ;
                break ;
            }
            if ( USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK ) {
                DDB_ERR ;
                break ;
            }
            if ( USBD_CDC_RegisterInterface(&hUsbDeviceHS,
                                            &USBD_Interface_fops_HS) !=
                 USBD_OK ) {
                DDB_ERR ;
                break ;
            }
            if ( USBD_Start(&hUsbDeviceHS) != USBD_OK ) {
                DDB_ERR ;
                break ;
            }

            // ? ancora ? vedi HAL_PCD_MspInit
            HAL_PWREx_EnableUSBVoltageDetector() ;

            iniz = true ;
        } while ( false ) ;
    }
    else {
        DDB_ERR ;
    }

    return iniz ;
}

#ifndef USB_UART_NO_CIRCO
uint32_t USBU_rx(
    void * rx,
    uint32_t dim)
{
    return CIRCO_est(&u.c, rx, dim) ;
}

#endif

bool USBU_tx(
    const void * v,
    uint32_t dim)
{
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, CONST_CAST(v), dim) ;

    (void) USBD_CDC_TransmitPacket(&hUsbDeviceHS) ;

    return true ;
}
