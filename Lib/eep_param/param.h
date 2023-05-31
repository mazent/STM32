#ifndef APP_PARAM_H_
#define APP_PARAM_H_

/*
 * Salvataggio di parametri in eeprom (interfaccia in eeprom.h)
 *
 * Usa:
 *     HASH: per generare la chiave
 *     RNG:  per IV
 *     CRYP: per AES
 */

// Poco meno di EEP_DIM_PAGINA
#define DIM_DATI       112

// Legge una pagina (da 0 a EEP_NUM_PAGINE-1)
// Torna NULL se errore
// La memoria viene da una zona unica: copiatela prima di invocare di nuovo
const void * prm_leggi(uint16_t) ;

// Oppure passate la zona dove copiarla
static inline bool prm_copia(
    uint16_t pag,
    void * v,
    uint8_t dim)
{
    const void * dati = prm_leggi(pag) ;
    if ( NULL == dati ) {
        return false ;
    }

    memcpy_(v, dati, dim) ;
    return true ;
}

// Scrive una pagina (da 0 a EEP_NUM_PAGINE-1)
bool prm_scrivi(uint16_t, const void *, uint8_t) ;

#else
#   warning param.h incluso
#endif
