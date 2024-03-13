#define STAMPA_DBG
#include "utili.h"
#include "can_config.h"
#include "bsp.h"
#include <math.h>

/*
 * Formule
 * =================================
 *
 *                       CK
 *     Fbit = -------------------------
 *            PRESCALER (1 + BS1 + BS2)
 *
 *
 *            1 + BS1
 *     % = --------------
 *         1 + BS1 + BS2
 */

bool cfg_std(
    S_CAN_CFG * cc,
    const BASE_CAN * nmnl)
{
    bool esito = false ;

    const uint32_t MAX_DEN = 1 + CAN_MAX_BS1 + CAN_MAX_BS2 ;

    do {
        double rapp_freq = CAN_CK_KBS ;
        rapp_freq /= nmnl->Kbs ;
        if ( ceil(rapp_freq) != floor(rapp_freq) ) {
            // Impossibile: rapporto non intero
            DBG_ERR ;
            break ;
        }

        uint32_t pxb = CAN_CK_KBS / nmnl->Kbs ;
        if ( pxb <= MAX_DEN ) {
            cc->prescaler = 1 ;
        }
        else {
            // Cerco il denominatore piu' grande
            uint32_t den = MAX_DEN ;
            for ( ; den > 3 ; den-- ) {
                double prescaler = rapp_freq / den ;
                if ( ceil(prescaler) == floor(prescaler) ) {
                    // Trovato!
                    break ;
                }
            }
            if ( den < 3 ) {
                DBG_ERR ;
                break ;
            }

            cc->prescaler = pxb / den ;
            if ( cc->prescaler > CAN_MAX_PRESCALER ) {
                DBG_ERR ;
                break ;
            }
        }

        uint32_t den = pxb / cc->prescaler ;
        if ( den > MAX_DEN ) {
            DBG_ERR ;
            break ;
        }

        uint32_t num = nmnl->perc * den / 100 ;
        if ( num < 2 ) {
            num = 2 ;
        }

        cc->bs1 = num - 1 ;
        if ( cc->bs1 > CAN_MAX_BS1 ) {
            DBG_QUA ;
            cc->bs1 = CAN_MAX_BS1 ;
            num = 1 + CAN_MAX_BS1 ;
        }
        cc->bs2 = den - num ;
        if ( cc->bs2 > CAN_MAX_BS2 ) {
            DBG_QUA ;
            cc->bs2 = CAN_MAX_BS2 ;
            num = den - cc->bs2 ;
            cc->bs1 = num - 1 ;
        }

        // Verifico
        double kb = CAN_CK_KBS ;
        kb /= cc->prescaler ;
        kb /= 1.0 + cc->bs1 + cc->bs2 ;
        if ( ceil(kb) != floor(kb) ) {
            DBG_ERR ;
            break ;
        }

        esito = (kb == nmnl->Kbs) ;
    } while ( false ) ;

    return esito ;
}

#if 0

// per VS

#define DBG_ERR //printf("ERR %s %d\n", __FILE__, __LINE__)
#define DBG_QUA //printf("QUA %s %d\n", __FILE__, __LINE__)

#define CAN_CK_KBS          36000
#define CAN_MAX_PRESCALER   1024
#define CAN_MAX_BS1         16
#define CAN_MAX_BS2         8

typedef struct {
    uint32_t Kbs ;
    uint8_t perc ;
} BASE_CAN ;

typedef struct {
    uint32_t prescaler ;
    uint32_t bs1 ;
    uint32_t bs2 ;
} S_CAN_CFG ;

int main()
{
    BASE_CAN bc ;

    bc.perc = 80 ;
    uint32_t kbmin = 1 ;

//    for (bc.perc = 50; bc.perc <= 100; bc.perc++) {
    printf("%u %%\n", bc.perc) ;
    for ( bc.Kbs = kbmin ; bc.Kbs <= 1000 ; bc.Kbs++ ) {
        S_CAN_CFG cfg ;

        if ( cfg_std(&cfg, &bc) ) {
            double kb = CAN_CK_KBS ;
            kb /= cfg.prescaler ;
            kb /= 1.0 + cfg.bs1 + cfg.bs2 ;
            if ( ceil(kb) != floor(kb) ) {
                DBG_ERR ;
            }
            else if ( kb == bc.Kbs ) {
                printf("%u Kb %u %% -> P %u BS1 %u BS2 %u\n",
                       bc.Kbs, bc.perc, cfg.prescaler, cfg.bs1, cfg.bs2) ;
            }
            else {
                printf("%u Kb %u %% -X P %u BS1 %u BS2 %u ! %f !\n",
                       bc.Kbs,
                       bc.perc, cfg.prescaler, cfg.bs1, cfg.bs2, kb) ;
            }
        }
        else {
            kbmin++ ;
            printf("%u Kb %u %%: ERR\n", bc.Kbs, bc.perc) ;
        }
    }
//    }

    return 0 ;
}

#endif
