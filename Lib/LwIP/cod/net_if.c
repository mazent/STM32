//#define STAMPA_DBG
#include "utili.h"
#include "stm32h7xx_hal.h"
#include "cmsis_rtos/cmsis_os.h"
#include "bsp.h"
#include "net.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/timeouts.h"
#include "stack/lista.h"

//#define DIARIO_LIV_DBG
#include "stampe.h"

extern void phy_reset(void) ;
extern void phy_stop(void) ;

// La trasmissione a polling degrada col tempo
// https://community.st.com/t5/stm32-mcus-embedded-software/strange-long-eth-transmission/td-p/597629

//#define STAMPA_ROBA	1
#define DBG_PBUF_RX     0
#define DBG_PBUF_TX     0
#define DBG_EVN         0
#define DBG_LST_RX      0

#define EVN_IRQ_PHY     (1 << 0)
#define EVN_IRQ_TX      (1 << 1)
#define EVN_IRQ_RX      (1 << 2)
#define EVN_RIC         (1 << 3)
//#define EVN_IRQ_ERR     (1 << 4)
#define EVN_FINE        (1 << 5)
#define EVN_INIZ        (1 << 6)

static bool trasm = false ;

// se occupato, salvo e trasmetto dopo
#define MAX_LISTA       10

static
__attribute__( ( section(".dtcm") ) )
union {
    UNA_LISTA s ;
    uint32_t b[MAX_LISTA + 2] ;
} lstTx ;

static
__attribute__( ( section(".dtcm") ) )
union {
    UNA_LISTA s ;
    uint32_t b[MAX_LISTA + 2] ;
} lstRx ;

#if LWIP_DHCP + LWIP_AUTOIP
static bool ip_bound = false ;
#endif

struct netif netif ;

static osThreadId tid = NULL ;

static enum {
    STT_SPENTO,
    STT_Q_ACCESO,
    STT_ACCESO,
    STT_Q_SPENTO,
    STT_ERR
} stato = STT_SPENTO ;

#if LWIP_TCP + LWIP_UDP
#define ETH_RX_BUFFER_CNT             (2 * ETH_RX_DESC_CNT + 1)
#else
#define ETH_RX_BUFFER_CNT             ETH_RX_DESC_CNT
#endif

static
__attribute__( ( section(".no_cache"),
                 aligned( sizeof(uint32_t) ) ) )
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] ;

static
__attribute__( ( section(".no_cache"),
                 aligned( sizeof(uint32_t) ) ) )
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] ;

static
__attribute__( ( section(".no_cache") ) )
ETH_TxPacketConfig TxConfig ;

static void stampa_tx_desc(void)
{
#if 0
    for ( int i = 0 ; i < ETH_TX_DESC_CNT ; i++ ) {
        DBG_PRINTF("\t %d", i) ;
        DBG_PRINTF("\t\t DESC0 %08X", DMATxDscrTab[i].DESC0) ;
        DBG_PRINTF("\t\t DESC1 %08X", DMATxDscrTab[i].DESC1) ;
        DBG_PRINTF("\t\t DESC2 %08X", DMATxDscrTab[i].DESC2) ;
        DBG_PRINTF("\t\t DESC3 %08X", DMATxDscrTab[i].DESC3) ;
        DBG_PRINTF("\t\t BKUP0 %08X", DMATxDscrTab[i].BackupAddr0) ;
        DBG_PRINTF("\t\t BKUP1 %08X", DMATxDscrTab[i].BackupAddr1) ;
    }
#endif
}

static void stampa_pbuf(const struct pbuf * p)
{
    INUTILE(p) ;
#if 1
#elif 0
    DBG_PRINTF("\t %u", p->tot_len) ;
#else
    DBG_PRINTF( "\t %u[%u]", p->tot_len, pbuf_clen(p) ) ;
    const struct pbuf * q = p ;
    while ( q ) {
        //DBG_PRINTF( "\t\t %u", q->len ) ;
        DBG_PRINT_HEX("\t\t ", q->payload, q->len) ;
        q = q->next ;
    }
#endif
}

// payload: cfr https://en.wikipedia.org/wiki/Ethernet_frame
// o forse e' 1522, o 1530, o 1536 come il cubo?
#define RX_BUF_LEN          1536

#define ETH_RX_BUFFER_SIZE      RX_BUF_LEN

#ifdef USA_CACHE_
#define DCACHE_LINE_ARR(a)  \
    ( (a + __SCB_DCACHE_LINE_SIZE - 1) & NEGA(__SCB_DCACHE_LINE_SIZE - 1) )
#define ALLINEAMENTO        __ALIGNED(__SCB_DCACHE_LINE_SIZE)
#else
#define DCACHE_LINE_ARR(a)  a
#define ALLINEAMENTO
#endif

#define RXBUFF_T_DIM_BUFF   DCACHE_LINE_ARR(ETH_RX_BUFFER_SIZE)

typedef struct {
    struct pbuf_custom pbuf_custom ;
    uint8_t buff[RXBUFF_T_DIM_BUFF] ALLINEAMENTO ;
} RxBuff_t ;

LWIP_MEMPOOL_DECLARE(RX_POOL,
                     ETH_RX_BUFFER_CNT,
                     sizeof(RxBuff_t),
                     "Zero-copy RX PBUF pool")

static void pbuf_free_custom(struct pbuf * p)
{
    struct pbuf_custom * custom_pbuf = (struct pbuf_custom *) p ;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf) ;

    // Lo elimino dalla lista: ...
    uint32_t elem ;
    // ... il primo
    CONTROLLA( LST_est(&lstRx.s, &elem) ) ;
    DBG_PRN( DBG_LST_RX, ( "lst rx -> %08X = %d", elem, LST_quanti(&lstRx.s) ) ) ;
    // ... deve essere lui
    ASSERT(UINTEGER(p) == elem) ;

    // vedi esame_cust_pbuf.py
    DBG_PRN( DBG_PBUF_RX, ("pbuf_rx_free %08X", p) ) ;
}

static
__attribute__( ( section(".no_cache") ) )
ETH_BufferTypeDef txB[ETH_TX_DESC_CNT] ;

static NET_MODO modo ;
static NET_MDI mdi ;
static bool far_loopback ;

static uint8_t mac[NETIF_MAX_HWADDR_LEN] ;
static ip4_addr_t ip ;
static ip4_addr_t msk ;
static ip4_addr_t gw ;

static const ETH_HandleTypeDef eth_cfg = {
    .Instance = ETH,
    .Init = {
        .MediaInterface = HAL_ETH_RMII_MODE,
        .MACAddr = mac,
        .TxDesc = DMATxDscrTab,
        .RxDesc = DMARxDscrTab,
        .RxBuffLen = RX_BUF_LEN,
    }
} ;
static ETH_HandleTypeDef heth ;

static bool cfg_ok(const S_NET_CFG * p)
{
    bool esito = false ;

    switch ( p->modo ) {
    case NET_MODO_10:
    case NET_MODO_10_FD:
    case NET_MODO_100:
    case NET_MODO_100_FD:
    case NET_MODO_AUTO:
        break ;
    default:
        DBG_ERR ;
        goto err ;
    }

    switch ( p->mdi ) {
    case NET_MDI_DRITTO:
    case NET_MDI_STORTO:
    case NET_MDI_AUTO:
        break ;
    default:
        DBG_ERR ;
        goto err ;
    }

    esito = true ;
    modo = p->modo ;
    mdi = p->mdi ;
    far_loopback = p->far_loopback ;

err:
    return esito ;
}

__WEAK void net_bound(const char * _)
{
    INUTILE(_) ;
}

__WEAK void net_start(void) {}

__WEAK void net_stop(void) {}

__WEAK void net_link(bool _)
{
    INUTILE(_) ;
}

static void trasmetti(struct pbuf * p)
{
#if DBG_PBUF_TX
    DBG_PRN( DBG_PBUF_TX, ("TX %08X[%d]", p, p->tot_len) ) ;
#else
    //DBG_PRINTF( "TX %u[%u]", p->tot_len, pbuf_clen(p) ) ;
    stampa_pbuf(p) ;
#endif

    // preparo
    memset( txB, 0, sizeof(txB) ) ;
    struct pbuf * q = p ;
    txB[0].buffer = q->payload ;
    txB[0].len = q->len ;
    q = q->next ;
    for ( int i = 1 ; q != NULL ; q = q->next, i++ ) {
        if ( i >= ETH_TX_DESC_CNT ) {
            DBG_ERR ;
            break ;
        }

        txB[i].buffer = q->payload ;
        txB[i].len = q->len ;
        txB[i - 1].next = &txB[i] ;
    }

    // invio
    TxConfig.Length = p->tot_len ;
    TxConfig.TxBuffer = txB ;
    TxConfig.pData = p ;

    trasm = HAL_OK == HAL_ETH_Transmit_IT(&heth, &TxConfig) ;
    if ( !trasm ) {
        DBG_ERR ;
        HAL_ETH_ReleaseTxPacket(&heth) ;
    }
    else {
        LINK_STATS_INC(link.xmit) ;
    }
}

static err_t
port_netif_output(
    struct netif * n_if,
    struct pbuf * p)
{
    INUTILE(n_if) ;
    assert(osThreadGetId() == tid) ;

    if ( !PHY_link() ) {
        DBG_QUA ;
    }
    else if ( trasm || (LST_quanti(&lstTx.s) > 0) ) {
        // accodo
        DBG_PRN( DBG_PBUF_TX, ("%08X = pbuf_tx_accoda", p) ) ;
        if ( LST_ins( &lstTx.s, UINTEGER(p) ) ) {
            pbuf_ref(p) ;
        }
        else {
            DBG_ERR ;
        }
    }
    else {
        pbuf_ref(p) ;

        trasmetti(p) ;
    }

    return ERR_OK ;
}

static void hw_iniz(void)
{
    DBG_FUN ;

    memset( DMATxDscrTab, 0, sizeof(DMATxDscrTab) ) ;
    memset( DMARxDscrTab, 0, sizeof(DMARxDscrTab) ) ;

    memset( &TxConfig, 0, sizeof(ETH_TxPacketConfig) ) ;
    TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM
                          | ETH_TX_PACKETS_FEATURES_CRCPAD ;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC ;
    TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT ;

    heth = eth_cfg ;

    // Resetto
    phy_reset() ;

    // MAC
    switch ( HAL_ETH_Init(&heth) ) {
    case HAL_OK:
        // Ottimo
        break ;
    case HAL_ERROR:
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if ( HAL_ETH_ERROR_TIMEOUT == heth.ErrorCode ) {
            DBG_PUTS("ETH: ? manca il clock ?") ;
        }
        else {
            DBG_ERR ;
        }
        break ;
    default:
        DBG_ERR ;
        break ;
    }

    // PHY
    if ( far_loopback ) {
        PHY_iniz_loopback(mdi) ;
    }
    else {
        PHY_iniz(modo, mdi) ;
    }
}

static err_t port_netif_init(struct netif * n_if)
{
    DBG_FUN ;

    n_if->name[0] = 'E' ;
    n_if->name[1] = 'T' ;

    n_if->linkoutput = port_netif_output ;
    n_if->output = etharp_output ;
    n_if->mtu = ETH_MAX_PAYLOAD ;
    n_if->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP ;
    //MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);

    memcpy(n_if->hwaddr, mac, ETH_HWADDR_LEN) ;
    n_if->hwaddr_len = ETH_HWADDR_LEN ;

    LST_iniz(&lstTx.s, MAX_LISTA) ;
    DBG_PRN( DBG_LST_RX, ( "lst rx iniz = %d", LST_quanti(&lstRx.s) ) ) ;
    LST_iniz(&lstRx.s, MAX_LISTA) ;

    hw_iniz() ;

    return ERR_OK ;
}

#if LWIP_NETIF_STATUS_CALLBACK
#error LWIP_NETIF_STATUS_CALLBACK INUTILE!!!
#endif

#if LWIP_NETIF_LINK_CALLBACK
#error LWIP_NETIF_LINK_CALLBACK INUTILE!!!
#endif

#if LWIP_NETIF_EXT_STATUS_CALLBACK
#error LWIP_NETIF_EXT_STATUS_CALLBACK INUTILE!!!
#endif

static void evn_irq_rx(void)
{
    struct pbuf * p = NULL ;
    while ( true ) {
        if ( HAL_OK != HAL_ETH_ReadData(&heth, (void * *) &p) ) {
            break ;
        }

        LINK_STATS_INC(link.recv) ;
#if DBG_PBUF_RX
        DBG_PRN( DBG_PBUF_RX, ("\t%08X [%u]", p, p->tot_len) )
#else
        stampa_pbuf(p) ;
#endif
#ifdef STAMPA_ROBA
        DBG_PRINT_HEX("\t", p->payload, p->len) ;
#endif
        // Conviene consumare subito il dato
        if ( netif.input(p, &netif) != ERR_OK ) {
            DBG_ERR ;
            pbuf_free(p) ;
        }
    }
}

static void evn_irq_tx(void)
{
    stampa_tx_desc() ;
    HAL_ETH_ReleaseTxPacket(&heth) ;

    if ( !trasm ) {
        uint32_t elem ;
        if ( LST_est(&lstTx.s, &elem) ) {
            struct pbuf * p = POINTER(elem) ;

            DBG_PRN( DBG_PBUF_TX, ("pbuf_tx_scoda %08X", elem) ) ;

            trasmetti(p) ;
        }
    }
}

#if USA_NET_FINE

static void evn_fine(void)
{
    if ( PHY_link() ) {
        MAC_fine() ;
    }

    CONTROLLA( HAL_OK == HAL_ETH_DeInit(&heth) ) ;
    phy_stop() ;
#if LWIP_DHCP
    dhcp_stop(&netif) ;
#endif
#if LWIP_AUTOIP && (LWIP_DHCP_AUTOIP_COOP == 0)
    autoip_stop(&netif) ;
#endif
    netif_set_down(&netif) ;
    //netif_remove(&netif) ;
#if LWIP_TCP + LWIP_UDP
    netop_fine() ;
#endif
    // Elimino i buffer che erano in ricezione ...
    uint32_t elem ;
    while ( LST_est(&lstRx.s, &elem) ) {
        DBG_PRN( DBG_LST_RX, ( "lst rx -> %08X = %d", elem, LST_quanti(&lstRx.s) ) ) ;

        struct pbuf_custom * custom_pbuf = POINTER(elem) ;
        LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf) ;

        // vedi esame_cust_pbuf.py
        DBG_PRN( DBG_PBUF_RX, ("pbuf_rx_free %08X", custom_pbuf) ) ;
    }
    // ... e in trasmissione
    while ( LST_est(&lstTx.s, &elem) ) {
        struct pbuf * p = POINTER(elem) ;
        pbuf_free(p) ;

        // vedi esame_cust_pbuf.py
        DBG_PRN( DBG_PBUF_TX, ("pbuf_tx_free %08X", elem) ) ;
    }

    stato = STT_SPENTO ;
    net_stop() ;
}

static void evn_iniz(void)
{
    hw_iniz() ;
    netif_set_up(&netif) ;
#if LWIP_DHCP
    dhcp_start(&netif) ;
#endif
#if LWIP_AUTOIP && (LWIP_DHCP_AUTOIP_COOP == 0)
    autoip_start(&netif) ;
#endif

    stato = STT_ACCESO ;
    net_start() ;
}

#endif

static void netTHD(void * argument)
{
    INUTILE(argument) ;

    DBG_FUN ;

    netop_iniz(EVN_RIC) ;

    lwip_init() ;

    netif_add(&netif,
              (ip4_addr_t *) &ip,
              (ip4_addr_t *) &msk,
              (ip4_addr_t *) &gw,
              NULL,
              port_netif_init,
              netif_input) ;
#if LWIP_NETIF_EXT_STATUS_CALLBACK
    netif_add_ext_callback(&nec, netif_ext_callback) ;
#endif
    netif_set_default(&netif) ;
    netif_set_up(&netif) ;
#if LWIP_DHCP
    dhcp_start(&netif) ;
#endif
#if LWIP_AUTOIP && (LWIP_DHCP_AUTOIP_COOP == 0)
    autoip_start(&netif) ;
#endif

    stato = STT_ACCESO ;
    net_start() ;

    while ( true ) {
        // senza link attendo le interruzioni del PHY ...
        uint32_t attesa = osWaitForever ;
        if ( PHY_link() ) {
            // ... altrimenti ha senso chiedere se ci sono timer attivi
            uint32_t st = sys_timeouts_sleeptime() ;
            if ( 0 == st ) {
                sys_check_timeouts() ;
                st = sys_timeouts_sleeptime() ;
                //DBG_PRINTF("sys_timeouts_sleeptime %08X",st);
            }

            if ( SYS_TIMEOUTS_SLEEPTIME_INFINITE != st ) {
                attesa = st ;
            }
#if LWIP_DHCP
            if ( !ip_bound ) {
                struct dhcp * x = netif_dhcp_data(&netif) ;
                if ( NULL == x ) {}
                else if ( 10 == x->state ) {
                    DBG_PUTS("DHCP_STATE_BOUND") ;

                    DBG_PRINTF( "\t ip  %s", ip4addr_ntoa(&netif.ip_addr) ) ;
                    DBG_PRINTF( "\t msk %s", ip4addr_ntoa(&netif.netmask) ) ;
                    DBG_PRINTF( "\t gw  %s", ip4addr_ntoa(&netif.gw) ) ;

                    ip_bound = true ;

                    net_bound( ip4addr_ntoa(&netif.ip_addr) ) ;
                }
            }
#endif
#if LWIP_AUTOIP
            if ( !ip_bound ) {
                struct autoip * aip = netif_autoip_data(&netif) ;
                if ( NULL == aip ) {}
                else if ( 3 == aip->state ) {
                    // p.e. 169.254.86.68
                    DBG_PUTS("AUTOIP_STATE_BOUND") ;

                    DBG_PRINTF( "\t ip  %s", ip4addr_ntoa(&netif.ip_addr) ) ;
                    DBG_PRINTF( "\t msk %s", ip4addr_ntoa(&netif.netmask) ) ;
                    DBG_PRINTF( "\t gw  %s", ip4addr_ntoa(&netif.gw) ) ;

                    ip_bound = true ;
                    net_bound( ip4addr_ntoa(&netif.ip_addr) ) ;
                }
            }
#endif
        }

        osEvent evn = osSignalWait(0, attesa) ;

        if ( osEventSignal != evn.status ) {
            continue ;
        }
#ifdef EVN_IRQ_ERR
        if ( EVN_IRQ_ERR & evn.value.signals ) {
            heth.gState = HAL_ETH_STATE_STARTED ;
            CONTROLLA( HAL_OK == HAL_ETH_Stop_IT(&heth) ) ;
            CONTROLLA( HAL_OK == HAL_ETH_Start_IT(&heth) ) ;
        }
#endif
        if ( EVN_IRQ_PHY & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_IRQ_PHY") ;

            PHY_isr() ;
        }

        if ( EVN_IRQ_RX & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_IRQ_RX") ;

            evn_irq_rx() ;
        }

#if LWIP_TCP + LWIP_UDP
        if ( EVN_RIC & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_RIC") ;

            netop_ric() ;
        }
#endif
        if ( EVN_IRQ_TX & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_IRQ_TX") ;

            evn_irq_tx() ;
        }
#if USA_NET_FINE
        if ( EVN_FINE & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_FINE") ;

            evn_fine() ;
        }

        if ( EVN_INIZ & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_INIZ") ;

            evn_iniz() ;
        }
#endif
    }
}

bool NET_iniz(const S_NET_CFG * p)
{
    bool esito = false ;

    // It is recommended to put a minimum ring descriptor length of 4
    static_assert(ETH_TX_DESC_CNT >= 4, "OKKIO") ;
    // deve essere potenza di due
    static_assert(POTENZA_DI_2(ETH_TX_DESC_CNT), "OKKIO") ;
    static_assert(ETH_RX_DESC_CNT >= 4, "OKKIO") ;
    static_assert(ETH_TX_DESC_CNT <= MEMP_NUM_FRAG_PBUF, "OKKIO") ;
    static_assert(PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS, "OKKIO") ;

    DBG_FUN ;

    do {
        if ( STT_ERR == stato ) {
            DBG_ERR ;
            break ;
        }

        if ( !cfg_ok(p) ) {
            break ;
        }

        if ( NULL != tid ) {
            switch ( stato ) {
            case STT_SPENTO:
                esito = true ;
#if USA_NET_FINE
                stato = STT_Q_ACCESO ;
                (void) osSignalSet(tid, EVN_INIZ) ;
#endif
                break ;
            case STT_ACCESO:
                esito = true ;
                break ;
            case STT_Q_ACCESO:
            case STT_Q_SPENTO:
            case STT_ERR:
                DBG_ERR ;
                break ;
            }
            break ;
        }
        stato = STT_Q_ACCESO ;

        if ( NULL == p->mac ) {
            DBG_ERR ;
            break ;
        }
        memcpy(mac, p->mac, NETIF_MAX_HWADDR_LEN) ;
#if LWIP_DHCP + LWIP_AUTOIP
        ip = ip_addr_any ;
        msk = ip_addr_any ;
        gw = ip_addr_any ;
#else
        if ( NULL == p->ip ) {
            DBG_ERR ;
            break ;
        }
        memcpy( &ip, p->ip, sizeof(ip) ) ;

        if ( NULL == p->msk ) {
            DBG_ERR ;
            break ;
        }
        memcpy( &msk, p->msk, sizeof(msk) ) ;

        if ( NULL == p->gw ) {
            DBG_ERR ;
            break ;
        }
        memcpy( &gw, p->gw, sizeof(gw) ) ;
#endif

        LWIP_MEMPOOL_INIT(RX_POOL) ;

        osThreadDef(netTHD, osPriorityNormal, 0, 1500) ;

        tid = osThreadCreate(osThread(netTHD), NULL) ;

        esito = NULL != tid ;
    } while ( false ) ;

    if ( !esito ) {
        stato = STT_ERR ;
    }

    return esito ;
}

void NET_fine(void)
{
    DBG_FUN ;
#if USA_NET_FINE
    switch ( stato ) {
    case STT_SPENTO:
        // Ottimo
        break ;
    case STT_ACCESO:
        stato = STT_Q_SPENTO ;
        (void) osSignalSet(tid, EVN_FINE) ;
        break ;
    case STT_Q_ACCESO:
    case STT_Q_SPENTO:
    case STT_ERR:
        DBG_ERR ;
        break ;
    }
#else
    DBG_ERR ;
#endif
}

void HAL_ETH_RxLinkCallback(
    void * * pStart,
    void * * pEnd,
    uint8_t * buff,
    uint16_t Length)
{
    struct pbuf * * ppStart = (struct pbuf * *) pStart ;
    struct pbuf * * ppEnd = (struct pbuf * *) pEnd ;
    struct pbuf * p = NULL ;

    /* Get the struct pbuf from the buff address. */
    p = (struct pbuf *) ( buff - offsetof(RxBuff_t, buff) ) ;
    p->next = NULL ;
    p->tot_len = 0 ;
    p->len = Length ;

    /* Chain the buffer. */
    if ( !*ppStart ) {
        /* The first buffer of the packet. */
        *ppStart = p ;
    }
    else {
        /* Chain the buffer to the end of the packet. */
        (*ppEnd)->next = p ;
    }
    *ppEnd = p ;

    /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
     * set to its own length, plus the length of all the following pbufs in the chain. */
    for ( p = *ppStart ; p != NULL ; p = p->next ) {
        p->tot_len += Length ;
    }

#ifdef USA_CACHE_
    /* Invalidate data cache because Rx DMA's writing to physical memory makes it stale. */
    SCB_InvalidateDCache_by_Addr( (uint32_t *) buff, Length ) ;
#endif
}

void HAL_ETH_RxAllocateCallback(uint8_t * * buff)
{
    struct pbuf_custom * p = LWIP_MEMPOOL_ALLOC(RX_POOL) ;
    if ( p ) {
        CONTROLLA( LST_ins( &lstRx.s, UINTEGER(p) ) ) ;
        DBG_PRN( DBG_LST_RX,
                 ( "%08X -> lst rx = %d", UINTEGER(p), LST_quanti(&lstRx.s) ) ) ;

        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *) p + offsetof(RxBuff_t, buff) ;

        // vedi esame_cust_pbuf.py
        DBG_PRN( DBG_PBUF_RX, ("%08X = pbuf_rx_alloc", p) ) ;

        p->custom_free_function = pbuf_free_custom ;
        /* Initialize the struct pbuf.
        * This must be performed whenever a buffer's allocated because it may be
        * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
        pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUFFER_SIZE) ;
    }
    else {
        *buff = NULL ;
        DBG_ERR ;
        LINK_STATS_INC(link.memerr) ;
    }
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef * h)
{
    INUTILE(h) ;

    if ( tid ) {
        (void) osSignalSet(tid, EVN_IRQ_RX) ;
    }
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef * h)
{
    INUTILE(h) ;
#ifdef DDB_LIV_ERR_ABIL
    DDB_ERROR("%s", __func__) ;
    DDB_ERROR("\t ErrorCode %08X", h->ErrorCode) ;
    if ( HAL_ETH_ERROR_DMA & h->ErrorCode ) {
        DDB_ERROR("\t\t DMACSR %08X", h->DMAErrorCode) ;
    }
    if ( HAL_ETH_ERROR_MAC & h->ErrorCode ) {
        DDB_ERROR("\t\t MACRXTXSR %08X", h->MACErrorCode) ;
    }
    switch ( h->gState ) {
    // NOLINTNEXTLINE(bugprone-branch-clone)
    case HAL_ETH_STATE_RESET:
        DDB_ERROR("\t HAL_ETH_STATE_RESET  ") ;
        break ;
    case HAL_ETH_STATE_READY:
        DDB_ERROR("\t HAL_ETH_STATE_READY  ") ;
        break ;
    case HAL_ETH_STATE_STARTED:
        DDB_ERROR("\t HAL_ETH_STATE_STARTED") ;
        break ;
    case HAL_ETH_STATE_ERROR:
        DDB_ERROR("\t HAL_ETH_STATE_ERROR  ") ;
        break ;
    default:
        DDB_ERROR("\t ???") ;
        break ;
    }
#endif
#ifdef EVN_IRQ_ERR
    if ( tid ) {
        (void) osSignalSet(tid, EVN_IRQ_ERR) ;
    }
#endif
}

void HAL_ETH_TxFreeCallback(uint32_t * buff)
{
    DBG_PRN( DBG_PBUF_TX, ("TX free %08X", buff) ) ;
    pbuf_free( (struct pbuf *) buff ) ;

    // adesso posso trasmettere
    trasm = false ;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef * h)
{
    INUTILE(h) ;
    (void) osSignalSet(tid, EVN_IRQ_TX) ;
}

void ETH_IRQHandler(void)
{
    HAL_ETH_IRQHandler(&heth) ;
}

void PHY_irq(void)
{
    if ( tid ) {
        (void) osSignalSet(tid, EVN_IRQ_PHY) ;
    }
}

bool is_net_running(void)
{
    return stato == STT_ACCESO ;
}

uint32_t reg_leggi(uint32_t reg)
{
    uint32_t val = ERRORE_REG_L ;

    HAL_StatusTypeDef stt = HAL_ETH_ReadPHYRegister(&heth,
                                                    LAN87xx_IND,
                                                    reg,
                                                    &val) ;
    if ( HAL_OK == stt ) {
#ifdef STAMPA_REG
        DBG_PRINTF("[%u] -> %04X", reg, val) ;
#endif
    }
    else {
        DBG_ERR ;
    }
    return val ;
}

bool reg_scrivi(
    uint32_t reg,
    uint32_t val)
{
    HAL_StatusTypeDef stt = HAL_ETH_WritePHYRegister(&heth,
                                                     LAN87xx_IND,
                                                     reg,
                                                     val) ;

    if ( HAL_OK == stt ) {
#ifdef STAMPA_REG
        DBG_PRINTF("[%u] <- %04X", reg, val) ;
#endif
    }
    else {
        DBG_ERR ;
    }

    return HAL_OK == stt ;
}

void MAC_iniz(
    bool fullduplex,
    bool centomega)
{
    ETH_MACConfigTypeDef MACConf = {
        0
    } ;

    DBG_FUN ;

    HAL_ETH_GetMACConfig(&heth, &MACConf) ;
    MACConf.DuplexMode =
        fullduplex ? ETH_FULLDUPLEX_MODE : ETH_HALFDUPLEX_MODE ;
    MACConf.Speed = centomega ? ETH_SPEED_100M : ETH_SPEED_10M ;
    HAL_ETH_SetMACConfig(&heth, &MACConf) ;
    CONTROLLA( HAL_OK == HAL_ETH_Start_IT(&heth) ) ;

#if LWIP_DHCP + LWIP_AUTOIP
    ip_bound = false ;
#endif
    trasm = false ;
    // lwip
    netif_set_link_up(&netif) ;
    net_link(true) ;
}

//#ifdef NDEBUG
//static void stampa_diario(void){}
//#else
//static void stampa_diario(void)
//{
//#ifdef DDB_DIM_MSG
//    static
//    __attribute__( ( section(".dtcm") ) )
//    char msg[DDB_DIM_MSG + 100] ;
//
//    while ( true ) {
//        int dim = DDB_leggi(msg) ;
//        if ( dim > 0 ) {
//            printf(msg) ;
//        }
//        else {
//            break ;
//        }
//    }
//#endif
//}
//#endif

void MAC_fine(void)
{
    DBG_FUN ;

    // lwip
    netif_set_link_down(&netif) ;

    // mac
    CONTROLLA( HAL_OK == HAL_ETH_Stop_IT(&heth) ) ;

    // thd
#if LWIP_STATS
#if LWIP_STATS_DISPLAY
    stats_display() ;
#endif
#endif

    uint32_t elem ;
    while ( LST_est(&lstTx.s, &elem) ) {
        struct pbuf * p = POINTER(elem) ;

        // vedi esame_cust_pbuf.py
        DBG_PRN( DBG_PBUF_TX, ("pbuf_tx_free %08X", elem) ) ;

        pbuf_free(p) ;
    }

    //stampa_diario() ;

#if LWIP_DHCP + LWIP_AUTOIP
    net_bound(NULL) ;
#endif
    net_link(false) ;
}
