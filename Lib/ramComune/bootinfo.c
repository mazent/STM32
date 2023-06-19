#define STAMPA_DBG
#include "utili.h"

#ifndef DESCRITTORE_MIO

#include "ramcomune.h"
#include "../boot/fwh.h"

/*
 * Scambio di informazioni col resto del mondo stile MB
 */
#define SR_SIGNATURE  0x55AA55AA

typedef struct {
    uint32_t mainAppIsCorrupted : 1,
             production_mode : 1,
             freebits : 30 ;

    uint8_t free[60] ;
} ServiceAppStructure ;

typedef struct  {
    uint32_t serviceAppIsCorrupted : 1,
             freebits : 31 ;

    uint8_t free[60] ;
} MainAppStructure ;

typedef struct  {
    uint32_t appToExecute ;         // APPLICATION_TYPE
    uint32_t productionMode : 1,    // 1: forza collaudo
             freebits : 31 ;

    uint8_t free[60] ;
} BootloaderStructure ;

typedef struct  {
    uint32_t structVersion ;
    uint32_t signature ;

    MainAppStructure mainApp ;
    ServiceAppStructure serviceApp ;
    BootloaderStructure bootloader ;

    // Checksum from signature to DATA
    uint32_t checksum ;
} BootInfo ;

static
USED
__attribute__( ( section(".ram_comune") ) )
BootInfo bi ;

static uint32_t GetChecksum(void)
{
    uint32_t cks = 0 ;
    uint8_t * pStart = (uint8_t *) &bi.signature ;
    uint8_t * pEnd = (uint8_t *) &bi.checksum ;
    for ( uint8_t * pc = pStart ; pc < pEnd ; pc++ ) {
        cks += *pc ;
    }
    return cks ;
}

void RC_iniz(void)
{
    bool valida = false ;

    if ( bi.signature != SR_SIGNATURE ) {
        DBG_ERR ;
    }
    else {
        valida = bi.checksum == GetChecksum() ;
    }

    if ( !valida ) {
        DBG_ERR ;
        memset( &bi, 0, sizeof(bi) ) ;
        bi.signature = SR_SIGNATURE ;
        bi.checksum = GetChecksum() ;
    }
#ifdef DBG_ABIL
    else {
        DBG_PUTS("BootInfo") ;
        DBG_PRINTF("\t service app %s corrupted ",
                   bi.mainApp.serviceAppIsCorrupted ? "IS" : "is not") ;
        DBG_PRINTF("\t main app %s corrupted ",
                   bi.serviceApp.mainAppIsCorrupted ? "IS" : "is not") ;
        DBG_PRINTF("\t app to execute: %08X", bi.bootloader.appToExecute) ;
        DBG_PRINTF("\t %s production mode",
                   bi.bootloader.productionMode ? "IN" : "not in") ;
    }
#endif
}

RC_TIPO_BOOT RC_cosa_butto(void)
{
    if ( bi.bootloader.productionMode ) {
        return TB_COLLAUDO ;
    }

    switch ( bi.bootloader.appToExecute ) {
    case APPTYPE_BL:
        return TB_COLLAUDO ;
    case APPTYPE_MAINAPP:
        return TB_MAIN ;
    case APPTYPE_SERVICEAPP:
        return TB_SRV ;
    default:
        return TB_SCONO ;
    }
}

void RC_butta_questo(RC_TIPO_BOOT tb)
{
    bool scrivi = true ;

    switch ( tb ) {
    case TB_COLLAUDO:
        bi.bootloader.productionMode = 1 ;
        break ;
    case TB_MAIN:
        bi.bootloader.appToExecute = APPTYPE_MAINAPP ;
        break ;
    case TB_SRV:
        bi.bootloader.appToExecute = APPTYPE_SERVICEAPP ;
        break ;
    default:
        DBG_ERR ;
        scrivi = false ;
        break ;
    }

    if ( scrivi ) {
        bi.checksum = GetChecksum() ;
    }
}

void RC_ma(bool valida)
{
    bi.serviceApp.mainAppIsCorrupted = valida ? 0 : 1 ;
    bi.checksum = GetChecksum() ;
}

void RC_sa(bool valida)
{
    bi.mainApp.serviceAppIsCorrupted = valida ? 0 : 1 ;
    bi.checksum = GetChecksum() ;
}

#endif
