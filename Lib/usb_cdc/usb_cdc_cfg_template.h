#ifndef LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_
#define LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_

#define USBD_PID                     0xCxxx
#define USBD_PRODUCT_STRING          "638 - TXB 1 - SCxxx"
#define USBD_CONFIGURATION_STRING    "SCxxx cfg"
#define USBD_INTERFACE_STRING        "SCxxx int"

// Se definite, disp.i diversi mostrano lo stesso serial number
#define USBD_SERIAL_0  0xXXXXXXXX
#define USBD_SERIAL_1  0xXXXXXXXX

// Commentare se FS
//#define USBD_HS			1

// Se definita
//     i dati ricevuti vanno in un buffer circolare (consigliato per FS)
// altrimenti
//     si riceve su piu' buffer (consigliato per HS)
#define USB_UART_USA_BUF_CIRC       1000

#else
#   warning usb_cdc_cfg_template.h incluso
#endif
