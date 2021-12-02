#ifndef DBGP_H_
#define DBGP_H_

// TerminalIO
// ==========================================

#ifdef NDEBUG
	// In release non stampo mai
#	define PRINTF(f, ...)
#	define PUTS(a)
#	define DBG_print_hex(t, x, d)
#else
	// In debug e' utile stampare
#   include <stdio.h>

#	define PRINTF(f, ...)	printf(f, __VA_ARGS__)
#	define PUTS(a)			puts(a)

#include <stdint.h>

static inline void DBG_print_hex(const char * titolo, const void * v, const int dim)
{
	const uint8_t * msg = v ;

	if (titolo) {
		printf(titolo) ;
	}

	printf("[%d] ", dim) ;

	for (int i=0 ; i<dim ; i++) {
		printf("%02X ", msg[i]) ;
	}

	puts("");
}

#endif

// Alcune stampe possono essere disabilitate
#ifdef STAMPA_DBG
#	define DBG_FUN						PUTS(__FUNCTION__)
#	define DBG_ERR						PRINTF("ERR %s %d\n", __FILE__, __LINE__)
#	define DBG_QUA						PRINTF("QUA %s %d\n", __FILE__, __LINE__)
#	define DBG_PRINTF(f, ...)			PRINTF(f, __VA_ARGS__);PUTS("")
#	define DBG_PUTS(a)					PUTS(a)
#	define DBG_PRINT_HEX(t, x, d)		DBG_print_hex(t, x, d)
#	define ASSERT(x)	\
	do {                \
		if ( !(x) ) {   \
			PRINTF("ASSERT %s %d\n", __FILE__, __LINE__) ; \
		}               \
	} while (false)
#else
#	define DBG_FUN
#	define DBG_ERR
#	define DBG_QUA
#	define DBG_PRINTF(f, ...)
#	define DBG_PUTS(a)
#	define DBG_PRINT_HEX(t, x, d)
#	define ASSERT(x)
#endif

// Gli errori li stampo sempre
#define CHECK(x)        \
    do {                \
        if ( !(x) ) {   \
            DBG_ERR ;   \
        }               \
    } while (false)


#endif
