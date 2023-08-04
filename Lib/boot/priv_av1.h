#ifndef LIB_BOOT_FWH_H_
#define LIB_BOOT_FWH_H_

/*
 * Uno degli header di Badanai
 */

// Il vettore delle interruzioni viene allungato a:
#define DIM_VI      1024

// Indirizzi
// ------------------------
// BL+CLD abita il blocco 0
// SA occupa i blocchi 1 e 2
#define SRVC_APP_BASE       0x8020000
#define SRVC_APP_DIMB       (256 * 1024)
// MA occupa i restanti 3..7
#define MAIN_APP_BASE       0x8060000
#define MAIN_APP_DIMB       (640 * 1024)

// APPLICATION_TYPE
#define APPTYPE_BL          1
#define APPTYPE_MAINAPP     2

/*
 * In ordine di apparizione nel fw:
 *      vett int      1K
 *      com header	  128
 *      prv header	  128 (opz)
 *      fw
 */

#define HEADER_FILE_SIGNATURE    0x5A5AA5A5
#define HEADER_FILE_VERSION      1

typedef struct commonHeader {
    /**
     * @brief checksum of all next bytes of this structure
     */
    uint32_t headerChecksum32 ;

    /**
     * @brief signature of the header.
     * This record must be @p HEADER_FILE_SIGNATURE
     */
    uint32_t headerSignature ;

    /**
     * @brief version of the header.
     */
    uint32_t headerVersion ;

    // The compilation time in seconds from 01/01/2022
    uint32_t compileTime ;

    /**
     * @brief begin address where header is stored
     */
    uint32_t headerAddressStart ;

    /**
     * @brief begin address where VCI must store the file
     */
    uint32_t fwStoreAddress ;

    /**
     * @brief crc32 of the file without the contribution of the header and the last @p paddingBytes
     *
     */
    uint32_t fwCrc32 ;

    /**
     * @brief fwIntVectorAddress The address of interrupt vector table. It is needed to retrieve  the start address of application
     */
    uint32_t fwIntVectorAddress ;

    /**
     * @brief size of the file
     */
    uint32_t fileSize ;

    uint32_t fwVersion ;

    /**
     * @brief type of firmware
     * see \ref APPLICATION_TYPE
     */
    uint32_t fwType ;   // APPTYPE_???

    /**
     * @brief free for future
     */
    uint8_t reserved[84] ;
} commonHeader ;

bool app_valida(
    uint32_t dove,
    uint32_t * vi) ;



#else
#   warning fwh.h incluso
#endif
