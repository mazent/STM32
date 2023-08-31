#ifndef LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_
#define LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_

#define USBD_PID                     0xC777
#define USBD_PRODUCT_STRING          "483 - MULTIHUB - SC777"
#define USBD_CONFIGURATION_STRING    "SC777 cfg"
#define USBD_INTERFACE_STRING        "SC777 int"

// Se definite, disp.i diversi mostrano lo stesso serial number
#define USBD_SERIAL_0  0x4B159D67
#define USBD_SERIAL_1  0x89E4FE29


// Se definita
//     i dati ricevuti vanno in un buffer circolare
// altrimenti
//     si riceve su piu' buffer
//     Consigliato se DIM_DATI_CMD > CDC_DATA_HS_MAX_PACKET_SIZE
//#define USB_UART_USA_BUF_CIRC       1

#else
#   warning usb_cdc_cfg_template.h incluso
#endif
