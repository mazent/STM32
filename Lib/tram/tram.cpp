#define STAMPA_DBG
#include "tram.h"
#include <cassert>

#ifdef __cplusplus
#   define NEGA(x)             ( ~static_cast<unsigned int>(x) )
#else
#   define NEGA(x)             ( ~(unsigned int) (x) )
#endif

/*
 * Test della ram
 *
 * Con data walking 1 e 0 si verifica completamente il bus dati
 *
 * Con address walking 1 e 0 si identificano i problemi di linee fisse alte/basse
 *
 * L'accesso e' un test aggiuntivo
 *
 * Cfr https://barrgroup.com/Embedded-Systems/How-To/Memory-Test-Suite-C
 */

static const uint32_t RIF = 0x55555555 ;
static const uint32_t VAL = 0xAAAAAAAA ;

static void * casta(uint32_t questo)
{
    union {
        void * v ;
        uint32_t u ;
    } u ;

    u.u = questo ;

    return u.v ;
}

template <class TIPO>
static uint32_t DataWalk1(uint32_t Base)
{
    const int NUM_BIT = 8 * sizeof(TIPO) ;
    volatile TIPO * ram = static_cast<volatile TIPO *>( casta(Base) ) ;
    TIPO uno = 1 ;
    int i ;

    // Ciclo sui dati
    for ( i = 0 ; i < NUM_BIT ; i++ ) {
        ram[0] = uno ;

        if ( uno != ram[0] ) {
            break ;
        }

        uno <<= 1 ;
    }

    if ( NUM_BIT == i ) {
        uno = 0 ;
    }

    return uno ;
}

template <class TIPO>
static uint32_t DataWalk0(uint32_t Base)
{
    const int NUM_BIT = 8 * sizeof(TIPO) ;
    volatile TIPO * ram = static_cast<volatile TIPO *>( casta(Base) ) ;
    TIPO uno = 1 ;
    int i ;

    // Ciclo sui dati
    for ( i = 0 ; i < NUM_BIT ; i++ ) {
        TIPO zero = NEGA(uno) ;

        ram[0] = zero ;

        if ( zero != ram[0] ) {
            break ;
        }

        uno <<= 1 ;
    }

    if ( NUM_BIT == i ) {
        uno = 0 ;
    }

    return uno ;
}

template <class TIPO>
static uint32_t AddrWalk1(
    uint32_t Base,
    uint32_t numByte)
{
    volatile TIPO * ram = static_cast<volatile TIPO *>( casta(Base) ) ;
    uint32_t uno = 1 ;

    numByte /= sizeof(TIPO) ;

    // Segnalino
    ram[0] = static_cast<TIPO>(RIF) ;

    // Ciclo sugli indirizzi
    while ( uno != numByte ) {
        ram[uno] = static_cast<TIPO>(VAL) ;

        // In questo modo mi fermo al primo errore
        if ( static_cast<TIPO>(RIF) != ram[0] ) {
            break ;
        }

        uno <<= 1 ;
    }

    if ( numByte == uno ) {
        uno = 0 ;
    }

    return uno ;
}

template <class TIPO>
static uint32_t AddrWalk0(
    uint32_t Base,
    uint32_t numByte)
{
    volatile TIPO * ram = static_cast<volatile TIPO *>( casta(Base) ) ;
    uint32_t uno = 1 ;

    numByte /= sizeof(TIPO) ;

    // Segnalino
    ram[numByte - 1] = static_cast<TIPO>(RIF) ;

    // Ciclo sugli indirizzi
    while ( uno != numByte ) {
        uint32_t zero = NEGA(uno) ;

        zero &= numByte - 1 ;

        ram[zero] = static_cast<TIPO>(VAL) ;

        if ( static_cast<TIPO>(RIF) != ram[numByte - 1] ) {
            break ;
        }

        uno <<= 1 ;
    }

    if ( numByte == uno ) {
        uno = 0 ;
    }

    return uno ;
}

template <class TIPO>
static bool accedi(
    volatile TIPO * ram,
    uint32_t sposta)
{
    const uint32_t DIM = ( 1 << 8 * sizeof(TIPO) ) / sizeof(TIPO) ;
    uint32_t i, pos ;

    do {
        // Scrivo
        TIPO val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            pos = i << sposta ;
            ram[pos] = val ;
        }

        // Verifico
        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            pos = i << sposta ;
            if ( ram[pos] != val ) {
                break ;
            }
        }

        // E se fossi sfortunato?
        // Scrivo un valore diverso
        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            TIPO nval = NEGA(val) ;
            pos = i << sposta ;
            ram[pos] = nval ;
        }

        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            TIPO nval = NEGA(val) ;
            pos = i << sposta ;
            if ( ram[pos] != nval ) {
                break ;
            }
        }
    } while ( false ) ;

    return i == DIM ;
}

/*
 * Evito di scrivere tutto il dispositivo eseguendo
 * il test a finestre di dimensione pari al bus dati
 *
 * Primo passo           <-------------> dimensione bus dati
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | | | | | | | | | |   finestra  |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Secondo passo
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | | | | | | | | |   finestra  | |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Terzo passo
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | | | | | | | |   finestra  | | |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Ultimo passo
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |   finestra  | | | | | | | | | |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

template <class TIPO>
static bool Accesso(
    uint32_t Base,
    uint32_t numByte)
{
    volatile TIPO * ram = static_cast<volatile TIPO *>( casta(Base) ) ;
    bool esito = true ;
    uint32_t sposta = 0 ;
    uint32_t finestra = 1 << sposta ;
    const uint32_t LIMITE = numByte >> ( 8 * sizeof(TIPO) ) ;

    while ( finestra <= LIMITE ) {
        if ( !accedi(ram, sposta) ) {
            esito = false ;
            break ;
        }

        // Prossimo indirizzo
        ++sposta ;
        finestra <<= 1 ;
    }

    return esito ;
}

// In questo caso il bus dati copre tutto (il dispositivo viene scritto completamente)

static bool Accesso32(
    uint32_t Base,
    uint32_t numByte)
{
    volatile uint32_t * ram = static_cast<volatile uint32_t *>( casta(Base) ) ;
    const uint32_t DIM = numByte / sizeof(uint32_t) ;
    uint32_t i ;

    do {
        // Scrivo
        uint32_t val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            ram[i] = val ;
        }

        // Verifico
        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            if ( ram[i] != val ) {
                break ;
            }
        }

        // E se fossi sfortunato?
        // Scrivo un valore diverso
        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            ram[i] = NEGA(val) ;
        }

        val = 1 ;
        for ( i = 0 ; i < DIM ; ++i, ++val ) {
            if ( ram[i] != NEGA(val) ) {
                break ;
            }
        }
    } while ( false ) ;

    return i == DIM ;
}

extern "C"
uint32_t TRAM_DataWalk1(
    uint32_t Base,
    RAM_DATA_BUS rdb)
{
    switch ( rdb ) {
    case RDB_8_BIT:
        return DataWalk1<uint8_t>(Base) ;
    case RDB_16_BIT:
        return DataWalk1<uint16_t>(Base) ;
    case RDB_32_BIT:
        return DataWalk1<uint32_t>(Base) ;
    default:
        assert(false) ;
        return NEGA(0) ;
    }
}

extern "C"
uint32_t TRAM_DataWalk0(
    uint32_t Base,
    RAM_DATA_BUS rdb)
{
    switch ( rdb ) {
    case RDB_8_BIT:
        return DataWalk0<uint8_t>(Base) ;
    case RDB_16_BIT:
        return DataWalk0<uint16_t>(Base) ;
    case RDB_32_BIT:
        return DataWalk0<uint32_t>(Base) ;
    default:
        assert(false) ;
        return NEGA(0) ;
    }
}

extern "C"
uint32_t TRAM_AddrWalk1(
    uint32_t Base,
    uint32_t numByte,
    RAM_DATA_BUS rdb)
{
    switch ( rdb ) {
    case RDB_8_BIT:
        return AddrWalk1<uint8_t>(Base, numByte) ;
    case RDB_16_BIT:
        return AddrWalk1<uint16_t>(Base, numByte) ;
    case RDB_32_BIT:
        return AddrWalk1<uint32_t>(Base, numByte) ;
    default:
        assert(false) ;
        return NEGA(0) ;
    }
}

extern "C"
uint32_t TRAM_AddrWalk0(
    uint32_t Base,
    uint32_t numByte,
    RAM_DATA_BUS rdb)
{
    switch ( rdb ) {
    case RDB_8_BIT:
        return AddrWalk0<uint8_t>(Base, numByte) ;
    case RDB_16_BIT:
        return AddrWalk0<uint16_t>(Base, numByte) ;
    case RDB_32_BIT:
        return AddrWalk0<uint32_t>(Base, numByte) ;
    default:
        assert(false) ;
        return NEGA(0) ;
    }
}

extern "C"
bool TRAM_accedi(
    uint32_t Base,
    uint32_t numByte,
    RAM_DATA_BUS rdb)
{
    switch ( rdb ) {
    case RDB_8_BIT:
        return Accesso<uint8_t>(Base, numByte) ;
    case RDB_16_BIT:
        return Accesso<uint16_t>(Base, numByte) ;
    case RDB_32_BIT:
        return Accesso32(Base, numByte) ;
    default:
        assert(false) ;
        return false ;
    }
}
