#ifndef BSP_CAN_H_
#define BSP_CAN_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_Kbit_CAN        1000
#define MAX_Kbit_CANFD      8000

#define MAX_DATI_CAN        8
#define MAX_DATI_CAN_FD     64

// 0 == nessun errore
typedef uint16_t CAN_ERR ;

typedef void * CAN_HANDLE ;

typedef struct {
    bool ext ;
    uint32_t id ;

    size_t dim ;

    bool FDcan ;
    bool ESIpassive ;
} CAN_RX_H ;

typedef void (*PF_CAN_RX)(const CAN_RX_H *, const uint8_t *) ;
typedef void (*PF_CAN_TX)(void) ;
typedef void (*PF_CAN_ERR_S)(bool warn, bool pass, bool busoff) ;
typedef void (*PF_CAN_ERR_P)(bool arb, bool data) ;

#define CAN_UF_DISAB            0
#define CAN_UF_TIPO_RANGE       1   // Da id1 a id2
#define CAN_UF_TIPO_DUAL        2   // id1 o id2 (anche uguali)
#define CAN_UF_TIPO_MASK        3   // id & id2 = id1

typedef struct {
    uint32_t id1 ;
    uint32_t id2 ;

    uint8_t tipo ;

    bool scarta ;
} UN_FILTRO ;

#define MAX_FLT_STD        28
#define MAX_FLT_EST         8

#define CAN_MODO_NORM       0
#define CAN_MODO_BUSMON     1
#define CAN_MODO_INTLBK     2
#define CAN_MODO_EXTLBK     3
#define CAN_MODO_RESTOP     4

typedef struct {
    // Parametri
    bool fd ;

    bool tx_auto ;
    uint8_t modo ;

    // Filtri
    UN_FILTRO fStd[MAX_FLT_STD] ;
    UN_FILTRO fEst[MAX_FLT_EST] ;

    // Callback
    // Ricevuto messaggio
    PF_CAN_RX rx_cb ;
    // Trasmissione conclusa
    PF_CAN_TX tx_cb ;
    // Errori
    PF_CAN_ERR_S stt_cb ;
    PF_CAN_ERR_P err_cb ;
} S_CAN_PRM ;


// Ottiene l'handle dei can
CAN_HANDLE CAN_auto(int /* 1 o 3 */) ;

typedef struct {
    uint32_t Kbs ;
    uint8_t perc ;
    uint8_t sjw ;

    // transmitter delay compensation
    bool tdc ;
    uint8_t tdco ;
} BASE_CAN ;

// Imposta i parametri base
void CAN_base_std(
    CAN_HANDLE ch,
    BASE_CAN * prm) ;
void CAN_base_fd(
    CAN_HANDLE ch,
    BASE_CAN * nmnl,
    BASE_CAN * data) ;

// Gli esperti possono impostare da se

typedef struct {
    uint32_t prescaler ;
    uint32_t bs1 ;
    uint32_t bs2 ;
    uint32_t sjw ;
} BASE_CAN_X ;

void CAN_base_std_x(
    CAN_HANDLE ch,
    BASE_CAN_X * prm) ;
void CAN_base_fd_x(
    CAN_HANDLE ch,
    BASE_CAN_X * nmnl,
    BASE_CAN_X * data) ;
void CAN_leggi_x(
    CAN_HANDLE ch,
    BASE_CAN_X * nmnl,
    BASE_CAN_X * data) ;

// Inizia con questa
CAN_ERR CAN_iniz(
    CAN_HANDLE ch,
    const S_CAN_PRM * cp) ;

// Poi imposta i filtri (se ne hai)
CAN_ERR CAN_filtri(
    CAN_HANDLE ch,
    const S_CAN_PRM * cp) ;

// Se hai filtri, digli cosa fare dei messaggi che sono svicolati (primo bool)
// e dei remote (secondo bool)
// Se veri li scarta
CAN_ERR CAN_glob_filt(
    CAN_HANDLE ch,
    bool rejNoMatch,
    bool rejRem) ;

// Finalmente puoi accendere ...
CAN_ERR CAN_cfg(CAN_HANDLE ch) ;

// ... e abilitare le interruzioni
CAN_ERR CAN_attiva(
    CAN_HANDLE ch,
    const S_CAN_PRM * prm) ;

// Se baudrate CANFD elevato, e' necessaria la compensazione
uint16_t CAN_compensa(
    CAN_HANDLE ch,
    bool tdc,
    uint8_t tdco) ;

typedef struct {
    bool ide ;
    uint32_t id ;

    bool fd ;
    bool brs ;

    uint8_t dim ;
    uint8_t * dati ;
} CAN_DATI ;

bool CAN_tx(
    CAN_HANDLE ch,
    const CAN_DATI * sd) ;

bool CAN_txabile(CAN_HANDLE ch) ;
bool CAN_attivo(CAN_HANDLE ch) ;

void CAN_riavvia(CAN_HANDLE ch) ;

void CAN_canc_tx(CAN_HANDLE ch) ;

// No error occurred since LEC has been reset by successful reception or transmission.
#define CAN_LEC_NO_ERROR        0
// More than 5 equal bits in a sequence have occurred in a part of a received message where this is not allowed.
#define CAN_LEC_STUFF_ERROR     1
// A fixed format part of a received frame has the wrong format.
#define CAN_LEC_FORM_ERROR      2
// The message transmitted by the FDCAN was not acknowledged by another node.
#define CAN_LEC_ACK_ERROR       3
// During the transmission of a message (with the exception of the arbitration field),
// the device wanted to send a recessive level (bit of logical value 1), but the monitored bus value was dominant.
#define CAN_LEC_BIT1_ERROR      4
// During the transmission of a message (or acknowledge bit, or active error flag, or overload flag),
// the device wanted to send a dominant level (data or identifier bit logical value 0),
// but the monitored bus value was recessive.
// During Bus_Off recovery this status is set each time a sequence of 11 recessive
// bits has been monitored. This enables the CPU to monitor the proceeding of the Bus_Off recovery sequence
// (indicating the bus is not stuck at dominant or continuously disturbed).
#define CAN_LEC_BIT0_ERROR      5
// The CRC check sum of a received message was incorrect.
// The CRC of an incoming message does not match with the CRC calculated from the received data.
#define CAN_LEC_CRC_ERROR       6
// Any read access to the Protocol status register re-initializes the LEC to ‘7’.
// When the LEC shows the value ‘7’, no CAN bus event was detected since the last CPU read access
// to the Protocol status register.
#define CAN_LEC_NO_CHANGE       7

// Node is synchronizing on CAN communication
#define CAN_ACT_SYNCHRONIZING           0
// Node is neither receiver nor transmitter.
#define CAN_ACT_IDLE                    1
// Node is operating as receiver
#define CAN_ACT_RECEIVER                2
// Node is operating as transmitter
#define CAN_ACT_TRANSMITTER             3

typedef struct {
    // Transmit error counter
    uint8_t tec ;

    // Receive error counter
    uint8_t rec ;

    // Varie
    uint16_t
        lec : 3,    // Last error code
        ep : 1,     // Error Passive state
        bo : 1,     // Bus Off state
        ew : 1,     // Error Warning
        act : 2,    // Activity
        dlec : 3,   // Data last error code
        nu : 5 ;
} S_CAN_ERR ;

void CAN_err(
    CAN_HANDLE ch,
    S_CAN_ERR * pCE) ;

CAN_ERR CAN_ctrl_rx(
    CAN_HANDLE ch,
    bool abil) ;
CAN_ERR CAN_err_irq_abil(
    CAN_HANDLE ch,
    bool warning,
    bool passive,
    bool busoff,
    bool arbi,
    bool data) ;
CAN_ERR CAN_err_irq_disa(
    CAN_HANDLE ch,
    bool warning,
    bool passive,
    bool busoff,
    bool arbi,
    bool data) ;
CAN_ERR CAN_ctrl_ta(
    CAN_HANDLE ch,
    bool abil) ;
CAN_ERR CAN_modo(
    CAN_HANDLE ch,
    uint8_t modo) ;

CAN_ERR CAN_agg_filtro(
    CAN_HANDLE ch,
    bool std,
    size_t quale,
    S_CAN_PRM * cp) ;
CAN_ERR CAN_dsb_filtri(
    CAN_HANDLE ch,
    bool std,
    S_CAN_PRM * cp) ;

void CAN_disab(CAN_HANDLE ch) ;

#endif
