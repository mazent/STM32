#include "utili.h"
#include "cmsis_rtos/cmsis_os.h"
//#define USA_DIARIO
#include "diario/diario.h"
#include "lwip/mem.h"
#include "lwip/timeouts.h"
#include "lwip/priv/tcp_priv.h"

#if 0
void mem_init(void){}

void * mem_trim(
    void * mem,
    mem_size_t size)
{
    INUTILE(size) ;
    return mem ;
}

void * mem_malloc(mem_size_t size)
{
    return ose_malloc(size) ;
}

void * mem_calloc(
    mem_size_t count,
    mem_size_t size)
{
    size_t dim = count * size ;
    void * x = ose_malloc(dim) ;
    if ( x ) {
        memset(x, 0, dim) ;
    }
    return x ;
}

void mem_free(void * mem)
{
    ose_free(mem) ;
}

#endif

u32_t sys_now(void)
{
    return HAL_GetTick() ;
}

#if 0
static osTimerId tcptim = NULL ;
static bool tcpip_tcp_timer_active = false ;

static void tcpip_tcp_timer(void * arg)
{
    INUTILE(arg) ;

    /* call TCP timer handler */
    tcp_tmr() ;
    /* timer still needed? */
    if ( tcp_active_pcbs || tcp_tw_pcbs ) {
        /* restart timer */
        DDB_CONTROLLA( osOK == osTimerStart(tcptim, TCP_TMR_INTERVAL) ) ;
    }
    else {
        /* disable timer */
        tcpip_tcp_timer_active = false ;
    }
}

osTimerDef(tcptim, tcpip_tcp_timer) ;

void sys_timeouts_init(void)
{
    if ( NULL == tcptim ) {
        tcptim = osTimerCreate(osTimer(tcptim), osTimerOnce, NULL) ;
    }
}

void tcp_timer_needed(void)
{
    if ( NULL == tcptim ) {
        DDB_ERR ;
        return ;
    }

    /* timer is off but needed again? */
    if ( !tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs) ) {
        /* enable and start timer */
        tcpip_tcp_timer_active = true ;
        DDB_CONTROLLA( osOK == osTimerStart(tcptim, TCP_TMR_INTERVAL) ) ;
    }
}

#endif
