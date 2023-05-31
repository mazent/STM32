#ifndef CC_H_
#define CC_H_

// vedi https://lwip.fandom.com/wiki/Porting_for_an_OS#cc.h

#define USA_DIARIO
#include "diario/diario.h"

#define BYTE_ORDER      LITTLE_ENDIAN

#define LWIP_CHKSUM_ALGORITHM       2

#define PACK_STRUCT_BEGIN       __packed
#define PACK_STRUCT_FIELD(a)     a
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END

#ifdef NDEBUG
#define LWIP_PLATFORM_DIAG(x)
#define LWIP_PLATFORM_ASSERT(x)
#else
#define LWIP_PLATFORM_DIAG(x) do { DDB_INFO x ; } while ( 0 )
#define LWIP_PLATFORM_ASSERT(x) do { DDB_printf(DDB_L_ERROR, \
                                         "Assertion \"%s\" failed at line %d in %s\n", \
                                         x, \
                                         __LINE__, \
                                         __FILE__) ; } while ( 0 )

#endif

#define SYS_ARCH_PROTECT(x)
#define SYS_ARCH_UNPROTECT(x)
#define SYS_ARCH_DECL_PROTECT(x)

#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size)    \
    __attribute__( ( section(".ethernet"),                  \
                     aligned(MEM_ALIGNMENT) ) )             \
    u8_t variable_name[size]

#endif
