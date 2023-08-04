#ifndef LIB_DIARIO_DIARIO_H_
#define LIB_DIARIO_DIARIO_H_

#include "diario_cfg.h"
#include <stdbool.h>

/*
 * I messaggi vengono salvati in ram
 *
 * Un task a bassa priorita' li scrive invocando ddb_scrivi,
 * che deve ritornare solo dopo la fine dell'operazione
 */

typedef enum {
    // In ordine crescente di verbosita'
    DDB_NONE,
    DDB_ERROR,
    DDB_WARNING,
    DDB_INFO,
    DDB_DEBUG
} DDB_LEVEL ;

bool DDB_iniz(DDB_LEVEL /*a*/) ;

void DDB_level(DDB_LEVEL) ;

void DDB_puts(DDB_LEVEL /*b*/, const char * /*a*/) ;

void DDB_printf(DDB_LEVEL /*b*/,
                const char * /*a*/,
                ...) ;

void DDB_print_hex(DDB_LEVEL /*d*/,
                   const char * /*a*/,
                   const void * /*b*/,
                   int /*c*/) ;

// Interfaccia verso lo strato che trasmette/salva
extern void ddb_scrivi(
    const char *,
    int) ;

// Macro utili
// -------------------------------
// Il livello fissato a compile-time permette di eliminare
// totalmente le stampe dal codice
#if DDB_LIV >= DDB_LIV_ERR
#define DDB_ERROR(f, ...)       DDB_printf(DDB_ERROR, f, ## __VA_ARGS__)
#define DDB_ERR                 DDB_printf(DDB_ERROR, "%s %d", \
                                           __FILE__, \
                                           __LINE__)

#else
#define DDB_ERROR(f, ...)
#define DDB_ERR
#endif

#if DDB_LIV >= DDB_LIV_WARN
#define DDB_WARNING(f, ...)     DDB_printf(DDB_WARNING, f, ## __VA_ARGS__)
#define DDB_WRN                 DDB_printf(DDB_WARNING, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_WARNING(f, ...)
#define DDB_WRN
#endif

#if DDB_LIV >= DDB_LIV_INFO
#define DDB_INFO(f, ...)        DDB_printf(DDB_INFO, f, ## __VA_ARGS__)
#define DDB_INF                 DDB_printf(DDB_INFO, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_INFO(f, ...)
#define DDB_INF
#endif

#if DDB_LIV >= DDB_LIV_DBG
#define DDB_DEBUG(f, ...)       DDB_printf(DDB_DEBUG, f, ## __VA_ARGS__)
#define DDB_PRINT_HEX(t, x, d)  DDB_print_hex(DDB_DEBUG, t, x, d)
#define DDB_DBG                 DDB_printf(DDB_DEBUG, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_DEBUG(f, ...)
#define DDB_PRINT_HEX(t, x, d)
#define DDB_DBG
#endif

#else
#   warning diario.h incluso
#endif
