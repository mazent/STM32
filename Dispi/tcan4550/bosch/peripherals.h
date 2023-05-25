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

#ifndef PERIPHERALS_H
#define PERIPHERALS_H
 
//MZ#include "global_includes.h"

// Sets bit a through bit b (inclusive), as long as 0 <= a <= 31 and 0 <= b <= 31.
//   example1: PERI_BITMASK_FROM_B_DOWNTO_A( 7,  4) = 0x00 00 00 F0
//   example2: PERI_BITMASK_FROM_B_DOWNTO_A(31, 28) = 0xF0 00 00 00
#define PERI_BITMASK_FROM_B_DOWNTO_A(b, a) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))

/* read edge-capture registers of the buttons, debounce before value is returned */
unsigned int button_get_edge_debounced();


/*  This function prints an hexadecimal number on the seven segment display.
 */
void sevenseg_set_hex(unsigned int hex);

/*  This function prints an decimal number on the seven segment display.
 */
void sevenseg_set_dec(unsigned int dec);

/* This function shows a float value on the sevenseg leds
 */
void sevenseg_set_float(float value, int decimal_places);

/*  This function clears the seven segment display, i.e., all segments are off afterwards
 */
void sevenseg_clear();


#if __ALTERA
/*  The period register is used to set the clock cycle of the timer.
 *  It is splitted into an 16-Bit high and low part. So this
 *  function just takes the 32-Bit argument and writes the high and low
 *  parts.
 */
void timer32_set_period(unsigned int timer_base_addr, unsigned int period);

/*  After setting the timer period, this function can be used
 *  to start the timer.
 */
void timer_start(unsigned int timer_base_addr);

/* STOP timer */
void timer_stop(unsigned int timer_base_addr);

/*  Use the timers snapshot register to get a snapshot. The current
 *  register value is read via a high and a low register. This function
 *  is inlined to further reduce latency, when making a snapshot.
 */
unsigned int timer1_snapshot();
unsigned int timer2_snapshot();

#elif __XILINX
// no timers used
#endif


/*  This function sets the Output pins on the board according to the given argument.
 */
void pio_set(unsigned int data);

/*  This function adds the output pins on the board according to the given argument.
*/
void pio_add(unsigned int data);

/*  This function removes the output pins on the board according to the given argument.
 */
void pio_remove(unsigned int data);


/*  This function sets the LEDs on the board according to the given argument.
 */
void led_set(unsigned char data);

/*  This function gets the LED's status on the board.
 */
unsigned char led_get();

/*  This function sets individual LEDs on the board according to the given argument.
 */
void led_set_bitwise(unsigned char data);

/*  This function clears individual LEDs on the board according to the given argument.
 */
void led_clear_bitwise(unsigned char data);

#endif /*PERIPHERALS_H*/
