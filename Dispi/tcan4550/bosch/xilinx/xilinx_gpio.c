/* GPIO Access
 * Bases on Xilinx GPIO Application Example */

/* Copyright (c) 2018, Robert Bosch GmbH, Gerlingen, Germany
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*     1. Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*     2. Redistributions in binary form must reproduce the above copyright
*        notice, this list of conditions and the following disclaimer in the
*        documentation and/or other materials provided with the distribution.
*     3. Neither the name of the Robert Bosch GmbH nor the
*        names of its contributors may be used to endorse or promote products
*        derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL the Robert Bosch GmbH BE LIABLE FOR ANY
*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
   Note: This software is snapshot of an ongoing project and serves as example.
*/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include <xgpio.h>
#include "stdio.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/

/* following constant is used to determine which channel of the GPIO is
 * used if there are 2 channels supported in the GPIO.
 */
#define INPUT_CHANNEL 1
#define OUTPUT_CHANNEL 1


/************************** Variable Definitions **************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XGpio GpioOutput; /* The driver instance for GPIO Device configured as O/P */



/******************************************************************************
* Configure GPIO as INPUT
*
* @param	GPioInstancePtr is a pointer to an XGpio instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*			  xparameters.h
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
*
*******************************************************************************/
int GpioInputInit(XGpio *XGPioInstancePtr, u16 DeviceId)
{
	 int Status;

	 /* Initialize the GPIO driver so that it's ready to use,
	  * specify the device ID that is generated in xparameters.h */
	 Status = XGpio_Initialize(XGPioInstancePtr, DeviceId);
	 if (Status != XST_SUCCESS) {
		  return XST_FAILURE;
	 }

	 /* Set the direction for all signals to be inputs */
	 XGpio_SetDataDirection(XGPioInstancePtr, INPUT_CHANNEL, 0xFFFFFFFF);

	 return XST_SUCCESS;
}

/*******************************************************************************
* Read from GPIO, (has to be configured as input before)
*
* @param	GPioInstancePtr is a pointer to an XGpio instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*			  xparameters.h
*******************************************************************************/
u32 GpioInputRead(XGpio *XGPioInstancePtr)
{
	 /* Read the state of the data so that it can be  verified */
	 return XGpio_DiscreteRead(XGPioInstancePtr, INPUT_CHANNEL);
}



/****************************************************************************
*
* Configure GPIO as OUTPUT
*
* @param	GPioInstancePtr is a pointer to an XGpio instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		    xparameters.h
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
****************************************************************************/
int GpioOutputInit(XGpio *XGPioInstancePtr, u16 DeviceId)
{
	int status;

	/*
	 * Initialize the GPIO driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	 status = XGpio_Initialize(XGPioInstancePtr, DeviceId);
	 if (status != XST_SUCCESS)  {
		  return XST_FAILURE;
	 }

	 /* Set the direction for all signals to be outputs */
	 XGpio_SetDataDirection(XGPioInstancePtr, OUTPUT_CHANNEL, 0x0);

	 return XST_SUCCESS;
}



/****************************************************************************
*
* Write to Gpio Output
*
* @param	GPioInstancePtr is a pointer to an XGpio instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
*
* @param    data to be written
*
****************************************************************************/
void GpioOutputWrite(XGpio *XGPioInstancePtr, u32 data)
{
	// write
	XGpio_DiscreteWrite(XGPioInstancePtr, OUTPUT_CHANNEL, data);
}



/****************************************************************************
*
* Clear Gpio Output
*
* @param	GPioInstancePtr is a pointer to an XGpio instance. The memory the
*		pointer references must be pre-allocated by the caller. Further
*		calls to manipulate the instance/driver through the XGpio API
*		must be made with this pointer.
*
* @param    data to be written
*
****************************************************************************/
void GpioOutputClear(XGpio *XGPioInstancePtr, u32 data)
{
	// clear
	XGpio_DiscreteClear(XGPioInstancePtr, OUTPUT_CHANNEL, data);
}
