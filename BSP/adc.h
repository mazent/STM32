#ifndef ADC_H_
#define ADC_H_

#include "utili.h"

#define ADC_VREF_MV		2500
#define ADC_MAX_VAL     ( (1 << 16) - 1 )

typedef struct {
    uint16_t ch1 ;
    uint16_t ch2 ;
    uint16_t vbatt ;
    uint16_t vkey ;
} S_ADC ;

bool ADC_dammi(S_ADC * /*p*/) ;

bool batteria_bassa(void) ;

#endif
