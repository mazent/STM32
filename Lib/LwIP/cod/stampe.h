#ifndef LIB_LWIP_PORT_COD_STAMPE_H_
#define LIB_LWIP_PORT_COD_STAMPE_H_

#if 1

// Uso il diario
#undef DBG_PRINTF
#undef DBG_PUTS
#undef DBG_PRINT_HEX
#undef DBG_FUN
#undef DBG_QUA
#undef DBG_ERR
#undef DBG_ASSERT
#undef DBG_PRN
#undef DBG_PUT

// definire prima di includere
//#define DIARIO_LIV_DBG
#include "diario/diario.h"

#define DBG_PRINTF          DDB_PRINTF
#define DBG_PUTS            DDB_PUTS
#define DBG_PRINT_HEX       DDB_PRINT_HEX
#define DBG_FUN             DDB_FUN
#define DBG_QUA             DDB_QUA
#define DBG_ERR             DDB_ERR
#define DBG_ASSERT          DDB_ASSERT

// condizionate
#   define DBG_PRN(c, p)    \
    do {                    \
        if ( c ) {          \
            DDB_PRINTF p ;  \
        }                   \
    } while ( false )

#   define DBG_PUT(c, p)    \
    do {                    \
        if ( c ) {          \
            DDB_PUTS(p) ;   \
        }                   \
    } while ( false )

#endif

#else
#   warning stampe.h incluso
#endif
