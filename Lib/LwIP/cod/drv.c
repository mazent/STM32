//#define STAMPA_DBG
#include "utili.h"

#include "drv.h"

struct S_DRV_REQ {
    // From the pool
    void * data ;

    // Here the caller wait for the reply
    osMessageQId rsp ;

    // Link to the omonimous list
    struct list_head suspended ;

    // true if the request was cancelled
    bool aborted ;

    // called when the request is freed (optional)
    void (* pf_free)(void *) ;
} ;

// Frees all suspended request that was canceled

static void free_susp_canc(S_DRIVER * pDrv)
{
    if ( !list_empty(&pDrv->suspended) ) {
        struct S_DRV_REQ * cursor ;
        struct S_DRV_REQ * next ;
#ifdef __ICCARM__
        list_for_each_entry_safe(cursor,
                                 struct S_DRV_REQ,
                                 next,
                                 &pDrv->suspended,
                                 suspended) {
#else
        list_for_each_entry_safe(cursor, next, &pDrv->suspended, suspended) {
#endif
            if ( cursor->aborted ) {
                S_DRV_REQ * pR = cursor ;

                list_del(&cursor->suspended) ;

                if ( pR->pf_free ) {
                    pR->pf_free(pR->data) ;
                }

                osMailFree(pDrv->req, pR) ;
            }
        }
    }
}

bool DRV_begin(
    S_DRIVER * pDrv,
    osPoolId mem,
    int max_req)
{
    bool result = false ;
    osMailQDef(drvReq, max_req, S_DRV_REQ) ;
    osMessageQDef(drvRsp, 1, uint32_t) ;

    DBG_PRINTF("%s(%d)\n", __FUNCTION__, max_req) ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert(mem) ;
        if ( NULL == mem ) {
            DBG_ERR ;
            break ;
        }

        assert(max_req > 0) ;
        if ( max_req <= 0 ) {
            DBG_ERR ;
            break ;
        }

        assert(pDrv->signal) ;
        if ( 0 == pDrv->signal ) {
            DBG_ERR ;
            break ;
        }

        if ( pDrv->req ) {
            // Already initialized
            DRV_free_all(pDrv) ;
            result = true ;
            DBG_QUA ;
            break ;
        }

        INIT_LIST_HEAD(&pDrv->suspended) ;
        pDrv->drvTHD = osThreadGetId() ;
        osMailQId req = osMailCreate(osMailQ(drvReq), NULL) ;
        if ( NULL == req ) {
            break ;
        }

        // Initialisation of every request
        int i = 0 ;
        for ( ; i < max_req ; i++ ) {
            S_DRV_REQ * mail = (S_DRV_REQ *) osMailAlloc(req, 0) ;
            if ( NULL == mail ) {
                break ;
            }

            // Surely not suspended
            INIT_LIST_HEAD(&mail->suspended) ;

            // Each request has ...
            // ... a response queue
            mail->rsp = osMessageCreate(osMessageQ(drvRsp), NULL) ;
            if ( NULL == mail->rsp ) {
                break ;
            }
            // ... and some data
            mail->data = osPoolAlloc(mem) ;
            if ( NULL == mail->data ) {
                DBG_ERR ;
                break ;
            }

            // Park
            CHECK_IT( osOK == osMailPut(req, mail) ) ;
        }

        if ( i < max_req ) {
            break ;
        }

        // All the parked req must be available
        for ( i = 0 ; i < max_req ; i++ ) {
            osEvent evt = osMailGet(req, 0) ;

            assert(evt.status == osEventMail) ;

            if ( evt.status == osEventMail ) {
                CHECK_IT( osOK == osMailFree(req, evt.value.p) ) ;
            }
            else {
                break ;
            }
        }

        if ( i < max_req ) {
            break ;
        }

        // A miracle!
        result = true ;
        pDrv->req = req ;
    } while ( false ) ;

    return result ;
}

void DRV_clean(S_DRIVER * pDrv)
{
    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        free_susp_canc(pDrv) ;
    } while ( false ) ;
}

void DRV_free_all(S_DRIVER * pDrv)
{
    // free suspended
    if ( !list_empty(&pDrv->suspended) ) {
        struct S_DRV_REQ * cursor ;
        struct S_DRV_REQ * next ;
#ifdef __ICCARM__
        list_for_each_entry_safe(cursor,
                                 struct S_DRV_REQ,
                                 next,
                                 &pDrv->suspended,
                                 suspended) {
#else
        list_for_each_entry_safe(cursor, next, &pDrv->suspended, suspended) {
#endif
            list_del(&cursor->suspended) ;

            osMailFree(pDrv->req, cursor) ;
        }
    }

    // free waiting
    osEvent evt = osMailGet(pDrv->req, 0) ;
    while ( evt.status == osEventMail ) {
        S_DRV_REQ * pR = (S_DRV_REQ *) evt.value.p ;
        osMailFree(pDrv->req, pR) ;
        evt = osMailGet(pDrv->req, 0) ;
    }
}

S_DRV_REQ * DRV_alloc_wcb(
    S_DRIVER * pDrv,
    DRV_FREE_CB free_cb)
{
    S_DRV_REQ * mail = NULL ;

    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD != osThreadGetId() ) ;
        if ( pDrv->drvTHD == osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(NULL != pDrv->req) ;
        if ( NULL == pDrv->req ) {
            DBG_ERR ;
            break ;
        }

        mail = (S_DRV_REQ *) osMailAlloc(pDrv->req, osWaitForever) ;
        if ( mail ) {
            // Surely valid
            mail->aborted = false ;

            // callback
            mail->pf_free = free_cb ;

            // No reply
            ose_MessageReset(mail->rsp) ;
        }
    } while ( false ) ;

    return mail ;
}

bool DRV_send(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    bool result = false ;

    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD != osThreadGetId() ) ;
        if ( pDrv->drvTHD == osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        if ( osOK != osMailPut(pDrv->req, pR) ) {
            DBG_ERR ;
            break ;
        }

        if ( osOK != osSignalSet(pDrv->drvTHD, pDrv->signal) ) {
            DBG_ERR ;
            break ;
        }

        result = true ;
    } while ( false ) ;

    return result ;
}

bool DRV_outcome(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR,
    size_t milli)
{
    bool result = false ;

    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD != osThreadGetId() ) ;
        if ( pDrv->drvTHD == osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        osEvent evt = osMessageGet(pR->rsp, milli) ;
        if ( evt.status == osEventMessage ) {
            result = true ;
        }
    } while ( false ) ;

    return result ;
}

void DRV_cancel(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD != osThreadGetId() ) ;
        if ( pDrv->drvTHD == osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        pR->aborted = true ;

        // The thread will free the request
        CHECK_IT( osOK == osSignalSet(pDrv->drvTHD, pDrv->signal) ) ;
    } while ( false ) ;
}

S_DRV_REQ * DRV_receive(
    S_DRIVER * pDrv,
    size_t milli)
{
    S_DRV_REQ * pR = NULL ;
    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        // A chance to find a free request
        free_susp_canc(pDrv) ;

        osEvent evt = osMailGet(pDrv->req, milli) ;
        if ( evt.status == osEventMail ) {
            pR = (S_DRV_REQ *) evt.value.p ;
            if ( pR->aborted ) {
                // Too late
                if ( pR->pf_free ) {
                    pR->pf_free(pR->data) ;
                }
                osMailFree(pDrv->req, pR) ;

                pR = NULL ;
            }
        }
    } while ( false ) ;

    return pR ;
}

void DRV_reply(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        if ( pR->aborted ) {
            // Too late
            if ( pR->pf_free ) {
                pR->pf_free(pR->data) ;
            }

            CHECK_IT( osOK == osMailFree(pDrv->req, pR) ) ;
        }
        else {
            // Unlock the api
            CHECK_IT( osOK == osMessagePut(pR->rsp, (uint32_t) 0, 0) ) ;
        }
    } while ( false ) ;
}

void DRV_free(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    DBG_FUN ;
    
    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD != osThreadGetId() ) ;
        if ( pDrv->drvTHD == osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        if ( pR->pf_free ) {
            pR->pf_free(pR->data) ;
        }
        CHECK_IT( osOK == osMailFree(pDrv->req, pR) ) ;
    } while ( false ) ;
}

void DRV_suspend(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        // Clean ...
        free_susp_canc(pDrv) ;

        // ... before add
        list_add(&pR->suspended, &pDrv->suspended) ;
    } while ( false ) ;
}

S_DRV_REQ * DRV_first_susp(S_DRIVER * pDrv)
{
    S_DRV_REQ * pR = NULL ;

    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        // Clean before serch
        free_susp_canc(pDrv) ;

        if ( !list_empty(&pDrv->suspended) ) {
            pR = list_first_entry(&pDrv->suspended, S_DRV_REQ, suspended) ;
        }
    } while ( false ) ;

    return pR ;
}

S_DRV_REQ * DRV_next_susp(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    S_DRV_REQ * next = NULL ;

    DBG_FUN ;
    
    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        if ( list_is_last(&pR->suspended, &pDrv->suspended) ) {
            break ;
        }
#ifdef __ICCARM__
        next = list_next_entry(pR, S_DRV_REQ, suspended) ;
#else
        next = list_next_entry(pR, suspended) ;
#endif
    } while ( false ) ;

    return next ;
}

void DRV_remove_susp(
    S_DRIVER * pDrv,
    S_DRV_REQ * pR)
{
    DBG_FUN ;

    do {
        assert(pDrv) ;
        if ( NULL == pDrv ) {
            DBG_ERR ;
            break ;
        }

        assert( pDrv->drvTHD == osThreadGetId() ) ;
        if ( pDrv->drvTHD != osThreadGetId() ) {
            DBG_ERR ;
            break ;
        }

        assert(pR) ;
        if ( NULL == pR ) {
            DBG_ERR ;
            break ;
        }

        if ( !list_empty(&pDrv->suspended) ) {
            list_del(&pR->suspended) ;
        }
    } while ( false ) ;
}

void * DRV_data(S_DRV_REQ * pR)
{
    return pR->data ;
}
