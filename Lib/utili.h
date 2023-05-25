#ifndef UTILI_H_
#define UTILI_H_

/********************************************
    Inclusione di varie cose utili
********************************************/

// Azzerare per togliere le funzioni _s
#define __STDC_WANT_LIB_EXT1__      0
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stm32h7xx.h>
// Per le PRIu32 ecc
#include <inttypes.h>

// Rimappo
// ==========================================
// dovrebbe essere cosi':
//#if __STDC_LIB_EXT1__ == 1
// ma invece occorre usare questo:
#if __STDC_WANT_LIB_EXT1__ == 1
#define printf_             printf_s
static inline void strncpy_(
    char * dest,
    const char * src,
    size_t count)
{
    snprintf_s(dest, count, "%s", src) ;
}

#define vsnprintf_          vsnprintf_s
#define snprintf_           snprintf_s
#define vprintf_            vprintf_s
#define memcpy_(a, b, c)    memcpy_s(a, c, b, c)
#define memset_(a, b, c)    memset_s(a, c, b, c)
#define vsnprintf_          vsnprintf_s
#else
#define printf_             printf
static inline void strncpy_(
    char * dest,
    const char * src,
    size_t count)
{
    strncpy(dest, src, count) ;
    if ( dest[count - 1] != 0 ) {
        dest[count - 1] = 0 ;
    }
}

#define vsnprintf_          vsnprintf
#define snprintf_           snprintf
#define vprintf_            vprintf
#define memcpy_             memcpy
#define memset_             memset
#define vsnprintf_          vsnprintf
#endif

// Questi mancano
// ==========================================

#define MINI(a, b)          ( (a) < (b) ? (a) : (b) )
#define MAXI(a, b)          ( (a) > (b) ? (a) : (b) )

#define INUTILE(x)          (void) ( sizeof(x) )
#ifdef __cplusplus
#   define NEGA(x)             ( ~static_cast<unsigned int>(x) )
#else
#   define NEGA(x)             ( ~(unsigned int) (x) )
#endif
#define ABS(x)              (x < 0 ? -(x) : x)

#define DIM_VETT(a)         sizeof(a) / sizeof(a[0])

#define POTENZA_DI_2(v)     ( ( v & (v - 1) ) == 0 )

#define INIZIO_SEZ_CRITICA             \
    uint32_t prim = __get_PRIMASK() ;  \
    __disable_irq() ;
#define FINE_SEZ_CRITICA       \
    if ( !prim ) {             \
        __enable_irq() ;       \
    }

// Conversione a stringa (token stringification)
//      #define foo 4
//      STRINGA(foo)  -> "foo"
//      STRINGA2(foo) -> "4"
#define STRINGA(a)       # a
#define STRINGA2(a)      STRINGA(a)

#ifdef __ICCARM__
#   define USED     __root
#else
#   define USED     __attribute__( (used) )
#endif

#define BIT_0        (1 << 0)
#define BIT_1        (1 << 1)
#define BIT_2        (1 << 2)
#define BIT_3        (1 << 3)
#define BIT_4        (1 << 4)
#define BIT_5        (1 << 5)
#define BIT_6        (1 << 6)
#define BIT_7        (1 << 7)
#define BIT_8        (1 << 8)
#define BIT_9        (1 << 9)
#define BIT_10       (1 << 10)
#define BIT_11       (1 << 11)
#define BIT_12       (1 << 12)
#define BIT_13       (1 << 13)
#define BIT_14       (1 << 14)
#define BIT_15       (1 << 15)

// Se manca la define
#define SIZEOF_MEMBER(STRUCT, MEMBER) sizeof( ( (STRUCT *) 0 )->MEMBER )

// https://en.wikipedia.org/wiki/Offsetof
#ifndef offsetof
#define offsetof(STRUCT, MEMBER)     \
    ( (size_t)& ( ( (STRUCT *) 0 )->MEMBER ) )
#endif
#define container_of(MEMBER_PTR, STRUCT, MEMBER)    \
    ( (STRUCT *) ( (char *) (MEMBER_PTR) -offsetof(STRUCT, MEMBER) ) )

static inline void * CONST_CAST(const void * cv)
{
    union {
        const void * cv ;
        void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline const void * CPOINTER(uint32_t cv)
{
    union {
        uint32_t cv ;
        const void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline void * POINTER(uint32_t cv)
{
    union {
        uint32_t cv ;
        void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline uint32_t UINTEGER(const void * v)
{
    union {
        uint32_t cv ;
        const void * v ;
    } u ;
    u.v = v ;

    return u.cv ;
}

// EWARM TerminalIO
// ==========================================

#ifdef NDEBUG
// In release non stampo mai
#   define PRINTF(f, ...)
#   define PUTS(a)

#   define BPOINT
#else
// In debug e' utile stampare
#   define PRINTF(f, ...)   printf_(f, ## __VA_ARGS__)
#   define PUTS(a)          puts(a)

#ifndef __cplusplus

#define PRINT_HEX_MAX       10

static inline void print_hex(
    const char * titolo,
    const void * v,
    int dim)
{
    const uint8_t * msg = v ;

    PRINTF("%s [%d]: ", titolo, dim) ;
#ifdef PRINT_HEX_MAX
    dim = MINI(dim, PRINT_HEX_MAX) ;
#endif
    for ( int i = 0 ; i < dim ; i++ ) {
        PRINTF("%02X ", msg[i]) ;
    }

    PRINTF("\n") ;
}

#endif

#   define BPOINT           __BKPT(0)
#endif

// Alcune stampe possono essere disabilitate localmente
#ifdef STAMPA_DBG
#   ifndef NDEBUG
// Allora esce qualcosa!
#   define DBG_ABIL
#   endif
#endif

#ifdef DBG_ABIL

// Globalmente o localmente si puo' definire anche STAMPA_TIK
#ifdef STAMPA_TIK
#ifdef __cplusplus
extern "C" uint32_t HAL_GetTick(void) ;
#else
uint32_t HAL_GetTick(void) ;
#endif

#   define DBG_PRINTF(f, ...)         \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PRINTF(f, ## __VA_ARGS__) ; PUTS("")
#   define DBG_PUTS(a)                \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PUTS(a)
#   define DBG_PRINT_HEX(t, x, d)     \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    print_hex(t, x, d)
#   define DBG_FUN                    \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PUTS(__func__)
#   define DBG_QUA                    \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PRINTF("QUA %s %d\n", __FILE__, __LINE__)
#   define DBG_ERR                    \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PRINTF("ERR %s %d\n", __FILE__, __LINE__)
#   define DBG_ASSERT                 \
    PRINTF( "%u) ", HAL_GetTick() ) ; \
    PRINTF("ASSERT %s %d\n", __FILE__, __LINE__)
#else
#   define DBG_PRINTF(f, ...)           PRINTF(f, ## __VA_ARGS__) ; PUTS("")
#   define DBG_PUTS(a)                  PUTS(a)
#   define DBG_PRINT_HEX(t, x, d)       print_hex(t, x, d)
#   define DBG_FUN                      PUTS(__func__)
#   define DBG_QUA                      PRINTF("QUA %s %d\n", \
                                               __FILE__, \
                                               __LINE__)
#   define DBG_ERR                      PRINTF("ERR %s %d\n", \
                                               __FILE__, \
                                               __LINE__)
#   define DBG_ASSERT                   PRINTF("ASSERT %s %d\n", \
                                               __FILE__, \
                                               __LINE__)
// condizionate
#	define DBG_PRN(c, p)    \
    if ( c ) {               \
        DBG_PRINTF p ;       \
    }

#	define DBG_PUT(c, p)    \
    if ( c ) {               \
        DBG_PUTS(p) ;       \
    }
#endif

#else
#   define DBG_PRINTF(f, ...)
#   define DBG_PUTS(a)
#   define DBG_PRINT_HEX(t, x, d)
#   define DBG_FUN
#   define DBG_QUA
#   define DBG_ERR
#   define DBG_ASSERT

#	define DBG_PRN(c, p)
#	define DBG_PUT(c, p)
#endif

// In debug queste stampano (se stampa abilitata)
#define ASSERT(x)         \
    do {                  \
        if ( !(x) ) {     \
            DBG_ASSERT ;  \
            BPOINT ;      \
        }                 \
    } while ( false )

#define CONTROLLA(x)        \
    do {                \
        if ( !(x) ) {   \
            DBG_ERR ;   \
        }               \
    } while ( false )

// Sei inglese? usa queste
// ==========================================

#define CHECK_IT		CONTROLLA

#endif
