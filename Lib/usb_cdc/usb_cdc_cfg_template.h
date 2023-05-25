#ifndef LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_
#define LIB_USB_CDC_USB_CDC_CFG_TEMPLATE_H_

#define USBD_PID_HS                     0xC777
#define USBD_PRODUCT_STRING_HS          "483 - MULTIHUB - SC777"
#define USBD_CONFIGURATION_STRING_HS    "SC777 cfg"
#define USBD_INTERFACE_STRING_HS        "SC777 int"

// Se definite, disp.i diversi mostrano lo stesso serial number
#define USBD_SERIAL_0  0x4B159D67
#define USBD_SERIAL_1  0x89E4FE29


// Definire globalmente (se serve)
//#define USB_UART_NO_CIRCO       1

#else
#   warning usb_cdc_cfg_template.h incluso
#endif
