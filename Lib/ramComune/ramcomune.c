#define STAMPA_DBG
#include "utili.h"

#ifdef DESCRITTORE_MIO
#include "ramcomune.h"
#include "vp/vp.h"

/*
 * Scambio di informazioni col resto del mondo stile mio
 */

#define VAL_TB_MAIN         0x84CC1C74
#define VAL_TB_SRV          0xB57C9A3B
#define VAL_TB_COLLAUDO     0x2FCBAD16

typedef struct {
    S_VP tipoBoot ;
} S_RAMCOMUNE ;

__attribute__( ( section(".ram_comune") ) )
static S_RAMCOMUNE rc ;

void RC_iniz(void)
{
    CONTROLLA( VP_ini(&rc.tipoBoot) ) ;
}

RC_TIPO_BOOT RC_cosa_butto(void)
{
    switch ( VP_leggi(&rc.tipoBoot) ) {
    default:
    case 0:
        return TB_SCONO ;
    case VAL_TB_MAIN:
        return TB_MAIN ;
    case VAL_TB_SRV:
        return TB_SRV ;
    case VAL_TB_COLLAUDO:
        return TB_COLLAUDO ;
    }
}

void RC_butta_questo(RC_TIPO_BOOT tb)
{
    switch ( tb ) {
    case TB_COLLAUDO:
        VP_nuovo(&rc.tipoBoot, VAL_TB_COLLAUDO) ;
        break ;
    case TB_MAIN:
        VP_nuovo(&rc.tipoBoot, VAL_TB_MAIN) ;
        break ;
    case TB_SRV:
        VP_nuovo(&rc.tipoBoot, VAL_TB_SRV) ;
        break ;
    default:
        DBG_ERR ;
        break ;
    }
}

void RC_ma(bool valida)
{
    UNUSED(valida) ;
}

void RC_sa(bool valida)
{
    UNUSED(valida) ;
}

#endif
