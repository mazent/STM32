/*
 * Tutorial: https://os.mbed.com/handbook/CMSIS-RTOS
 * API: https://arm-software.github.io/CMSIS_5/RTOS/html/group__CMSIS__RTOS.html
 */

#define STAMPA_DBG          1
#include "utili.h"

#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "stm32h7xx_hal.h"

static bool xPortInIsrContext(void)
{
    return __get_IPSR() != 0 ;
}

static TickType_t ms_in_tick(uint32_t millisec)
{
    TickType_t ticks ;

    static_assert(portTICK_PERIOD_MS, "OKKIO") ;

    if ( millisec == osWaitForever ) {
        ticks = portMAX_DELAY ;
    }
    else if ( 0 == millisec ) {
        ticks = 0 ;
    }
    else {
        // Rounding
        ticks = 1 + (millisec - 1) / portTICK_PERIOD_MS ;
        if ( 0 == ticks ) {
            ticks = 1 ;
        }
    }

    return ticks ;
}

#if configMAX_PRIORITIES == 3
static unsigned portBASE_TYPE freeRtos_pri(osPriority priority)
{
    switch ( priority ) {
    case osPriorityIdle:
    case osPriorityLow:
        return tskIDLE_PRIORITY ;

    case osPriorityBelowNormal:
    case osPriorityNormal:
    case osPriorityAboveNormal:
        return tskIDLE_PRIORITY + 1 ;

    case osPriorityHigh:
    case osPriorityRealtime:
        return tskIDLE_PRIORITY + 2 ;

    default:
        DBG_ERR ;
        return tskIDLE_PRIORITY ;
    }
}

#else
static unsigned portBASE_TYPE freeRtos_pri(osPriority priority)
{
    ASSERT(configMAX_PRIORITIES >= 7) ;

    switch ( priority ) {
    case osPriorityIdle:
        return tskIDLE_PRIORITY ;
    case osPriorityLow:
        return tskIDLE_PRIORITY + 1 ;
    case osPriorityBelowNormal:
        return tskIDLE_PRIORITY + 2 ;
    case osPriorityNormal:
        return tskIDLE_PRIORITY + 3 ;
    case osPriorityAboveNormal:
        return tskIDLE_PRIORITY + 4 ;
    case osPriorityHigh:
        return tskIDLE_PRIORITY + 5 ;
    case osPriorityRealtime:
        return tskIDLE_PRIORITY + 6 ;
    default:
        DBG_ERR ;
        return tskIDLE_PRIORITY ;
    }
}

#endif

static osMessageQId queue_create_and_register(
    const UBaseType_t uxQueueLength,
    const UBaseType_t uxItemSize,
    const char * nome)
{
    QueueHandle_t q = xQueueCreate(uxQueueLength, uxItemSize) ;
#if (configQUEUE_REGISTRY_SIZE > 0)
    vQueueAddToRegistry(q, nome) ;
#else
    INUTILE(nome) ;
#endif
    return q ;
}

//  ==== Kernel Control Functions ====

osStatus osKernelInitialize(void)
{
    return osOK ;
}

osStatus osKernelStart(void)
{
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }

    vTaskStartScheduler() ;

    return osOK ;
}

///// Check if the RTOS kernel is already started.
///// \note MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
///// \return 0 RTOS is not started, 1 RTOS is started.
//int32_t osKernelRunning(void);

#if (defined (osFeature_SysTick) && (osFeature_SysTick != 0 ) )       // System Timer available

uint32_t osKernelSysTick(void)
{
    if ( xPortInIsrContext() ) {
        return xTaskGetTickCountFromISR() ;
    }

    return xTaskGetTickCount() ;
}

///// The RTOS kernel system timer frequency in Hz
///// \note Reflects the system timer setting and is typically defined in a configuration file.
//#define osKernelSysTickFrequency 100000000
//
///// Convert a microseconds value to a RTOS kernel system timer value.
///// \param         microsec     time value in microseconds.
///// \return time value normalized to the \ref osKernelSysTickFrequency
//#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)
//
#endif    // System Timer available

//  ==== Memory (extension) ====

void * ose_malloc(size_t dim)
{
    if ( dim ) {
        return pvPortMalloc(dim) ;
    }

    return NULL ;
}

void ose_free(void * v)
{
    if ( v ) {
        vPortFree(v) ;
    }
}

//  ==== Thread Management ====

osThreadId osThreadCreate(
    const osThreadDef_t * td,
    void * argument)
{
    TaskHandle_t handle ;
    uint32_t ss = td->stacksize ;

    // 0 -> dimensione minima
    if ( 0 == ss ) {
        ss = configMINIMAL_STACK_SIZE * sizeof(StackType_t) ;
    }
    // Converto la dimensione dello stack da byte a freertos
    ss = (ss + sizeof(StackType_t) - 1) / sizeof(StackType_t) ;

    if ( xTaskCreate(td->pthread,
                     td->nome,
                     ss,
                     argument,
                     freeRtos_pri(td->tpriority),
                     &handle) != pdPASS ) {
        return NULL ;
    }

    return handle ;
}

#if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1) || (configUSE_MUTEXES == 1 ) )
osThreadId osThreadGetId(void)
{
    return xTaskGetCurrentTaskHandle() ;
}

#endif

#if (INCLUDE_vTaskDelete == 1)
osStatus osThreadTerminate(osThreadId thread_id)
{
    vTaskDelete(thread_id) ;

    return osOK ;
}

#endif

osStatus osThreadYield(void)
{
    taskYIELD() ;

    return osOK ;
}

///// Change priority of an active thread.
///// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
///// \param[in]     priority      new priority value for the thread function.
///// \return status code that indicates the execution status of the function.
///// \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
//osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);
//
///// Get current priority of an active thread.
///// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
///// \return current priority value of the thread function.
///// \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
//osPriority osThreadGetPriority (osThreadId thread_id);

//  ==== Generic Wait Functions ====

osStatus osDelay(uint32_t millisec)
{
    if ( 0 == millisec ) {
        millisec = 1 ;
    }

    vTaskDelay( ms_in_tick(millisec) ) ;

    return osOK ;
}

#if (defined (osFeature_Wait) && (osFeature_Wait != 0 ) )       // Generic Wait available

///// Wait for Signal, Message, Mail, or Timeout.
///// \param[in] millisec          \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
///// \return event that contains signal, message, or mail information or error code.
///// \note MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
//osEvent osWait (uint32_t millisec);

#endif  // Generic Wait available

//  ==== Timer Management Functions ====

typedef struct os_timer_cb {
    uint32_t milli ;
    TimerHandle_t h ;
    os_ptimer cb ;
    void * arg ;
} os_timer_id ;

static void timer_cb(TimerHandle_t xTimer)
{
    osTimerId pT = pvTimerGetTimerID(xTimer) ;

    ASSERT(xTimer == pT->h) ;

    pT->cb(pT->arg) ;
}

osTimerId osTimerCreate(
    const osTimerDef_t * timer_def,
    os_timer_type type,
    void * argument)
{
    os_timer_id * tid = NULL ;
    bool esito = false ;

    do {
        ASSERT( !xPortInIsrContext() ) ;
        if ( xPortInIsrContext() ) {
            break ;
        }

        ASSERT(timer_def) ;
        if ( NULL == timer_def ) {
            break ;
        }

        ASSERT(timer_def->ptimer) ;
        if ( NULL == timer_def->ptimer ) {
            break ;
        }

        tid = pvPortMalloc( sizeof(os_timer_id) ) ;
        if ( NULL == tid ) {
            break ;
        }

        tid->milli = 100 ;
        tid->cb = timer_def->ptimer ;
        tid->arg = argument ;
        tid->h = xTimerCreate(
            timer_def->nome,
            ms_in_tick(tid->milli),
            (type == osTimerPeriodic) ? pdTRUE : pdFALSE,
            tid,
            timer_cb) ;
        if ( NULL == tid->h ) {
            break ;
        }

        esito = true ;
    } while ( false ) ;

    if ( !esito ) {
        if ( tid ) {
            vPortFree(tid) ;
            tid = NULL ;
        }
    }

    return tid ;
}

osStatus osTimerStart(
    osTimerId timer_id,
    uint32_t millisec)
{
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }

    if ( millisec != timer_id->milli ) {
        if ( pdPASS ==
             xTimerChangePeriod(timer_id->h, ms_in_tick(millisec), 0) ) {
            timer_id->milli = millisec ;
        }
        else {
            DBG_ERR ;
            return osErrorOS ;
        }
    }

    if ( pdPASS == xTimerStart(timer_id->h, 0) ) {
        return osOK ;
    }

    DBG_ERR ;
    return osErrorParameter ;
}

osStatus osTimerStop(osTimerId timer_id)
{
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( pdPASS == xTimerStop(timer_id->h, 0) ) {
        return osOK ;
    }

    DBG_ERR ;
    return osErrorParameter ;
}

osStatus osTimerDelete(osTimerId timer_id)
{
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( pdPASS == xTimerDelete(timer_id->h, 0) ) {
        vPortFree(timer_id) ;

        return osOK ;
    }

    DBG_ERR ;
    return osErrorParameter ;
}

//  ==== Signal Management ====

osStatus osSignalSet(
    osThreadId thread_id,
    int32_t signal)
{
    ASSERT(signal >= 0) ;
    if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( xTaskNotifyFromISR(thread_id, (uint32_t) signal, eSetBits,
                                &xHigherPriorityTaskWoken) != pdPASS ) {
            DBG_ERR ;
            return osErrorOS ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else if ( xTaskNotify(thread_id, (uint32_t) signal, eSetBits) != pdPASS ) {
        DBG_ERR ;
        return osErrorOS ;
    }

    return osOK ;
}

osEvent osSignalWait(
    int32_t signals,
    uint32_t millisec)
{
    osEvent evn = {
        .status = osOK
    } ;
    TickType_t ticks = ms_in_tick(millisec) ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        evn.status = osErrorISR ;
    }
    else {
        ASSERT(signals >= 0) ;
        if ( 0 == signals ) {
            signals = NEGA(0x80000000) ;
        }

        if ( xTaskNotifyWait(
                 0, // ulBitsToClearOnEntry: non cancello niente quando mi metto in attesa
                 (uint32_t) signals,
                 (uint32_t *) &evn.value.signals,
                 ticks) != pdTRUE ) {
            evn.status = osEventTimeout ;
        }
        else if ( evn.value.signals < 0 ) {
            DBG_ERR ;
            evn.status = osErrorValue ;
        }
        else {
            evn.status = osEventSignal ;
        }
    }

    return evn ;
}

//  ==== Mutex Management ====

osMutexId osMutexCreate(const osMutexDef_t * mutex_def)
{
    INUTILE(mutex_def) ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return NULL ;
    }

    return xSemaphoreCreateMutex() ;
}

osStatus osMutexWait(
    osMutexId mutex_id,
    uint32_t millisec)
{
    ASSERT(mutex_id) ;
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( NULL == mutex_id ) {
        return osErrorParameter ;
    }

    TickType_t attesa = ms_in_tick(millisec) ;

    return pdTRUE ==
           xSemaphoreTake(mutex_id, attesa) ?
           osOK : osErrorTimeoutResource ;
}

osStatus osMutexRelease(osMutexId mutex_id)
{
    ASSERT(mutex_id) ;
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( NULL == mutex_id ) {
        return osErrorParameter ;
    }

    return pdTRUE ==
           xSemaphoreGive(mutex_id) ?
           osOK : osErrorResource ;
}

osStatus osMutexDelete(osMutexId mutex_id)
{
    ASSERT(mutex_id) ;
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( NULL == mutex_id ) {
        return osErrorParameter ;
    }

    vSemaphoreDelete(mutex_id) ;

    return osOK ;
}

//  ==== Semaphore Management Functions ====

#if (defined (osFeature_Semaphore) && (osFeature_Semaphore != 0 ) )       // Semaphore available

osSemaphoreId osSemaphoreCreate(
    const osSemaphoreDef_t * semaphore_def,
    int32_t count)
{
    INUTILE(semaphore_def) ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return NULL ;
    }
    if ( count == 1 ) {
        osSemaphoreId sema ;
        vSemaphoreCreateBinary(sema) ;
        return sema ;
    }

    return xSemaphoreCreateCounting(count, count) ;
}

int32_t osSemaphoreWait(
    osSemaphoreId semaphore_id,
    uint32_t millisec)
{
    ASSERT(semaphore_id) ;
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( NULL == semaphore_id ) {
        return osErrorParameter ;
    }

    TickType_t attesa = ms_in_tick(millisec) ;

    return pdTRUE ==
           xSemaphoreTake(semaphore_id, attesa) ?
           osOK : osErrorTimeoutResource ;
}

osStatus osSemaphoreRelease(osSemaphoreId semaphore_id)
{
    ASSERT(semaphore_id) ;

    if ( NULL == semaphore_id ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( xSemaphoreGiveFromISR(semaphore_id,
                                   &xHigherPriorityTaskWoken) != pdTRUE ) {
            DBG_ERR ;
            return osErrorResource ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        if ( xSemaphoreGive(semaphore_id) != pdTRUE ) {
            DBG_ERR ;
            return osErrorResource ;
        }
    }
    return osOK ;
}

osStatus osSemaphoreDelete(osSemaphoreId semaphore_id)
{
    ASSERT(semaphore_id) ;
    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    if ( NULL == semaphore_id ) {
        return osErrorParameter ;
    }

    vSemaphoreDelete(semaphore_id) ;

    return osOK ;
}

#endif     // Semaphore available

//  ==== Memory Pool Management Functions ====

#if (defined (osFeature_Pool) && (osFeature_Pool != 0 ) )    // Memory Pool Management available

/*
             free  +--------+
           -------->        |
      API    alloc | liberi |
           <-------+        |
                   +--------+
*/

typedef struct os_pool_cb {
    QueueHandle_t liberi ;
    size_t dim_elem ;
    uint8_t * mem ;
} os_pool_cb_t ;

osPoolId osPoolCreate(const osPoolDef_t * pool_def)
{
    osPoolId pid = NULL ;
    os_pool_cb_t tmp = {
        .dim_elem = pool_def->item_sz
    } ;
    uint8_t * mem = NULL ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        goto err1 ;
    }

    tmp.liberi = queue_create_and_register(pool_def->pool_sz,
                                           sizeof(void *),
                                           pool_def->nome) ;
    ASSERT(tmp.liberi) ;
    if ( NULL == tmp.liberi ) {
        goto err1 ;
    }

    tmp.mem = pvPortMalloc(pool_def->pool_sz * pool_def->item_sz) ;
    ASSERT(tmp.mem) ;
    if ( NULL == tmp.mem ) {
        goto err2 ;
    }

    // All'inizio tutti i buffer sono liberi
    mem = tmp.mem ;
    for ( uint32_t i = 0 ; i < pool_def->pool_sz ;
          i++, mem += pool_def->item_sz ) {
        (void) xQueueSend(tmp.liberi, &mem, 0) ;
    }

    pid = pvPortMalloc( sizeof(os_pool_cb_t) ) ;
    ASSERT(pid) ;
    if ( NULL == pid ) {
        goto err3 ;
    }

    *pid = tmp ;
    goto err1 ;

err3:
    vPortFree(tmp.mem) ;
err2:
    vQueueDelete(tmp.liberi) ;
err1:
    return pid ;
}

void * osPoolAlloc(osPoolId pool_id)
{
    void * buf = NULL ;

    ASSERT(pool_id) ;
    if ( NULL == pool_id ) {}
    else if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( pdFAIL ==
             xQueueReceiveFromISR(pool_id->liberi, &buf,
                                  &xHigherPriorityTaskWoken) ) {
            buf = NULL ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        (void) xQueueReceive(pool_id->liberi, &buf, 0) ;
    }

    return buf ;
}

void * osPoolCAlloc(osPoolId pool_id)
{
    void * buf = osPoolAlloc(pool_id) ;
    if ( buf ) {
        memset_(buf, 0, pool_id->dim_elem) ;
    }

    return buf ;
}

osStatus osPoolFree(
    osPoolId pool_id,
    void * buf)
{
    ASSERT(pool_id) ;
    ASSERT(buf) ;

    if ( NULL == pool_id ) {
        return osErrorParameter ;
    }
    if ( NULL == buf ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        osStatus esito = osOK ;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( pdTRUE !=
             xQueueSendFromISR(pool_id->liberi, &buf,
                               &xHigherPriorityTaskWoken) ) {
            DBG_ERR ;
            esito = osErrorOS ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;

        return esito ;
    }
    if ( pdTRUE == xQueueSend(pool_id->liberi, &buf, 0) ) {
        return osOK ;
    }

    DBG_ERR ;
    return osErrorOS ;
}

#endif   // Memory Pool Management available

//  ==== Message Queue Management Functions ====

#if (defined (osFeature_MessageQ) && (osFeature_MessageQ != 0 ) )       // Message Queues available

osMessageQId osMessageCreate(
    const osMessageQDef_t * queue_def,
    osThreadId thread_id)
{
    INUTILE(thread_id) ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        return NULL ;
    }

    return queue_create_and_register(queue_def->queue_sz,
                                     queue_def->item_sz,
                                     queue_def->nome) ;
}

osStatus osMessagePut(
    osMessageQId queue_id,
    uint32_t info,
    uint32_t millisec)
{
    TickType_t ticks = ms_in_tick(millisec) ;

    ASSERT(queue_id) ;

    if ( queue_id == NULL ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( xQueueSendFromISR(queue_id, &info,
                               &xHigherPriorityTaskWoken) != pdTRUE ) {
            DBG_ERR ;
            return osErrorResource ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        if ( xQueueSend(queue_id, &info, ticks) != pdTRUE ) {
            DBG_ERR ;
            return osErrorResource ;
        }
    }

    return osOK ;
}

osEvent osMessageGet(
    osMessageQId queue_id,
    uint32_t millisec)
{
    TickType_t ticks = ms_in_tick(millisec) ;
    osEvent event ;

    ASSERT(queue_id) ;

    event.def.message_id = queue_id ;
    event.value.v = 0 ;

    if ( queue_id == NULL ) {
        event.status = osErrorParameter ;
        return event ;
    }

    if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( xQueueReceiveFromISR(queue_id, &event.value.v,
                                  &xHigherPriorityTaskWoken) == pdTRUE ) {
            /* We have mail */
            event.status = osEventMessage ;
        }
        else {
            event.status = osOK ;
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        if ( xQueueReceive(queue_id, &event.value.v, ticks) == pdTRUE ) {
            /* We have mail */
            event.status = osEventMessage ;
        }
        else {
            event.status = (ticks == 0) ? osOK : osEventTimeout ;
        }
    }

    return event ;
}

void ose_MessageReset(osMessageQId m)
{
    xQueueReset(m) ;
}

int ose_MessageWaiting(osMessageQId queue_id)
{
    ASSERT(queue_id) ;

    if ( queue_id == NULL ) {
        // invalid queue: no messages waiting
        return 0 ;
    }
    if ( xPortInIsrContext() ) {
        return uxQueueMessagesWaitingFromISR(queue_id) ;
    }

    return uxQueueMessagesWaiting(queue_id) ;
}

osStatus ose_MessageDelete(osMessageQId queue_id)
{
    ASSERT(queue_id) ;

    if ( queue_id == NULL ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        return osErrorISR ;
    }
    vQueueDelete(queue_id) ;

    return osOK ;
}

#endif     // Message Queues available

//  ==== Mail Queue Management Functions ====

#if (defined (osFeature_MailQ) && (osFeature_MailQ != 0 ) )       // Mail Queues available

/*
    +-----+                           +-----+
    |     |   put  +---------+  get   |     |
    |     +--------> spedite +-------->     |
    |     |        +---------+        |     |
    | MIT |                           | DST |
    |     |  alloc +---------+  free  |     |
    |     <--------+  libere <--------+     |
    |     |        +---------+        |     |
    +-----+                           +-----+
*/

typedef struct os_mailQ_cb {
    const osMailQDef_t * queue_def ;

    QueueHandle_t libere ;
    QueueHandle_t spedite ;
    uint8_t * mem ;
} os_mailQ_cb_t ;

osMailQId osMailCreate(
    const osMailQDef_t * queue_def,
    osThreadId thread_id)
{
    osMailQId mq = NULL ;
    os_mailQ_cb_t tmp = {
        .queue_def = queue_def
    } ;
    uint8_t * mail = NULL ;

    INUTILE(thread_id) ;

    ASSERT( !xPortInIsrContext() ) ;
    if ( xPortInIsrContext() ) {
        goto err1 ;
    }

    tmp.spedite = queue_create_and_register(queue_def->queue_sz,
                                            sizeof(void *),
                                            queue_def->nome_o) ;
    ASSERT(tmp.spedite) ;
    if ( NULL == tmp.spedite ) {
        goto err1 ;
    }

    tmp.libere = queue_create_and_register(queue_def->queue_sz,
                                           sizeof(void *),
                                           queue_def->nome_l) ;
    ASSERT(tmp.libere) ;
    if ( NULL == tmp.libere ) {
        goto err2 ;
    }

    tmp.mem = pvPortMalloc(queue_def->queue_sz * queue_def->item_sz) ;
    ASSERT(tmp.mem) ;
    if ( NULL == tmp.mem ) {
        goto err3 ;
    }

    // All'inizio tutte le mail sono disponibili
    mail = tmp.mem ;
    for ( uint32_t i = 0 ; i < queue_def->queue_sz ;
          i++, mail += queue_def->item_sz ) {
        (void) xQueueSend(tmp.libere, &mail, 0) ;
    }

    mq = pvPortMalloc( sizeof(os_mailQ_cb_t) ) ;
    ASSERT(mq) ;
    if ( NULL == mq ) {
        goto err4 ;
    }

    *mq = tmp ;
    goto err1 ;

err4:
    vPortFree(tmp.mem) ;
err3:
    vQueueDelete(tmp.libere) ;
err2:
    vQueueDelete(tmp.spedite) ;
err1:
    return mq ;
}

void * osMailAlloc(
    osMailQId queue_id,
    uint32_t millisec)
{
    void * mail = NULL ;

    ASSERT(queue_id) ;

    if ( NULL == queue_id ) {}
    else if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( pdFAIL ==
             xQueueReceiveFromISR(queue_id->libere, &mail,
                                  &xHigherPriorityTaskWoken) ) {
            DBG_ERR ;
            mail = NULL ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        (void) xQueueReceive(queue_id->libere, &mail, millisec) ;
    }

    return mail ;
}

void * osMailCAlloc(
    osMailQId queue_id,
    uint32_t millisec)
{
    void * mail = osMailAlloc(queue_id, millisec) ;
    if ( mail ) {
        memset_(mail, 0, queue_id->queue_def->item_sz) ;
    }

    return mail ;
}

osStatus osMailFree(
    osMailQId queue_id,
    void * mail)
{
    ASSERT(queue_id) ;
    ASSERT(mail) ;

    if ( NULL == queue_id ) {
        return osErrorParameter ;
    }
    if ( NULL == mail ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        osStatus esito = osOK ;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( pdTRUE !=
             xQueueSendFromISR(queue_id->libere, &mail,
                               &xHigherPriorityTaskWoken) ) {
            DBG_ERR ;
            esito = osErrorOS ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;

        return esito ;
    }
    if ( pdTRUE == xQueueSend(queue_id->libere, &mail, 0) ) {
        return osOK ;
    }

    DBG_ERR ;
    return osErrorOS ;
}

osStatus osMailPut(
    osMailQId queue_id,
    void * mail)
{
    ASSERT(queue_id) ;
    ASSERT(mail) ;

    if ( NULL == queue_id ) {
        return osErrorParameter ;
    }
    if ( NULL == mail ) {
        return osErrorParameter ;
    }
    if ( xPortInIsrContext() ) {
        osStatus esito = osOK ;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( pdTRUE !=
             xQueueSendFromISR(queue_id->spedite, &mail,
                               &xHigherPriorityTaskWoken) ) {
            esito = osErrorOS ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;

        return esito ;
    }
    if ( pdTRUE == xQueueSend(queue_id->spedite, &mail, 0) ) {
        return osOK ;
    }

    DBG_ERR ;
    return osErrorOS ;
}

osEvent osMailGet(
    osMailQId queue_id,
    uint32_t millisec)
{
    osEvent event ;

    ASSERT(queue_id) ;

    if ( NULL == queue_id ) {
        event.status = osErrorParameter ;
    }
    else if ( xPortInIsrContext() ) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE ;

        if ( xQueueReceiveFromISR(queue_id->spedite, &event.value.p,
                                  &xHigherPriorityTaskWoken) == pdTRUE ) {
            event.status = osEventMail ;
        }
        else {
            event.status = osOK ;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken) ;
    }
    else {
        TickType_t attesa = ms_in_tick(millisec) ;

        event.status = pdTRUE ==
                       xQueueReceive(queue_id->spedite,
                                     &event.value.p, attesa) ?
                       osEventMail : osOK ;
    }

    return event ;
}

#endif  // Mail Queues available

/*********************************************
    Non fanno parte dell'interfaccia
    ma dipendono da freertos
*********************************************/

static uint32_t tik = 0 ;

uint32_t HAL_GetTick(void)
{
    return tik ;
}

void SysTick_Handler(void)
{
    tik++ ;

#if INCLUDE_xTaskGetSchedulerState == 1
    if ( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED ) {
        xPortSysTickHandler() ;
    }
#else
#   error INCLUDE_xTaskGetSchedulerState deve valere 1
#endif
}

#if configCHECK_FOR_STACK_OVERFLOW > 0

void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char * pcTaskName)
{
    INUTILE(xTask) ;

    DBG_PRINTF("STACK OVERFLOW HOOK: %s \r\n", pcTaskName) ;

    BPOINT ;
}

#endif

#if configUSE_MALLOC_FAILED_HOOK == 1

void vApplicationMallocFailedHook(void)
{
    DBG_PUTS("MALLOC FAILED HOOK") ;

    BPOINT ;
}

#endif

#if configUSE_TICK_HOOK == 1

void vApplicationTickHook(void)
{}

#endif

#ifndef NDEBUG

#   ifdef traceMALLOC
void trace_malloc(
    void * p,
    size_t dim)
{
    ASSERT(p) ;

    DBG_PRINTF("%p = malloc(%zu)\n", p, dim) ;
}

#   endif

#   ifdef traceFREE
void trace_free(
    void * p,
    size_t dim)
{
    INUTILE(dim) ;

    DBG_PRINTF("free(%p)\n", p) ;
}

#   endif

#endif
