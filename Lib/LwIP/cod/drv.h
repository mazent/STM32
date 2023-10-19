#ifndef DRV_H_
#define DRV_H_

// Refer to this for jargon (signal, pool, etc)
#include "cmsis_rtos/cmsis_os.h"
#include "linux_list.h"

/*
    Boilerplate to send requests to driver DDD

    The driver has a task with wich it manages the communication
    with some device. A signal must be devoted to request management

    The driver exposes an Application Programming Interface that is called
    from other tasks and it want to serialize access to its structures
    sending the requests to its task (every api is mapped to a request)

    Each api is asynchronous: it must be started and then waited for or aborted.
    We need only the asynchronous api, because we easily write:
        static inline bool api_sync(parameters, uint32_t timeout)
            struct DDD_OP op ;
            if ( api_ini(&op, parameters) )
                return api_ris(op, timeout)
            else
                return false

    Life cycle:
        1) the api allocates a request (infinite wait)
        2) the api fills the request and sends it to the task ([in]finite wait)
        3a) the driver receives the request and, if it cannot be fulfilled it is suspended
        3b) the driver manages events that can complete suspended requests
        4) when the driver completes the request, it unlocks the api
        5) the api returns the result to the caller and frees the request


    Asynchronous request

        Every api is divided in two functions:
            1) the first one (ini suffix) sends the request
            2) the second (ris) get the result

        We need also a function (end) to abort requests with finite wait

           1: alloc   +--------+   free                \
         +------------   free   <------------------+    |
         |            +--------+                   |    |
         |                                         |     > mail[N]
         |  2: put    +--------+     3a: get       |    |
         | +---------> waiting  -----------+       |    |
         | |          +--------+           |       |   /
         | |                               v       |
         | |          +---------+  3b  +-------+   |
         | |           suspended <---->|  DDD  +-->+     list
         v |          +---------+      +---+---+   |
       +---+-----+                         |       |
   ----> API ini |                         |       |
       +---------+                         |       |
                                           |       |
       +----------+                        |       |
   ----> API ris  |                        |       |
       |          |					       |       |
       | t.o.     |               4: reply |       |
       | ---+  +---------------------------+       |
       |    |  |  |                                |
       |    v  v  |                                |
       |   ------ |                                |    message[1]
       |    |  |  |                                |
       |   err |  | 5: free                        |
       |       +-----------------------------------+
       +----------+

       +---------+
   ----> API end | abort
       +---------+


        DDD must trace requests, e.g.:
            struct DDD_OP {
                void * x ;
            } ;
            typedef struct DDD_OP * DDD_OP ;
*/

// You are not supposed to know this ...
struct S_DRV_REQ ;
// ... but you must feel confortable with it
typedef struct S_DRV_REQ S_DRV_REQ ;

// api and driver must access request's data
// (allocated from the pool passed to DRV_begin)
void * DRV_data(S_DRV_REQ * /*pR*/) ;

typedef struct {
    // Driver must choose a signal and init this member
    uint32_t signal ;

    // Following fields are private and generic
    // Mainly used to assert that DRV_?? are correctly used
    // Initialised by DRV_iniz
    osThreadId drvTHD ;
    // Free and waiting requests
    osMailQId req ;
    // Make an educated guess
    struct list_head suspended ;
} S_DRIVER ;

// Must be invoked by the driver in its thread
// The pool must have space for nr requests
bool DRV_begin(S_DRIVER * /*pDrv*/, osPoolId /*mem*/, int nr) ;

// Driver must call this function to clean up aborted requests
void DRV_clean(S_DRIVER * /*pDrv*/) ;

// Driver must call this function to free suspended and waiting requests
// at the end of activity (DRV_FREE_CB is not called)
void DRV_free_all(S_DRIVER * /*pDrv*/) ;

typedef void (*DRV_FREE_CB)(void * /* request data */) ;

// Invoked by api_ini to alloc an available request
// In case of problems it returns NULL
// The callback (optional) will be called when the request will be freed
S_DRV_REQ * DRV_alloc_wcb(S_DRIVER * /*pDrv*/, DRV_FREE_CB /*free_cb*/) ;
// Shortcut
static inline S_DRV_REQ * DRV_alloc(S_DRIVER * pDrv)
{
    return DRV_alloc_wcb(pDrv, NULL) ;
}

// Invoked by api_ini to send the request
bool DRV_send(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;

// Invoked by api_ris to wait for the outcome
bool DRV_outcome(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/,
    size_t milli) ;

// Invoked by api_end to cancel a request
void DRV_cancel(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;

// When the signal is fired, driver's thread must call this function
// to start servicing the requests
// Return NULL if no request is pending
S_DRV_REQ * DRV_receive(
    S_DRIVER * /*pDrv*/,
    size_t milli) ;

// The driver call this function to close the request
void DRV_reply(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;

// api_ris must free the request
void DRV_free(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;

// When a request must wait for something to happen,
// driver must park it ...
void DRV_suspend(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;
// ... eventually something happens, so the driver
// must examine the suspended requests
S_DRV_REQ * DRV_first_susp(S_DRIVER * /*pDrv*/) ;
S_DRV_REQ * DRV_next_susp(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;
// ... and when it find the request that can be completed,
// it must remove the request from the suspended list
void DRV_remove_susp(
    S_DRIVER * /*pDrv*/,
    S_DRV_REQ * /*pR*/) ;

#endif
