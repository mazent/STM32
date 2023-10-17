//#define STAMPA_DBG
#include "utili.h"
#include "priv.h"
#include "lwip/udp.h"

#if LWIP_UDP

//#define DIARIO_LIV_DBG
#include "stampe.h"

extern UN_SOCK vLoc[LWIP_P_NUM_SOCK] ;
extern S_DRIVER drv ;

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

    esegui_select(pS->idx, RT_RECVFROM) ;
}

int sok_udp(int sok)
{
    struct udp_pcb * pcb = udp_new_ip_type(IPADDR_TYPE_ANY) ;
    if ( NULL == pcb ) {
        DBG_ERR ;
        return -1 ;
    }

    vLoc[sok].tcp = false ;
    vLoc[sok].pcb = pcb ;
    udp_recv(pcb, udp_rx_cb, &vLoc[sok]) ;
    return sok ;
}

void rt_recvfrom(
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
        if ( vLoc[sok].tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        if ( NULL == vLoc[sok].p ) {
            DRV_suspend(&drv, pR) ;
            break ;
        }
        // ci sono dati!
        rd->result = copia_recvfrom(rd) ;
        DRV_reply(&drv, pR) ;
    } while ( false ) ;
}

void rt_sendto(
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
        if ( vLoc[sok].tcp ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        struct pbuf * p = pbuf_alloc(PBUF_TRANSPORT,
                                     rd->sendto.len,
                                     PBUF_RAM) ;
        if ( NULL == p ) {
            DBG_ERR ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }
        err_t e = pbuf_take(p, rd->sendto.buf, rd->sendto.len) ;
        if ( ERR_OK != e ) {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
            DRV_reply(&drv, pR) ;
            break ;
        }

        // nel pcb sono in host order
        uint16_t porta = ntohs(rd->sendto.ind.porta) ;

//        DBG_PRINTF("[%d] sendto %s:%04X", p->tot_len,
//                   ip4addr_ntoa(
//                       (ip_addr_t const *) &rd->sendto.ind.ip), porta) ;
        e = udp_sendto(vLoc[sok].pcb,
                       p,
                       (ip_addr_t const *) &rd->sendto.ind.ip, porta) ;
        if ( ERR_OK == e ) {
            rd->result = rd->sendto.len ;
        }
        else {
            DBG_ERR ;
            DBG_PUTS( lwip_strerr(e) ) ;
            rd->result = NET_RISUL_ERROR ;
        }
        pbuf_free(p) ;
        DRV_reply(&drv, pR) ;
    } while ( false ) ;
}

#endif
