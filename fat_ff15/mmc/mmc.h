#ifndef MMC_H_
#define MMC_H_

#include <stdbool.h>

bool MMC_iniz(void) ;

void MMC_cid(uint8_t * cid16) ;
void MMC_csd(uint8_t * csd16) ;

bool MMC_leggi(uint32_t, uint8_t *) ;
bool MMC_scrivi(uint32_t, uint8_t *) ;

bool MMC_trim(
    uint32_t da,
    uint32_t a) ;

#else
#   warning klm8g1getf.h incluso
#endif
