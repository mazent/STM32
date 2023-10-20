#include "utili.h"
#include "diario_priv.h"

#ifdef DDB_RAM_BASE

#include "cmsis_rtos/cmsis_os.h"

#ifdef DDB_RAM_POT_2
static uint32_t msk_pos = 0 ;
#define INCREMENTA(a)   \
    a++ ;               \
    a &= msk_pos ;
#else
static const uint32_t QUANTE = DDB_RAM_DIM / sizeof(DDB_RIGA) ;
#define INCREMENTA(a)        \
    a++ ;                    \
    if ( a >= QUANTE ) {     \
        a = 0 ;              \
    }
#endif

static uint32_t primo = 0 ;
static uint32_t ultimo = 0 ;
static uint32_t mux = 0 ;

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

DDB_RIGA * riga_alloca(void)
{
    DDB_RIGA * riga = POINTER(DDB_RAM_BASE) ;

    if ( blocca() ) {
        uint32_t pos = ultimo ;
        INCREMENTA(ultimo) ;
        if ( ultimo == primo ) {
            // Pieno!
#ifdef DDB_RAM_SVRSCRV
            // ... sovrascrivo
            INCREMENTA(primo) ;
            riga = riga + pos ;
#else
            // ... mi fermo
            riga = NULL ;
            ultimo = pos ;
#endif
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

void riga_libera(DDB_RIGA * riga)
{
    riga->level = DDB_L_NONE ;
}

void riga_scrivi(DDB_RIGA * _)
{
    INUTILE(_) ;
}

#ifdef DDB_RAM_POT_2
bool ddb_iniz(void)
{
    uint32_t nr ;

    do {
        nr = DDB_RAM_DIM / sizeof(DDB_RIGA) ;
        if ( POTENZA_DI_2(nr) ) {
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

#else
bool ddb_iniz(void)
{
    return true ;
}

#endif

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

            INCREMENTA(primo) ;
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
        ddb_stampa(riga) ;
        dim = riga->dim ;
        memcpy(msg, riga->msg, dim) ;
    }

    return dim ;
}

#endif
