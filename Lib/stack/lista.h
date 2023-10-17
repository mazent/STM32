#ifndef LISTA_H_
#define LISTA_H_

#include <stdint.h>
#include <stdbool.h>

/*
    Per ottenere una lista (FIFO) di word di MAX_LISTA elementi:

    Definire

        static union {
            UNA_LISTA s ;
            uint32_t b[MAX_LISTA + 2] ;
        } lst ;

    Inizializzare con

        LST_iniz(&lst.s, MAX_LISTA)

    Successivamente si possono usare le altre funzioni

*/

typedef struct {
    uint16_t DIM_LISTA ;

    uint16_t primo ;
    uint16_t ultimo ;
    uint16_t quanti ;

    uint32_t buf[1] ;
} UNA_LISTA ;

static inline void LST_iniz(
    UNA_LISTA * x,
    uint16_t dim)
{
    x->DIM_LISTA = dim ;
    x->primo = x->ultimo = x->quanti = 0 ;
}

static inline uint16_t LST_quanti(UNA_LISTA * x)
{
    return x->quanti ;
}

static inline bool LST_ins(
    UNA_LISTA * x,
    uint32_t elem)
{
    if ( x->quanti < x->DIM_LISTA ) {
        x->quanti++ ;
        x->buf[x->ultimo] = elem ;
        x->ultimo++ ;
        if ( x->ultimo == x->DIM_LISTA ) {
            x->ultimo = 0 ;
        }

        return true ;
    }

    return false ;
}

static inline bool LST_est(
    UNA_LISTA * x,
    uint32_t * elem)
{
    if ( x->quanti > 0 ) {
        x->quanti-- ;
        *elem = x->buf[x->primo] ;
        x->primo++ ;
        if ( x->primo == x->DIM_LISTA ) {
            x->primo = 0 ;
        }

        return true ;
    }

    return false ;
}

static inline void LST_test(UNA_LISTA * x)
{
    uint32_t elem = 1 ;
    for ( int _ = 0 ; _ < 4 ; _++, elem++ ) {
        if ( !LST_ins(x, elem) ) {
            DBG_ERR ;
            continue ;
        }
    }

    for ( int _ = 0 ; _ < x->DIM_LISTA ; _++ ) {
        uint32_t p ;
        if ( !LST_est(x, &p) ) {
            DBG_ERR ;
            continue ;
        }
        elem++ ;
    }
}

#else
#   warning lista.h incluso
#endif
