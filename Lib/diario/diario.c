#include "utili.h"
#include "diario_priv.h"

#if defined (DDB_RAM_BASE) || DDB_NUM_MSG

#ifdef DDB_QUANDO
#define FORMATO     "%010u) %s -"
#define MSG_POS     (10 + 1 + 1 + 3 + 3)
#else
#define FORMATO     "%s -"
#define MSG_POS     (3 + 3)
#endif

static const char LVL_ERROR[] = "ERR" ;
static const char LVL_WARNING[] = "WRN" ;
static const char LVL_INFO[] = "INF" ;
static const char LVL_DEBUG[] = "DBG" ;

static DDB_LEVEL level = DDB_L_NONE ;

static void acapo(DDB_RIGA * pR)
{
    char * msg = pR->msg ;
    int dim = pR->dim ;
    msg += MSG_POS ;
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

void ddb_stampa(DDB_RIGA * riga)
{
    int dim = 0 ;

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
    dim = snprintf_(riga->msg,
                    DDB_DIM_MSG,
                    FORMATO,
                    riga->quando,
                    slev) ;
#else
    dim = snprintf_(riga->msg, DDB_DIM_MSG, FORMATO, slev) ;
#endif
    riga->msg[dim] = ' ' ;
    riga->dim += dim ;
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

    DDB_RIGA * riga = riga_alloca() ;
    if ( riga ) {
#ifdef DDB_QUANDO
        riga->quando = DDB_QUANDO() ;
#endif
        riga->dim = snprintf_(riga->msg + MSG_POS,
                              DDB_DIM_MSG - MSG_POS,
                              "%s",
                              c) ;

        if ( riga->dim > 0 ) {
            riga->level = l ;
            riga_scrivi(riga) ;
        }
        else {
            riga_libera(riga) ;
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

        riga->dim = vsnprintf_(riga->msg + MSG_POS,
                               DDB_DIM_MSG - MSG_POS,
                               fmt,
                               args) ;

        va_end(args) ;

        if ( riga->dim > DDB_DIM_MSG - MSG_POS ) {
            riga->dim = DDB_DIM_MSG - MSG_POS - 1 ;
        }

        if ( riga->dim > 0 ) {
            riga->level = l ;
            riga_scrivi(riga) ;
        }
        else {
            riga_libera(riga) ;
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
    if ( NULL == riga ) {
        return ;
    }

#ifdef DDB_QUANDO
    riga->quando = DDB_QUANDO() ;
#endif
    if ( NULL == v ) {
        dimv = 0 ;
    }

    const uint8_t * dati = v ;
    bool esito = false ;

    do {
        int dim = MSG_POS ;
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
                             dati[i]) ;
            if ( dim >= DDB_DIM_MSG ) {
                break ;
            }
        }

        if ( dim > DDB_DIM_MSG ) {
            dim = DDB_DIM_MSG - 1 ;
        }
        riga->dim = dim - MSG_POS ;

        esito = true ;
    } while ( false ) ;

    if ( esito ) {
        riga->level = l ;
        riga_scrivi(riga) ;
    }
    else {
        riga_libera(riga) ;
    }
}

bool DDB_iniz(DDB_LEVEL l)
{
    level = l ;

    return ddb_iniz() ;
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

#endif
