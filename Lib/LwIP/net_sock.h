#ifndef MIDDLEWARES_THIRD_PARTY_LWIP_PORT_NET_SOCK_H_
#define MIDDLEWARES_THIRD_PARTY_LWIP_PORT_NET_SOCK_H_

// vedi https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html

#define ESEMPI_NET
#include "net_if.h"

#define INUTILE(x)          (void) ( sizeof(x) )

#define AF_INET         2
#define PF_INET         AF_INET

#define SOCK_STREAM     1
#define SOCK_DGRAM      2

#define INADDR_ANY          ( (uint32_t) 0 )

struct in_addr {
    uint32_t s_addr ;
} ;

typedef uint32_t in_addr_t ;

struct sockaddr_in {
    uint8_t sin_len ;
    uint8_t sin_family ;
    uint16_t sin_port ;
    struct in_addr sin_addr ;
#define SIN_ZERO_LEN 8
    char sin_zero[SIN_ZERO_LEN] ;
} ;

char * NET_inet_ntoa(struct in_addr in) ;
int NET_inet_aton(
    const char * cp,
    struct in_addr * inp) ;
in_addr_t NET_inet_addr(const char * cp) ;

#define inet_ntoa       NET_inet_ntoa
#define inet_aton       NET_inet_aton
#define inet_addr       NET_inet_addr

uint32_t NET_htonl(uint32_t) ;
uint16_t NET_htons(uint16_t) ;
uint32_t NET_ntohl(uint32_t) ;
uint16_t NET_ntohs(uint16_t) ;

#define htonl    NET_htonl
#define htons    NET_htons
#define ntohl    NET_ntohl
#define ntohs    NET_ntohs

static inline int socket(
    int domain,
    int type,
    int protocol)
{
    int sok = -1 ;

    INUTILE(protocol) ;

    do {
        if ( domain != PF_INET ) {
            break ;
        }

        bool tcp ;
        if ( SOCK_STREAM == type ) {
            tcp = true ;
        }
        else if ( SOCK_DGRAM == type ) {
            tcp = false ;
        }
        else {
            break ;
        }

        sok = NET_socket(tcp) ;
    } while ( false ) ;

    return sok ;
}

static inline int close(int fd)
{
    return NET_close(fd) ;
}

static inline int connect(
    int sockfd,
    void * sa,
    int addrlen)
{
    struct sockaddr_in * sai = sa ;
    S_NET_IND ind = {
        .porta = sai->sin_port,
        .ip = sai->sin_addr.s_addr
    } ;

    INUTILE(addrlen) ;

    return NET_connect(sockfd, &ind) ;
}

static inline int bind(
    int sockfd,
    void * sa,
    int addrlen)
{
    struct sockaddr_in * sai = sa ;

    INUTILE(addrlen) ;

    if ( NET_bind(sockfd, sai->sin_port) ) {
        return 0 ;
    }

    return -1 ;
}

static inline int recvfrom(
    int sockfd,
    void * buf,
    int len,
    unsigned int flags,
    void * from,
    int * fromlen)
{
    S_NET_IND ind ;
    int letti = NET_recvfrom(sockfd, buf, len, &ind) ;

    INUTILE(flags) ;
    INUTILE(fromlen) ;

    if ( (letti > 0) && from ) {
        struct sockaddr_in * sai = from ;
        sai->sin_port = ind.porta ;
        sai->sin_addr.s_addr = ind.ip ;
    }

    return letti ;
}

static inline int sendto(
    int sockfd,
    const void * msg,
    int len,
    unsigned int flags,
    const void * to,
    int tolen)
{
    const struct sockaddr_in * sai = to ;

    INUTILE(flags) ;
    INUTILE(tolen) ;

    S_NET_IND ind = {
        .porta = sai->sin_port,
        .ip = sai->sin_addr.s_addr
    } ;

    return NET_sendto(sockfd, msg, len, &ind) ;
}

static inline int listen(
    int sockfd,
    int backlog)
{
    if ( NET_listen(sockfd, backlog) ) {
        return 0 ;
    }

    return -1 ;
}

static inline int accept(
    int sockfd,
    void * from,
    int * fromlen)
{
    S_NET_IND ind ;
    int sok = NET_accept(sockfd, &ind) ;

    INUTILE(fromlen) ;

    if ( (sok > 0) && from ) {
        struct sockaddr_in * sai = from ;
        sai->sin_port = ind.porta ;
        sai->sin_addr.s_addr = ind.ip ;
    }

    return sok ;
}

static inline int recv(
    int sockfd,
    void * buf,
    int len,
    int flags)
{
    INUTILE(flags) ;
    return NET_recv(sockfd, buf, len) ;
}

static inline int send(
    int sockfd,
    const void * msg,
    int len,
    int flags)
{
    INUTILE(flags) ;
    return NET_send(sockfd, msg, len) ;
}

#else
#   warning net_sock.h incluso
#endif