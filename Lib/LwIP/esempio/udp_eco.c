// vedi https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c

#define STAMPA_DBG
#include "utili.h"
#include "cmsis_rtos/cmsis_os.h"
#include "net_sock.h"
#include "lwip/opt.h"

// udp payload 1460 (come TCP_MSS)
// udp len     1468
// frame       1502
#define BUFSIZE         ( (ETH_RX_DESC_CNT - 1) * TCP_MSS )

static
__attribute__( ( section(".dtcm") ) )
char buf[BUFSIZE] ;    /* message buf */

const int PORTA_ECO = 7 ;

static osThreadId srvTHD ;

static void udp_eco_srv(void * argument)
{
#ifdef NDEBUG
    bool test_close1 = false ;
    bool test_close2 = false ;
#else
    bool test_close1 = true ;
    bool test_close2 = true ;
#endif
    INUTILE(argument) ;

    DBG_PRINTF("%s %d B", __func__, BUFSIZE) ;

    int sockfd ; /* socket */

ripeti:
    /*
     * socket: create the parent socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0) ;
    if ( sockfd < 0 ) {
        DBG_PUTS("ERROR opening socket") ;
    }

    if ( test_close1 ) {
        test_close1 = false ;
        CONTROLLA( 0 == close(sockfd) ) ;
        goto ripeti ;
    }

    /*
     * build the server's Internet address
     */
    struct sockaddr_in serveraddr ; /* server's addr */

    memset( &serveraddr, 0, sizeof(serveraddr) ) ;
    serveraddr.sin_family = AF_INET ;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    serveraddr.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    /*
     * bind: associate the parent socket with a port
     */
    if ( bind( sockfd, (struct sockaddr *) &serveraddr,
               sizeof(serveraddr) ) < 0 ) {
        DBG_PUTS("ERROR on binding") ;
    }

    if ( test_close2 ) {
        test_close2 = false ;
        CONTROLLA( 0 == close(sockfd) ) ;
        goto ripeti ;
    }

    while ( 1 ) {
        /*
         * recvfrom: receive a UDP datagram from a client
         */
        struct sockaddr_in clientaddr ;     /* client addr */
        int clientlen ;         /* byte size of client's address */

        int n = recvfrom(sockfd, buf, BUFSIZE, 0,
                         (struct sockaddr *) &clientaddr, &clientlen) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR in recvfrom") ;
            break ;
        }
#ifdef DBG_ABIL
        // cinema
        uint16_t hport = ntohs(clientaddr.sin_port) ;
        DBG_PRINTF("[srv] ricevo %d B da %s:%04X",
        		n,
				inet_ntoa(clientaddr.sin_addr),
				hport) ;
#endif
        /*
         * sendto: echo the input back to the client
         */
        n = sendto(sockfd, buf, n, 0,
                   (struct sockaddr *) &clientaddr, clientlen) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR in sendto") ;
            break ;
        }
    }

    DBG_QUA ;
    srvTHD = NULL ;
    osThreadTerminate(NULL) ;
}

void udp_eco_srv_iniz(void)
{
    if ( NULL == srvTHD ) {
        osThreadDef(udp_eco_srv, osPriorityNormal, 0, 800) ;
        srvTHD = osThreadCreate(osThread(udp_eco_srv), NULL) ;
        ASSERT(srvTHD) ;
    }
}

/*******************************************************************/

static osThreadId clnTHD = NULL ;
static void * dati ;
static uint16_t dim ;
static uint16_t quanti ;

static void udp_eco_cln(void * _)
{
    uint32_t tot = 0 ;
    uint16_t i = 0 ;
    uint32_t inizio, durata ;

    INUTILE(_) ;

    DBG_PRINTF("%s %d B", __func__, BUFSIZE) ;

    int sok = socket(AF_INET, SOCK_DGRAM, 0) ;
    if ( sok < 0 ) {
        DBG_PUTS("ERROR opening socket") ;
        goto esci ;
    }

    struct sockaddr_in dest, mitt ;
    int mit_dim ;

    memset( &dest, 0, sizeof(dest) ) ;
    dest.sin_family = AF_INET ;
    dest.sin_addr.s_addr = NET_inet_addr("10.1.20.254") ;
    dest.sin_port = htons( (unsigned short) PORTA_ECO ) ;

    if ( connect( sok, (struct sockaddr *) &dest,
                  sizeof(dest) ) < 0 ) {
        DBG_PUTS("ERROR on connect") ;
        goto esci ;
    }


    inizio = HAL_GetTick() ;
    for ( ; i < quanti ; ++i ) {
        int n = sendto( sok, dati, dim, 0, &dest, sizeof(dest) ) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR in sendto") ;
            break ;
        }

        DBG_PUTS("risposta ...");
        n = recvfrom(sok, buf, dim, 0, &mitt, &mit_dim) ;
        if ( n < 0 ) {
            DBG_PUTS("ERROR in recvfrom") ;
            break ;
        }
        DBG_PRINTF("ricevo %d B da %s:%04X", n,
                   inet_ntoa(mitt.sin_addr), mitt.sin_port) ;
        tot += dim ;
    }
    durata = HAL_GetTick() - inizio ;
    if ( i == quanti ) {
        double sec = durata ;
        sec /= 1000.0 ;
        double tput = tot ;
        tput /= sec ;
        DBG_PRINTF("%u / %u = %.3f B/s", tot, durata, tput) ;
    }
esci:
    CONTROLLA( 0 == close(sok) ) ;
    DBG_QUA ;
    clnTHD = NULL ;
    osThreadTerminate(NULL) ;
}

void udp_eco_cln_iniz(
    void * v,
    uint16_t d,
    uint16_t q)
{
    if ( NULL == clnTHD ) {
        dati = v ;
        dim = d ;
        quanti = q ;

        osThreadDef(udp_eco_cln, osPriorityNormal, 0, 800) ;
        clnTHD = osThreadCreate(osThread(udp_eco_cln), NULL) ;
        ASSERT(clnTHD) ;
    }
}
