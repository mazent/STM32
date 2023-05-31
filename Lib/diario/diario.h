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
    DDB_L_NONE,
    DDB_L_ERROR,
    DDB_L_WARNING,
    DDB_L_INFO,
    DDB_L_DEBUG
} DDB_LEVEL ;

bool DDB_iniz(DDB_LEVEL) ;

void DDB_level(DDB_LEVEL) ;

void DDB_puts(DDB_LEVEL, const char *) ;

void DDB_printf(DDB_LEVEL,
                const char *,
                ...) ;

void DDB_print_hex(DDB_LEVEL,
                   const char *,
                   const void *,
                   int) ;

// Interfaccia verso lo strato che trasmette/salva
extern void ddb_scrivi(
    const char *,
    int) ;

// Macro utili
// -------------------------------
// Il livello fissato a compile-time permette di eliminare
// totalmente le stampe dal codice, ma esse possono essere
// eliminate localmente sul singolo file
#ifndef USA_DIARIO
#undef DDB_LIV
#define DDB_LIV        DDB_LIV_NONE
#endif

#if DDB_LIV >= DDB_LIV_ERR
#define DDB_ERROR(f, ...)       DDB_printf(DDB_L_ERROR, f, ## __VA_ARGS__)
#define DDB_ERR                 DDB_printf(DDB_L_ERROR, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_ERROR(f, ...)
#define DDB_ERR
#endif
#define DDB_ASSERT(x)         \
    do {                  \
        if ( !(x) ) {     \
            DDB_ERR ;  \
        }                 \
    } while ( false )

#define DDB_CONTROLLA(x)        \
    do {                \
        if ( !(x) ) {   \
            DDB_ERR ;   \
        }               \
    } while ( false )

#if DDB_LIV >= DDB_LIV_WARN
#define DDB_WARNING(f, ...)     DDB_printf(DDB_L_WARNING, f, ## __VA_ARGS__)
#define DDB_WRN                 DDB_printf(DDB_L_WARNING, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_WARNING(f, ...)
#define DDB_WRN
#endif

#if DDB_LIV >= DDB_LIV_INFO
#define DDB_INFO(f, ...)        DDB_printf(DDB_L_INFO, f, ## __VA_ARGS__)
#define DDB_INF                 DDB_printf(DDB_L_INFO, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_INFO(f, ...)
#define DDB_INF
#endif

#if DDB_LIV >= DDB_LIV_DBG
#define DDB_DEBUG(f, ...)       DDB_printf(DDB_L_DEBUG, f, ## __VA_ARGS__)
#define DDB_PRINT_HEX(t, x, d)  DDB_print_hex(DDB_L_DEBUG, t, x, d)
#define DDB_DBG                 DDB_printf(DDB_L_DEBUG, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#define DDB_FUN                 DDB_puts(DDB_L_DEBUG, __func__)
#define DDB_PUTS(a)             DDB_puts(DDB_L_DEBUG, a)
#else
#define DDB_DEBUG(f, ...)
#define DDB_PRINT_HEX(t, x, d)
#define DDB_DBG
#define DDB_FUN
#define DDB_PUTS(a)
#endif
// condizionate
#   define DDB_PRN(c, p)    \
    if ( c ) {               \
        DDB_DEBUG p ;       \
    }

#   define DDB_PUT(c, p)    \
    if ( c ) {               \
        DDB_PUTS(p) ;       \
    }

#endif
