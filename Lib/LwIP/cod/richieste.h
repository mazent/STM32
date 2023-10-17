#ifndef RICHIESTE_H_
#define RICHIESTE_H_

#include "net_if.h"

typedef enum {
    RT_SOCKET,
    RT_CLOSE,
    RT_CONNECT,
    RT_BIND,
    RT_RECVFROM,
    RT_SENDTO,
    RT_LISTEN,
    RT_ACCEPT,
    RT_RECV,
    RT_SEND,
    RT_SELECT
} IO_REQ_TYPE ;

typedef struct {
    S_NET_IND ind ;
} S_REQ_CONNECT ;


typedef struct {
    uint16_t porta ;
} S_REQ_BIND ;

typedef struct {
    void * buf ;
    int len ;
    S_NET_IND * ind ;
} S_REQ_RECVFROM ;

typedef struct {
    const void * buf ;
    int len ;
    S_NET_IND ind ;
} S_REQ_SENDTO ;

typedef struct {
    int backlog ;
} S_REQ_LISTEN ;

typedef struct {
    S_NET_IND * ind ;
} S_REQ_ACCEPT ;

typedef struct {
    void * buf ;
    int len ;
} S_REQ_RECV ;

typedef struct {
    const void * buf ;
    int len ;
    int parz;
    int cb;
} S_REQ_SEND ;

typedef struct {
    fd_set * readfds ;
    fd_set * writefds ;
    fd_set * exceptfds ;
} S_REQ_SELECT ;
typedef struct {
    IO_REQ_TYPE type ;

    int sok;

    union {
        bool tcp ;
        S_REQ_BIND bind ;
        S_REQ_RECVFROM recvf ;
        S_REQ_SENDTO sendto ;
        S_REQ_LISTEN listen ;
        S_REQ_ACCEPT accept ;
        S_REQ_RECV recv ;
        S_REQ_SEND send ;
        S_REQ_CONNECT connect;
        S_REQ_SELECT select ;
    } ;

    uint32_t result ;
} S_REQUEST_DATA ;

#endif
