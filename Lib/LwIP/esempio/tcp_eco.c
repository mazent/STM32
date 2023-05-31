/*
 * echoserver.c - A simple connection-based echo server
 * usage: echoserver <port>
 */
// cfr http://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/www/class24code/echoserver.c
#include "utili.h"
#include "cmsis_rtos/cmsis_os.h"
//#define USA_DIARIO
#include "diario/diario.h"
#include "net_sock.h"
#include "lwip/opt.h"

#define BUFSIZE         ( (ETH_RX_DESC_CNT - 1) * TCP_MSS )

static
__attribute__( ( section(".dtcm") ) )
char buf[BUFSIZE] ;    /* message buf */

static osThreadId srvTHD = NULL ;

static const int PORTA_ECO = 7 ;

static void tcp_eco_srv(void * _)
{
    int listenfd ; /* listening socket */

    INUTILE(_) ;

    DDB_DEBUG("%s %d B", __func__, BUFSIZE) ;

    /* socket: create a socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0) ;
    if ( listenfd < 0 ) {
        DDB_PUTS("ERROR opening socket") ;
    }

    /*
     * build the server's Internet address
     */
    struct sockaddr_in serveraddr ; /* server's addr */

    memset( &serveraddr, 0, sizeof(serveraddr) ) ;
    serveraddr.sin_family = AF_INET ;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    serveraddr.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    /* bind: associate the listening socket with a port */
    if ( bind( listenfd, (struct sockaddr *) &serveraddr,
               sizeof(serveraddr) ) < 0 ) {
        DDB_PUTS("ERROR on binding") ;
    }

    /* listen: make it a listening socket ready to accept connection requests */
    if ( listen(listenfd, 1) < 0 ) {
        DDB_PUTS("ERROR on listen") ;
    }

    while ( 1 ) {
        struct sockaddr_in clientaddr ; /* client addr */
        int clientlen ; /* byte size of client's address */
        int connfd ;     /* connection socket */

        DDB_PUTS("attesa connessione ...") ;

        /* accept: wait for a connection request */
        connfd = accept(listenfd, &clientaddr, &clientlen) ;
        if ( connfd < 0 ) {
            DDB_PUTS("ERROR on accept") ;
            break ;
        }
#if DDB_LIV >= DDB_LIV_DBG
        // cinema
        uint16_t hport = ntohs(clientaddr.sin_port) ;
        DDB_DEBUG("%d connesso a %s:%04X", connfd,
                   inet_ntoa(clientaddr.sin_addr),
                   hport) ;
#endif
        while ( true ) {
            /* read: read input string from the client */
            int n = recv(connfd, buf, BUFSIZE, 0) ;
            if ( n <= 0 ) {
                DDB_PUTS("ERROR reading from socket") ;
                break ;
            }
            else {
                DDB_DEBUG("server received %d bytes", n) ;

                /* write: echo the input string back to the client */
                n = send(connfd, buf, n, 0) ;
                if ( n < 0 ) {
                    DDB_PUTS("ERROR writing to socket") ;
                    break ;
                }
                else {
                    DDB_DEBUG("inviati %d bytes", n) ;
                }
            }
        }

        DDB_CONTROLLA( 0 == close(connfd) ) ;
    }

    DDB_DBG ;
    srvTHD = NULL ;
    osThreadTerminate(NULL) ;
}

void tcp_eco_srv_iniz(void)
{
    if ( NULL == srvTHD ) {
        osThreadDef(tcp_eco_srv, osPriorityNormal, 0, 800) ;
        srvTHD = osThreadCreate(osThread(tcp_eco_srv), NULL) ;
        DDB_ASSERT(srvTHD) ;
    }
}

/*******************************************************************/

static osThreadId clnTHD = NULL ;
static void * dati ;
static uint16_t dim ;
static uint16_t quanti ;

static void tcp_eco_cln(void * _)
{
    uint32_t tot = 0 ;
    uint16_t i = 0 ;
    uint32_t inizio, durata ;

    INUTILE(_) ;

    DDB_DEBUG("%s %d B", __func__, BUFSIZE) ;

    int sok = socket(AF_INET, SOCK_STREAM, 0) ;
    if ( sok < 0 ) {
        DDB_PUTS("ERROR opening socket") ;
        goto esci ;
    }

    struct sockaddr_in serveraddr ;

    memset( &serveraddr, 0, sizeof(serveraddr) ) ;
    serveraddr.sin_family = AF_INET ;
    serveraddr.sin_addr.s_addr = NET_inet_addr("10.1.20.254") ;
    serveraddr.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    if ( connect( sok, (struct sockaddr *) &serveraddr,
                  sizeof(serveraddr) ) < 0 ) {
        DDB_PUTS("ERROR on connect") ;
        goto esci ;
    }

    inizio = HAL_GetTick() ;
    for ( ; i < quanti ; ++i ) {
        int n = send(sok, dati, dim, 0) ;
        if ( n < 0 ) {
            DDB_PUTS("ERROR writing to socket") ;
            break ;
        }
        else {
            //DDB_DEBUG("inviati %d bytes", n) ;
        }

        n = recv(sok, buf, dim, 0) ;
        if ( n < 0 ) {
            DDB_PUTS("ERROR reading socket") ;
            break ;
        }
        else {
            //DDB_DEBUG("ricevuti %d bytes", n) ;
        }
        tot += dim ;
    }
    durata = HAL_GetTick() - inizio ;
    if ( i == quanti ) {
        double sec = durata ;
        sec /= 1000.0 ;
        double tput = tot ;
        tput /= sec ;
        DDB_DEBUG("%u B / %u ms = %.3f B/s", tot, durata, tput) ;
    }
esci:
    DDB_CONTROLLA( 0 == close(sok) ) ;

    DDB_DBG ;
    clnTHD = NULL ;
    osThreadTerminate(NULL) ;
}

void tcp_eco_cln_iniz(
    void * v,
    uint16_t d,
    uint16_t q)
{
    if ( NULL == clnTHD ) {
        dati = v ;
        dim = d ;
        quanti = q ;

        osThreadDef(tcp_eco_cln, osPriorityNormal, 0, 800) ;
        clnTHD = osThreadCreate(osThread(tcp_eco_cln), NULL) ;
        DDB_ASSERT(clnTHD) ;
    }
}
