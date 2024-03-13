#define STAMPA_DBG
#include "utili.h"
#include "can.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include "bsp.h"

extern const int CK_KBS ;

#define MAX_NOMI_PRESCALER      512
#define MAX_NOMI_BS1    256
#define MAX_NOMI_BS2    128

#define MAX_DATA_PRESCALER      32
#define MAX_DATA_BS1    32
#define MAX_DATA_BS2    16

#define TSEG1_MIN       1
#define TSEG2_MIN       1

#ifndef MIN
#define MIN(x, y) ( ( (x) < (y) ) ? (x) : (y) )
#endif
#ifndef MAX
#define MAX(x, y) ( ( (x) > (y) ) ? (x) : (y) )
#endif

typedef struct {
    // Ingresso
    int TSEG1_MAX ;
    int TSEG2_MAX ;
    int PRESC_MAX ;

    // Uscita
    int Prescaler ;
    int TimeSeg1 ;
    int TimeSeg2 ;
} PARAM ;

/*
 * CK = 85 MHz
 *
 *                   CK
 * Fbit = --------------------------
 *        PRESCALER (1 + BS1 + BS2)
 *
 *        1 + BS1        N
 * % = -------------- = ---
 *     1 + BS1 + BS2     D
 *
 *  CK
 * ---- = PRESCALER D
 * Fbit
 *
 */

/*
 * essendo
 * T = 1 + TSEG1 + TSEG2
 * p = (1 + TSEG1) / T
 * si ha
 * T = TSEG2 / (1-p)
 * T = (1+TSEG1) / p
 * e quindi
*/

static inline double T_from_tseg2(
    double tseg2,
    double p)
{
    return tseg2 / (1 - p) ;
}

static inline double T_from_tseg1(
    double tseg1,
    double p)
{
    return (tseg1 + 1) / p ;
}

static void calcola(
    PARAM * usc,
    const BASE_CAN * prm)
{
    const double p = ( (double) prm->perc ) / 100.0 ;

    const double Tmax1 = T_from_tseg1(usc->TSEG1_MAX, p) ;
    const double Tmax2 = T_from_tseg2(usc->TSEG2_MAX, p) ;
    const double Tmin1 = T_from_tseg1(TSEG1_MIN, p) ;
    const double Tmin2 = T_from_tseg2(TSEG2_MIN, p) ;

    const double Tmax = MIN(Tmax1, Tmax2) ;
    const double Tmin = MAX(Tmin1, Tmin2) ;

    /*
     essendo anche
     T = (fclk/fvol)/prescaler
     si ha
     presc = (fclk/fvol) / T
     e quindi
    */

    const double fclk = CK_KBS ;
    const double fclk_on_f = fclk / prm->Kbs ;
    const double presc_max_ = fclk_on_f / Tmin ;
    const double presc_min_ = fclk_on_f / Tmax ;

    /*
      il prescaler puo' essere solo intero
      il minimo intero e' l'intero minimo piu' grande del presc_min
      il massimo intero e' l'intero massimo piu' piccolo del presc_max
    */

    const int presc_min = (int) ceil(presc_min_) ;
    const int presc_max = MIN( (int) floor(presc_max_), usc->PRESC_MAX ) ;

    /*
      cerco una combinazione che minimizzi l'errore
      del periodo e della percentuale secondo i pesi
    */
    const double peso_errore_T = 0.9 ;
    const double peso_errore_p = 0.1 ;

    double e_attuale = 1 ;

    for ( int presc = presc_min ; presc <= presc_max ; presc++ ) {
        const double T = fclk_on_f / presc ;
        const int Tr = (int) round(T) ;

        const double Tseg2 = (1 - p) * Tr ;
        const int Tseg2r = (int) round(Tseg2) ;

        const int Tseg1r = Tr - Tseg2r - 1 ;

        const double etr = fabs( (T - Tr) / T ) ;
        const double epr = fabs( (Tseg2r - Tseg2) / T ) ;

        if ( etr < 1e-4 && epr < 1e-3 ) {
            // mi accontento
            usc->TimeSeg1 = Tseg1r ;
            usc->TimeSeg2 = Tseg2r ;
            usc->Prescaler = presc ;
            break ;
        }

        const double e = epr * peso_errore_p + etr * peso_errore_T ;

        if ( e_attuale > e ) {
            e_attuale = e ;
            usc->TimeSeg1 = Tseg1r ;
            usc->TimeSeg2 = Tseg2r ;
            usc->Prescaler = presc ;
        }
    }
}

void cfg_std(
    FDCAN_HandleTypeDef * uc,
    BASE_CAN * prm)
{
    PARAM p = {
        .TSEG1_MAX = MAX_NOMI_BS1,
        .TSEG2_MAX = MAX_NOMI_BS2,
        .PRESC_MAX = MAX_NOMI_PRESCALER
    } ;

    calcola(&p, prm) ;

    uc->Init.NominalPrescaler = p.Prescaler ;
    uc->Init.NominalTimeSeg1 = p.TimeSeg1 ;
    uc->Init.NominalTimeSeg2 = p.TimeSeg2 ;
    DBG_PRINTF("nominal: Prescaler=%d TimeSeg1=%d TimeSeg2=%d",
               p.Prescaler,
               p.TimeSeg1,
               p.TimeSeg2) ;
}

void cfg_fd(
    FDCAN_HandleTypeDef * uc,
    BASE_CAN * nmnl,
    BASE_CAN * data)
{
    PARAM p = {
        .TSEG1_MAX = MAX_NOMI_BS1,
        .TSEG2_MAX = MAX_NOMI_BS2,
        .PRESC_MAX = MAX_NOMI_PRESCALER
    } ;

    calcola(&p, nmnl) ;

    uc->Init.NominalPrescaler = p.Prescaler ;
    uc->Init.NominalTimeSeg1 = p.TimeSeg1 ;
    uc->Init.NominalTimeSeg2 = p.TimeSeg2 ;
    DBG_PRINTF("nominal: Prescaler=%d TimeSeg1=%d TimeSeg2=%d",
               p.Prescaler,
               p.TimeSeg1,
               p.TimeSeg2) ;

    p.TSEG1_MAX = MAX_DATA_BS1 ;
    p.TSEG2_MAX = MAX_DATA_BS2 ;
    p.PRESC_MAX = MAX_DATA_PRESCALER ;

    calcola(&p, data) ;

    uc->Init.DataPrescaler = p.Prescaler ;
    uc->Init.DataTimeSeg1 = p.TimeSeg1 ;
    uc->Init.DataTimeSeg2 = p.TimeSeg2 ;
    DBG_PRINTF("data: Prescaler=%d TimeSeg1=%d TimeSeg2=%d",
               p.Prescaler,
               p.TimeSeg1,
               p.TimeSeg2) ;
}

#if 0

#include <stdio.h>

#warning OKKIO

void app(void)
{
    BASE_CAN ing = {
        .Kbs = 45,
        .perc = 50
    } ;
#if 0
    // standard/nominal
    // Freq da -0.58 % a 0.60 %
    // Camp da -2.06 % a 2.12 %

    PARAM usc = {
        .TSEG1_MAX = MAX_NOMI_BS1,
        .TSEG2_MAX = MAX_NOMI_BS2,
        .PRESC_MAX = MAX_NOMI_PRESCALER
    } ;
#   define MIN_FREQ     1
#   define MAX_FREQ     1000
#else
    // Data
    // Freq da -4.40 % a 4.54 %
    // Camp da -7.14 % a 6.71 %

    PARAM usc = {
        .TSEG1_MAX = MAX_DATA_BS1,
        .TSEG2_MAX = MAX_DATA_BS2,
        .PRESC_MAX = MAX_DATA_PRESCALER
    } ;
#   define MIN_FREQ     1000
#   define MAX_FREQ     8000
#endif
    float df_min = 0.0, df_max = 0.0, dp_min = 0.0, dp_max = 0.0 ;

    for ( ing.Kbs = MIN_FREQ ; ing.Kbs <= MAX_FREQ ; ++ing.Kbs ) {
        for ( ing.perc = 50 ; ing.perc <= 95 ; ++ing.perc ) {
            calcola(&usc, &ing) ;

            const float d = 1 + usc.TimeSeg1 + usc.TimeSeg2 ;
            float f = CK_KBS ;
            f /= usc.Prescaler ;
            f /= d ;
            float p = 100.0 * (1 + usc.TimeSeg1) ;
            p /= d ;

            float df = 100.0 * (f - ing.Kbs) ;
            df /= ing.Kbs ;

            float dp = p - ing.perc ;

            if ( df < df_min ) {
                df_min = df ;
            }
            else if ( df > df_max ) {
                df_max = df ;
            }

            if ( dp < dp_min ) {
                dp_min = dp ;
            }
            else if ( dp > dp_max ) {
                dp_max = dp ;
            }
#if 0
            if ( fabs(df) >= 4.0 ) {
                printf("%d Kbs - %d %%\n", ing.Kbs, ing.perc) ;
                printf("\t Prescaler %d\n", usc.Prescaler) ;
                printf("\t TimeSeg1  %d\n", usc.TimeSeg1) ;
                printf("\t TimeSeg2  %d\n", usc.TimeSeg2) ;

                printf("\t %.2f Kbs - %.2f %%\n", f, p) ;
                printf("\t %.2f - %.2f\n", df, dp) ;
            }
#endif
        }
    }

    printf("Freq da %.2f %% a %.2f %%\n", df_min, df_max) ;
    printf("Camp da %.2f %% a %.2f %%\n", dp_min, dp_max) ;
}

#endif
