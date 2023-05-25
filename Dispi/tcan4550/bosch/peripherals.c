/* Drivers for Board Peripherals - Xilinx and Altera */

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

#include "global_includes.h"


#if __XILINX
extern XGpio XGpioInputButtons;    // The driver instance for a GPIO Device configured as Input
extern XGpio XGpioOutputLEDs;      // The driver instance for a GPIO Device configured as Output
#define USED_TmrCtrNumber 0
#endif // __XILINX

#define __XILINX_ENABLE_PERIPHERALS 0

/* read edge-capture registers of the buttons, debounce before value is returned */
unsigned int button_get_edge_debounced()
{
	unsigned int debounce_cnt;
	unsigned int button_edge;
	unsigned int button_data;
#if __ALTERA
	const unsigned int button_data_when_released = BITMASK_FROM_B_DOWNTO_A(PERI_PIO_BUTTON_DATA_WIDTH-1, 0);
#endif /* __ALTERA */
	#define DEBOUNCE_STEPS 100

#if __ALTERA
	// check button press: edge-capture register
	button_edge = IORD_ALTERA_AVALON_PIO_EDGE_CAP(PERI_PIO_BUTTON_BASE);

	// check if pressed
	if (button_edge != 0) {
		// debounce

		debounce_cnt = DEBOUNCE_STEPS;
		while (debounce_cnt) {
			button_data = IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_BUTTON_BASE);
			if (button_data == button_data_when_released) {
				// nothing pressed
				debounce_cnt--;
			} else {
				// re-arm counter
				debounce_cnt = DEBOUNCE_STEPS;
			}
		} // while

		/* Reset the Button's edge capture register. */
		IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PERI_PIO_BUTTON_BASE, 0);
	}

	return button_edge;
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
	// check button press: edge-capture register
	button_edge = GpioInputRead(&XGpioInputButtons);

	// check if pressed
	if (button_edge != 0) {
		// debounce

		debounce_cnt = DEBOUNCE_STEPS;
		while (debounce_cnt) {
			button_data = GpioInputRead(&XGpioInputButtons);
			if (button_data == 0x0) {
				// nothing pressed
				debounce_cnt--;
			} else {
				// re-arm counter
				debounce_cnt = DEBOUNCE_STEPS;
			}
		} // while
	}

	return (unsigned int)button_edge;
#endif // __XILINX
}


/*  This function displays a given hex value on the Seven Segment Display.
 *  The SevenSeg is basically just a set of LEDs. The decoding, which
 *  LEDs to switch on, is done in this function.
 */
void sevenseg_set_hex(unsigned int hex)
{
#if __ALTERA
  /* decoder table */
  const unsigned char segments[16] = {
    0x81, 0xcf, 0x92, 0x86,
    0xcc, 0xa4, 0xa0, 0x8f,
    0x80, 0x84, 0x88, 0xe0,
    0xf2, 0xc2, 0xb0, 0xb8};

  int data = 0; //output data

  /* start the decoding */
  if (hex < 0x100)
    data = segments[hex & 0xF] | (segments[(hex >> 4) & 0xF] << 8);

  /* 0x100: all segments off */
  if (hex == 0x100)
    data = -1;
 
  /* 0x101..: all segments on */
  if (hex > 0x100)
    data = 0xffff;

  /* set the value */
  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_7SEG0_BASE, data);
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS

  // not implemented
  printf("[SevenSeg Set Hex] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}



/*  This function writes out an decimal value from 00..99.
 *  Everything else is not specified.
 */
void sevenseg_set_dec(unsigned int dec)
{
#if __ALTERA
	sevenseg_set_hex(((dec / 10) << 4) + (dec % 10));
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
	// not implemented
	printf("[SevenSeg Set Dec] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}



/* This function shows a float value on the sevenseg leds */
void sevenseg_set_float(float value, int decimal_places)
{
	// decimal_places (Nachkommastellen)
	// e.g.:  0  =>   display: xy.
	// e.g.:  1  =>   display: x.y
	// e.g.:  2  =>   display: .xy
	// e.g.:  3  =>   display: .-xy

#if __ALTERA
  float myfloat;
  int i;

  myfloat = value;

  for (i = 0; i < decimal_places; ++i) {
	myfloat = myfloat * 10;
  }

  // xy.z    x: integer digit  y: integer digit  z: first decimal place
  unsigned int x, y;
  x = ((unsigned int)(myfloat * 0.1)) % 10;
  y = ((unsigned int)(myfloat + 0.5)) % 10;    // + 0.5 to achieve a rounding
  sevenseg_set_hex((x << 4) + y);

  // set decimal point
  if (decimal_places == 0) {
	  // add dot   (0x80 is the dot) after the second digit
	  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_7SEG0_BASE,
			                      IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_7SEG0_BASE) & 0xFF7F );
  }
  if (decimal_places == 1) {
	  // add dot   (0x80 is the dot) between the digits
	  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_7SEG0_BASE,
			                      IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_7SEG0_BASE) & 0x7FFF );
  }

//  // x.y    x: integer digit  y: non-integer digit
//  unsigned int x, y;
//  x = ((unsigned int)value)      % 10;
//  y = ((unsigned int)(value*10 + 0.5)) % 10 ;    // +0.5 to achieve a rounding
//  sevenseg_set_hex((x << 4) + y);
//
//  // add dot   (0x80 is the dot)
//  IOWR_ALTERA_AVALON_PIO_DATA(PIO_SEVEN_SEG_BASE,
//		                      IORD_ALTERA_AVALON_PIO_DATA(PIO_SEVEN_SEG_BASE) & 0x7FFF );

#elif __XILINX  && __XILINX_ENABLE_PERIPHERALS

  // not implemented
  printf("[SevenSeg Set Float] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}



void sevenseg_clear()
{
#if __ALTERA
	sevenseg_set_dec(256);
#elif __XILINX  && __XILINX_ENABLE_PERIPHERALS
	// not implemented
	printf("[SevenSeg Set Clear] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}



/* Timer =====================================
 *
 * Functionality:
 * An internal counter counts down to zero, and whenever it reaches zero,
 * it is immediately reloaded from the period registers.
 *
 * Set Period:
 * A processor can specify the timer period by writing a value to the period registers.
 * It is split into a 16-Bit high and low part. So this function
 * just takes the 32-Bit argument and writes the high and low parts.
 */

#if __ALTERA
// general
void timer32_set_period(unsigned int timer_base_addr, unsigned int period)
{ // 32-Bit
  IOWR_ALTERA_AVALON_TIMER_PERIODL(timer_base_addr, period & 0xffff);
  IOWR_ALTERA_AVALON_TIMER_PERIODH(timer_base_addr, period >> 16);
}


/*  START timer
 *  After setting the timer period, this function can be used to start the timer.
 */
void timer_start(unsigned int timer_base_addr)
{
	// 0x1 => set IRQ on TimeOut (TO)
	// 0x2 => set continuous run
	// 0x4 => set START timer
	// 0x8 => set STOP  timer
	IOWR_ALTERA_AVALON_TIMER_CONTROL(timer_base_addr, 0x7);
}


/* STOP timer */
void timer_stop(unsigned int timer_base_addr)
{
	// 0x1 => set IRQ on TimeOut (TO)
	// 0x2 => set continuous run
	// 0x4 => set START timer
	// 0x8 => set STOP  timer
	IOWR_ALTERA_AVALON_TIMER_CONTROL(timer_base_addr, 0x8);
}


/* Reading the timer's snapshot register:
 * A processor can read the current counter value by
 * 1. first writing to one of the snap registers to request a coherent snapshot of the counter,
 * 2. then reading the snap registers for the full value in two steps
 *    low_register and high_register
 *
 * Snapshot functions are inline to reduce latency.
 */

inline unsigned int timer1_snapshot()
{
  /* snapshot the timer */
  IOWR_ALTERA_AVALON_TIMER_SNAPL(TIMER32_1_BASE, 0);

  /* read snapshot registers */
  return  IORD_ALTERA_AVALON_TIMER_SNAPL  (TIMER32_1_BASE) |
    (IORD_ALTERA_AVALON_TIMER_SNAPH(TIMER32_1_BASE)<<16);
}

inline unsigned int timer2_snapshot()
{
  /* snapshot the timer */
  IOWR_ALTERA_AVALON_TIMER_SNAPL(TIMER32_2_BASE, 0);

  /* read snapshot registers */
  return  IORD_ALTERA_AVALON_TIMER_SNAPL  (TIMER32_2_BASE) |
    (IORD_ALTERA_AVALON_TIMER_SNAPH  (TIMER32_2_BASE)<<16);
}

#elif __XILINX  && __XILINX_ENABLE_PERIPHERALS
// no timers used
#endif // __XILINX


/*  This function sets the LEDs on the board according to the given argument.
 */
void led_set(unsigned char data)
{
#if __ALTERA
  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE, data);
#elif __XILINX  && __XILINX_ENABLE_PERIPHERALS
  GpioOutputWrite(&XGpioOutputLEDs, data);
#endif
}

/*  This function gets the LED's status on the board.
 */
unsigned char led_get()
{
#if __ALTERA
  return IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE);
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  printf("[led_get] Function not implemented for Xilinx SoPCs.\n");
#endif
}

/*  This function sets individual LEDs on the board according to the given argument.
 */
void led_set_bitwise(unsigned char data)
{
#if __ALTERA
  unsigned char curr_led_data;
  // set led-wise: e.g.: data = 0x08 sets LED_3 without affecting the other LEDs
  //IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_LED_BASE, data); // Befehl TUT NICHT
  curr_led_data = IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE);
  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE, (data | curr_led_data) );
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  printf("[led_set_bitwise] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}

/*  This function clears individual LEDs on the board according to the given argument.
 */
void led_clear_bitwise(unsigned char data)
{
#if __ALTERA
  unsigned char curr_led_data;
  // clear led-wise: e.g.: data = 0x08 clears LED_3 without affecting the other LEDs
  // IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_LED_BASE, data); <= Befehl TUT NICHT

  curr_led_data = IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE);
  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_LED_BASE, ((~data) & curr_led_data) );
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  printf("[led_clear_bitwise] Function not implemented for Xilinx SoPCs.\n");
#endif // __XILINX
}


/*  This function sets the output pins on the board according to the given argument.
 */
void pio_set(unsigned int data)
{
#if __ALTERA
  IOWR_ALTERA_AVALON_PIO_DATA(PERI_PIO_OUT_BASE, data);
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  printf("[pio_set] Function not implemented for Xilinx SoPCs.\n");
#endif
}


/*  This function adds the output pins on the board according to the given argument.
*/
void pio_add(unsigned int data)
{
#if __ALTERA
	/* Descrioption: data=0x2  => second pin is driven additionally;
	 *               the other pins are unchanged
	 */
  uint32_t pio_value;
  const uint32_t pio_mask = PERI_BITMASK_FROM_B_DOWNTO_A(PERI_PIO_OUT_DATA_WIDTH-1,  0);

  // read current value - modifiy it - set new value
  pio_value = IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_OUT_BASE);
  pio_value = (pio_value | data) & pio_mask;
  pio_set(pio_value); 		// pio_out
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  print("[pio_add] Function not implemented for Xilinx SoPCs.\n");
# endif // __XILINX
}

/*  This function removes the output pins on the board according to the given argument.
 */
void pio_remove(unsigned int data)
{
#if __ALTERA
	/* Descrioption: data=0x2  => second pin is NOT driven any more;
	 *               the other pins are unchanged
	 */
  uint32_t pio_value;
  const uint32_t pio_mask = PERI_BITMASK_FROM_B_DOWNTO_A( PERI_PIO_OUT_DATA_WIDTH-1,  0);

  // read current value - modifiy it - set new value
  pio_value = IORD_ALTERA_AVALON_PIO_DATA(PERI_PIO_OUT_BASE);
  pio_value = (pio_value | data) & pio_mask;
  pio_set(pio_value); 		// pio_out
#elif __XILINX && __XILINX_ENABLE_PERIPHERALS
  // not implemented
  print("[pio_remove] Function not implemented for Xilinx SoPCs.\n");
# endif // __XILINX
}
