/* M_CAN test cases - help functions */
 
/* Copyright (c) 2015, Robert Bosch GmbH, Gerlingen, Germany
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

#ifndef M_CAN_TEST_HELP_FUNC_H
#define M_CAN_TEST_HELP_FUNC_H


/* compare messages for equal contents */
boolean msg_compare(can_msg_struct *msg1_ptr, can_msg_struct *msg2_ptr, M_CAN_operating_mode operating_mode);

/* calculate number of arbitration phase bits of a CAN frame (without dynmaic stuff bits!)*/
unsigned int frame_get_bit_count_of_nominal_bits(can_msg_struct *can_msg);

/* calculate the number of data phase bits of a CAN frame */
unsigned int frame_get_bit_count_of_data_bits(can_msg_struct *can_msg);


/* Message List ################################################# */

/* Reset the RX MSG List */
void rx_msg_list_reset();

/* Copy Message from Message RAM to Message List */
void copy_msg_from_msg_ram_to_msg_list(can_struct *can_ptr, uint32_t msg_addr_in_msg_ram, rx_info_struct);

/* Get Head Message from Message List */
can_msg_struct * msg_list_get_head_msg(can_struct *can_ptr);

/* Remove Head Message from Message List */
void msg_list_remove_head_msg(can_struct *can_ptr);

/* Get Next Free Element Pointer of Message List */
can_msg_struct * msg_list_get_next_free_element(can_struct *can_ptr);

/* Add Message to Tail of Message List */
void msg_list_add_msg_at_tail(can_struct *can_ptr);

/* Check, if Message List is EMPTY */
boolean msg_list_is_empty(can_struct *can_ptr);

/* Check, if Message List is FULL */
boolean msg_list_is_full(can_struct *can_ptr);

/* Check, if there is any RX Message List in any of the Message Lists */
boolean is_any_msg_in_any_rx_msg_list();



/* Tx Event List ################################################# */

/* Reset the Tx Event FIFO list */
void tx_event_list_reset();

/* Copy a Tx Event FIFO element from Message RAM to TX Event List */
void copy_event_element_from_msg_ram_to_tx_event_list(can_struct *can_ptr, uint32_t tx_event_element_addr_in_msg_ram);

/* Get Head element from Tx Event List */
tx_event_element_struct * tx_event_list_get_head_element(can_struct *can_ptr);

/* Remove Head element from Tx Event List */
void tx_event_list_remove_head_element(can_struct *can_ptr);

/* Get Next Free Element Pointer of Tx Event List */
tx_event_element_struct * tx_event_list_get_next_free_element(can_struct *can_ptr);

/* Add Element to Tail of Tx Event List */
void tx_event_list_add_element_at_tail(can_struct *can_ptr);

/* Check, if Tx Event List is EMPTY */
boolean tx_event_list_is_empty(can_struct *can_ptr);

/* Check, if Tx Event List is FULL */
boolean tx_event_list_is_full(can_struct *can_ptr);


/* Statistics ########################################### */

/* Update Message Statistics */
void update_msg_statistics_at_mram_access(can_struct *can_ptr, can_msg_struct *msg_ptr);

/* Print Statistics */
void print_statistics(can_struct *can_ptr);

/* Reset Statistics */
void reset_statistic();


/* Misc ################################################# */

/* Print Message */
void print_msg(can_msg_struct *msg_ptr);

/* prints the content of one Tx Event Element */
void print_tx_event_fifo_element(tx_event_element_struct *tx_event_element_ptr);

/* Print relative Message RAM Start-Addresses */
void print_mram_startaddresses();

/* RAM Check for initial Value (at system startup) */
int ram_check_reset_value(int m_can_id, int ram_base_addr, int ram_size_in_words, int ram_word_width_in_bytes);

/* RAM Print non-zero values */
int ram_content_print(int m_can_id, int ram_base_addr, int ram_size_in_words, int ram_word_width_in_bytes);

#endif /* M_CAN_TEST_HELP_FUNC_H */
