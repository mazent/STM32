#ifndef LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_
#define LIB_DIARIO_DIARIO_CFG_TEMPLATE_H_

// diario_ram.c
// ===========================

// Buffer messaggi (non definito disabilita il diario)
#define DDB_RAM_BASE    0
#define DDB_RAM_DIM     0

// Se definita, limita le righe alla potenza di 2 piu' vicina
#define DDB_RAM_POT_2         1

// Se definita, le righe piu' vecchie vengono perse
// altrimenti si perdono le nuove
#define DDB_RAM_SVRSCRV     1

// diario_mail.c
// ===========================

// Numero di messaggi pendenti (0 disabilita il diario)
#define DDB_NUM_MSG     50

// comuni
// ===========================

// Se definita, numero massimo di byte stampati da DDB_print_hex
#define DDB_MAX_HEX     20

// Caratteri di ogni messaggio
#define DDB_DIM_MSG     100

// Quando e' capitato?
// Se definita invoca la funzione
// Esempio:
uint32_t HAL_GetTick(void) ;
#define DDB_QUANDO      HAL_GetTick

#else
#   warning diario_cfg_template.h incluso
#endif
