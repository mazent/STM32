#ifndef LIB_DATAORA_DATAORA_H_
#define LIB_DATAORA_DATAORA_H_

#include <stdbool.h>

// Recupera data e ora di compilazione

typedef struct {
    int ora ;
    int min ;
    int sec ;
} S_ORA_COMP ;

bool trova_ora_comp(S_ORA_COMP *) ;

#define DO_JANUARY                   1
#define DO_FEBRUARY                  2
#define DO_MARCH                     3
#define DO_APRIL                     4
#define DO_MAY                       5
#define DO_JUNE                      6
#define DO_JULY                      7
#define DO_AUGUST                    8
#define DO_SEPTEMBER                 9
#define DO_OCTOBER                   10
#define DO_NOVEMBER                  11
#define DO_DECEMBER                  12

#define DO_MONDAY                  1
#define DO_TUESDAY                 2
#define DO_WEDNESDAY               3
#define DO_THURSDAY                4
#define DO_FRIDAY                  5
#define DO_SATURDAY                6
#define DO_SUNDAY                  7

typedef struct {
    int anno ;
    int mese ;       // DO_JANUARY ...
    int giorno ;
    int g_sett ;	// DO_MONDAY ...
} S_DATA_COMP ;

bool trova_data_comp(S_DATA_COMP *) ;

#else
#   warning dataora.h incluso
#endif
