//#define STAMPA_DBG
#include "utili.h"
#include "priv.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

extern bool is_btp_running(void) ;

#define DBG_REQ         0

#define MAX_RIC     5

//#define DIARIO_LIV_DBG
#include "stampe.h"

#if LWIP_TCP + LWIP_UDP

osPoolDef(ric, MAX_RIC, S_REQUEST_DATA) ;
static osPoolId ric = NULL ;

S_DRIVER drv = {
    .signal = 0
} ;

// creati localmente (tcp e/o udp)
UN_SOCK vLoc[LWIP_P_NUM_SOCK] = {
    0
} ;

// creati a seguito di connessione (tcp)
UN_SOCK vRem[LWIP_P_NUM_SOCK] = {
    0
} ;

#endif

void netop_iniz(uint32_t segnale)
{
#if LWIP_TCP + LWIP_UDP
    static_assert(LWIP_P_NUM_SOCK < ID_SOCK_REM, "OKKIO") ;

    if ( NULL == ric ) {
        ric = osPoolCreate( osPool(ric) ) ;
        ASSERT(ric) ;
    }

    for ( int i = 0 ; i < LWIP_P_NUM_SOCK ; ++i ) {
        vLoc[i].idx = i ;
        vRem[i].idx = i ;
        vRem[i].tcp = true ;
    }

    drv.signal = segnale ;
    CONTROLLA( DRV_begin(&drv, ric, MAX_RIC) ) ;
#else
    INUTILE(segnale) ;
#endif
}

#if LWIP_TCP + LWIP_UDP
static int uno_libero(void)
{
    int i = 0 ;
    for ( ; i < LWIP_P_NUM_SOCK ; ++i ) {
        if ( NULL == vLoc[i].pcb ) {
            break ;
        }
    }
    return i == LWIP_P_NUM_SOCK ? -1 : i ;
}

void libera_pbuf(UN_SOCK * pS)
{
    if ( pS->p ) {
        pbuf_free(pS->p) ;
        pS->p = NULL ;
    }
}

#endif

#if LWIP_TCP + LWIP_UDP
S_DRV_REQ * remove_suspended(
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

#endif

#if LWIP_TCP + LWIP_UDP
void reply_suspended(
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

#endif

#if LWIP_TCP + LWIP_UDP

static void rt_socket(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int tmp = uno_libero() ;
    if ( -1 == tmp ) {
        DBG_ERR ;
        rd->result = NET_RISUL_ERROR ;
    }
    else if ( rd->tcp ) {
#if LWIP_TCP
        rd->result = sok_tcp(tmp) ;
#else
        rd->result = NET_RISUL_ERROR ;
#endif
    }
    else {
#if LWIP_UDP
        rd->result = sok_udp(tmp) ;
#else
        rd->result = NET_RISUL_ERROR ;
#endif
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
#if LWIP_TCP
        err_t e = tcp_close(pS->pcb) ;
        if ( ERR_OK == e ) {
            pS->pcb = NULL ;
            libera_pbuf(pS) ;

            error_susp_recv(rd->sok) ;
            error_susp_send(rd->sok) ;
            error_susp_accept(rd->sok) ;
        }
        else {
            DBG_ERR ;
        }
#endif
    }
    else {
#if LWIP_UDP
        udp_remove(pS->pcb) ;

        pS->pcb = NULL ;
        libera_pbuf(pS) ;

        error_susp_recvfrom(rd->sok) ;
        error_susp_sendto(rd->sok) ;
#endif
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
        int rem = sok & ~ID_SOCK_REM ;
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

    for ( int r = 0 ; r < LWIP_P_NUM_SOCK ; r++ ) {
        rd.sok = r | ID_SOCK_REM ;
        rt_close_x(vRem + r, NULL, &rd) ;
    }

    for ( int l = 0 ; l < LWIP_P_NUM_SOCK ; l++ ) {
        rd.sok = l ;
        rt_close_x(vLoc + l, NULL, &rd) ;
    }

    DRV_free_all(&drv) ;
}

#endif

#if LWIP_TCP
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
        DBG_ERR ;
        return ERR_MEM ;
    }

    S_REQUEST_DATA * rd = DRV_data(pR) ;
    rd->result = 0 ;
    DRV_reply(&drv, pR) ;

    return ERR_OK ;
}

#endif

#if LWIP_TCP + LWIP_UDP
static void rt_connect(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    if ( NULL == vLoc[sok].pcb ) {
        DBG_ERR ;
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
#if LWIP_TCP
        tcp_arg(vLoc[sok].pcb, &vLoc[sok]) ;
        err_t e = tcp_connect(vLoc[sok].pcb,
                              &ipaddr,
                              porta,
                              tcp_connected_cb) ;
        if ( ERR_OK != e ) {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
        }
        else {
            DRV_suspend(&drv, pR) ;
        }
#else
        DBG_ERR ;
        rd->result = NET_RISUL_ERROR ;
        DRV_reply(&drv, pR) ;
#endif
    }
    else {
#if LWIP_UDP
        err_t e = udp_connect(vLoc[sok].pcb, &ipaddr, porta) ;
        if ( ERR_OK != e ) {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        else {
            rd->result = 0 ;
        }
        DRV_reply(&drv, pR) ;
#else
        DBG_ERR ;
        rd->result = NET_RISUL_ERROR ;
        DRV_reply(&drv, pR) ;
#endif
    }
}

static void rt_bind(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int sok = rd->sok ;
    if ( NULL == vLoc[sok].pcb ) {
        DBG_ERR ;
        rd->result = NET_RISUL_ERROR ;
    }
    else {
        err_t e = ERR_ARG ;

        // nel pcb sono in host order
        uint16_t porta = ntohs(rd->bind.porta) ;

        if ( vLoc[sok].tcp ) {
#if LWIP_TCP
            e = tcp_bind(vLoc[sok].pcb, NULL, porta) ;
#endif
        }
        else {
#if LWIP_UDP
            e = udp_bind(vLoc[sok].pcb, NULL, porta) ;
#endif
        }

        if ( ERR_OK == e ) {
            rd->result = 0 ;
        }
        else {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(e) ) ;
        }
    }

    DRV_reply(&drv, pR) ;
}

#endif

#if LWIP_TCP + LWIP_UDP

typedef bool (*PF_SELECT)(int sok) ;

static bool leggibile(int sok)
{
    UN_SOCK * pS = NULL ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & ~ID_SOCK_REM ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    return pS->p != NULL ;
}

static bool scrivibile(int sok)
{
    UN_SOCK * pS = NULL ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & ~ID_SOCK_REM ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    if ( NULL == pS->pcb ) {
        return false ;
    }

#if LWIP_TCP + LWIP_UDP == 2
    if ( !pS->tcp ) {
        return true ;
    }

    return tcp_sndbuf( (struct tcp_pcb *) pS->pcb ) > 0 ;
#elif LWIP_UDP
    return true ;
#else
    return tcp_sndbuf( (struct tcp_pcb *) pS->pcb ) > 0 ;
#endif
}

static bool errorabile(int sok)
{
    UN_SOCK * pS = NULL ;

    if ( sok & ID_SOCK_REM ) {
        int rem = sok & ~ID_SOCK_REM ;
        pS = vRem + rem ;
    }
    else {
        pS = vLoc + sok ;
    }

    return NULL == pS->pcb ;
}

static int select_qlc(
    sok_set * set,
    PF_SELECT qlc)
{
    int quanti = 0 ;
    uint32_t cset = *set ;
    uint32_t ris = 0 ;

    while ( true ) {
        uint32_t clz = __CLZ(cset) ;
        if ( 32 == clz ) {
            break ;
        }
        int sok = 31 - clz ;
        cset &= NEGA(1 << sok) ;
        if ( qlc(sok) ) {
            quanti++ ;
            ris |= 1 << sok ;
        }
    }

    if ( quanti ) {
        *set = ris ;
    }

    return quanti ;
}

static int select_read(sok_set * set)
{
    return select_qlc(set, leggibile) ;
}

static int select_write(sok_set * set)
{
    return select_qlc(set, scrivibile) ;
}

static int select_error(sok_set * set)
{
    return select_qlc(set, errorabile) ;
}

static void rt_select(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd)
{
    int conta_r = 0 ;
    int conta_w = 0 ;
    int conta_e = 0 ;
    int conta = 0 ;

    if ( rd->select.readfds ) {
        conta_r = select_read(rd->select.readfds) ;
        conta += conta_r ;
    }
    if ( rd->select.writefds ) {
        conta_w = select_write(rd->select.writefds) ;
        conta += conta_w ;
    }
    if ( rd->select.exceptfds ) {
        conta_e = select_error(rd->select.exceptfds) ;
        conta += conta_e ;
    }

    if ( conta ) {
        rd->result = conta ;
        if ( rd->select.readfds && (0 == conta_r) ) {
            *rd->select.readfds = 0 ;
        }
        if ( rd->select.writefds && (0 == conta_w) ) {
            *rd->select.writefds = 0 ;
        }
        if ( rd->select.exceptfds && (0 == conta_e) ) {
            *rd->select.exceptfds = 0 ;
        }

        DRV_reply(&drv, pR) ;
    }
    else {
        DRV_suspend(&drv, pR) ;
    }
}

static bool questa_select(
    int sok,
    IO_REQ_TYPE rt,
    S_REQUEST_DATA * rd)
{
    sok_set * set = NULL ;

    switch ( rt ) {
    case RT_RECV:
    case RT_RECVFROM:
        set = rd->select.readfds ;
        break ;
    case RT_SELECT:
        set = rd->select.exceptfds ;
        break ;
    case RT_SEND:
        set = rd->select.writefds ;
        break ;
    default:
        break ;
    }

    if ( NULL == set ) {
        return false ;
    }

    return (1 << sok) == ( *set & (1 << sok) ) ;
}

void esegui_select(
    int sok,
    IO_REQ_TYPE rt)
{
    S_DRV_REQ * pR = DRV_first_susp(&drv) ;

    while ( pR ) {
        S_REQUEST_DATA * rd = DRV_data(pR) ;
        if ( RT_SELECT != rd->type ) {
            // alla prossima
        }
        else if ( questa_select(sok, rt, rd) ) {
            DRV_remove_susp(&drv, pR) ;

            rd->result = 1 ;

            switch ( rt ) {
            case RT_SEND:
                *rd->select.writefds = 1 << sok ;
                if ( rd->select.readfds ) {
                    *rd->select.readfds = 0 ;
                }
                if ( rd->select.exceptfds ) {
                    *rd->select.exceptfds = 0 ;
                }
                break ;
            case RT_RECV:
            case RT_RECVFROM:
                *rd->select.readfds = 1 << sok ;
                if ( rd->select.writefds ) {
                    *rd->select.writefds = 0 ;
                }
                if ( rd->select.exceptfds ) {
                    *rd->select.exceptfds = 0 ;
                }
                break ;
            case RT_SELECT:
                // Questa serve per gli errori
                *rd->select.exceptfds = 1 << sok ;
                if ( rd->select.writefds ) {
                    *rd->select.writefds = 0 ;
                }
                if ( rd->select.readfds ) {
                    *rd->select.readfds = 0 ;
                }
                break ;
            }

            DRV_reply(&drv, pR) ;
            break ;
        }

        pR = DRV_next_susp(&drv, pR) ;
    }
}

#endif

#if LWIP_TCP + LWIP_UDP

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
            DBG_PUT(DBG_REQ, "RT_SOCKET") ;
            rt_socket(pR, rd) ;
            break ;
        case RT_CLOSE:
            DBG_PUT(DBG_REQ, "RT_CLOSE") ;
            rt_close(pR, rd) ;
            break ;
        case RT_CONNECT:
            DBG_PUT(DBG_REQ, "RT_CONNECT") ;
            rt_connect(pR, rd) ;
            break ;
        case RT_BIND:
            DBG_PUT(DBG_REQ, "RT_BIND") ;
            rt_bind(pR, rd) ;
            break ;
        case RT_SELECT:
            DBG_PUT(DBG_REQ, "RT_SELECT") ;
            rt_select(pR, rd) ;
            break ;

#if LWIP_TCP
        case RT_LISTEN:
            DBG_PUT(DBG_REQ, "RT_LISTEN") ;
            rt_listen(pR, rd) ;
            break ;
        case RT_ACCEPT:
            DBG_PUT(DBG_REQ, "RT_ACCEPT") ;
            rt_accept(pR, rd) ;
            break ;
        case RT_RECV:
            DBG_PUT(DBG_REQ, "RT_RECV") ;
            rt_recv(pR, rd) ;
            break ;
        case RT_SEND:
            DBG_PUT(DBG_REQ, "RT_SEND") ;
            rt_send(pR, rd) ;
            break ;
#endif
#if LWIP_UDP
        case RT_RECVFROM:
            DBG_PUT(DBG_REQ, "RT_RECVFROM") ;
            rt_recvfrom(pR, rd) ;
            break ;
        case RT_SENDTO:
            DBG_PUT(DBG_REQ, "RT_SENDTO") ;
            rt_sendto(pR, rd) ;
            break ;
#endif
        default:
            ASSERT(false) ;
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
        ASSERT(pOp) ;
        if ( NULL == pOp ) {
            break ;
        }

        // Get the request
        S_DRV_REQ * pR = (S_DRV_REQ *) pOp->x ;

        ASSERT(pR) ;
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
            case RT_SELECT:
                esito = rd->result ;
                break ;
            default:
                DBG_ERR ;
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & ~ID_SOCK_REM ;
            if ( (rem < 0) || (rem >= LWIP_P_NUM_SOCK) ) {
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

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

#endif

#if LWIP_UDP
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
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

#endif

#if LWIP_TCP
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
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
        if ( sok >= LWIP_P_NUM_SOCK ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

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
        if ( sok >= LWIP_P_NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & ~ID_SOCK_REM ;
            if ( (rem < 0) || (rem >= LWIP_P_NUM_SOCK) ) {
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

    DBG_FUN ;

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
        if ( sok >= LWIP_P_NUM_SOCK ) {
            // Potrebbe essere remoto
            int rem = sok & ~ID_SOCK_REM ;
            if ( (rem < 0) || (rem >= LWIP_P_NUM_SOCK) ) {
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

#endif

#if LWIP_TCP + LWIP_UDP

bool NET_select_ini(
    NET_OP op,
    int nfds,
    sok_set * readfds,
    sok_set * writefds,
    sok_set * exceptfds)
{
    bool esito = false ;

    DBG_FUN ;

    do {
        assert(op) ;
        if ( NULL == op ) {
            break ;
        }

        assert(NULL == op->x) ;
        if ( NULL != op->x ) {
            break ;
        }

        if ( nfds < 1 ) {
            break ;
        }
#if LWIP_TCP
        int max_sok = 1 + ID_SOCK_REM + LWIP_P_NUM_SOCK ;
#else
        int max_sok = 1 + LWIP_P_NUM_SOCK ;
#endif
        nfds = MINI(nfds, max_sok) ;

        uint32_t msk = (1 << nfds) - 1 ;
        if ( readfds ) {
            *readfds &= msk ;
        }
        if ( writefds ) {
            *writefds &= msk ;
        }
        if ( exceptfds ) {
            *exceptfds &= msk ;
        }

        if ( (NULL == readfds) && (NULL == writefds) && (NULL == exceptfds) ) {
            break ;
        }

        if ( !is_btp_running() ) {
            break ;
        }

        S_DRV_REQ * pR = DRV_alloc(&drv) ;
        if ( NULL == pR ) {
            break ;
        }

        S_REQUEST_DATA * rd = DRV_data(pR) ;

        rd->type = RT_SELECT ;

        rd->select.readfds = readfds ;
        rd->select.writefds = writefds ;
        rd->select.exceptfds = exceptfds ;

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

#endif

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
