#ifndef APP_RAMCOMUNE_H_
#define APP_RAMCOMUNE_H_

#include <stdbool.h>

void RC_iniz(void) ;

typedef enum {
    TB_SCONO,
    TB_COLLAUDO,
    TB_MAIN,
    TB_SRV
} RC_TIPO_BOOT ;

RC_TIPO_BOOT RC_cosa_butto(void) ;
void RC_butta_questo(RC_TIPO_BOOT) ;

void RC_ma(bool valida) ;
void RC_sa(bool valida) ;

#else
#   warning ramcomune.h incluso
#endif
