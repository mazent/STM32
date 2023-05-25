/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS ;

/*
 * -- Insert your variables declaration here --
 */

/*
 * -- Insert your external function declaration here --
 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
    /* Init Device Library, add supported class and start the library. */
    if ( USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK ) {
        Error_Handler() ;
    }
    if ( USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK ) {
        Error_Handler() ;
    }
    if ( USBD_CDC_RegisterInterface(&hUsbDeviceHS,
                                    &USBD_Interface_fops_HS) != USBD_OK ) {
        Error_Handler() ;
    }
    if ( USBD_Start(&hUsbDeviceHS) != USBD_OK ) {
        Error_Handler() ;
    }

    HAL_PWREx_EnableUSBVoltageDetector() ;
}

/**
  * @}
  */

/**
  * @}
  */
