#ifndef LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_
#define LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_

// diario_ram.c
// ===========================

// Buffer messaggi (0 disabilita)
#define DDB_RAM_BASE    0
#define DDB_RAM_DIM     0

#define DDB_MAX_HEX		20

// diario.c
// ===========================

// Numero di messaggi pendenti
#define DDB_NUM_MSG     50

// Caratteri di ogni messaggio
#define DDB_DIM_MSG     100

// comuni
// ===========================

// Quando e' capitato?
// Se definita invoca la funzione
// Esempio:
uint32_t HAL_GetTick(void) ;
#define DDB_QUANDO      HAL_GetTick

#else
#   warning diario_cfg_template.h incluso
#endif
