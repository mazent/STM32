#ifndef LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_
#define LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_

// Numero di messaggi pendenti
#define DDB_NUM_MSG     50

// Caratteri di ogni messaggio
#define DDB_DIM_MSG     100

// Quando e' capitato?
// Se definita invoca la funzione
// Esempio:
uint32_t HAL_GetTick(void) ;
#define DDB_QUANDO      HAL_GetTick

// Cosa stampo?
// In ordine decrescente di verbosita'
#define DDB_LIV_DBG         4
#define DDB_LIV_INFO		3
#define DDB_LIV_WARN        2
#define DDB_LIV_ERR         1
// Questo disabilita
#define DDB_LIV_NONE        0

#define DDB_LIV        DDB_LIV_NONE


#else
#   warning diario_cfg_template.h incluso
#endif
