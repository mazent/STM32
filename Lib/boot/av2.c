#define STAMPA_DBG
#include "utili.h"

/*
 * Uno degli header di Badanai
 *
 * Sequenza:
 *     headerFW
 *     vettore interruzioni
 *     ...
 */

#define HEADER_FILE_SIGNATURE           0x5A5AA5A5
//#define HEADER_FILE_VERSION_SKIP_AREA   2 //
//#define HEADER_FILE_VERSION		        2 // 1 -> 2 add skip area
//#define HEADER_FILE_DETAILS_SIZE        32
//#define SKIP_AREA_ADDRESS_IS_INVALID    0

/**
 * @brief This structure describes the common part of the header of every file
 * that the host can send to the VCI.
 * @warning the field @p headerIsIncluded specifies if the header
 * is included in the file.
 * if headerIsIncluded is 0 the header is only a meta-data structure
 */
typedef struct __headerFileCmn {
    /**
     * @brief checksum of all next bytes of this structure
     *
     * @error NO: e' il csum di headerFileCmn + infoFwDetails
     */
    uint32_t headerCks ;

    /**
     * @brief signature of the header.
     * This record must be @p HEADER_FILE_SIGNATURE
     */
    uint32_t headerSignature ;

    /**
     * @brief version of the header.
     */
    uint32_t headerVersion ;

    /**
     * @brief    0 ==> header is not included in the File
     *        != 0 ==> header is included in the file
     */
    uint32_t headerIsIncluded ;

    /**
     * @brief begin address where VCI must store the file
     */
    uint32_t fileAddressStart ;

    /**
     * @brief this record must be ignored if @p headerIsIncluded is false.
     * if @p headerIsIncluded is true, this record is the begin address of the header
     * Typically, @p fileHeaderAddressStart == fileAddressStart but in the uLoader context
     * this condition is false!
     */
    uint32_t fileHeaderAddressStart ;

    /**
     * @brief crc of the file (crc 1021) without the contribution of the header and the last @p paddingBytes
     *
     */
    uint16_t fileCrc ;

    uint16_t reserved1 ;

    /**
     * @brief size of the file
     */
    uint32_t fileSize ;

    /**
     * @brief size of the file
     */
    uint32_t paddingBytes ;

    uint32_t skipArea_address ;

    uint16_t skipArea_size ;

    uint8_t reserved2[2] ;
} headerFileCmn ;

#define INFO_FW_SIGNATURE   0x55AA55AA
//#define INFO_FW_VERSION		2 // from version 1 -> version 2: new field called intVectTableAddressStart
//
//// PRODUCT NAME
#define INFO_SIZE_PRODUCT_NAME_STRING   (15 + 1)
//#define INFO_FW_TXB2_PRODUCT_NAME     "TXB2"
//
//// CUSTOMER NAME
//#define INFO_SIZE_CUSTOMER_NAME_STRING    CUSTOMER_SIZE_NAME_STRING
//#define INFO_FW_CUSTOMER_NAME_TEXA        CUSTOMER_NAME_TEXA
//
//// APP NAME
#define INFO_SIZE_APP_NANE_STRING       (3 + 1)
//#define INFO_FW_APP_NAME_DEF			"DEF" //"TST" per mainapp di test su server sdk-test
//#define INFO_FW_APP_NAME_TEST1			"EMC"
//#define INFO_FW_APP_NAME_EMC			INFO_FW_APP_NAME_TEST1
//#define INFO_FW_APP_NAME_TEST2			"SME"
//#define INFO_FW_APP_NAME_SIM_ECU		INFO_FW_APP_NAME_TEST2
//#define INFO_FW_APP_NAME_BT_TEST        "BTT"
//#define INFO_FW_APP_NAME_BT_CERT        "BTC"
//#define INFO_FW_APP_NAME_CRASH          "CRS"
//#define INFO_FW_APP_NAME_PROD_TEST      "PRD"
//
//
//// BOARD NAME
#define BOARD_SIZE_NAME_STRING (3 + 1)
//#define INFO_SIZE_BOARD_NANE_STRING     BOARD_SIZE_NAME_STRING
//#define INFO_FW_BOARD_NAME_763			BOARD_NAME_SC763  // TXB2

#define INFO_FW_VERSION_SIZE            4

typedef struct __infoFwDetails {
    /**
     * @brief signature of the structure
     * must be @p INFO_FW_SIGNATURE
     */
    uint32_t structure_signature ;

    /**
     * @brief version of the structure
     * must be @p INFO_FW_VERSION
     */
    uint32_t structure_version ;

    /**
     * @brief version of the firmware
     * index 0 ==> MAIN version
     * index 1 ==> SUB MAIN version
     * index 2 ==> BUILD version
     * index 3 ==> BETA version
     *
     */
    uint8_t fwVersion[INFO_FW_VERSION_SIZE] ;

    uint32_t compileTime ;

    /**
     * @brief customer name @see INFO_FW_PRODUCT_NAME*
     */
    uint8_t productNameString[INFO_SIZE_PRODUCT_NAME_STRING] ;

    /**
     * @brief customer name @see INFO_FW_CUSTOMER_NAME*
     */
    uint8_t customerNameString[INFO_SIZE_PRODUCT_NAME_STRING] ;

    /**
     * @brief application name @see INFO_FW_APP_NAME*
     */
    uint8_t appNameString[INFO_SIZE_APP_NANE_STRING] ;

    /**
     * @brief board name @see INFO_FW_BOARD_NAME*
     */
    uint8_t boardNameString[BOARD_SIZE_NAME_STRING] ;

    /**
     * @brief @see APP_TYPE in appType.h
     */
    uint32_t appType ;

    uint32_t intVectTableAddressStart ;

    uint8_t _free[64] ;
} infoFwDetails ;

typedef struct __headerFW {
    headerFileCmn hfc ;

    infoFwDetails ifd ;

    //uint8_t mancano[256 - 172] ;
} headerFW ;

static bool hfc_csum(const headerFileCmn * pH)
{
    uint8_t cs = 0 ;
    const uint8_t * decossa = (const uint8_t *) &pH->headerSignature ;
    // Il commento e' sbagliato!
    // NON mettete commenti: https://www.youtube.com/watch?v=Bf7vDBBOBUA&ab_channel=CodeAesthetic
    const size_t DIM = sizeof(headerFW) - sizeof(uint32_t) ;
    for ( size_t i = 0 ; i < DIM ; i++ ) {
        cs += decossa[i] ;
    }

    union {
        uint32_t x ;
        uint8_t cs ;
    } u = {
        .x = pH->headerCks
    } ;

    return cs == u.cs ;
}

static bool crc_valido(const headerFW * pH)
{
    bool esito = false ;

    INUTILE(pH);
#if 1
#warning DA FARE
#else
#warning OKKIO
    esito=true;
#endif

    return esito ;
}

/**
 *
 * @param dove[in] indirizzo da controllare
 * @param vi[out]  vettore delle interruzioni
 * @return		   torna true se all'indirizzo indicato c'e' una
 *                 immagine valida (*vi contiene l'indirizzo del vettore
 *                 delle interruzioni)
 */

bool app_valida(
    uint32_t dove,
    uint32_t * vi)
{
    bool esito = false ;

//    DBG_PRINTF( "sizeof(headerFileCmn) %d", sizeof(headerFileCmn) ) ;
//    DBG_PRINTF( "sizeof(infoFwDetails) %d", sizeof(infoFwDetails) ) ;
//    DBG_PRINTF( "sizeof(headerFW) %d", sizeof(headerFW) ) ;

    DBG_PRINTF("%s(%08X)", __func__, dove) ;

    do {
        const headerFW * pH = CPOINTER(dove) ;
        if ( pH->hfc.headerSignature != HEADER_FILE_SIGNATURE ) {
            DBG_ERR ;
            break ;
        }

        if ( !hfc_csum(&pH->hfc) ) {
            DBG_ERR ;
            break ;
        }

        if ( !crc_valido(pH) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
        *vi = pH->ifd.intVectTableAddressStart ;
    } while ( false ) ;

    return esito ;
}

#if 0
void app_iniz(void)
{
    uint32_t vi ;

    // settore 2: falso
    uint32_t dove = 0x8040000 ;
    DBG_PRINTF("%08X: %s / FALSO", dove, app_valida(dove,
                                                    &vi) ? "VERO" : "FALSO") ;

    // settore 1: vero
    dove = 0x8020000 ;
    DBG_PRINTF("%08X: %s / VERO", dove, app_valida(dove,
                                                   &vi) ? "VERO" : "FALSO") ;

    BPOINT ;
}

#endif
