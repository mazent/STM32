#define STAMPA_DBG
#include "utili.h"
#include "ttc.h"
#include "cmsis_rtos/cmsis_os.h"

extern uint16_t CRC_1021_v(
    uint16_t,
    const void *,
    int) ;
extern uint16_t CRC_1021(
    uint16_t,
    uint8_t) ;

extern const uint16_t TTC_CRC_INI ;

#define EVN_TX              (1 << 0)

static osThreadId idTHD = NULL ;

#define IN_CODA     5

osMessageQDef(trasmetti, IN_CODA, UN_TTC *) ;
static osMessageQId trasmetti = NULL ;

#define INIZIO_TRAMA             0xC5
#define FINE_TRAMA               0xC2
#define CARATTERE_DI_FUGA        0xCF

/*
 * I bit piu' alti identificano ...
 */
#define CMD_BIT_RISPOSTA   ( (CMD_T) (1 << 15) )
#define CMD_BIT_ERRORE     ( (CMD_T) (1 << 14) )

/*
 * Ci devono essere almeno: mitt, dest, cmd, crc
 *        1     1     2           2
 *     +-----+-----+-----+-----+-----+
 *     | mit | dst | cmd | ... | crc |
 *     +-----+-----+-----+-----+-----+
 */

static const uint32_t MINI_DIM = 1 + 1 + sizeof(CMD_T) + 2 ;

static void da_capo(UN_TTC * uc)
{
    uc->dimRx = 0 ;
    uc->nega = false ;
}

static void comTHD(void * v)
{
    INUTILE(v) ;

    while ( true ) {
        osEvent evn = osSignalWait(0, osWaitForever) ;

        if ( osEventSignal != evn.status ) {
            DBG_ERR ;
            continue ;
        }

        if ( EVN_TX & evn.value.signals ) {
            while ( true ) {
                osEvent event = osMessageGet(trasmetti, 0) ;
                if ( osEventMessage == event.status ) {
                    UN_TTC * uc = (UN_TTC *) event.value.p ;

                    CONTROLLA( uc->disp_tx(uc->memTx, uc->dimTx) ) ;
                }
                else {
                    break ;
                }
            }
        }
    }
}

static void appendi(
    UN_TTC * uc,
    uint8_t x)
{
    uint8_t * mem = uc->memTx ;

    if ( (INIZIO_TRAMA == x) ||
         (FINE_TRAMA == x) ||
         (CARATTERE_DI_FUGA == x) ) {
        mem[uc->dimTx++] = CARATTERE_DI_FUGA ;
        mem[uc->dimTx++] = NEGA(x) ;
    }
    else {
        mem[uc->dimTx++] = x ;
    }
}

static void componi(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd,
    const void * p,
    uint32_t dim)
{
    uint8_t * mem = uc->memTx ;
    uint16_t crc = CRC_1021(TTC_CRC_INI, dst) ;

    crc = CRC_1021(crc, uc->io) ;

    uc->dimTx = 0 ;
    mem[uc->dimTx++] = INIZIO_TRAMA ;

    // Indirizzi
    appendi(uc, dst) ;
    appendi(uc, uc->io) ;

    // Comando
    union {
        CMD_T c ;
        uint8_t b[sizeof(CMD_T)] ;
    } u ;
    u.c = cmd ;
    crc = CRC_1021_v( crc, u.b, sizeof(CMD_T) ) ;
    for ( uint32_t j = 0 ; j < sizeof(CMD_T) ; j++ ) {
        appendi(uc, u.b[j]) ;
    }

    // Dati
    if ( p ) {
        const uint8_t * dati = (const uint8_t *) p ;
        crc = CRC_1021_v(crc, dati, dim) ;
        for ( uint32_t j = 0 ; (j < dim) && ( uc->dimTx < (uc->DIM_TX - 5) ) ;
              j++ ) {
            appendi(uc, dati[j]) ;
        }
    }

    // Crc
    union {
        uint16_t u ;
        uint8_t b[sizeof(uint16_t)] ;
    } v ;
    v.u = crc ;
    appendi(uc, v.b[1]) ;
    appendi(uc, v.b[0]) ;

    mem[uc->dimTx++] = FINE_TRAMA ;

    ASSERT(uc->dimTx < uc->DIM_TX) ;
}

static void invia(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd,
    const void * v,
    uint32_t dim)
{
    if ( 0 == dim ) {
        v = NULL ;
    }
    else if ( NULL == v ) {
        dim = 0 ;
    }

    componi(uc, dst, cmd, v, dim) ;

    // NOLINTNEXTLINE(bugprone-branch-clone)
    if ( osOK != osMessagePut(trasmetti, (uint32_t) uc, osWaitForever) ) {
        DBG_ERR ;
    }
    else if ( osOK != osSignalSet(idTHD, EVN_TX) ) {
        DBG_ERR ;
    }
}

/*
 * Se la linea e' usata in half-duplex,
 *     posso rispondere nelle singole funzioni
 * altrimenti
 *     posso comporre la risposta, ma il thd deve sincronizzare
 *     l'accesso al dispositivo
 *
 * Per quanto riguarda l'inizializzazione, si potrebbe invocare
 * la funzione nella COM_iniz, ma preferisco che se ne occupi il thd
 */

bool TTC_iniz(UN_TTC * uc)
{
    bool esito = false ;

    do {
        ASSERT(uc) ;
        if ( NULL == uc ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->memRx) ;
        if ( NULL == uc->memRx ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->DIM_RX) ;
        if ( 0 == uc->DIM_RX ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->memTx) ;
        if ( NULL == uc->memTx ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->DIM_TX) ;
        if ( 0 == uc->DIM_TX ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->cb) ;
        if ( NULL == uc->cb ) {
            DBG_ERR ;
            break ;
        }
        ASSERT(uc->disp_tx) ;
        if ( NULL == uc->disp_tx ) {
            DBG_ERR ;
            break ;
        }

        if ( NULL == trasmetti ) {
            trasmetti = osMessageCreate(osMessageQ(trasmetti), NULL) ;

            ASSERT(trasmetti) ;
            if ( NULL == trasmetti ) {
                DBG_ERR ;
                break ;
            }
        }

        // Alla fine, cosi' lo uso per vedere se tutto e' a posto
        if ( NULL == idTHD ) {
            osThreadDef(comTHD, osPriorityNormal, 0, 0) ;
            idTHD = osThreadCreate(osThread(comTHD), NULL) ;

            ASSERT(idTHD) ;
            if ( NULL == idTHD ) {
                DBG_ERR ;
                break ;
            }
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

void TTC_elabora(
    UN_TTC * uc,
    const uint8_t * dati,
    uint32_t LETTI)
{
    uint8_t * Dati = (uint8_t *) uc->memRx ;

    //DBG_PRINT_HEX("RX", dati, LETTI) ;

    for ( uint32_t i = 0 ; i < LETTI ; i++ ) {
        uint8_t rx = dati[i] ;
        if ( uc->nega ) {
            rx = NEGA(rx) ;

            switch ( rx ) {
            case INIZIO_TRAMA:
            case FINE_TRAMA:
            case CARATTERE_DI_FUGA:
                // Solo questi sono ammessi
                if ( uc->DIM_RX == uc->dimRx ) {
                    // Non ci stanno
                    da_capo(uc) ;
                }
                else {
                    Dati[uc->dimRx++] = rx ;
                    uc->nega = false ;
                }
                break ;
            default:
                da_capo(uc) ;
                DBG_ERR ;
                break ;
            }
        }
        else if ( INIZIO_TRAMA == rx ) {
            da_capo(uc) ;
        }
        else if ( CARATTERE_DI_FUGA == rx ) {
            uc->nega = true ;
        }
        else if ( FINE_TRAMA == rx ) {
            do {
                // Ci devono essere almeno: mitt, dest, cmd, crc
                if ( uc->dimRx < MINI_DIM ) {
                    DBG_ERR ;
                    break ;
                }

                // Il crc deve essere valido
                if ( 0 != CRC_1021_v(TTC_CRC_INI, uc->memRx, uc->dimRx) ) {
                    DBG_ERR ;
                    break ;
                }

                // Avviso
                CMD_T cmd ;
                memcpy_( &cmd, Dati + COM_POS_CMD, sizeof(CMD_T) ) ;
                // tolgo inutili
                uc->dimRx -= MINI_DIM ;
                void * prm = Dati + COM_POS_DATI ;
                if ( uc->io == Dati[COM_POS_DST] ) {
                    // MIO!
                    uc->cb(Dati[COM_POS_MIT], cmd, prm, uc->dimRx) ;
                }
                else if ( NULL == uc->err ) {
                    // Ottimo
                }
                else {
                    uc->err(Dati[COM_POS_DST],
                            Dati[COM_POS_MIT],
                            cmd,
                            prm,
                            uc->dimRx) ;
                }
            } while ( false ) ;
        }
        else if ( uc->DIM_RX == uc->dimRx ) {
            // Non ci stanno
            da_capo(uc) ;

            DBG_ERR ;
        }
        else {
            Dati[uc->dimRx++] = rx ;
        }
    }
}

void TTC_risposta(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd,
    const void * v,
    uint32_t dim)
{
    if ( idTHD ) {
        cmd |= CMD_BIT_RISPOSTA ;

        invia(uc, dst, cmd, v, dim) ;
    }
}

void TTC_scono(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd)
{
    if ( idTHD ) {
        cmd |= CMD_BIT_ERRORE ;

        invia(uc, dst, cmd, NULL, 0) ;
    }
}

void TTC_errore(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd)
{
    if ( idTHD ) {
        cmd |= CMD_BIT_RISPOSTA | CMD_BIT_ERRORE ;

        invia(uc, dst, cmd, NULL, 0) ;
    }
}

void TTC_domanda(
    UN_TTC * uc,
    IND_T dst,
    CMD_T cmd,
    const void * v,
    uint32_t dim)
{
    if ( idTHD ) {
        invia(uc, dst, cmd, v, dim) ;
    }
}
