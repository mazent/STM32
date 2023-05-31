#include "utili.h"
#define USA_DIARIO
#include "diario.h"
#include "cmsis_rtos/cmsis_os.h"

#ifdef DDB_QUANDO
// "%08X) " + livello + a capo + null
#define DIM_MIN     (8 + 1 + 1 + 4 + 2 + 1)
#else
// livello + a capo + null
#define DIM_MIN     (4 + 2 + 1)
#endif

#if DDB_DIM_MSG <= DIM_MIN
#error OKKIO
#endif

#if DDB_LIV != DDB_LIV_NONE

static const char LVL_ERROR[] = "ERR" ;
static const char LVL_WARNING[] = "WRN" ;
static const char LVL_INFO[] = "INF" ;
static const char LVL_DEBUG[] = "DBG" ;

static osThreadId idTHD = NULL ;
static osMailQId idMQ = NULL ;
static DDB_LEVEL level = DDB_L_NONE ;

typedef struct {
#ifdef DDB_QUANDO
    uint32_t quando ;
#endif
    int dim ;
    DDB_LEVEL level ;
    char msg[DDB_DIM_MSG] ;
} DDB_RIGA ;

static void acapo(DDB_RIGA * pR)
{
    char * msg = pR->msg ;
    int dim = pR->dim ;
    if ( msg[dim - 1] == 0x0A ) {
        // Aggiungo lo 0 finale
        pR->dim++ ;
        return ;
    }

    if ( dim + 2 < DDB_DIM_MSG ) {
        msg[dim] = 0x0D ;
        msg[dim + 1] = 0x0A ;
        msg[dim + 2] = 0 ;
        pR->dim += 3 ;
    }
    else {
        msg[DDB_DIM_MSG - 3] = 0x0D ;
        msg[DDB_DIM_MSG - 2] = 0x0A ;
        msg[DDB_DIM_MSG - 1] = 0 ;
        pR->dim = DDB_DIM_MSG ;
    }
}

static void diario(void * v)
{
    INUTILE(v) ;

    while ( true ) {
        osEvent evn = osMailGet(idMQ, osWaitForever) ;
        if ( osEventMail == evn.status ) {
            DDB_RIGA * riga = evn.value.p ;

            acapo(riga) ;

            const char * slev = "???" ;
            switch ( riga->level ) {
            case DDB_L_ERROR:
                slev = LVL_ERROR ;
                break ;
            case DDB_L_WARNING:
                slev = LVL_WARNING ;
                break ;
            case DDB_L_INFO:
                slev = LVL_INFO ;
                break ;
            case DDB_L_DEBUG:
                slev = LVL_DEBUG ;
                break ;
            default:
                break ;
            }

            static char msg[DIM_MIN + DDB_DIM_MSG] ;
#ifdef DDB_QUANDO
            int dim = snprintf_(msg,
                                sizeof(msg),
                                "%08X) %s - ",
                                riga->quando,
                                slev) ;

#else
            int dim = snprintf_(msg, sizeof(msg), "%s - ", slev) ;
#endif
            memcpy_(msg + dim, riga->msg, riga->dim) ;
            ddb_scrivi(msg, riga->dim + dim) ;
            (void) osMailFree(idMQ, riga) ;
        }
    }
}

bool DDB_iniz(DDB_LEVEL l)
{
    do {
        if ( NULL == idMQ ) {
            osMailQDef(messaggi, DDB_NUM_MSG, DDB_RIGA) ;
            idMQ = osMailCreate(osMailQ(messaggi), NULL) ;
            ASSERT(idMQ) ;
            if ( NULL == idMQ ) {
                break ;
            }
        }

        if ( NULL == idTHD ) {
            osThreadDef(diario, osPriorityIdle, 0, 0) ;
            idTHD = osThreadCreate(osThread(diario), NULL) ;
            ASSERT(idTHD) ;
            if ( NULL == idTHD ) {
                break ;
            }
        }

        level = l ;
    } while ( false ) ;

    return NULL != idTHD ;
}

void DDB_level(DDB_LEVEL l)
{
    level = l ;
}

void DDB_puts(
    DDB_LEVEL l,
    const char * c)
{
    if ( l < level ) {
        return ;
    }

    DDB_RIGA * riga = osMailAlloc(idMQ, 0) ;
    if ( riga ) {
#ifdef DDB_QUANDO
        riga->quando = DDB_QUANDO() ;
#endif
        riga->dim = snprintf_(riga->msg, DDB_DIM_MSG, "%s", c) ;

        if ( riga->dim > 0 ) {
            riga->level = l ;
            (void) osMailPut(idMQ, riga) ;
        }
        else {
            (void) osMailFree(idMQ, riga) ;
        }
    }
}

void DDB_printf(
    DDB_LEVEL l,
    const char * fmt,
    ...)
{
    if ( l < level ) {
        return ;
    }

    DDB_RIGA * riga = osMailAlloc(idMQ, 0) ;
    if ( riga ) {
#ifdef DDB_QUANDO
        riga->quando = DDB_QUANDO() ;
#endif
        va_list args ;

        va_start(args, fmt) ;

        riga->dim = vsnprintf_(riga->msg, DDB_DIM_MSG, fmt, args) ;

        va_end(args) ;

        if ( riga->dim > DDB_DIM_MSG ) {
            riga->dim = DDB_DIM_MSG - 1 ;
        }

        if ( riga->dim > 0 ) {
            riga->level = l ;
            (void) osMailPut(idMQ, riga) ;
        }
        else {
            (void) osMailFree(idMQ, riga) ;
        }
    }
}

void DDB_print_hex(
    DDB_LEVEL l,
    const char * titolo,
    const void * v,
    int dimv)
{
    if ( l < level ) {
        return ;
    }

    DDB_RIGA * riga = osMailAlloc(idMQ, 0) ;
    if ( riga ) {
#ifdef DDB_QUANDO
        riga->quando = DDB_QUANDO() ;
#endif
        if ( NULL == v ) {
            dimv = 0 ;
        }

        const uint8_t * msg = v ;
        bool esito = false ;

        do {
            int dim = 0 ;
            if ( titolo ) {
                dim += snprintf_(riga->msg + dim,
                                 DDB_DIM_MSG - dim,
                                 "%s ",
                                 titolo) ;
            }

            if ( dim >= DDB_DIM_MSG ) {
                break ;
            }

            dim += snprintf_(riga->msg + dim, DDB_DIM_MSG - dim, "[%d]: ", dimv) ;
            if ( dim >= DDB_DIM_MSG ) {
                break ;
            }

            for ( int i = 0 ; i < dimv ; i++ ) {
                dim += snprintf_(riga->msg + dim,
                                 DDB_DIM_MSG - dim,
                                 "%02X ",
                                 msg[i]) ;
                if ( dim >= DDB_DIM_MSG ) {
                    break ;
                }
            }

            if ( dim > DDB_DIM_MSG ) {
                dim = DDB_DIM_MSG - 1 ;
            }
            riga->dim = dim ;

            esito = true ;
        } while ( false ) ;

        if ( esito ) {
            riga->level = l ;
            (void) osMailPut(idMQ, riga) ;
        }
        else {
            (void) osMailFree(idMQ, riga) ;
        }
    }
}

#else
bool DDB_iniz(DDB_LEVEL a)
{
	INUTILE(a) ;
    return false ;
}

void DDB_printf(
		DDB_LEVEL b,
    const char * a,
    ...)
{
    INUTILE(a) ;
    INUTILE(b) ;
}

void DDB_puts(DDB_LEVEL b,const char * a)
{
    INUTILE(a) ;
    INUTILE(b) ;
}

void DDB_print_hex(DDB_LEVEL d,
    const char * a,
    const void * b,
    int c)
{
    INUTILE(a) ;
    INUTILE(b) ;
    INUTILE(c) ;
    INUTILE(d) ;
}

#endif
