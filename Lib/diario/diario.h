#ifndef LIB_DIARIO_DIARIO_H_
#define LIB_DIARIO_DIARIO_H_

#include "diario_cfg.h"
#include <stdbool.h>

/*
 * I messaggi vengono salvati in ram
 *
 * Due gusti
 *     1) Un task (a bassa priorita') li scrive invocando ddb_scrivi,
 *        che deve ritornare solo dopo la fine dell'operazione
 *        Uso una osMail
 *     2) I messaggi vengono recuperati con DDB_leggi e gestiti a parte
 *        Uso una zona di ram
 * La scelta si esegue usando rispettivamente:
 *     diario_mail.c
 *     diario_ram.c
 *
 * Esempio:
       // Abilito le DBG_xxx
       #define STAMPA_DBG
       #include "utili.h"
       ...
       #if 1
       // Voglio usare il diario:
       // 1) elimino le DBG_xxx
       #undef DBG_PRINTF
       #undef DBG_PUTS
       #undef DBG_PRINT_HEX
       #undef DBG_FUN
       #undef DBG_QUA
       #undef DBG_ERR
       #undef DBG_ASSERT

       // 2) includo e scelgo il livello
       #define DIARIO_LIV_DBG
       #include "diario/diario.h"

       // 3) rimappo sul diario
       #define DBG_PRINTF           DDB_PRINTF
       #define DBG_PUTS             DDB_PUTS
       #define DBG_PRINT_HEX        DBG_PRINT_HEX
       #define DBG_FUN              DDB_FUN
       #define DBG_QUA              DDB_QUA
       #define DBG_ERR              DDB_ERR
       #define DBG_ASSERT           DDB_ASSERT

       #endif
 *
 */

typedef enum {
    // In ordine crescente di verbosita'
    DDB_L_NONE,
    DDB_L_ERROR,
    DDB_L_WARNING,
    DDB_L_INFO,
    DDB_L_DEBUG
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
// Interfaccia per recuperare il msg piu' vecchio
int DDB_leggi(char */*almeno DDB_DIM_MSG*/) ;

// Macro utili
// -------------------------------
// Selezione del livello sul singolo file
// Definire una di queste prima di includere questo file
//#define DIARIO_LIV_DBG
//#define DIARIO_LIV_INFO
//#define DIARIO_LIV_WARN
//#define DIARIO_LIV_ERR
//#define DIARIO_LIV_NONE o anche non definire niente

#define DDB_LIV_DBG         4
#define DDB_LIV_INFO        3
#define DDB_LIV_WARN        2
#define DDB_LIV_ERR         1
#define DDB_LIV_NONE        0

#ifdef DIARIO_LIV_DBG
#define DDB_LIV        DDB_LIV_DBG
#elif defined DIARIO_LIV_INFO
#define DDB_LIV        DDB_LIV_INFO
#elif defined DIARIO_LIV_WARN
#define DDB_LIV        DDB_LIV_WARN
#elif defined DIARIO_LIV_ERR
#define DDB_LIV        DDB_LIV_ERR
#else
#define DDB_LIV        DDB_LIV_NONE
#endif

#if DDB_LIV >= DDB_LIV_ERR
#define DDB_LIV_ERR_ABIL        1
#define DDB_ERROR(f, ...)       DDB_printf(DDB_L_ERROR, f, ## __VA_ARGS__)
#define DDB_ERR                 DDB_printf(DDB_L_ERROR, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#define DDB_ASSERT              DDB_printf(DDB_L_ERROR, "ASSERT %s %d\n", \
                                           __FILE__, \
                                           __LINE__)

#else
#define DDB_ERROR(f, ...)
#define DDB_ERR
#define DDB_ASSERT
#endif

#if DDB_LIV >= DDB_LIV_WARN
#define DDB_LIV_WARN_ABIL       1
#define DDB_WARNING(f, ...)     DDB_printf(DDB_L_WARNING, f, ## __VA_ARGS__)
#define DDB_WRN                 DDB_printf(DDB_L_WARNING, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_WARNING(f, ...)
#define DDB_WRN
#endif

#if DDB_LIV >= DDB_LIV_INFO
#define DDB_LIV_INFO_ABIL       1
#define DDB_INFO(f, ...)        DDB_printf(DDB_L_INFO, f, ## __VA_ARGS__)
#define DDB_INF                 DDB_printf(DDB_L_INFO, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#else
#define DDB_INFO(f, ...)
#define DDB_INF
#endif

#if DDB_LIV >= DDB_LIV_DBG
#define DDB_LIV_DBG_ABIL        1
#define DDB_DEBUG(f, ...)       DDB_printf(DDB_L_DEBUG, f, ## __VA_ARGS__)
#define DDB_PRINT_HEX(t, x, d)  DDB_print_hex(DDB_L_DEBUG, t, x, d)
#define DDB_DBG                 DDB_printf(DDB_L_DEBUG, "%s %d", \
                                           __FILE__, \
                                           __LINE__)
#define DDB_FUN                 DDB_puts(DDB_L_DEBUG, __func__)
#define DDB_PUTS(a)             DDB_puts(DDB_L_DEBUG, a)
#define DDB_QUA                 DDB_printf(DDB_L_DEBUG, "QUA %s %d", \
                                           __FILE__, \
                                           __LINE__)

#else
#define DDB_DEBUG(f, ...)
#define DDB_PRINT_HEX(t, x, d)
#define DDB_DBG
#define DDB_FUN
#define DDB_PUTS(a)
#define DDB_QUA
#endif
#define DDB_PRINTF      DDB_DEBUG

#else
#   warning diario.h incluso
#endif
