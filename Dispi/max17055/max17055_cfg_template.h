#ifndef DISPI_MAX17055_MAX17055_CFG_TEMPLATE_H_
#define DISPI_MAX17055_MAX17055_CFG_TEMPLATE_H_

#include "max17055/max17055.h"

// nominal capacity (in .5 mAh)
#define MAX17055_DESIGN_CAP     2000

// opzionale
// VE || VR
//#define MAX17055_EMPTY_VOLTAGE  0x9646

// to detect when charge termination has occurred
// The device detects end of charge if all the following conditions are met:
//        VFSOC register > FullSOCThr register
//    AND IChgTerm x 0.125 < Current register < IChgTerm x 1.25
//    AND IChgTerm x 0.125 < AvgCurrent register < IChgTerm x 1.25
#define MAX17055_ICHGTERM       0x0640

#define MAX17055_MODELCFG       0x8400

#else
#   warning max17055_cfg_template.h incluso
#endif
