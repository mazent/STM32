#ifndef LTC2631_H_
#define LTC2631_H_

#include <stdbool.h>
#include <stdint.h>

//        +----------+        +----------+
// write  | Input    | update |   DAC    |
// ------>| Register |------->| Register |---> Vout
//        +----------+        +----------+

bool LTC2631_iniz(void) ;

// Write to Input Register
bool LTC2631_wir(uint16_t) ;

// Update (Power Up) DAC Register
bool LTC2631_udac(void) ;

// Write to and Update (Power Up) DAC Register
bool LTC2631_wudac(uint16_t) ;

// Power Down
bool LTC2631_pd(void) ;

// Select Internal Reference
bool LTC2631_sir(void) ;

// Select External Reference
bool LTC2631_ser(void) ;


#endif
