#ifndef BSP_SC830_CAN_CONFIG_H_
#define BSP_SC830_CAN_CONFIG_H_

/*
 * bsp.h deve contenere:
 *    CAN_CK_KBS
 *    CAN_MAX_PRESCALER
 *    CAN_MAX_BS1
 *    CAN_MAX_BS2
 */

#include "can.h"

typedef struct {
    uint32_t prescaler ;
    uint32_t bs1 ;
    uint32_t bs2 ;
} S_CAN_CFG ;

bool cfg_std(
    S_CAN_CFG * conf,
    const BASE_CAN * prm) ;

#else
#   warning can_config.h incluso
#endif
