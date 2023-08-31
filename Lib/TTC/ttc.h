#ifndef TTC_H_
#define TTC_H_

/*
 * Trasporto + Trama + Comandi in un solo modulo!
 *
 * Occorre che da qualche parte ci sia:
 *     const uint16_t TTC_CRC_INI = ? ;
 */

#include <stdint.h>
#include <stdbool.h>

// Trama: indirizzi
typedef uint8_t IND_T ;

// Comando
typedef uint16_t CMD_T ;

// Eventi
typedef void (*COM_CALLBACK)(IND_T mitt, CMD_T, void *, uint32_t) ;
typedef void (*ERR_CALLBACK)(IND_T dst, IND_T mitt, CMD_T, void *, uint32_t) ;

typedef struct {
    IND_T io ;

    // Spazio per comandi
    // Deve contenere un pacchetto completo (senza caratteri di fuga)
    const uint32_t DIM_RX ;
    void * memRx ;

    // Spazio per risposte
    // Deve contenere un pacchetto completo (inclusi i caratteri di fuga -> 2 * DIM_RX)
    const uint32_t DIM_TX ;
    void * memTx ;

    // Invocata quando arriva un messaggio (comando o risposta)
    COM_CALLBACK cb ;

    // Invocata se arriva qualcosa destinata ad altri (opzionale)
    ERR_CALLBACK err ;

    // Trasporto da usare
    bool (* disp_tx)(
        const void *,
        uint32_t) ;

    // Gestiti da TTC
    bool nega ;
    uint32_t dimRx ;
    uint32_t dimTx ;
} UN_TTC ;

bool TTC_iniz(UN_TTC *) ;

void TTC_elabora(
    UN_TTC *,
    const uint8_t *,
    const uint32_t) ;

// Comandi
void TTC_domanda(UN_TTC *, IND_T, CMD_T, const void *, uint32_t) ;

// Risposte
void TTC_scono(UN_TTC *, IND_T, CMD_T) ;
void TTC_errore(UN_TTC *, IND_T, CMD_T) ;
void TTC_risposta(UN_TTC *, IND_T, CMD_T, const void *, uint32_t) ;

/*
 * Trucco
 * ------
 *
 * Quando le callback vengono invocate:
 *     memRx[COM_POS_DST]		destinatario
 *     memRx[COM_POS_MIT]		mittente
 *     memRx[COM_POS_CMD]		comando/risposta
 *     memRx + COM_POS_DATI		inizio dati
 *     dimRx					dimensione dati
 */
#define COM_POS_DST     0
#define COM_POS_MIT     1
#define COM_POS_CMD     2
#define COM_POS_DATI    4

#endif
