#define STAMPA_DBG
#include "utili.h"
#include "mmc.h"
#include "hash.h"
#include "stm32h7xx_hal.h"

/*
 * Tentativo di accesso alla rpmb della mmc
 *
 * Vedi: https://community.st.com/s/question/0D53W00002IeEdQSAV
 */

//#define RPMB_TEST		1

extern bool mmc_wait(void) ;
extern bool mmc_cmd23(
    bool reliable,
    bool packed,
    uint32_t num_blocchi) ;
extern bool mmc_leggi(
    uint32_t blocco,
    void * dati) ;
extern bool mmc_scrivi(
    uint32_t blocco,
    const void * dati) ;

extern RNG_HandleTypeDef hrng ;

#define UNO_FISSO       1

#define REQ_AUTH_KEY_PROG       0x0001
#define REQ_READ_WRITE_COUNTER  0x0002
#define REQ_DATA_WRITE          0x0003
#define REQ_DATA_READ           0x0004
#define REQ_RESULT              0x0005

#define RSP_AUTH_KEY_PROG       0x0100
#define RSP_READ_WRITE_COUNTER  0x0200
#define RSP_DATA_WRITE          0x0300
#define RSP_DATA_READ           0x0400

// se alto, wc = 0xFFFFFFFF e non incrementa piu'
#define RES_WC_STATUS_BIT       (1 << 7)

#define RES_OPERATION_OK        0x0000
#define RES_GENERAL_FAILURE     0x0001
#define RES_AUTH_FAILURE        0x0002
#define RES_COUNTER_FAILURE     0x0003
#define RES_ADDRESS_FAILURE     0x0004
#define RES_WRITE_FAILURE       0x0005
#define RES_READ_FAILURE        0x0006
#define RES_AUTH_KEY_NOT_PROG   0x0007
#define RES_ERRORE_PRIMA        0xCA22
#define RES_ERRORE_DOPO         0x5F16

#define DIM_NONCE       16
#define DIM_STUFF       196
#define DIM_DATA        256

typedef struct {
    uint8_t stuff[DIM_STUFF] ;

    // Request (key or MAC) / Response (MAC)
    uint8_t mac[HASH256_DIM] ;
    // Data to be written or read by signed access
    uint8_t data[DIM_DATA] ;
    // Random number generated by the host for the Requests and copied to the Response
    uint32_t nonce[4] ;
    // total amount of the successful authenticated data write requests
    uint32_t w_counter ;
    // Address argument in CMD18 and CMD25 will be ignored
    // In every packet the address is the start address of the full access
    // (not address of the individual block)
    uint16_t address ;
    // Number of blocks (half sectors, 256B) requested to be read/programmed.
    uint16_t block_count ;
    // includes information about the status of the write counter
    uint16_t result ;
    // type of request and response to/from the memory
    uint16_t type ;
} RPMB_DATA_FRAME ;

#define DIM_RSP    (sizeof(RPMB_DATA_FRAME) - DIM_STUFF - HASH256_DIM)

static RPMB_DATA_FRAME rpmb_req, rpmb_rsp ;

static uint8_t kiave[HASH256_DIM] ;

// quando arriva a questo valore, non si puo' piu' scrivere!
#define WC_MAX      0xFFFFFFFF
static uint32_t wc ;

// senza RES_WC_STATUS_BIT
static uint16_t ultimo_result = RES_ERRORE_PRIMA ;

static bool nuovo_nonce(void)
{
    uint32_t * nonce = rpmb_req.nonce ;
    int i = 0 ;
    for ( ; i < 4 ; ++i, nonce++ ) {
        if ( HAL_OK !=
             HAL_RNG_GenerateRandomNumber(&hrng, nonce) ) {
            DBG_ERR ;
            break ;
        }
    }

    return i == 4 ;
}

static bool mac_valido(void)
{
    bool esito = false ;
    HASH_HandleTypeDef hmac = {
        .State = HAL_HASH_STATE_RESET,
        .Init = {
            .DataType = HASH_DATATYPE_8B,
            .pKey = kiave,
            .KeySize = HASH256_DIM
        }
    } ;

    // Butto quello della bsp
    HASH_fine() ;

    do {
        // voglio lo stesso nonce
        if ( 0 != memcmp(rpmb_req.nonce, rpmb_rsp.nonce, DIM_NONCE) ) {
            DBG_ERR ;
            break ;
        }

        uint8_t mac[HASH256_DIM] ;

        // vale il mio
        if ( HAL_OK != HAL_HASH_Init(&hmac) ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_OK !=
             HAL_HMACEx_SHA256_Start(&hmac, rpmb_rsp.data, DIM_RSP,
                                     mac, 10000) ) {
            DBG_ERR ;
            break ;
        }

        // voglio lo stesso mac
        if ( 0 != memcmp(mac, rpmb_rsp.mac, HASH256_DIM) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    // butto il mio
    HAL_HASH_DeInit(&hmac) ;

    // ripristino quello della bsp
    CONTROLLA( HASH_iniz() ) ;

    return esito ;
}

static bool mac_crea(void)
{
    bool esito = false ;
    HASH_HandleTypeDef hmac = {
        .State = HAL_HASH_STATE_RESET,
        .Init = {
            .DataType = HASH_DATATYPE_8B,
            .pKey = kiave,
            .KeySize = HASH256_DIM
        }
    } ;

    // Butto quello della bsp
    HASH_fine() ;

    do {
        // vale il mio
        if ( HAL_OK != HAL_HASH_Init(&hmac) ) {
            DBG_ERR ;
            break ;
        }

        if ( HAL_OK !=
             HAL_HMACEx_SHA256_Start(&hmac, rpmb_req.data, DIM_RSP,
                                     rpmb_req.mac, 10000) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    // butto il mio
    HAL_HASH_DeInit(&hmac) ;

    // ripristino quello della bsp
    CONTROLLA( HASH_iniz() ) ;

    return esito ;
}

static bool read_result_register(void)
{
    bool esito = false ;

    do {
        // the block count is set to 1 by CMD23
        if ( !mmc_cmd23(false, false, UNO_FISSO) ) {
            DBG_ERR ;
            break ;
        }

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;
        rpmb_req.type = __REV16(REQ_RESULT) ;

        // The result read sequence is initiated by Write Multiple Block command
        if ( !mmc_scrivi(0, &rpmb_req) ) {
            DBG_ERR ;
            break ;
        }

        // the block count is set to 1 by CMD23
        if ( !mmc_cmd23(false, false, UNO_FISSO) ) {
            DBG_ERR ;
            break ;
        }

        // The result itself is read out with the Read Multiple Block command
        if ( !mmc_leggi(0, &rpmb_rsp) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool programming_of_the_authentication_key(void)
{
    bool esito = false ;

    DBG_FUN ;

    do {
        // segnalo errore di altro tipo
        ultimo_result = RES_ERRORE_PRIMA ;

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;

        rpmb_req.type = __REV16(REQ_AUTH_KEY_PROG) ;
        memcpy( rpmb_req.mac, kiave, sizeof(kiave) ) ;

        // the block count is set to 1 by CMD23, with ... Reliable Write type of programming
        if ( !mmc_cmd23(true, false, UNO_FISSO) ) {
            DBG_ERR ;
            break ;
        }

        // The Authentication Key is programmed with the Write Multiple Block command, CMD25
        if ( !mmc_scrivi(0, &rpmb_req) ) {
            DBG_ERR ;
            break ;
        }

        // The busy signaling in the Dat0 line after the CRC status by the eMMC
        // is indicating programming busy of the key
        // The status can also be polled with CMD13
        if ( !mmc_wait() ) {
            DBG_ERR ;
            break ;
        }

        // The successfulness of the programming of the key should be checked
        // by reading the result register of the Replay Protected Memory Block
        // ----------------------------------------------
        if ( !read_result_register() ) {
            DBG_ERR ;
            break ;
        }

        if ( RSP_AUTH_KEY_PROG != __REV16(rpmb_rsp.type) ) {
            DBG_ERR ;
            break ;
        }

        // inutile: mai scritta
        ultimo_result = __REV16(rpmb_rsp.result) ;
        if ( RES_WC_STATUS_BIT & ultimo_result ) {
            wc = WC_MAX ;
            ultimo_result &= NEGA(RES_WC_STATUS_BIT) ;
        }

        switch ( ultimo_result ) {
        case RES_OPERATION_OK:
            DBG_PUTS("OPERATION_OK") ;
            esito = true ;
            break ;
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case RES_GENERAL_FAILURE:
            DBG_PUTS("GENERAL_FAILURE") ;
            break ;
        case RES_AUTH_FAILURE:
            DBG_PUTS("AUTH_FAILURE") ;
            break ;
        case RES_COUNTER_FAILURE:
            DBG_PUTS("COUNTER_FAILURE") ;
            break ;
        case RES_ADDRESS_FAILURE:
            DBG_PUTS("ADDRESS_FAILURE") ;
            break ;
        case RES_WRITE_FAILURE:
            DBG_PUTS("WRITE_FAILURE") ;
            break ;
        case RES_READ_FAILURE:
            DBG_PUTS("READ_FAILURE") ;
            break ;
        case RES_AUTH_KEY_NOT_PROG:
            DBG_PUTS("AUTH_KEY_NOT_PROG") ;
            break ;
        default:
            DBG_ERR ;
            break ;
        }
    } while ( false ) ;

    return esito ;
}

static bool x_read_sequence(
    uint16_t type,
    uint16_t numb)
{
    bool esito = false ;

    do {
        if ( !nuovo_nonce() ) {
            DBG_ERR ;
            break ;
        }

        rpmb_req.type = __REV16(type) ;

        // the block count is set to 1 by CMD23
        if ( !mmc_cmd23(false, false, UNO_FISSO) ) {
            DBG_ERR ;
            break ;
        }

        // The [counter | Data read sequence] is initiated by Write Multiple Block command, CMD25
        if ( !mmc_scrivi(0, &rpmb_req) ) {
            DBG_ERR ;
            break ;
        }

        // Prior to the read command, the block count is set by CMD23
        if ( !mmc_cmd23(false, false, numb) ) {
            DBG_ERR ;
            break ;
        }

        // The data itself is read out with the Read Multiple Block command, CMD18
        if ( !mmc_leggi(0, &rpmb_rsp) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool reading_of_the_counter_value(void)
{
    bool esito = false ;

    DBG_FUN ;

    do {
        // segnalo errore di altro tipo
        ultimo_result = RES_ERRORE_PRIMA ;

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;

        if ( !x_read_sequence(REQ_READ_WRITE_COUNTER, UNO_FISSO) ) {
            DBG_ERR ;
            break ;
        }

        if ( RSP_READ_WRITE_COUNTER != __REV16(rpmb_rsp.type) ) {
            DBG_ERR ;
            break ;
        }

        ultimo_result = __REV16(rpmb_rsp.result) ;
        if ( RES_WC_STATUS_BIT & ultimo_result ) {
            wc = WC_MAX ;
            ultimo_result &= NEGA(RES_WC_STATUS_BIT) ;
        }

        switch ( ultimo_result ) {
        case RES_OPERATION_OK:
            DBG_PUTS("OPERATION_OK") ;
            esito = mac_valido() ;
            if ( esito ) {
                wc = __REV(rpmb_rsp.w_counter) ;
                DBG_PRINTF("write counter = %u", wc) ;
            }
            break ;
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case RES_GENERAL_FAILURE:
            DBG_PUTS("GENERAL_FAILURE") ;
            break ;
        case RES_AUTH_FAILURE:
            DBG_PUTS("AUTH_FAILURE") ;
            break ;
        case RES_COUNTER_FAILURE:
            DBG_PUTS("COUNTER_FAILURE") ;
            break ;
        case RES_ADDRESS_FAILURE:
            DBG_PUTS("ADDRESS_FAILURE") ;
            break ;
        case RES_WRITE_FAILURE:
            DBG_PUTS("WRITE_FAILURE") ;
            break ;
        case RES_READ_FAILURE:
            DBG_PUTS("READ_FAILURE") ;
            break ;
        case RES_AUTH_KEY_NOT_PROG:
            DBG_PUTS("AUTH_KEY_NOT_PROG") ;
            esito = true ;
            wc = 0 ;
            break ;
        default:
            DBG_ERR ;
            break ;
        }
    } while ( false ) ;

    return esito ;
}

// Queste funzioni accedono a un solo blocco
// max 2 (cfr REL_WR_SEC_C)
static const uint16_t BLOCK_COUNT = 1 ;

bool rpmb_leggi(
    uint16_t blocco,
    uint16_t numb,
    void * dati)
{
    bool esito = false ;

    DBG_PRINTF("%s(%u)", __func__, (unsigned) blocco) ;

    do {
        // segnalo errore di altro tipo
        ultimo_result = RES_ERRORE_PRIMA ;

        if ( numb != BLOCK_COUNT ) {
            DBG_ERR ;
            break ;
        }

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;

        rpmb_req.address = __REV16(blocco) ;

        if ( !x_read_sequence(REQ_DATA_READ, numb) ) {
            DBG_ERR ;
            break ;
        }

        ultimo_result = __REV16(rpmb_rsp.result) ;
        if ( RES_WC_STATUS_BIT & ultimo_result ) {
            wc = WC_MAX ;
            ultimo_result &= NEGA(RES_WC_STATUS_BIT) ;
        }

        switch ( ultimo_result ) {
        // it first checks the address
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case RES_ADDRESS_FAILURE:
            break ;
        case RES_AUTH_FAILURE:
            break ;
        case RES_COUNTER_FAILURE:
            break ;
        case RES_WRITE_FAILURE:
            break ;
        case RES_GENERAL_FAILURE:
            break ;
        case RES_OPERATION_OK:
            do {
                ultimo_result = RES_ERRORE_DOPO ;

                if ( RSP_DATA_READ != __REV16(rpmb_rsp.type) ) {
                    DBG_ERR ;
                    break ;
                }

                if ( __REV16(rpmb_rsp.address) != blocco ) {
                    DBG_ERR ;
                    break ;
                }

                if ( __REV16(rpmb_rsp.block_count) != numb ) {
                    DBG_ERR ;
                    break ;
                }

                esito = mac_valido() ;
                if ( !esito ) {
                    DBG_ERR ;
                    break ;
                }

                ultimo_result = RES_OPERATION_OK ;
                memcpy(dati, rpmb_rsp.data, DIM_DATA) ;
            } while ( false ) ;
            break ;
        case RES_AUTH_KEY_NOT_PROG:
            wc = 0 ;
            break ;
        case RES_READ_FAILURE:
            break ;
        default:
            DBG_ERR ;
            break ;
        }
    } while ( false ) ;

    return esito ;
}

bool rpmb_scrivi(
    uint16_t blocco,
    uint16_t numb,
    const void * dati)
{
    bool esito = false ;

    DBG_PRINTF("%s(%u)", __func__, (unsigned) blocco) ;

    do {
        // segnalo errore di altro tipo
        ultimo_result = RES_ERRORE_PRIMA ;

        if ( numb != BLOCK_COUNT ) {
            DBG_ERR ;
            break ;
        }

        memset( &rpmb_req, 0, sizeof(RPMB_DATA_FRAME) ) ;

        rpmb_req.type = __REV16(REQ_DATA_WRITE) ;
        rpmb_req.w_counter = __REV(wc) ;
        rpmb_req.block_count = __REV16(numb) ;
        rpmb_req.address = __REV16(blocco) ;
        memcpy(rpmb_req.data, dati, DIM_DATA) ;

        if ( !mac_crea() ) {
            DBG_ERR ;
            break ;
        }

        // the block count is set by CMD23, with ... Reliable Write type of programming
        if ( !mmc_cmd23(true, false, numb) ) {
            DBG_ERR ;
            break ;
        }

        // Data to the Replay Protected Memory Block is programmed with the Write Multiple Block command, CMD25
        if ( !mmc_scrivi(0, &rpmb_req) ) {
            DBG_ERR ;
            break ;
        }

        // The busy signaling in the Dat0 line after the CRC status by the eMMC
        // is indicating buffer busy between the sent blocks (in multiple block
        // write case) and programming busy of the key after the last block (or
        // in single block case).
        // The status can also be polled with CMD13
        if ( !mmc_wait() ) {
            DBG_ERR ;
            break ;
        }

        // The successfulness of the programming of the data should be checked
        // by the host by reading the result register of the Replay Protected Memory Block
        // ----------------------------------------------
        if ( !read_result_register() ) {
            DBG_ERR ;
            break ;
        }

        ultimo_result = __REV16(rpmb_rsp.result) ;
        // it first checks whether the write counter has expired
        if ( RES_WC_STATUS_BIT & ultimo_result ) {
            // If the write counter is expired then eMMC sets the result to 0x85
            // (write failure, write counter expired).
            // No data is written to the eMMC
            wc = WC_MAX ;
            ultimo_result &= NEGA(RES_WC_STATUS_BIT) ;
            DBG_ERR ;
            break ;
        }

        // NOLINTNEXTLINE(bugprone-branch-clone)
        switch ( ultimo_result ) {
        // Next the address is checked ...
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case RES_ADDRESS_FAILURE:
            break ;
        // ...  the two MAC’s are different
        case RES_AUTH_FAILURE:
            break ;
        // ... then the eMMC compares the write counter
        case RES_COUNTER_FAILURE:
            break ;
        // The data from the request are written to the address indicated in the request and the write counter is incremented by 1
        // If write fails then returned result is ...
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case RES_WRITE_FAILURE:
            wc++ ;
            break ;
        // If some other error occurs during the write
        case RES_GENERAL_FAILURE:
            wc++ ;
            break ;
        case RES_OPERATION_OK:
            do {
                ultimo_result = RES_ERRORE_DOPO ;

                if ( RSP_DATA_WRITE != __REV16(rpmb_rsp.type) ) {
                    DBG_ERR ;
                    break ;
                }

                if ( __REV16(rpmb_rsp.address) != blocco ) {
                    DBG_ERR ;
                    break ;
                }

                esito = mac_valido() ;
                if ( !esito ) {
                    DBG_ERR ;
                    break ;
                }

                ultimo_result = RES_OPERATION_OK ;
                wc = __REV(rpmb_rsp.w_counter) ;
                DBG_PRINTF("write counter = %u", wc) ;
            } while ( false ) ;
            break ;
        // ???
        case RES_AUTH_KEY_NOT_PROG:
            wc = 0 ;
            break ;
        case RES_READ_FAILURE:
            break ;
        default:
            DBG_ERR ;
            break ;
        }
    } while ( false ) ;

    return esito ;
}

#ifdef RPMB_TEST
static void stampa_ures(void)
{
    switch ( ultimo_result ) {
    case RES_OPERATION_OK:
        DBG_PUTS("\t OPERATION_OK") ;
        break ;
    case RES_GENERAL_FAILURE:
        DBG_PUTS("\t GENERAL_FAILURE") ;
        break ;
    case RES_AUTH_FAILURE:
        DBG_PUTS("\t AUTH_FAILURE") ;
        break ;
    case RES_COUNTER_FAILURE:
        DBG_PUTS("\t COUNTER_FAILURE") ;
        break ;
    case RES_ADDRESS_FAILURE:
        DBG_PUTS("\t ADDRESS_FAILURE") ;
        break ;
    case RES_WRITE_FAILURE:
        DBG_PUTS("\t WRITE_FAILURE") ;
        break ;
    case RES_READ_FAILURE:
        DBG_PUTS("\t READ_FAILURE") ;
        break ;
    case RES_AUTH_KEY_NOT_PROG:
        DBG_PUTS("\t AUTH_KEY_NOT_PROG") ;
        break ;
    case RES_ERRORE_PRIMA:
        DBG_PUTS("\t ERRORE_PRIMA") ;
        break ;
    case RES_ERRORE_DOPO:
        DBG_PUTS("\t ERRORE_DOPO") ;
        break ;
    default:
        DBG_PUTS("\t ???") ;
        break ;
    }
}
#endif

void rpmb_iniz(void)
{
    uint8_t tmp[HASH256_DIM] ;
    uint32_t uid[3] = {
        HAL_GetUIDw1(),
        HAL_GetUIDw2(),
        HAL_GetUIDw0()
    } ;

    static_assert(512 == sizeof(RPMB_DATA_FRAME), "OKKIO") ;

    // calcolo la chiave da uid (micro) e cid (mmc)
    // --------------------------------------------
    S_MMC_CID cid ;
    MMC_cid(&cid) ;

    // Primo giro
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if ( !HASH_acc_256( uid, sizeof(uid) ) ) {
        DBG_ERR ;
    }
    else if ( !HASH_end_256(&cid, sizeof(cid), tmp) ) {
        DBG_ERR ;
    }
    // Secondo giro
    else if ( !HASH_cal_256(tmp, HASH256_DIM, kiave) ) {
        DBG_ERR ;
    }
    else {
        // Ok
    }

    // mi serve wc
    // --------------------------------------------
    CONTROLLA( MMC_access(MMC_PART_RPMB) ) ;

    CONTROLLA( reading_of_the_counter_value() ) ;

    if ( RES_AUTH_KEY_NOT_PROG == ultimo_result ) {
        CONTROLLA( programming_of_the_authentication_key() ) ;
    }
#ifdef RPMB_TEST
    //++++++ inizio test
    const uint32_t RPMB_DIM_B = 512 * 1024 ;
    const uint32_t RPMB_DIM_S = RPMB_DIM_B / DIM_DATA ;

    uint8_t settore[DIM_DATA] ;

    // questo fallisce
    if ( rpmb_leggi(RPMB_DIM_S, 1, settore) ) {
        DBG_ERR ;
    }
    stampa_ures() ;

    // questo fallisce
    kiave[0] ^= kiave[1] ;
    if ( rpmb_leggi(RPMB_DIM_S - 1, 1, settore) ) {
        DBG_ERR ;
    }
    kiave[0] ^= kiave[1] ;
    stampa_ures() ;

    // questo riesce
    if ( rpmb_leggi(RPMB_DIM_S - 1, 1, settore) ) {
        DBG_PRINT_HEX("letto", settore, DIM_DATA) ;
    }
    stampa_ures() ;

    for ( int i = 0 ; i < DIM_DATA ; i++ ) {
        settore[i]++ ;
    }

    // questo fallisce
    if ( rpmb_scrivi(RPMB_DIM_S, 1, settore) ) {
        DBG_ERR ;
    }
    stampa_ures() ;

    // questo fallisce
    wc-- ;
    if ( rpmb_scrivi(RPMB_DIM_S - 1, 1, settore) ) {
        DBG_ERR ;
    }
    stampa_ures() ;
    wc++ ;

    // questo fallisce
    kiave[0] ^= kiave[1] ;
    if ( rpmb_scrivi(RPMB_DIM_S - 1, 1, settore) ) {
        DBG_ERR ;
    }
    kiave[0] ^= kiave[1] ;
    stampa_ures() ;

    // questo riesce
    if ( rpmb_scrivi(RPMB_DIM_S - 1, 1, settore) ) {
        DBG_PRINT_HEX("scritto", settore, DIM_DATA) ;
    }
    stampa_ures() ;

    //------ fine test
#endif
    CONTROLLA( MMC_access(MMC_PART_USER) ) ;
}
