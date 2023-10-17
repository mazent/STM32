#ifndef LIB_LWIP_PORT_COD_PRIV_H_
#define LIB_LWIP_PORT_COD_PRIV_H_

#include "drv.h"
#include "richieste.h"
#include "lwip/inet.h"

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

void libera_pbuf(UN_SOCK * pS) ;

S_DRV_REQ * remove_suspended(
    IO_REQ_TYPE tipo,
    int sok) ;

#define remove_susp_recvf(s)   remove_suspended(RT_RECVFROM, s)
#define remove_susp_accept(s)  remove_suspended(RT_ACCEPT, s)
#define remove_susp_recv(s)    remove_suspended(RT_RECV, s)
#define remove_susp_send(s)    remove_suspended(RT_SEND, s)
#define remove_susp_connect(s) remove_suspended(RT_CONNECT, s)

void reply_suspended(
    IO_REQ_TYPE tipo,
    int sok,
    uint32_t res) ;

#define error_susp_recvfrom(s)  reply_suspended(RT_RECVFROM, s, NET_RISUL_ERROR)
#define error_susp_sendto(s)    reply_suspended(RT_SENDTO, s, NET_RISUL_ERROR)
#define error_susp_recv(s)      reply_suspended(RT_RECV, s, 0)
#define error_susp_send(s)      reply_suspended(RT_SEND, s, NET_RISUL_ERROR)
#define error_susp_accept(s)    reply_suspended(RT_ACCEPT, s, NET_RISUL_ERROR)

/*
 * Un socket e' l'indice dell'elemento che contiene il pcb
 *
 * Il socket remoto che si e' connesso al socket i-esimo
 * verra' identificato con un bit
 *     rem = i | ID_SOCK_REM
 */
#define ID_SOCK_REM_SHIFT   4
#define ID_SOCK_REM         (1 << ID_SOCK_REM_SHIFT)

int sok_udp(int sok) ;
void rt_sendto(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;
void rt_recvfrom(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;

int sok_tcp(int sok) ;
void rt_send(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;
void rt_recv(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;
void rt_accept(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;
void rt_listen(
    S_DRV_REQ * pR,
    S_REQUEST_DATA * rd) ;

void esegui_select(
    int sok,
    IO_REQ_TYPE rt) ;

#else
#   warning priv.h incluso
#endif
