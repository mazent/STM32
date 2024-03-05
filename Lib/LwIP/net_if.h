#ifndef CM7_CORE_INC_NET_IF_H_
#define CM7_CORE_INC_NET_IF_H_

/*
 * Interfaccia verso LwIP
 ******************************************************
 *
 * PHY
 *     Vengono gestiti due componenti: LAN8742A e LAN8720
 *
 *     Il file bsp.h deve definire LAN87xx_IND con l'indirizzo
 *     (il valore di PHYAD0)
 *
 *     Il file bsp.h deve dichiarare la funzione:
 *         void bsp_phy_nrst(bool alto) ;
 *     che muove il pin
 *
 * MAC
 *     Il numero di descrittori ETH_RX_DESC_CNT e ETH_TX_DESC_CNT si trova in
 *     stm32h7xx_hal_conf.h
 *     In particolare ETH_RX_DESC_CNT determina la dimensione della finestra tcp,
 *     in quanto bisogna essere in grado di ricevere gli N pacchetti che possono
 *     essere spediti contemporaneamente
 *     Invece ETH_TX_DESC_CNT puo' essere basso in quanto lwip spedisce un
 *     pacchetto alla volta
 *
 *     Se ETH_RX_DESC_CNT=4 allora BUFSIZE=TCP_WND=3*1460=4380
 *     Nei .py i valori massimi da usare saranno: "-m 3" o "-d 4380"
 *     NOTA
 *         udp usa la frammentazione ip e non manda ack, per cui:
 *             py udp_eco.py -m 1 -q 1000
 *                 Eco: OK 1000 in 684ms (0.684 ms = 2133747.7 B/s = 2083.7 KiB/s)
 *         sfruttando i descrittori per ricevere piu' dati aumenta il t.put:
 *             py udp_eco.py -m 3 -q 1000
 *                 Eco: OK 1000 in 1s 756ms (1.756 ms = 2494066.8 B/s = 2435.6 KiB/s)
 *
 *         tcp ha minori prestazioni a causa degli ack:
 *             py tcp_eco.py -m 1 -q 1000
 *                 Eco: OK 1000 in 965ms (0.965 ms = 1512495.7 B/s = 1477.0 KiB/s)
 *             py tcp_eco.py -m 3 -q 1000
 *                 Eco: OK 1000 in 2s 217ms (2.217 ms = 1975940.5 B/s = 1929.6 KiB/s)
 *
 * Sezioni:
 *     .ethernet	ram per lo stack
 *     .no_cache	descrittori per ETH [1]
 *     .dtcm		usata dagli esempi
 *
 * [1] I descrittori devono essere allocati in una area di memoria senza cache
 *     (hanno caratteristiche simili a quelle dei registri delle periferiche)
 *     Se si abilita la d-cache occorre creare una zona senza e allocare ivi
 *     .ethernet e .no_cache (vedi sotto: usa_mpu)
 *
 * Includere:
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\include
 *     $PROJ_DIR$\..\Lib\LwIP
 *
 * Aggiungere:
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\api\err.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\def.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\inet_chksum.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\init.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ip.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\mem.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\memp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\netif.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\pbuf.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\stats.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\tcp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\tcp_in.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\tcp_out.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\timeouts.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\udp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\autoip.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\dhcp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\etharp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\icmp.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\ip4.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\ip4_addr.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\core\ipv4\ip4_frag.c
 *     $PROJ_DIR$\..\Middlewares\Third_Party\LwIP\netif\ethernet.c
 *
 */

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	// Megabit + duplex
    NET_MODO_10,
    NET_MODO_10_FD,
    NET_MODO_100,
    NET_MODO_100_FD,
	// Auto negozia
    NET_MODO_AUTO,
} NET_MODO ;

typedef enum {
    NET_MDI_DRITTO,
    NET_MDI_STORTO,
    NET_MDI_AUTO,
} NET_MDI ;

typedef struct {
    NET_MODO modo ;

    NET_MDI mdi ;

    // Forza anche NET_MODO_100
    bool far_loopback ;

    // Con dhcp e/o autoip basta il mac
    const uint8_t * mac ;
    const uint8_t * ip ;
    const uint8_t * msk ;
    const uint8_t * gw ;
} S_NET_CFG ;

// Partenza
// Se c'e' da abilitare il clock, fatelo prima
bool NET_iniz(const S_NET_CFG *) ;

// Dopo questa non usate piu' la rete
// Dovete definire USA_NET_FINE
void NET_fine(void) ;

// Strato fisico
// ------------------------------------

// Invocare sul fronte di discesa del pin
void PHY_irq(void) ;

// collegamento attivo (o cavo scollegato)
bool PHY_link(void) ;
// se attivo: full o half duplex
bool PHY_fullduplex(void) ;
// se attivo: 100 o 10 Mbs
bool PHY_100M(void) ;

// Identificativo
// Vengono gestiti due PHY:
//    LAN8742A 0x0007C13r
//    LAN8720  0x0007C0Fr
// dove r e' la revisione
// Se torna 0 occorre invocare NET_iniz()
uint32_t PHY_id(void) ;

uint16_t PHY_sym_err_cnt(void) ;

// Callback
// ------------------------------------

// Invocate quando la rete e' accesa/spenta
// Sono weak, se volete fatevi le vostre
void net_start(void) ;
// Se c'e' da disabilitare il clock, fatelo qua
void net_stop(void) ;

// Invocata quando il phy e' connesso (o sconnesso)
// Questa e' weak, se volete fatevi la vostra
void net_link(bool link) ;

// In caso di dhcp e/o autoip, per debug puo'
// essere utile essere avvisati del bounding
// Questa e' weak, se volete fatevi la vostra
// null -> unbound
void net_bound(const char * ip) ;

/************************************************
 * Asynchronous operation must be started with: NET_?_ini
 *
 * If NET_?_ini succeeded, you can do other things
 *
 * When you have finished, you can call NET_abort or NET_risul
 *
 * Take a look at the examples
 ***********************************************/

// Handle for an asynchronous operation
struct _NET_OP {
    // guess what?
    void * x ;
} ;
typedef struct _NET_OP * NET_OP ;

typedef struct {
    uint32_t ip ;
    uint16_t porta ;
} S_NET_IND ;

/*!
 * Wait for the end of the operation
 *
 * \param[in] op        handle for the asynchronous operation
 * \param[in] milli     timeout in milliseconds (or osWaitForever)
 * \return    the result (it depends on the operation)
 */

// Generico errore
#define NET_RISUL_ERROR     0xFFFFFFFF
// Ancora in corso (o invocate ancora NET_risul, o invocate NET_abort)
#define NET_RISUL_TEMPO     0xFFFFFFFE

uint32_t NET_risul(
    NET_OP op,
    uint32_t milli) ;

/*!
 * Abort an operation
 *
 * \param[in] op        handle for the asynchronous operation
 * \return    none
 */

void NET_abort(NET_OP op) ;

/*!
 * Apre un socket tcp o udp
 *
 * @param op	handle
 * @param tcp	tipo di socket
 * @return se vera, NET_risul restituisce il socket
 */
bool NET_socket_ini(
    NET_OP op,
    bool tcp) ;

/*!
 * Chiude un socket
 *
 * @param op	handle
 * @param sok	esito di NET_[socket|accept]
 * @return se vera, NET_risul restituisce 0 se riesce
 */
bool NET_close_ini(
    NET_OP op,
    int sok) ;

/*!
 * Connette il socket
 *
 * @param op	handle
 * @param sok	esito di NET_socket
 * @param ind	indirizzo a cui connettersi
 * @return se vera, NET_risul restituisce 0 se riesce
 */
bool NET_connect_ini(
    NET_OP op,
    int sok,
    S_NET_IND * ind) ;

/*!
 * Associa il socket a una porta
 *
 * @param op	handle
 * @param sok	esito di NET_socket
 * @param port	porta
 * @return se vera, NET_risul restituisce 0 se riesce
 */
bool NET_bind_ini(
    NET_OP op,
    int sok,
    uint16_t port) ;

/*!
 * Riceve da un socket udp
 *
 * @param op	handle
 * @param sok	esito di NET_socket
 * @param buf	qua butta i dati
 * @param len	capacita' di buf
 * @param ind	qua mette l'indirizzo del mittente (opzionale)
 * @return se vera, NET_risul restituisce la dimensione della roba ricevuta
 */
bool NET_recvfrom_ini(
    NET_OP op,
    int sok,
    void * buf,
    int len,
    S_NET_IND * ind) ;

/*!
 * Trasmette su un socket udp
 *
 * @param op	handle
 * @param sok	esito di NET_socket
 * @param buf	roba da trasmettere
 * @param len	quanta roba
 * @param ind	dove mandarla
 * @return se vera, NET_risul restituisce la dimensione della roba trasmessa
 */
bool NET_sendto_ini(
    NET_OP op,
    int sok,
    const void * buf,
    int len,
    S_NET_IND * ind) ;

/*!
 * Crea una coda di connessioni in attesa
 *
 * @param op		handle
 * @param sok		esito di NET_socket
 * @param backlog	numero massimo di connessioni
 * @return se vera, NET_risul restituisce zero se riesce
 */
bool NET_listen_ini(
    NET_OP op,
    int sok,
    int backlog) ;

/*!
 * Estrae la prima (e l'unica) connessione in coda
 *
 * @param op	handle
 * @param sok	esito di NET_socket
 * @param ind	qua mette l'indirizzo del mittente (opzionale, non usato)
 * @return se vera, NET_risul restituisce il socket remoto
 */
bool NET_accept_ini(
    NET_OP op,
    int sok,
    S_NET_IND * ind) ;

/*!
 * Riceve dal socket tcp
 *
 * @param op	handle
 * @param sok	esito di NET_[socket|accept]
 * @param buf	qua butta i dati
 * @param len	capacita' di buf
 * @return se vera, NET_risul restituisce:
 *              la dimensione della roba ricevuta
 *              0  sconnessione
 *              -1 errore
 */
bool NET_recv_ini(
    NET_OP op,
    int sok,
    void * buf,
    int len) ;

/*!
 * Trasmette sul socket tcp
 *
 * @param op	handle
 * @param sok	esito di NET_[socket|accept]
 * @param buf	roba da trasmettere
 * @param len	quanta roba
 * @return se vera, NET_risul restituisce:
 *              la dimensione della roba trasmessa
 *              -1 errore
 */
bool NET_send_ini(
    NET_OP op,
    int sok,
    const void * buf,
    int len) ;

/*!
 * Aspetta che i socket siano "pronti"
 *
 * @param op		handle
 * @param nfds		il piu' alto descrittore fra tutti gli insiemi + 1
 * @param readfds	opzionale, pronti in lettura (lettura non bloccante)
 * @param writefds	opzionale, pronti in scrittura (scrittura non bloccante)
 * @param exceptfds opzionale, chiusi
 * @return se vera, NET_risul restituisce:
 *                  il numero totale di socket pronti (gli *fds sono modificati)
 *                  -1 se errore
 */

typedef uint32_t sok_set ;

static inline void SS_CLR(
    int fd,
    sok_set * set)
{
    *set &= NEGA(1 << fd) ;
}

static inline bool SS_ISSET(
    int fd,
    const sok_set * set)
{
    return 0 != ( (*set) & (1 << fd) ) ;
}

static inline void SS_SET(
    int fd,
    sok_set * set)
{
    *set |= 1 << fd ;
}

static inline void SS_ZERO(sok_set * set)
{
    *set = 0 ;
}

bool NET_select_ini(
    NET_OP op,
    int nfds,
    sok_set * readfds,
    sok_set * writefds,
    sok_set * exceptfds) ;

/*********************************************************************/

#ifdef ESEMPI_NET

static inline int NET_socket(bool tcp)
{
    int sck = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_socket_ini(&op, tcp) ) {
        uint32_t r = NET_risul(&op, osWaitForever) ;
        if ( r != NET_RISUL_ERROR ) {
            sck = r ;
        }
    }

    return sck ;
}

static inline int NET_close(int fd)
{
    int res = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_close_ini(&op, fd) ) {
        res = NET_risul(&op, osWaitForever) ;
    }

    return res ;
}

static inline bool NET_connect(
    int sok,
    S_NET_IND * ind)
{
    bool esito = false ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_connect_ini(&op, sok, ind) ) {
        esito = 0 == NET_risul(&op, osWaitForever) ;
    }

    return esito ;
}

static inline bool NET_bind(
    int sok,
    uint16_t port)
{
    bool esito = false ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_bind_ini(&op, sok, port) ) {
        esito = 0 == NET_risul(&op, osWaitForever) ;
    }

    return esito ;
}

static inline int NET_recvfrom(
    int sok,
    void * buf,
    int len,
    S_NET_IND * ind)
{
    int letti = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_recvfrom_ini(&op, sok, buf, len, ind) ) {
        uint32_t r = NET_risul(&op, osWaitForever) ;
        if ( r != NET_RISUL_ERROR ) {
            letti = r ;
        }
    }

    return letti ;
}

static inline int NET_sendto(
    int sok,
    const void * buf,
    int len,
    S_NET_IND * ind)
{
    int scritti = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_sendto_ini(&op, sok, buf, len, ind) ) {
        uint32_t r = NET_risul(&op, osWaitForever) ;
        if ( r != NET_RISUL_ERROR ) {
            scritti = r ;
        }
    }

    return scritti ;
}

static inline bool NET_listen(
    int sok,
    int backlog)
{
    bool esito = false ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_listen_ini(&op, sok, backlog) ) {
        esito = 0 == NET_risul(&op, osWaitForever) ;
    }

    return esito ;
}

static inline int NET_accept(
    int sok,
    S_NET_IND * ind)
{
    int cln = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_accept_ini(&op, sok, ind) ) {
        cln = NET_risul(&op, osWaitForever) ;
    }

    return cln ;
}

static inline int NET_recv(
    int sockfd,
    void * buf,
    int len)
{
    int letti = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_recv_ini(&op, sockfd, buf, len) ) {
        letti = NET_risul(&op, osWaitForever) ;
    }

    return letti ;
}

static inline int NET_send(
    int sockfd,
    const void * buf,
    int len)
{
    int scritti = -1 ;
    struct _NET_OP op = {
        NULL
    } ;

    if ( NET_send_ini(&op, sockfd, buf, len) ) {
        scritti = NET_risul(&op, osWaitForever) ;
    }

    return scritti ;
}

static inline int NET_select(
    int nfds,
    sok_set * readfds,
    sok_set * writefds,
    sok_set * exceptfds,
    uint32_t to_ms)
{
    int pronti = -1 ;
    if ( 0 == to_ms ) {
        // polling non disponibile
    }
    else {
        struct _NET_OP op = {
            NULL
        } ;

        if ( NET_select_ini(&op, nfds, readfds, writefds, exceptfds) ) {
            pronti = NET_risul(&op, to_ms) ;
        }
    }

    return pronti ;
}

#endif  // ESEMPI_NET

/*********************************************************************/

#if 0

void usa_mpu(void)
{
#ifdef USA_CACHE
    SCB_EnableDCache() ;

    // Vedi .icf per le zone
    // Vedi PM0253 - Additional memory access constraints for caches and shared memory
    // Vedi https://community.st.com/t5/stm32-mcus-products/maintaining-cpu-data-cache-coherence-for-dma-buffers/td-p/95746
    __disable_irq() ;

    HAL_MPU_Disable() ;

    {
        // Flash
        MPU_Region_InitTypeDef mpur = {
            .Enable = MPU_REGION_ENABLE,
            .Number = MPU_REGION_NUMBER0,
            .BaseAddress = 0x08000000,
            .Size = MPU_REGION_SIZE_256KB,
            .SubRegionDisable = 0x0,
            .AccessPermission = MPU_REGION_FULL_ACCESS,
            .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
            .TypeExtField = MPU_TEX_LEVEL0,
            .IsCacheable = MPU_ACCESS_CACHEABLE,
            .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
            .IsShareable = MPU_ACCESS_NOT_SHAREABLE,
        } ;

        HAL_MPU_ConfigRegion(&mpur) ;
    }
    {
        // Dtcm (non per la cache ma per accesso)
        MPU_Region_InitTypeDef mpur = {
            .Enable = MPU_REGION_ENABLE,
            .Number = MPU_REGION_NUMBER1,
            .BaseAddress = 0x20000000,
            .Size = MPU_REGION_SIZE_128KB,
            .SubRegionDisable = 0x0,
            .AccessPermission = MPU_REGION_FULL_ACCESS,
            .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
            .TypeExtField = MPU_TEX_LEVEL1,
            .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
            .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
            .IsShareable = MPU_ACCESS_NOT_SHAREABLE,
        } ;

        HAL_MPU_ConfigRegion(&mpur) ;
    }
    {
        // ram: prima parte
        MPU_Region_InitTypeDef mpur = {
            .Enable = MPU_REGION_ENABLE,
            .Number = MPU_REGION_NUMBER2,
            .BaseAddress = 0x24000000,
            .Size = MPU_REGION_SIZE_128KB,
            .SubRegionDisable = 0x0,
            .AccessPermission = MPU_REGION_FULL_ACCESS,
            .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
            .TypeExtField = MPU_TEX_LEVEL0,
            .IsCacheable = MPU_ACCESS_CACHEABLE,
            .IsBufferable = MPU_ACCESS_BUFFERABLE,
            .IsShareable = MPU_ACCESS_NOT_SHAREABLE,
        } ;

        HAL_MPU_ConfigRegion(&mpur) ;
    }
    {
        // ram: seconda parte
        MPU_Region_InitTypeDef mpur = {
            .Enable = MPU_REGION_ENABLE,
            .Number = MPU_REGION_NUMBER3,
            .BaseAddress = 0x24020000,
            .Size = MPU_REGION_SIZE_128KB,
            .SubRegionDisable = 0x0,
            .AccessPermission = MPU_REGION_FULL_ACCESS,
            .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
            // roba non kesciata
            .TypeExtField = MPU_TEX_LEVEL1,
            .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
            .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
            .IsShareable = MPU_ACCESS_NOT_SHAREABLE,
        } ;

        HAL_MPU_ConfigRegion(&mpur) ;
    }

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT) ;

    __enable_irq() ;

    DBG_QUA ;
#endif
}

#endif

#else
#   warning net_if.h incluso
#endif
