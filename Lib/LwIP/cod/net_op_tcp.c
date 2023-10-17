//#define STAMPA_DBG
#include "utili.h"
#include "priv.h"
#include "lwip/tcp.h"

#if LWIP_TCP

//#define DIARIO_LIV_DBG
#include "stampe.h"

extern UN_SOCK vLoc[LWIP_P_NUM_SOCK] ;
extern UN_SOCK vRem[LWIP_P_NUM_SOCK] ;
extern S_DRIVER drv ;

#define CHIUDI_SOK                  \
    error_susp_recv(sok) ;          \
    error_susp_send(sok) ;          \
    esegui_select(sok, RT_SELECT) ; \
                                    \
    tcp_arg(tpcb, NULL) ;           \
    tcp_sent(tpcb, NULL) ;          \
    tcp_recv(tpcb, NULL) ;          \
    tcp_err(tpcb, NULL) ;           \
    tcp_poll(tpcb, NULL, 0) ;       \
                                    \
    libera_pbuf(pS) ;               \
    pS->pcb = NULL ;                \
                                    \
    tcp_close(tpcb)

// non mi piace la versione originale
static struct pbuf * port_pbuf_dechain(struct pbuf * p)
{
    struct pbuf * q = p->next ;

    if ( q != NULL ) {
        q->tot_len = (uint16_t) (p->tot_len - p->len) ;
        p->next = NULL ;
        p->tot_len = p->len ;
    }

    return q ;
}

static int copia_recv(
    UN_SOCK * pS,
    S_REQUEST_DATA * rd)
{
    struct pbuf * q = pS->p ;
    int len = rd->recv.len ;
    uint8_t * buf = rd->recv.buf ;
    int dim = 0 ;
    while ( len >= q->len ) {
        pS->p = port_pbuf_dechain(q) ;
        memcpy(buf, q->payload, q->len) ;
        dim += q->len ;
        buf += q->len ;
        len -= q->len ;
        pbuf_free(q) ;
        if ( NULL == pS->p ) {
            break ;
        }
        // prox
        q = pS->p ;
    }

    if ( dim ) {
        tcp_recved(pS->pcb, dim) ;
    }

    return dim ;
}

static void trasmetti(
    struct tcp_pcb * pcb,
    S_DRV_REQ * pR)
{
    S_REQUEST_DATA * rd = DRV_data(pR) ;
    const uint8_t * buf = rd->send.buf ;
    buf += rd->send.parz ;
    int len = rd->send.len - rd->send.parz ;
    const int TCP_SNDBUF = tcp_sndbuf(pcb) ;
    do {
        const int DIM = MINI(TCP_SNDBUF, len) ;
        DBG_PRINTF("%s %d", __func__, DIM) ;
        if ( 0 == DIM ) {
            // stara' trasmettendo: aspetto la sent_cb
            DRV_suspend(&drv, pR) ;
            break ;
        }

        err_t err = tcp_write(pcb,
                              buf,
                              DIM,
                              TCP_WRITE_FLAG_COPY) ;
        if ( ERR_OK != err ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        err = tcp_output(pcb) ;
        if ( ERR_OK != err ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        rd->send.parz += DIM ;
        DRV_suspend(&drv, pR) ;
    } while ( false ) ;
}

static void x_recv_cb(
    UN_SOCK * pS,
    struct pbuf * p,
    int sok)
{
    if ( pS->p ) {
        // accodo
        pbuf_cat(pS->p, p) ;
    }
    else {
        pS->p = p ;
    }

    S_DRV_REQ * pR = remove_susp_recv(sok) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->result = copia_recv(pS, rd) ;
        DRV_reply(&drv, pR) ;
    }

    esegui_select(sok, RT_RECV) ;
}

static void x_sent_cb(
    int sok,
    uint16_t len,
    struct tcp_pcb * tpcb)
{
    S_DRV_REQ * pR = remove_susp_send(sok) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->send.cb += len ;
        if ( rd->send.cb == rd->send.parz ) {
            if ( rd->send.parz == rd->send.len ) {
                rd->result = rd->send.len ;
                DRV_reply(&drv, pR) ;
                esegui_select(sok, RT_SEND) ;
            }
            else {
                trasmetti(tpcb, pR) ;
            }
        }
        else {
            DRV_suspend(&drv, pR) ;
        }
    }
    else {
        esegui_select(sok, RT_SEND) ;
    }
}

static err_t
loc_recv_cb(
    void * arg,
    struct tcp_pcb * tpcb,
    struct pbuf * p,
    err_t err)
{
    err_t ret_err = ERR_OK ;
    UN_SOCK * pS = arg ;

    //DBG_PRINTF("%d %s %d", pS->idx, __func__, err) ;

    int sok = pS->idx ;

    if ( p == NULL ) {
        // the connection has been closed
        CHIUDI_SOK ;
    }
    else if ( err != ERR_OK ) {
        /* cleanup, for unknown reason */
        CHIUDI_SOK ;
        ret_err = err ;
    }
    else {
        // passare i dati
        x_recv_cb(pS, p, sok) ;
    }

    return ret_err ;
}

static void
loc_error_cb(
    void * arg,
    err_t err)
{
    UN_SOCK * pS = arg ;

    INUTILE(err) ;
    DBG_PRINTF( "%d %s %d = %s", pS->idx, __func__, err, lwip_strerr(err) ) ;

    S_DRV_REQ * pR = remove_susp_connect(pS->idx) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        rd->result = NET_RISUL_ERROR ;
        DRV_reply(&drv, pR) ;
    }
    else {
        DBG_ERR ;
    }

    int sok = pS->idx ;
    error_susp_recv(sok) ;
    error_susp_send(sok) ;
    esegui_select(sok, RT_SELECT) ;
    pS->pcb = NULL ;
}

static err_t
loc_sent_cb(
    void * arg,
    struct tcp_pcb * tpcb,
    uint16_t len)
{
    UN_SOCK * pS = arg ;

    INUTILE(tpcb) ;
    INUTILE(len) ;

    int sok = pS->idx ;
    x_sent_cb(sok, len, tpcb) ;

    return ERR_OK ;
}

static err_t
rem_recv_cb(
    void * arg,
    struct tcp_pcb * tpcb,
    struct pbuf * p,
    err_t err)
{
    err_t ret_err = ERR_OK ;
    UN_SOCK * pS = arg ;

    INUTILE(tpcb) ;

    //DBG_PRINTF("%d %s %d", pS->idx, __func__, err) ;

    int sok = pS->idx | ID_SOCK_REM ;

    if ( p == NULL ) {
        /* remote host closed connection */
        CHIUDI_SOK ;
    }
    else if ( err != ERR_OK ) {
        /* cleanup, for unknown reason */
        CHIUDI_SOK ;
        ret_err = err ;
    }
    else {
        x_recv_cb(pS, p, sok) ;
    }

    return ret_err ;
}

static void
rem_error_cb(
    void * arg,
    err_t err)
{
    UN_SOCK * pS = arg ;

    INUTILE(err) ;

    DBG_PRINTF( "%d %s %d = %s", pS->idx, __func__, err, lwip_strerr(err) ) ;

    libera_pbuf(pS) ;

    int sok = pS->idx | ID_SOCK_REM ;
    error_susp_recv(sok) ;
    error_susp_send(sok) ;
    pS->pcb = NULL ;
}

static err_t
rem_sent_cb(
    void * arg,
    struct tcp_pcb * tpcb,
    uint16_t _len)
{
    UN_SOCK * pS = arg ;

    INUTILE(_len) ;

    DBG_PRINTF("%s %d", __func__, _len) ;

    int sok = pS->idx | ID_SOCK_REM ;
    x_sent_cb(sok, _len, tpcb) ;

    return ERR_OK ;
}

int sok_tcp(int sok)
{
    struct tcp_pcb * pcb = tcp_new() ;
    if ( NULL == pcb ) {
        DBG_ERR ;
        return -1 ;
    }
    vLoc[sok].tcp = true ;
    vLoc[sok].pcb = pcb ;

    tcp_arg(pcb, &vLoc[sok]) ;
    tcp_recv(pcb, loc_recv_cb) ;
    tcp_err(pcb, loc_error_cb) ;
    tcp_sent(pcb, loc_sent_cb) ;

    return sok ;
}

static err_t tcp_accept_cb(
    void * arg,
    struct tcp_pcb * newpcb,
    err_t err)
{
    if ( (err != ERR_OK) || (newpcb == NULL) ) {
        return ERR_VAL ;
    }

    UN_SOCK * pS = arg ;
    S_DRV_REQ * pR = remove_susp_accept(pS->idx) ;
    if ( NULL == pR ) {
        DBG_ERR ;
        return ERR_MEM ;
    }

    int rem = pS->idx ;

    vRem[rem].pcb = newpcb ;
    tcp_arg(newpcb, &vRem[rem]) ;
    tcp_recv(newpcb, rem_recv_cb) ;
    tcp_err(newpcb, rem_error_cb) ;
    tcp_sent(newpcb, rem_sent_cb) ;
    //tcp_nagle_disable(newpcb);

    S_REQUEST_DATA * rd = DRV_data(pR) ;
    if ( rd->accept.ind ) {
        rd->accept.ind->ip = newpcb->remote_ip.addr ;
        rd->accept.ind->porta = htons(newpcb->remote_port) ;
    }
    rd->result = rem | ID_SOCK_REM ;
    DRV_reply(&drv, pR) ;

    return ERR_OK ;
}

void rt_listen(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( !vLoc[sok].tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        err_t err ;
        struct tcp_pcb * nuovo = tcp_listen_with_backlog_and_err(
            vLoc[sok].pcb,
            rd->listen.
            backlog,
            &err) ;
        if ( nuovo ) {
            vLoc[sok].pcb = nuovo ;
            rd->result = 0 ;
        }
        else {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(err) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        DRV_reply(&drv, pR) ;
    } while ( false ) ;
}

void rt_accept(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( !vLoc[sok].tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        DRV_suspend(&drv, pR) ;

        tcp_arg(vLoc[sok].pcb, &vLoc[sok]) ;
        tcp_accept(vLoc[sok].pcb, tcp_accept_cb) ;
    } while ( false ) ;
}

void rt_recv(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    UN_SOCK * pS = NULL ;

    DBG_FUN ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & NEGA(ID_SOCK_REM) ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    do {
        if ( NULL == pS->pcb ) {
            // chiuso dal client?
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( !pS->tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( NULL == pS->p ) {
            DRV_suspend(&drv, pR) ;
            break ;
        }

        // ci sono dati!
        rd->result = copia_recv(pS, rd) ;
        DRV_reply(&drv, pR) ;
    } while ( false ) ;
}

void rt_send(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    UN_SOCK * pS = NULL ;

    DBG_FUN ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & NEGA(ID_SOCK_REM) ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    do {
        if ( NULL == pS->pcb ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( !pS->tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        rd->send.parz = rd->send.cb = 0 ;
        trasmetti(pS->pcb, pR) ;
    } while ( false ) ;
}

#endif
