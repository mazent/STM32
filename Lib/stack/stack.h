#ifndef LIB_STACK_STACK_H_
#define LIB_STACK_STACK_H_

#include <stdint.h>
#include <stdbool.h>

/*
    Per ottenere uno stack di word di MAX_STACK elementi:

    Definire

        static union {
            UNO_STACK s ;
            uint32_t b[MAX_STACK + 1] ;
        } stk ;

    Inizializzare con

        STK_iniz(&stk.s, MAX_STACK)

    Successivamente si possono usare le altre funzioni

    Puo' essere comoda la macro:
        #define STACK_PUSH(a)   ASSERT( STK_push( &stk.s, PF_2_STACK(a) ) )
*/

typedef struct {
    uint16_t DIM_STACK ;

    /*
             0  1  2               DIM_STACK
            +--+--+--+     +--+--+
        buf |  |  |  | ... |  |  |
            +--+--+--+     +--+--+
             ^                     ^
             |                     |
             +-- top => vuoto      +-- top => pieno
    */
    uint16_t top ;

    uint32_t buf[1] ;
} UNO_STACK ;

static inline void STK_iniz(
    UNO_STACK * x,
    uint16_t dim)
{
    x->DIM_STACK = dim ;
    x->top = 0 ;
}

static inline uint16_t STK_quanti(UNO_STACK * x)
{
    return x->top ;
}

static inline bool STK_push(
    UNO_STACK * x,
    uint32_t elem)
{
    if ( x->top < x->DIM_STACK ) {
    	x->buf[x->top] = elem ;
        x->top++ ;

        return true ;
    }

    return false ;
}

static inline bool STK_pop(
    UNO_STACK * x,
    uint32_t * elem)
{
    if ( x->top > 0 ) {
        x->top-- ;
        *elem = x->buf[x->top] ;

        return true ;
    }

    return false ;
}

// Utili
typedef void (*PF_STACK)(void) ;

static inline PF_STACK STACK_2_PF(uint32_t w)
{
    union {
        uint32_t w ;
        PF_STACK f ;
    } u = {
        .w = w
    } ;

    return u.f ;
}

static inline uint32_t PF_2_STACK(PF_STACK f)
{
    union {
        uint32_t w ;
        PF_STACK f ;
    } u = {
        .f = f
    } ;

    return u.w ;
}

#else
#   warning stack.h incluso
#endif
