#ifndef LIB_DIARIO_DIARIO_PRIV_H_
#define LIB_DIARIO_DIARIO_PRIV_H_

#include "diario.h"

#ifndef  DDB_NUM_MSG
#define DDB_NUM_MSG     0
#endif

typedef struct {
#ifdef DDB_QUANDO
    uint32_t quando ;
#endif
    int dim ;
    DDB_LEVEL level ;
    char msg[DDB_DIM_MSG] ;
} DDB_RIGA ;

DDB_RIGA * riga_alloca(void) ;
void riga_libera(DDB_RIGA *) ;
void riga_scrivi(DDB_RIGA *) ;

bool ddb_iniz(void) ;

void ddb_stampa(DDB_RIGA * riga) ;

#else
#   warning diario_priv.h incluso
#endif
