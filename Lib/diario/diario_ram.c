#include "utili.h"
#include "diario.h"

#ifdef DDB_RAM_BASE

#include "cmsis_rtos/cmsis_os.h"
#include "stm32h7xx.h"

#ifdef DDB_QUANDO
// "%010X) " + livello + a capo + null
#define DIM_MIN     (10 + 1 + 1 + 4 + 2 + 1)
#else
// livello + a capo + null
#define DIM_MIN     (4 + 2 + 1)
#endif

#if DDB_DIM_MSG <= DIM_MIN
#error OKKIO
#endif

static const char LVL_ERROR[] = "ERR" ;
static const char LVL_WARNING[] = "WRN" ;
static const char LVL_INFO[] = "INF" ;
static const char LVL_DEBUG[] = "DBG" ;

static DDB_LEVEL level = DDB_L_NONE ;

typedef struct {
#ifdef DDB_QUANDO
    uint32_t quando ;
#endif
    int dim ;
    DDB_LEVEL level ;
    char msg[DDB_DIM_MSG] ;
} DDB_RIGA ;

static uint32_t msk_pos = 0 ;
static uint32_t primo = 0 ;
static uint32_t ultimo = 0 ;
static uint32_t mux = 0 ;
static uint32_t pieno = 0 ;

// DHT0008A: 1.3.2 Implementing a mutex

static bool blocca(void)
{
    bool esito = false ;
    uint32_t u, errore ;
ripeti:
    u = __LDREXW(&mux) ;
    if ( 1 == u ) {
        if ( __get_IPSR() != 0 ) {
            // non posso aspettare dentro IRQ
            goto esci ;
        }

        osThreadYield() ;
        goto ripeti ;
    }

    errore = __STREXW(1, &mux) ;
    if ( errore != 0 ) {
        goto ripeti ;
    }
    esito = true ;
    __DMB() ;
esci:
    return esito ;
}

static void sblocca(void)
{
    __DMB() ;
    mux = 0 ;
}

static DDB_RIGA * riga_alloca(void)
{
    DDB_RIGA * riga = POINTER(DDB_RAM_BASE) ;
    uint32_t pos ;

    if ( blocca() ) {
        pos = ultimo ;
        ultimo++ ;
        ultimo &= msk_pos ;
        if ( ultimo == primo ) {
            // Pieno!
            riga = NULL ;
            ultimo = pos ;
            pieno++ ;
        }
        else {
            riga = riga + pos ;
        }

        sblocca() ;
    }
    else {
        riga = NULL ;
    }

    return riga ;
}

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

bool DDB_iniz(DDB_LEVEL l)
{
    uint32_t nr ;

    do {
        level = l ;

        nr = DDB_RAM_DIM / sizeof(DDB_RIGA) ;
        if ( 0 == ( nr & (nr - 1) ) ) {
            // ottimo: e' gia' una potenza di due
            break ;
        }

        // prendo la pot. di due piu' vicina
        uint32_t clz = __CLZ(nr) ;
        uint32_t n = 31 - clz ;
        nr = 1 << n ;
    } while ( false ) ;

    msk_pos = nr - 1 ;

    return true ;
}

void DDB_level(DDB_LEVEL l)
{
    level = l ;
}

int DDB_leggi(char * msg)
{
    int dim = 0 ;
    DDB_RIGA * riga = POINTER(DDB_RAM_BASE) ;

leggi:
    if ( blocca() ) {
        if ( ultimo == primo ) {
            // Vuoto
            riga = NULL ;
        }
        else {
            riga = riga + primo ;

            primo++ ;
            primo &= msk_pos ;
        }

        sblocca() ;
    }
    else {
        riga = NULL ;
    }

    if ( NULL == riga ) {}
    else if ( DDB_L_NONE == riga->level ) {
        // Provo il prossimo
        riga = POINTER(DDB_RAM_BASE) ;
        goto leggi ;
    }
    else {
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

#ifdef DDB_QUANDO
        dim = snprintf_(msg,
                        DDB_DIM_MSG,
                        "%010u) %s - ",
                        riga->quando,
                        slev) ;
#else
        dim = snprintf_(msg, DDB_DIM_MSG, "%s - ", slev) ;
#endif
        memcpy_(msg + dim, riga->msg, riga->dim) ;
        dim += riga->dim ;
    }

    return dim ;
}

void DDB_puts(
    DDB_LEVEL l,
    const char * c)
{
    if ( l < level ) {
        return ;
    }

    DDB_RIGA * riga = riga_alloca() ;
    if ( riga ) {
#ifdef DDB_QUANDO
        riga->quando = DDB_QUANDO() ;
#endif
        riga->dim = snprintf_(riga->msg, DDB_DIM_MSG, "%s", c) ;

        if ( riga->dim > 0 ) {
            riga->level = l ;
        }
        else {
            // Annullo
            riga->level = DDB_L_NONE ;
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

    DDB_RIGA * riga = riga_alloca() ;
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
        }
        else {
            riga->level = DDB_L_NONE ;
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

    DDB_RIGA * riga = riga_alloca() ;
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

#ifdef DDB_MAX_HEX
            dimv = MINI(dimv, DDB_MAX_HEX) ;
#endif
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
        }
        else {
            riga->level = DDB_L_NONE ;
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

void DDB_puts(
    DDB_LEVEL b,
    const char * a)
{
    INUTILE(a) ;
    INUTILE(b) ;
}

void DDB_print_hex(
    DDB_LEVEL d,
    const char * a,
    const void * b,
    int c)
{
    INUTILE(a) ;
    INUTILE(b) ;
    INUTILE(c) ;
    INUTILE(d) ;
}

int DDB_leggi(char * _)
{
    INUTILE(_) ;
    return 0 ;
}

#endif      // DDB_RAM_BASE
