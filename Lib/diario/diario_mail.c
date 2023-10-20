#include "utili.h"
#include "diario_priv.h"

#if DDB_NUM_MSG

#include "cmsis_rtos/cmsis_os.h"

#ifdef DDB_QUANDO
// "%08X) " + livello + a capo + null
#define DIM_MIN     (8 + 1 + 1 + 4 + 2 + 1)
#else
// livello + a capo + null
#define DIM_MIN     (4 + 2 + 1)
#endif

#if DDB_DIM_MSG <= DIM_MIN
#error OKKIO
#endif

static osThreadId idTHD = NULL ;
static osMailQId idMQ = NULL ;

DDB_RIGA * riga_alloca(void)
{
    return osMailAlloc(idMQ, 0) ;
}

void riga_libera(DDB_RIGA * riga)
{
    (void) osMailFree(idMQ, riga) ;
}

void riga_scrivi(DDB_RIGA * riga)
{
    (void) osMailPut(idMQ, riga) ;
}

static void diario(void * v)
{
    INUTILE(v) ;

    while ( true ) {
        osEvent evn = osMailGet(idMQ, osWaitForever) ;
        if ( osEventMail == evn.status ) {
            DDB_RIGA * riga = evn.value.p ;

            ddb_stampa(riga) ;

            ddb_scrivi(riga->msg, riga->dim) ;
            (void) osMailFree(idMQ, riga) ;
        }
    }
}

bool ddb_iniz(void)
{
    do {
        if ( NULL == idMQ ) {
            osMailQDef(messaggi, DDB_NUM_MSG, DDB_RIGA) ;
            idMQ = osMailCreate(osMailQ(messaggi), NULL) ;
            ASSERT(idMQ) ;
            if ( NULL == idMQ ) {
                break ;
            }
        }

        if ( NULL == idTHD ) {
            osThreadDef(diario, osPriorityIdle, 0, 0) ;
            idTHD = osThreadCreate(osThread(diario), NULL) ;
            ASSERT(idTHD) ;
            if ( NULL == idTHD ) {
                break ;
            }
        }
    } while ( false ) ;

    return NULL != idTHD ;
}

#endif
