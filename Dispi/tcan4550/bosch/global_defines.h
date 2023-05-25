/* global setting */

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

#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

// Hints for settings
// 0 = FALSE
// 1 = TRUE


// allows to use define as a string
#define DEFINE2STRING(def) #def
/* Example:
 * 	assert_print_error(((sizeof(m_can_info_list)/sizeof(m_can_info_list[0])) == CAN_NUMBER_OF_PRESENT_NODES),
			"Array for info structures has not the expected amount of Elements.\n Expexted amount is "DEFINE2STRING(CAN_NUMBER_OF_PRESENT_NODES)
			" but instead read %u.", (sizeof(m_can_info_list)/sizeof(m_can_info_list[0])));
 */

// Asserts
// Macro assert_print_error displays an error message and terminates the program when an assertion condition is violated
#ifndef NDEBUG	// MZ
#	define assert_print_error(condition, message, ...) if(!(condition)) {printf("[ASSERT] " message"\n", ## __VA_ARGS__); assert(condition);}
#else
#	define assert_print_error(condition, message, ...)
#endif
/*  Assert Demo: how to print a string with user variables
    char error_msg[200];
    sprintf(error_msg, "M_CAN_%d  TX Buffer Config: More elements configured than existing! (exist=%d, req_fifo/queue=%d, req_ded=%d)",
    can_ptr->id, MAX_TX_BUFFER_ELEMS, fifo_queue_size, ded_buffers_number);
    assert_print_error((ded_buffers_number + fifo_queue_size) <= MAX_TX_BUFFER_ELEMS, error_msg);
*/

// generate value with '1' set at position 'x'
#define BIT(x) (1 << (x))

// Sets bit a through bit b (inclusive), as long as 0 <= a <= 31 and 0 <= b <= 31.
// example: BITMASK_FROM_B_DOWNTO_A( 7,  4) = 0x00 00 00 F0
// example: BITMASK_FROM_B_DOWNTO_A(31, 28) = 0xF0 00 00 00
#define BITMASK_FROM_B_DOWNTO_A(b, a) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))


// print the name of the test case
#ifndef NDEBUG	// MZ
#	define PRINT_TEST_CASE_NAME printf("\n\n*** TEST CASE: %s - FILE: %s ***\n", __FUNCTION__, __FILE__)
#else
#	define PRINT_TEST_CASE_NAME
#endif


#endif // GLOBAL_DEFINES_H_
