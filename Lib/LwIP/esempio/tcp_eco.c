/*
 * echoserver.c - A simple connection-based echo server
 * usage: echoserver <port>
 */
// cfr http://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/www/class24code/echoserver.c

//#define STAMPA_DBG
#include "utili.h"
#include "cmsis_rtos/cmsis_os.h"
#include "net_sock.h"
#include "lwip/opt.h"

//#define DIARIO_LIV_DBG
#include "../cod/stampe.h"

#if LWIP_TCP

#define BUFSIZE         TCP_WND

#define USA_SELECT      1

static
__attribute__( ( section(".dtcm") ) )
char buf[BUFSIZE] ;    /* message buf */

static osThreadId srvTHD = NULL ;

static const int PORTA_ECO = 7 ;

#ifdef USA_SELECT

static void servi_socket(int sok)
{
    int pos = 0 ;
    sok_set leggi ;
    sok_set scrivi ;
    sok_set errore ;

    SS_ZERO(&leggi) ;
    SS_ZERO(&scrivi) ;
    SS_ZERO(&errore) ;

    int conta = 0 ;
    while ( true ) {
        SS_SET(sok, &leggi) ;
        SS_SET(sok, &errore) ;

        int quanti = net_select(sok + 1, &leggi, &scrivi, &errore, NULL) ;
        if ( quanti > 0 ) {
            if ( SS_ISSET(sok, &errore) ) {
                DBG_ERR ;
                break ;
            }

            if ( SS_ISSET(sok, &leggi) ) {
                const int letti = net_recv(sok, buf + pos, BUFSIZE - pos, 0) ;
                if ( letti <= 0 ) {
                    DBG_PUTS("ERROR reading from socket") ;
                    break ;
                }

                if ( 0 == pos ) {
                    conta++ ;
                }
                pos += letti ;
                SS_SET(sok, &scrivi) ;
                DBG_PRINTF("%d) ricevuti %d bytes", conta, letti) ;
                continue ;
            }

            if ( SS_ISSET(sok, &scrivi) ) {
                const int scritti = net_send(sok, buf, pos, 0) ;
                if ( scritti < 0 ) {
                    DBG_PUTS("ERROR writing to socket") ;
                    break ;
                }
                if ( scritti != pos ) {
                    DBG_ERR ;
                }

                pos = 0 ;
                SS_CLR(sok, &scrivi) ;
                DBG_PRINTF("%d) inviati %d bytes", conta, scritti) ;
            }
        }
        else {
            DBG_ERR ;
        }
    }

    CONTROLLA( 0 == net_close(sok) ) ;
}

#else

static void servi_socket(int sok)
{
    int conta = 0 ;

    while ( true ) {
        /* read: read input string from the client */
        int n = net_recv(sok, buf, BUFSIZE, 0) ;
        if ( n <= 0 ) {
            DBG_PUTS("ERROR reading from socket") ;
            break ;
        }

        conta++ ;
        DBG_PRINTF("%d) ricevuti %d bytes", conta, n) ;

        /* write: echo the input string back to the client */
        n = net_send(sok, buf, n, 0) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR writing to socket") ;
            break ;
        }

        DBG_PRINTF("%d) inviati %d bytes", conta, n) ;
    }

    CONTROLLA( 0 == net_close(sok) ) ;
}

#endif

static void tcp_eco_srv(void * _)
{
    int listenfd ; /* listening socket */

    INUTILE(_) ;

    DBG_PRINTF("%s %d B", __func__, BUFSIZE) ;

    /* socket: create a socket */
    listenfd = net_socket(AF_INET, SOCK_STREAM, 0) ;
    if ( listenfd < 0 ) {
        DBG_PUTS("ERROR opening socket") ;
    }

    /*
     * build the server's Internet address
     */
    struct sockaddr_in serveraddr ; /* server's addr */

    memset( &serveraddr, 0, sizeof(serveraddr) ) ;
    serveraddr.sin_family = AF_INET ;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    serveraddr.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    /* net_bind: associate the listening socket with a port */
    if ( net_bind( listenfd, (struct sockaddr *) &serveraddr,
                   sizeof(serveraddr) ) < 0 ) {
        DBG_PUTS("ERROR on binding") ;
    }

    /* net_listen: make it a listening socket ready to net_accept connection requests */
    if ( net_listen(listenfd, 1) < 0 ) {
        DBG_PUTS("ERROR on listen") ;
    }

    while ( true ) {
        struct sockaddr_in clientaddr ; /* client addr */
        int clientlen ; /* byte size of client's address */
        int connfd ;     /* connection socket */

        DBG_PUTS("attesa connessione ...") ;

        /* net_accept: wait for a connection request */
        connfd = net_accept(listenfd, &clientaddr, &clientlen) ;
        if ( connfd < 0 ) {
            DBG_PUTS("ERROR on accept") ;
            break ;
        }

        // cinema
#ifdef DDB_LIV_DBG_ABIL
        uint16_t hport = ntohs(clientaddr.sin_port) ;
        DBG_PRINTF("%d connesso a %s:%04X", connfd,
                   inet_ntoa(clientaddr.sin_addr),
                   hport) ;
#endif
        servi_socket(connfd) ;
    }

    DBG_QUA ;
    srvTHD = NULL ;
    osThreadTerminate(NULL) ;
}

void tcp_eco_srv_iniz(void)
{
    if ( NULL == srvTHD ) {
        osThreadDef(tcp_eco_srv, osPriorityNormal, 0, 800) ;
        srvTHD = osThreadCreate(osThread(tcp_eco_srv), NULL) ;
        ASSERT(srvTHD) ;
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
#ifdef DBG_ABIL
    uint32_t inizio ;
    uint32_t durata ;
#endif
    INUTILE(_) ;

    DBG_PRINTF("%s %d B", __func__, BUFSIZE) ;

    int sok = net_socket(AF_INET, SOCK_STREAM, 0) ;
    if ( sok < 0 ) {
        DBG_PUTS("ERROR opening socket") ;
        goto esci ;
    }

    struct sockaddr_in serveraddr ;

    memset( &serveraddr, 0, sizeof(serveraddr) ) ;
    serveraddr.sin_family = AF_INET ;
    serveraddr.sin_addr.s_addr = NET_inet_addr("10.1.20.254") ;
    serveraddr.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    if ( net_connect( sok, (struct sockaddr *) &serveraddr,
                      sizeof(serveraddr) ) < 0 ) {
        DBG_PUTS("ERROR on connect") ;
        goto esci ;
    }
#ifdef DBG_ABIL
    inizio = HAL_GetTick() ;
#endif
    for ( ; i < quanti ; ++i ) {
        int n = net_send(sok, dati, dim, 0) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR writing to socket") ;
            break ;
        }
        //DBG_PRINTF("inviati %d bytes", n) ;

        n = net_recv(sok, buf, dim, 0) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR reading socket") ;
            break ;
        }
        //DBG_PRINTF("ricevuti %d bytes", n) ;

        tot += dim ;
    }
#ifdef DBG_ABIL
    durata = HAL_GetTick() - inizio ;
    if ( i == quanti ) {
        double sec = durata ;
        sec /= 1000.0 ;
        double tput = tot ;
        tput /= sec ;
        DBG_PRINTF("%u / %u = %.3f B/s", tot, durata, tput) ;
    }
#endif
esci:
    CONTROLLA( 0 == net_close(sok) ) ;

    DBG_QUA ;
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
        ASSERT(clnTHD) ;
    }
}

#else

void tcp_eco_srv_iniz(void){}

void tcp_eco_cln_iniz(
    void * v,
    uint16_t d,
    uint16_t q)
{
    INUTILE(v) ;
    INUTILE(d) ;
    INUTILE(q) ;
}

#endif      // LWIP_TCP
