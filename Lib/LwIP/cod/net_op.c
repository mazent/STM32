#include "utili.h"
//#define USA_DIARIO
#include "diario/diario.h"
#include "drv.h"
#include "richieste.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "net_mdma.h"

extern bool is_btp_running(void) ;

#define DBG_REQ         0

/*
 * Un socket e' l'indice dell'elemento che contiene il pcb
 *
 * Il socket remoto che si e' connesso al socket i-esimo
 * verra' identificato con un bit
 *     rem = i | ID_SOCK_REM
 */
#define ID_SOCK_REM_SHIFT   10
#define ID_SOCK_REM         (1 << ID_SOCK_REM_SHIFT)

#define MAX_RIC     5

osPoolDef(ric, MAX_RIC, S_REQUEST_DATA) ;
static osPoolId ric = NULL ;

static S_DRIVER drv = {
    .signal = 0
} ;

typedef struct {
    bool tcp ;

    int idx ;

    void * pcb ;

    // Ultimo ricevuto
    struct pbuf * p ;

    // UDP remoto (network order)
    ip_addr_t addr ;
    uint16_t port ;
} UN_SOCK ;

#define NUM_SOCK        4

// creati localmente (tcp e/o udp)
static UN_SOCK vLoc[NUM_SOCK] = {
    0
} ;

// creati a seguito di connessione (tcp)
static UN_SOCK vRem[NUM_SOCK] = {
    0
} ;

void netop_iniz(uint32_t segnale)
{
    if ( NULL == ric ) {
        ric = osPoolCreate( osPool(ric) ) ;
        DDB_ASSERT(ric) ;
    }

    for ( int i = 0 ; i < NUM_SOCK ; ++i ) {
        vLoc[i].idx = i ;
        vRem[i].idx = i ;
        vRem[i].tcp = true ;
    }

    drv.signal = segnale ;
    DDB_CONTROLLA( DRV_begin(&drv, ric, MAX_RIC) ) ;
}

static int uno_libero(void)
{
    int i = 0 ;
    for ( ; i < NUM_SOCK ; ++i ) {
        if ( NULL == vLoc[i].pcb ) {
            break ;
        }
    }
    return i == NUM_SOCK ? -1 : i ;
}

static S_DRV_REQ * remove_suspended(
    IO_REQ_TYPE tipo,
    int sok)
{
    S_DRV_REQ * pR = DRV_first_susp(&drv) ;

    while ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        if ( tipo != rd->type ) {}
        else if ( sok == rd->sok ) {
            DRV_remove_susp(&drv, pR) ;
            break ;
        }

        pR = DRV_next_susp(&drv, pR) ;
    }

    return pR ;
}

#define remove_susp_recvf(s)   remove_suspended(RT_RECVFROM, s)
#define remove_susp_accept(s)  remove_suspended(RT_ACCEPT, s)
#define remove_susp_recv(s)    remove_suspended(RT_RECV, s)
#define remove_susp_send(s)    remove_suspended(RT_SEND, s)
#define remove_susp_connect(s) remove_suspended(RT_CONNECT, s)

static void reply_suspended(
    IO_REQ_TYPE tipo,
    int sok,
    uint32_t res)
{
    S_DRV_REQ * pR = DRV_first_susp(&drv) ;
    while ( pR ) {
        S_DRV_REQ * pRR = DRV_next_susp(&drv, pR) ;

        S_REQUEST_DATA * rd = DRV_data(pR) ;
        if ( tipo != rd->type ) {}
        else if ( rd->sok == sok ) {
            DRV_remove_susp(&drv, pR) ;

            rd->result = res ;
            DRV_reply(&drv, pR) ;
        }

        pR = pRR ;
    }
}

#define error_susp_recvfrom(s)  reply_suspended(RT_RECVFROM, s, NET_RISUL_ERROR)
#define error_susp_sendto(s)    reply_suspended(RT_SENDTO, s, NET_RISUL_ERROR)
#define error_susp_recv(s)      reply_suspended(RT_RECV, s, 0)
#define error_susp_send(s)      reply_suspended(RT_SEND, s, NET_RISUL_ERROR)
#define error_susp_accept(s)    reply_suspended(RT_ACCEPT, s, NET_RISUL_ERROR)

#ifdef USA_MDMA

// copia net -> user in corso
static S_DRV_REQ * cnuCor = NULL ;
// copia user -> net in corso
static S_DRV_REQ * cunCor = NULL ;

//static bool copia_sendto(
//    S_REQUEST_DATA * rd,
//    struct pbuf * q)
//{
//    bool esito = true ;
//    int sok = rd->sendto.sok ;
//    uint8_t * buf = rd->sendto.buf ;
//    int len = rd->sendto.len ;
//    int dim = 0 ;
//    while ( dim < rd->sendto.len ) {
//        const int DIM = MINI(len, q->len) ;
//        if ( !net_mdma_tx_buf(buf, q->payload, DIM) ) {
//            esito = false ;
//            break ;
//        }
//        dim += DIM ;
//        q = q->next ;
//        if ( NULL == q ) {
//            break ;
//        }
//
//        buf += DIM ;
//        len -= DIM ;
//    }
//    if ( esito ) {
//        esito = net_mdma_tx() ;
//    }
//
//    return esito ;
//}

static bool copia_recvfrom(S_REQUEST_DATA * rd)
{
    bool esito = true ;
    int idx = rd->recvf.idx ;
    struct pbuf * q = vLoc[idx].p ;
    uint8_t * buf = rd->recvf.buf ;
    int len = rd->recvf.len ;
    int dim = 0 ;
    while ( dim < rd->recvf.len ) {
        const int DIM = MINI(len, q->len) ;
        if ( !net_mdma_rx_buf(buf, q->payload, DIM) ) {
            esito = false ;
            break ;
        }
        dim += DIM ;
        q = q->next ;
        if ( NULL == q ) {
            break ;
        }

        buf += DIM ;
        len -= DIM ;
    }
    if ( esito ) {
        rd->recvf.tot = dim ;
        if ( rd->recvf.ind ) {
            rd->recvf.ind->ip = vLoc[idx].addr.addr ;
            rd->recvf.ind->porta = vLoc[idx].port ;
        }
        esito = net_mdma_rx() ;
    }

    return esito ;
}

void netop_rx(bool esito)
{
    if ( NULL == cnuCor ) {
        DDB_ERR ;
        return ;
    }

    S_REQUEST_DATA * rd = DRV_data(cnuCor) ;
    pbuf_free(vLoc[rd->idx].p) ;
    vLoc[rd->idx].p = NULL ;

    if ( esito ) {
        rd->result = rd->recvf.tot ;
    }
    else {
        rd->result = NET_RISUL_ERROR ;
    }
    DRV_remove_susp(&drv, cnuCor) ;
    DRV_reply(&drv, cnuCor) ;
    cnuCor = NULL ;
}

void netop_tx(bool esito)
{
//    if ( NULL == cnuCor ) {
//        DDB_ERR ;
//        return ;
//    }
//
//    S_REQUEST_DATA * rd = DRV_data(cnuCor) ;
//    pbuf_free(vSock[rd->sok].p) ;
//    vSock[rd->sok].p = NULL ;
//
//    if ( esito ) {
//        rd->result = rd->recvf.tot ;
//    }
//    else {
//        rd->result = NET_RISUL_ERROR ;
//    }
//    DRV_remove_susp(&drv, cnuCor) ;
//    DRV_reply(&drv, cnuCor) ;
//    cnuCor = NULL ;
}

#else

static int copia_recvfrom(S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    struct pbuf * q = vLoc[sok].p ;
    uint8_t * buf = rd->recvf.buf ;
    int len = rd->recvf.len ;
    int dim = 0 ;
    while ( dim < rd->recvf.len ) {
        const int DIM = MINI(len, q->len) ;
        memcpy(buf, q->payload, DIM) ;
        dim += DIM ;
        q = q->next ;
        if ( NULL == q ) {
            break ;
        }

        buf += DIM ;
        len -= DIM ;
    }
    if ( rd->recvf.ind ) {
        rd->recvf.ind->ip = vLoc[sok].addr.addr ;
        rd->recvf.ind->porta = vLoc[sok].port ;
    }

    pbuf_free(vLoc[sok].p) ;
    vLoc[sok].p = NULL ;

    return dim ;
}

#endif

#ifdef USA_MDMA
static void udp_rx_cb(
    void * arg,
    struct udp_pcb * pcb,
    struct pbuf * p,
    const ip_addr_t * addr,
    uint16_t port)
{
    UN_SOCK * pUS = arg ;
    S_DRV_REQ * pR = NULL ;

    INUTILE(pcb) ;

    if ( pUS->p ) {
        pbuf_free(pUS->p) ;
    }

    pUS->p = p ;
    pUS->addr = *addr ;
    pUS->port = port ;

    pR = remove_susp_recvf(pUS->idx) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        DRV_suspend(&drv, pR) ;

        if ( net_mdma_rx_iniz() ) {
            cnuCor = pR ;
            DDB_CONTROLLA( copia_recvfrom(rd) ) ;
        }
        else {
            // alla prossima
            DDB_DBG ;
        }
    }
}

#else
static void udp_rx_cb(
    void * arg,
    struct udp_pcb * pcb,
    struct pbuf * p,
    const ip_addr_t * addr,
    uint16_t _port)
{
    UN_SOCK * pS = arg ;
    S_DRV_REQ * pR = NULL ;
    uint16_t port = htons(_port) ;

    INUTILE(pcb) ;

ancora:
    pR = remove_susp_recvf(pS->idx) ;
    if ( NULL == pR ) {
        if ( pS->p ) {
            pbuf_free(pS->p) ;
        }

        pS->p = p ;
        pS->addr = *addr ;
        pS->port = port ;
    }
    else {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        if ( pS->p ) {
            rd->result = copia_recvfrom(rd) ;
            DRV_reply(&drv, pR) ;
            goto ancora ;
        }

        pS->p = p ;
        pS->addr = *addr ;
        pS->port = port ;

        rd->result = copia_recvfrom(rd) ;
        DRV_reply(&drv, pR) ;
    }
}

#endif

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

static int sok_udp(int sok)
{
    struct udp_pcb * pcb = udp_new_ip_type(IPADDR_TYPE_ANY) ;
    if ( NULL == pcb ) {
        DDB_ERR ;
        return -1 ;
    }

    vLoc[sok].tcp = false ;
    vLoc[sok].pcb = pcb ;
    udp_recv(pcb, udp_rx_cb, &vLoc[sok]) ;
    return sok ;
}

static void libera_pbuf(UN_SOCK * pS)
{
    if ( pS->p ) {
        pbuf_free(pS->p) ;
        pS->p = NULL ;
    }
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
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        err = tcp_output(pcb) ;
        if ( ERR_OK != err ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        rd->send.parz += DIM ;
        DRV_suspend(&drv, pR) ;
    } while ( false ) ;
}

#define CHIUDI_SOK               \
    DDB_DBG ;                    \
    error_susp_recv(sok) ;       \
    error_susp_send(sok) ;       \
                                 \
    tcp_arg(tpcb, NULL) ;        \
    tcp_sent(tpcb, NULL) ;       \
    tcp_recv(tpcb, NULL) ;       \
    tcp_err(tpcb, NULL) ;        \
    tcp_poll(tpcb, NULL, 0) ;    \
                                 \
    libera_pbuf(pS) ;            \
    pS->pcb = NULL ;             \
                                 \
    tcp_close(tpcb)

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
//        DDB_DEBUG( "\t %d pbuf", pbuf_clen(pS->p) ) ;
//        DDB_DEBUG("\t %d byte", pS->p->tot_len) ;

    S_DRV_REQ * pR = remove_susp_recv(sok) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->result = copia_recv(pS, rd) ;
        DRV_reply(&drv, pR) ;
    }
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
        DDB_ERR ;
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

    //DDB_DEBUG("%d %s %d", pS->idx, __func__, err) ;

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
    DDB_DEBUG( "%d %s %d = %s", pS->idx, __func__, err, lwip_strerr(err) ) ;

    S_DRV_REQ * pR = remove_susp_connect(pS->idx) ;
    if ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        rd->result = NET_RISUL_ERROR ;
        DRV_reply(&drv, pR) ;
    }
    else {
        DDB_ERR ;
    }

    int sok = pS->idx ;
    error_susp_recv(sok) ;
    error_susp_send(sok) ;
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

static int sok_tcp(int sok)
{
    struct tcp_pcb * pcb = tcp_new() ;
    if ( NULL == pcb ) {
        DDB_ERR ;
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

static void rt_socket(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int tmp = uno_libero() ;
    if ( -1 == tmp ) {
        DDB_ERR ;
        rd->result = NET_RISUL_ERROR ;
    }
    else {
        if ( rd->tcp ) {
            rd->result = sok_tcp(tmp) ;
        }
        else {
            rd->result = sok_udp(tmp) ;
        }
    }

    DRV_reply(&drv, pR) ;
}

static void rt_close_x(
    UN_SOCK * pS,
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    if ( NULL == pS->pcb ) {
        // Ottimo
    }
    else if ( pS->tcp ) {
        err_t e = tcp_close(pS->pcb) ;
        if ( ERR_OK == e ) {
            pS->pcb = NULL ;
            libera_pbuf(pS) ;

            error_susp_recv(rd->sok) ;
            error_susp_send(rd->sok) ;
            error_susp_accept(rd->sok) ;
        }
        else {
            DDB_ERR ;
        }
    }
    else {
        udp_remove(pS->pcb) ;

        pS->pcb = NULL ;
        libera_pbuf(pS) ;

        error_susp_recvfrom(rd->sok) ;
        error_susp_sendto(rd->sok) ;
    }

    if ( pR ) {
        rd->result = 0 ;
        DRV_reply(&drv, pR) ;
    }
}

static void rt_close(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    UN_SOCK * pS = NULL ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & NEGA(ID_SOCK_REM) ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    rt_close_x(pS, pR, rd) ;
}

void netop_fine(void)
{
    S_REQUEST_DATA rd = {
        .sok = 0
    } ;

    for ( int r = 0 ; r < NUM_SOCK ; r++ ) {
        rd.sok = r | ID_SOCK_REM ;
        rt_close_x(vRem + r, NULL, &rd) ;
    }

    for ( int l = 0 ; l < NUM_SOCK ; l++ ) {
        rd.sok = l ;
        rt_close_x(vLoc + l, NULL, &rd) ;
    }

    DRV_free_all(&drv) ;
}

static err_t tcp_connected_cb(
    void * arg,
    struct tcp_pcb * tpcb,
    err_t err)
{
    UN_SOCK * pS = arg ;

    INUTILE(tpcb) ;
    INUTILE(err) ;

    S_DRV_REQ * pR = remove_susp_connect(pS->idx) ;
    if ( NULL == pR ) {
        DDB_ERR ;
        return ERR_MEM ;
    }

    S_REQUEST_DATA * rd = DRV_data(pR) ;
    rd->result = 0 ;
    DRV_reply(&drv, pR) ;

    return ERR_OK ;
}

static void rt_connect(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    if ( NULL == vLoc[sok].pcb ) {
        DDB_ERR ;
        rd->result = NET_RISUL_ERROR ;
        DRV_reply(&drv, pR) ;
        return ;
    }

    // nel pcb in host order
    uint16_t porta = ntohs(rd->connect.ind.porta) ;
    ip_addr_t ipaddr = {
        .addr = rd->connect.ind.ip
    } ;

    if ( vLoc[sok].tcp ) {
        tcp_arg(vLoc[sok].pcb, &vLoc[sok]) ;
        err_t e = tcp_connect(vLoc[sok].pcb,
                              &ipaddr,
                              porta,
                              tcp_connected_cb) ;
        if ( ERR_OK != e ) {
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
        }
        else {
            DRV_suspend(&drv, pR) ;
        }
    }
    else {
        err_t e = udp_connect(vLoc[sok].pcb, &ipaddr, porta) ;
        if ( ERR_OK != e ) {
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        else {
            rd->result = 0 ;
        }
        DRV_reply(&drv, pR) ;
    }
}

static void rt_bind(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    if ( NULL == vLoc[sok].pcb ) {
        DDB_ERR ;
        rd->result = NET_RISUL_ERROR ;
    }
    else {
        err_t e ;

        // nel pcb sono in host order
        uint16_t porta = ntohs(rd->bind.porta) ;

        if ( vLoc[sok].tcp ) {
            e = tcp_bind(vLoc[sok].pcb, NULL, porta) ;
        }
        else {
            e = udp_bind(vLoc[sok].pcb, NULL, porta) ;
        }

        if ( ERR_OK == e ) {
            rd->result = 0 ;
        }
        else {
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(e) ) ;
        }
    }

    DRV_reply(&drv, pR) ;
}

static void rt_recvfrom(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( vLoc[sok].tcp ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( NULL == vLoc[sok].p ) {
            DRV_suspend(&drv, pR) ;
            break ;
        }
        // ci sono dati!
#ifdef USA_MDMA
        DRV_suspend(&drv, pR) ;
        if ( net_mdma_rx_iniz() ) {
            // se riesce, la richiesta si conclude alla fine dell'mdma
            // altrimenti c'e' un errore irrecuperabile
            cnuCor = pR ;
            DDB_CONTROLLA( copia_recvfrom(rd) ) ;
        }
        else {
            // alla prossima
            DDB_DBG ;
        }
#else
        rd->result = copia_recvfrom(rd) ;
        DRV_reply(&drv, pR) ;
#endif
    } while ( false ) ;
}

static void rt_sendto(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( vLoc[sok].tcp ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        struct pbuf * p = pbuf_alloc(PBUF_TRANSPORT,
                                     rd->sendto.len,
                                     PBUF_RAM) ;
        if ( NULL == p ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
#ifdef USA_MDMA_
        if ( !net_mdma_tx_iniz() ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
        }
        else {
            cunCor = pR ;
            if ( copia_sendto(rd, p) ) {
                DRV_suspend(&drv, pR) ;
            }
            else {
                DDB_ERR ;
                rd->result = NET_RISUL_ERROR ;
                DRV_reply(&drv, pR) ;
            }
        }
#else
        err_t e = pbuf_take(p, rd->sendto.buf, rd->sendto.len) ;
        if ( ERR_OK != e ) {
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        // nel pcb sono in host order
        uint16_t porta = ntohs(rd->sendto.ind.porta) ;

        DDB_DEBUG("[%d] sendto %s:%04X", p->tot_len,
                   ip4addr_ntoa(
                       (ip_addr_t const *) &rd->sendto.ind.ip), porta) ;
        e = udp_sendto(vLoc[sok].pcb,
                       p,
                       (ip_addr_t const *) &rd->sendto.ind.ip, porta) ;
        if ( ERR_OK == e ) {
            rd->result = rd->sendto.len ;
        }
        else {
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        pbuf_free(p) ;
        DRV_reply(&drv, pR) ;
#endif
    } while ( false ) ;
}

static void rt_listen(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( !vLoc[sok].tcp ) {
            DDB_ERR ;
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
            DDB_ERR ;
            DDB_PUTS( lwip_strerr(err) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        DRV_reply(&drv, pR) ;
    } while ( false ) ;
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

    //DDB_DEBUG("%d %s %d", pS->idx, __func__, err) ;

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

    DDB_DEBUG( "%d %s %d = %s", pS->idx, __func__, err, lwip_strerr(err) ) ;

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

    //DDB_DEBUG("cb %d", _len) ;

    int sok = pS->idx | ID_SOCK_REM ;
    x_sent_cb(sok, _len, tpcb) ;

    return ERR_OK ;
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
        DDB_ERR ;
        return ERR_MEM ;
    }

    int rem = pS->idx ;

    vRem[rem].pcb = newpcb ;
    tcp_arg(newpcb, &vRem[rem]) ;
    tcp_recv(newpcb, rem_recv_cb) ;
    tcp_err(newpcb, rem_error_cb) ;
    tcp_sent(newpcb, rem_sent_cb) ;

    S_REQUEST_DATA * rd = DRV_data(pR) ;
    if ( rd->accept.ind ) {
        rd->accept.ind->ip = newpcb->remote_ip.addr ;
        rd->accept.ind->porta = htons(newpcb->remote_port) ;
    }
    rd->result = rem | ID_SOCK_REM ;
    DRV_reply(&drv, pR) ;

    return ERR_OK ;
}

static void rt_accept(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    do {
        if ( NULL == vLoc[sok].pcb ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        if ( !vLoc[sok].tcp ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        DRV_suspend(&drv, pR) ;

        tcp_arg(vLoc[sok].pcb, &vLoc[sok]) ;
        tcp_accept(vLoc[sok].pcb, tcp_accept_cb) ;
    } while ( false ) ;
}

static void rt_recv(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    UN_SOCK * pS = NULL ;

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
            DDB_ERR ;
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

static void rt_send(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    UN_SOCK * pS = NULL ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & NEGA(ID_SOCK_REM) ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    do {
        if ( NULL == pS->pcb ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( !pS->tcp ) {
            DDB_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        rd->send.parz = rd->send.cb = 0 ;
        trasmetti(pS->pcb, pR) ;
    } while ( false ) ;
}

void netop_ric(void)
{
    while ( true ) {
        S_DRV_REQ * pR = DRV_receive(&drv, 0) ;
        if ( NULL == pR ) {
            break ;
        }

        S_REQUEST_DATA * rd = DRV_data(pR) ;
        switch ( rd->type ) {
        case RT_SOCKET:
            DDB_PUT(DBG_REQ, "RT_SOCKET") ;
            rt_socket(pR, rd) ;
            break ;
        case RT_CLOSE:
            DDB_PUT(DBG_REQ, "RT_CLOSE") ;
            rt_close(pR, rd) ;
            break ;
        case RT_BIND:
            DDB_PUT(DBG_REQ, "RT_BIND") ;
            rt_bind(pR, rd) ;
            break ;
        case RT_RECVFROM:
            DDB_PUT(DBG_REQ, "RT_RECVFROM") ;
            rt_recvfrom(pR, rd) ;
            break ;
        case RT_SENDTO:
            DDB_PUT(DBG_REQ, "RT_SENDTO") ;
            rt_sendto(pR, rd) ;
            break ;
        case RT_LISTEN:
            DDB_PUT(DBG_REQ, "RT_LISTEN") ;
            rt_listen(pR, rd) ;
            break ;
        case RT_ACCEPT:
            DDB_PUT(DBG_REQ, "RT_ACCEPT") ;
            rt_accept(pR, rd) ;
            break ;
        case RT_RECV:
            DDB_PUT(DBG_REQ, "RT_RECV") ;
            rt_recv(pR, rd) ;
            break ;
        case RT_SEND:
            DDB_PUT(DBG_REQ, "RT_SEND") ;
            rt_send(pR, rd) ;
            break ;
        case RT_CONNECT:
            DDB_PUT(DBG_REQ, "RT_CONNECT") ;
            rt_connect(pR, rd) ;
            break ;
        default:
            DDB_ASSERT(false) ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
    }
}

void NET_abort(NET_OP op)
{
    if ( op ) {
        S_DRV_REQ * pR = (S_DRV_REQ *) op->x ;
        if ( pR ) {
            DRV_cancel(&drv, pR) ;

            op->x = NULL ;
        }
    }
}

uint32_t NET_risul(
    NET_OP pOp,
    uint32_t milli)
{
    uint32_t esito = NET_RISUL_ERROR ;

    do {
        DDB_ASSERT(pOp) ;
        if ( NULL == pOp ) {
            break ;
        }

        // Get the request
        S_DRV_REQ * pR = (S_DRV_REQ *) pOp->x ;

        DDB_ASSERT(pR) ;
        if ( NULL == pR ) {
            break ;
        }

        // Wait for the end
        if ( DRV_outcome(&drv, pR, milli) ) {
            // Get the data ...
            S_REQUEST_DATA * rd = DRV_data(pR) ;

            switch ( rd->type ) {
            case RT_SOCKET:
            case RT_CLOSE:
            case RT_BIND:
            case RT_RECVFROM:
            case RT_SENDTO:
            case RT_LISTEN:
            case RT_ACCEPT:
            case RT_RECV:
            case RT_SEND:
            case RT_CONNECT:
            	DDB_DEBUG("%s %08X",__func__,pR);
                esito = rd->result ;
                break ;
            default:
                DDB_ERR ;
                break ;
            }

            // Now we can free the request ...
            DRV_free(&drv, pR) ;

            // ... and empty the handle
            pOp->x = NULL ;
        }
        else {
            esito = NET_RISUL_TEMPO ;
        }
    } while ( false ) ;

    return esito ;
}

bool NET_socket_ini(
    NET_OP op,
    bool tcp)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_SOCKET ;
        rd->tcp = tcp ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_close_ini(
    NET_OP op,
    int sok)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & NEGA(ID_SOCK_REM) ;
            if ( (rem < 0) || (rem >= NUM_SOCK) ) {
                break ;
            }
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_CLOSE ;
        rd->sok = sok ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_connect_ini(
    NET_OP op,
    int sok,
    S_NET_IND * ind)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( NULL == ind ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_CONNECT ;
        rd->sok = sok ;
        rd->connect.ind = *ind ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_bind_ini(
    NET_OP op,
    int sok,
    uint16_t port)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_BIND ;
        rd->sok = sok ;
        rd->bind.porta = port ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_recvfrom_ini(
    NET_OP op,
    int sok,
    void * buf,
    int len,
    S_NET_IND * ind)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( NULL == buf ) {
            break ;
        }
        if ( 0 == len ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_RECVFROM ;
        rd->sok = sok ;
        rd->recvf.buf = buf ;
        rd->recvf.len = len ;
        rd->recvf.ind = ind ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_sendto_ini(
    NET_OP op,
    int sok,
    const void * buf,
    int len,
    S_NET_IND * ind)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( NULL == buf ) {
            break ;
        }
        if ( 0 == len ) {
            break ;
        }

        if ( NULL == ind ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_SENDTO ;
        rd->sok = sok ;
        rd->sendto.buf = buf ;
        rd->sendto.len = len ;
        rd->sendto.ind = *ind ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_listen_ini(
    NET_OP op,
    int sok,
    int backlog)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( backlog <= 0 ) {
            break ;
        }
        static_assert(1 == TCP_LISTEN_BACKLOG, "OKKIO") ;
        if ( backlog > TCP_LISTEN_BACKLOG ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_LISTEN ;
        rd->sok = sok ;
        rd->listen.backlog = backlog ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_accept_ini(
    NET_OP op,
    int sok,
    S_NET_IND * ind)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_ACCEPT ;
        rd->sok = sok ;
        rd->accept.ind = ind ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_recv_ini(
    NET_OP op,
    int sok,
    void * buf,
    int len)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & NEGA(ID_SOCK_REM) ;
            if ( (rem < 0) || (rem >= NUM_SOCK) ) {
                break ;
            }
        }

        if ( NULL == buf ) {
            break ;
        }
        if ( 0 == len ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_RECV ;
        rd->sok = sok ;
        rd->recv.buf = buf ;
        rd->recv.len = len ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool NET_send_ini(
    NET_OP op,
    int sok,
    const void * buf,
    int len)
{
    bool esito = false ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( sok < 0 ) {
            break ;
        }
        if ( sok >= NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & NEGA(ID_SOCK_REM) ;
            if ( (rem < 0) || (rem >= NUM_SOCK) ) {
                break ;
            }
        }

        if ( NULL == buf ) {
            break ;
        }
        if ( 0 == len ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        DDB_DEBUG("%s %08X",__func__,pR);

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_SEND ;
        rd->sok = sok ;
        rd->send.buf = buf ;
        rd->send.len = len ;

        op->x = pR ;

        if ( !DRV_send(&drv, pR) ) {
            op->x = NULL ;
            DRV_free(&drv, pR) ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

uint32_t NET_htonl(uint32_t x)
{
    return __REV(x) ;
}

uint16_t NET_htons(uint16_t x)
{
    return __REV16(x) ;
}

uint32_t NET_ntohl(uint32_t x)
{
    return __REV(x) ;
}

uint16_t NET_ntohs(uint16_t x)
{
    return __REV16(x) ;
}

char * NET_inet_ntoa(struct in_addr in)
{
    return ip4addr_ntoa( (const ip4_addr_t *) &in ) ;
}

int NET_inet_aton(
    const char * cp,
    struct in_addr * inp)
{
    return ip4addr_aton(cp, (ip4_addr_t *) inp) ;
}

in_addr_t NET_inet_addr(const char * cp)
{
    return ipaddr_addr(cp) ;
}
