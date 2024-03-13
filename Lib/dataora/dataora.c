//#define STAMPA_DBG
#include "utili.h"
#include "dataora.h"

// cfr https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
// Schwerdtfeger's method
static int weekday(
    int y,
    int m,
    int d)
{
    int c, g ;
    if ( m >= 3 ) {
        c = y / 100 ;
        g = y - 100 * c ;
    }
    else {
        c = (y - 1) / 100 ;
        g = y - 1 - 100 * c ;
    }

    static const int tab_e[] = {
        0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4
    } ;
    int e = tab_e[m - 1] ;

    static const int tab_f[] = {
        0, 5, 3, 1
    } ;
    int f = tab_f[c % 4] ;

    return ( d + e + f + g + (g >> 2) ) % 7 ;
}

bool trova_data_comp(S_DATA_COMP * pDC)
{
    char data[20] = __DATE__ ;

    char * mese = strtok(data, " ") ;
    if ( NULL == mese ) {
        return false ;
    }

    char * giorno = strtok(NULL, " ") ;
    if ( NULL == giorno ) {
        return false ;
    }

    char * anno = strtok(NULL, " ") ;
    if ( NULL == anno ) {
        return false ;
    }

    switch ( mese[0] ) {
    case 'D':
        pDC->mese = DO_DECEMBER ;
        break ;
    case 'F':
        pDC->mese = DO_FEBRUARY ;
        break ;
    case 'N':
        pDC->mese = DO_NOVEMBER ;
        break ;
    case 'O':
        pDC->mese = DO_OCTOBER ;
        break ;
    case 'S':
        pDC->mese = DO_SEPTEMBER ;
        break ;
    case 'A':
        if ( 'p' == mese[1] ) {
            pDC->mese = DO_APRIL ;
        }
        else {
            pDC->mese = DO_AUGUST ;
        }
        break ;
    case 'J':
        if ( 'a' == mese[1] ) {
            pDC->mese = DO_JANUARY ;
        }
        else if ( 'l' == mese[2] ) {
            pDC->mese = DO_JULY ;
        }
        else {
            pDC->mese = DO_JUNE ;
        }
        break ;
    case 'M':
        if ( 'r' == mese[2] ) {
            pDC->mese = DO_MARCH ;
        }
        else {
            pDC->mese = DO_MAY ;
        }
        break ;

    default:
        return false ;
    }

    pDC->giorno = atoi(giorno) ;
    pDC->anno = atoi(anno) ;
    pDC->g_sett = weekday(pDC->anno, pDC->mese, pDC->giorno) ;

    return true ;
}

bool trova_ora_comp(S_ORA_COMP * pOC)
{
    char sora[20] = __TIME__ ;

    char * ora = strtok(sora, ":") ;
    if ( NULL == ora ) {
        return false ;
    }

    char * minuti = strtok(NULL, ":") ;
    if ( NULL == minuti ) {
        return false ;
    }

    char * secondi = strtok(NULL, ":") ;
    if ( NULL == secondi ) {
        return false ;
    }

    pOC->ora = atoi(ora) ;
    pOC->min = atoi(minuti) ;
    pOC->sec = atoi(secondi) ;

    return true ;
}
