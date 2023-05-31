#include "utili.h"
#include "stm32h7xx_hal.h"
#include "cmsis_rtos/cmsis_os.h"
//#define USA_DIARIO
#include "diario/diario.h"
#include "bsp.h"
#include "net.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/timeouts.h"
#include "net_mdma.h"
#include "lwipopts.h"

/*
 * Vedi
 *  https://community.st.com/s/question/0D53W00001Gi9BoSAJ/ethernet-hal-driver-reworked-by-st-and-available-in-22q1-preview-now-available-on-github
 */

extern void phy_reset(void) ;
extern void phy_stop(void) ;

//#define STAMPA_ROBA	1
#define DBG_PBUF_RX     1
#define DBG_PBUF_TX     1
#define DBG_EVN         0

#define EVN_ESCI        (1 << 0)
#define EVN_IRQ_PHY     (1 << 1)
#define EVN_TX          (1 << 2)
#define EVN_IRQ_RX      (1 << 4)
#define EVN_IRQ_ERR     (1 << 5)
#define EVN_RIC         (1 << 6)
#define EVN_MDMA_RX_B   (1 << 7)
#define EVN_MDMA_RX_M   (1 << 8)
#define EVN_MDMA_TX_B   (1 << 8)
#define EVN_MDMA_TX_M   (1 << 9)

#if LWIP_DHCP + LWIP_AUTOIP
static bool ip_bound = false ;
#endif

struct netif netif ;

static bool init_pool = false ;

static osThreadId tid = NULL ;

#define ETH_RX_BUFFER_CNT             (2 * ETH_RX_DESC_CNT + 1)

static
__attribute__( ( section(".no_cache"),
                 aligned(32) ) )
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] ;

static
__attribute__( ( section(".no_cache"),
                 aligned(32) ) )
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] ;

static
__attribute__( ( section(".no_cache") ) )
ETH_TxPacketConfig TxConfig ;

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

#if LWIP_TCP + LWIP_UDP == 0
#error SCEGLI ALMENO UNO DEI DUE
#endif

#if USA_NET_FINE

/*
 * La chiusura prevede di liberare i buffer usati da ETH
 *
 * Per farlo tengo una lista dei buffer allocati
 */

#include "linux_list.h"

typedef struct {
    struct pbuf_custom pbuf_custom ;
    struct list_head allocated ;
    uint8_t buff[RXBUFF_T_DIM_BUFF] ALLINEAMENTO ;
} RxBuff_t ;

static struct list_head allocated ;

LWIP_MEMPOOL_DECLARE(RX_POOL,
                     ETH_RX_BUFFER_CNT,
                     sizeof(RxBuff_t),
                     "Zero-copy RX PBUF pool")

static void eth_rx_free(void)
{
    if ( !list_empty(&allocated) ) {
        RxBuff_t * cursor ;
        RxBuff_t * next ;
#ifdef __ICCARM__
        list_for_each_entry_safe(cursor,
                                 RxBuff_t,
                                 next,
                                 &allocated,
                                 allocated) {
#else
        list_for_each_entry_safe(cursor, next, &allocated, allocated) {
#endif
            list_del(&cursor->allocated) ;

            struct pbuf_custom * custom_pbuf = (struct pbuf_custom *) cursor ;
            LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf) ;
        }
    }
}

static void pbuf_free_custom(struct pbuf * p)
{
    RxBuff_t * pB = (RxBuff_t *) p ;
    if ( !list_empty(&allocated) ) {
        RxBuff_t * cursor ;
        RxBuff_t * next ;
#ifdef __ICCARM__
        list_for_each_entry_safe(cursor,
                                 RxBuff_t,
                                 next,
                                 &allocated,
                                 allocated) {
#else
        list_for_each_entry_safe(cursor, next, &allocated, allocated) {
#endif
            if ( cursor == pB ) {
                list_del(&cursor->allocated) ;
            }
        }
    }

    struct pbuf_custom * custom_pbuf = (struct pbuf_custom *) p ;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf) ;

    // vedi esame_cust_pbuf.py
    DDB_PRN( DBG_PBUF_RX, ("%s %08X", __func__, p) )
}

#else

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

    // vedi esame_cust_pbuf.py
    DDB_PRN( DBG_PBUF_RX, ("%s %08X", __func__, p) )
}

#endif

static
__attribute__( ( section(".no_cache") ) )
ETH_BufferTypeDef txB[ETH_TX_DESC_CNT] ;

osMessageQDef(mq_tx, 10, void *) ;
static osMessageQId mq_tx = NULL ;

static uint8_t mac[NETIF_MAX_HWADDR_LEN] ;
static ip4_addr_t ip ;
static ip4_addr_t msk ;
static ip4_addr_t gw ;

static ETH_HandleTypeDef heth = {
    .Instance = ETH,
    .Init = {
        .MediaInterface = HAL_ETH_RMII_MODE,
        .MACAddr = mac,
        .TxDesc = DMATxDscrTab,
        .RxDesc = DMARxDscrTab,
        .RxBuffLen = RX_BUF_LEN,
    }
} ;

__WEAK void net_start(bool pronto)
{
    INUTILE(pronto) ;
    DDB_DEBUG("WEAK %s(%s)", __func__, pronto ? "PRONTO" : "morto") ;
}

__WEAK void net_link(bool link)
{
    INUTILE(link) ;
    DDB_DEBUG("WEAK %s(%s)", __func__, link ? "SU" : "giu") ;
}

__WEAK void net_bound(const char * _ip)
{
    if ( _ip ) {
        DDB_DEBUG("WEAK %s(%s)", __func__, _ip) ;
    }
    else {
        DDB_DEBUG("WEAK %s(NULL)", __func__) ;
    }
}

static void trasmetti(struct pbuf * p)
{
    static_assert(ETH_TX_DESC_CNT == MEMP_NUM_FRAG_PBUF, "OKKIO") ;

    DDB_PRN( DBG_PBUF_TX, ("TX %08X[%d]", p, p->tot_len) ) ;

    // preparo
    struct pbuf * q = p ;
    txB[0].buffer = q->payload ;
    txB[0].len = q->len ;
    q = q->next ;

    for ( int i = 1 ; i < ETH_TX_DESC_CNT ; ++i ) {
        if ( q ) {
            txB[i - 1].next = &txB[i] ;
            txB[i].buffer = q->payload ;
            txB[i].len = q->len ;
#ifdef USA_CACHE_
            SCB_CleanDCache_by_Addr( (uint32_t *) q->payload, q->len ) ;
#endif
            q = q->next ;
        }
        else {
            txB[i - 1].next = NULL ;
            txB[i].buffer = NULL ;
            txB[i].len = 0 ;
        }
    }
    txB[ETH_TX_DESC_CNT - 1].next = NULL ;
    DDB_ASSERT(NULL == q) ;

    // invio
    TxConfig.Length = p->tot_len ;
    TxConfig.TxBuffer = txB ;
    TxConfig.pData = p ;
    DDB_CONTROLLA( HAL_OK == HAL_ETH_Transmit(&heth, &TxConfig, 1000) ) ;
}

static err_t
port_netif_output(
    struct netif * n_if,
    struct pbuf * p)
{
    INUTILE(n_if) ;
    assert(osThreadGetId() == tid) ;

    if ( PHY_link() ) {
        trasmetti(p) ;
    }

    return ERR_OK ;
}

static err_t port_netif_init(struct netif * n_if)
{
    n_if->name[0] = 'E' ;
    n_if->name[1] = 'T' ;

    n_if->linkoutput = port_netif_output ;
    n_if->output = etharp_output ;
    n_if->mtu = ETH_MAX_PAYLOAD ;
    n_if->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP ;
    //MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);

    memcpy(n_if->hwaddr, mac, ETH_HWADDR_LEN) ;
    n_if->hwaddr_len = ETH_HWADDR_LEN ;

    // memoria
    if ( NULL == mq_tx ) {
        mq_tx = osMessageCreate(osMessageQ(mq_tx), NULL) ;
        DDB_CONTROLLA(NULL != mq_tx) ;
    }

    /*
     * In certe schede il reset abilita il clock ETH, per cui
     * bisogna darlo prima di HAL_ETH_Init() e non
     * dentro trova()
     */
    phy_reset() ;

    // MAC
    switch ( HAL_ETH_Init(&heth) ) {
    case HAL_OK:
        // Ottimo
        break ;
    case HAL_ERROR:
        if ( HAL_ETH_ERROR_TIMEOUT == heth.ErrorCode ) {
            DDB_ERROR("ETH: ? manca il clock ?") ;
        }
        else {
            DDB_ERR ;
        }
        break ;
    default:
        DDB_ERR ;
        break ;
    }

    // PHY
    PHY_iniz(PHY_SPEED_AUTO, PHY_DUPLEX_FULL) ;

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

static bool iniz_lwip = false ;

static void netTHD(void * argument)
{
    INUTILE(argument) ;

    net_mdma_iniz() ;
    netop_iniz(EVN_RIC) ;

    if ( !iniz_lwip ) {
        iniz_lwip = true ;
        lwip_init() ;
    }

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

    net_start(true) ;

    while ( true ) {
        // senza link attendo le interruzioni del PHY ...
        uint32_t attesa = osWaitForever ;
        if ( PHY_link() ) {
            // ... altrimenti ha senso chiedere se ci sono timer attivi
            uint32_t st = sys_timeouts_sleeptime() ;
            if ( 0 == st ) {
                sys_check_timeouts() ;
                st = sys_timeouts_sleeptime() ;
            }

            if ( SYS_TIMEOUTS_SLEEPTIME_INFINITE != st ) {
                attesa = st ;
            }
#if LWIP_DHCP
            if ( !ip_bound ) {
                struct dhcp * x = netif_dhcp_data(&netif) ;
                if ( NULL == x ) {}
                else if ( 10 == x->state ) {
                    DDB_DEBUG("DHCP_STATE_BOUND") ;

                    DDB_DEBUG( "\t ip  %s", ip4addr_ntoa(&netif.ip_addr) ) ;
                    DDB_DEBUG( "\t msk %s", ip4addr_ntoa(&netif.netmask) ) ;
                    DDB_DEBUG( "\t gw  %s", ip4addr_ntoa(&netif.gw) ) ;

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
                    DDB_DEBUG("AUTOIP_STATE_BOUND") ;

                    DDB_DEBUG( "\t ip  %s", ip4addr_ntoa(&netif.ip_addr) ) ;
                    DDB_DEBUG( "\t msk %s", ip4addr_ntoa(&netif.netmask) ) ;
                    DDB_DEBUG( "\t gw  %s", ip4addr_ntoa(&netif.gw) ) ;

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
#if USA_NET_FINE
        if ( EVN_ESCI & evn.value.signals ) {
            break ;
        }
#endif
        if ( EVN_IRQ_ERR & evn.value.signals ) {
//            heth.gState = HAL_ETH_STATE_STARTED ;
//            DDB_CONTROLLA( HAL_OK == HAL_ETH_Stop_IT(&heth) ) ;
//            DDB_CONTROLLA( HAL_OK == HAL_ETH_Start_IT(&heth) ) ;
        }

        if ( EVN_IRQ_PHY & evn.value.signals ) {
            PHY_isr() ;
        }
//        if ( EVN_TX & evn.value.signals ) {
//            DBG_PUTS("EVN_TX") ;
//            trasmetti() ;
//        }

        if ( EVN_IRQ_RX & evn.value.signals ) {
            DDB_PUT(DBG_EVN, "EVN_IRQ_RX") ;

            struct pbuf * p = NULL ;
            while ( true ) {
                if ( HAL_OK != HAL_ETH_ReadData(&heth, (void * *) &p) ) {
                    break ;
                }

                if ( NULL == p ) {
                    DBG_QUA ;
                    LINK_STATS_INC(link.memerr) ;
                    break ;
                }
                LINK_STATS_INC(link.recv) ;
                //DDB_PRN( DBG_PBUF_RX, ("\t%08X", p) )
#ifdef STAMPA_ROBA
                DBG_PRINT_HEX("\t", p->payload, p->len) ;
#endif
                if ( netif.input(p, &netif) != ERR_OK ) {
                    pbuf_free(p) ;
                }
            }
        }

        if ( EVN_RIC & evn.value.signals ) {
            netop_ric() ;
        }
    }
#if USA_NET_FINE
    DBG_QUA ;

    DDB_CONTROLLA( HAL_OK == HAL_ETH_DeInit(&heth) ) ;
    phy_stop() ;
#if LWIP_DHCP
    dhcp_stop(&netif) ;
#endif
#if LWIP_AUTOIP && (LWIP_DHCP_AUTOIP_COOP == 0)
    autoip_stop(&netif) ;
#endif
    netif_set_down(&netif) ;
    netif_remove(&netif) ;

    netop_fine() ;

    eth_rx_free() ;

    tid = NULL ;
    net_start(false) ;
    osThreadTerminate(NULL) ;
#endif
}

bool NET_iniz(
    const uint8_t * m,
    const uint8_t * _ip,
    const uint8_t * _msk,
    const uint8_t * _gw)
{
    bool esito = false ;

    static_assert(ETH_TX_DESC_CNT >= 2, "OKKIO") ;
    static_assert(H_ETH_PAYL <= RX_BUF_LEN, "OKKIO") ;
    static_assert(TCP_MAX_PAYL ==TCP_MSS, "OKKIO") ;

    do {
        if ( NULL != tid ) {
            DDB_ERR ;
            break ;
        }

        if ( NULL == m ) {
            DDB_ERR ;
            break ;
        }
        memcpy(mac, m, NETIF_MAX_HWADDR_LEN) ;

#if LWIP_DHCP + LWIP_AUTOIP
        INUTILE(_ip) ;
        INUTILE(_msk) ;
        INUTILE(_gw) ;

        ip = ip_addr_any ;
        msk = ip_addr_any ;
        gw = ip_addr_any ;
#else
        if ( NULL == _ip ) {
            DDB_ERR ;
            break ;
        }
        memcpy( &ip, _ip, sizeof(ip) ) ;

        if ( NULL == _msk ) {
            DDB_ERR ;
            break ;
        }
        memcpy( &msk, _msk, sizeof(msk) ) ;

        if ( NULL == _gw ) {
            DDB_ERR ;
            break ;
        }
        memcpy( &gw, _gw, sizeof(gw) ) ;
#endif

        if ( !init_pool ) {
            LWIP_MEMPOOL_INIT(RX_POOL) ;
            init_pool = true ;
#if USA_NET_FINE
            INIT_LIST_HEAD(&allocated) ;
#endif
        }

        memset( DMATxDscrTab, 0, sizeof(DMATxDscrTab) ) ;
        memset( DMARxDscrTab, 0, sizeof(DMARxDscrTab) ) ;

        memset( &TxConfig, 0, sizeof(ETH_TxPacketConfig) ) ;
        TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM
                              | ETH_TX_PACKETS_FEATURES_CRCPAD ;
        TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC ;
        TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT ;

        osThreadDef(netTHD, osPriorityNormal, 0, 1000) ;

        tid = osThreadCreate(osThread(netTHD), NULL) ;

        esito = NULL != tid ;
    } while ( false ) ;

    return esito ;
}

void NET_fine(void)
{
#if USA_NET_FINE
    if ( tid ) {
        (void) osSignalSet(tid, EVN_ESCI) ;
    }
#else
    DDB_ERR ;
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
#if USA_NET_FINE
        RxBuff_t * pB = (RxBuff_t *) p ;
        INIT_LIST_HEAD(&pB->allocated) ;
        list_add(&pB->allocated, &allocated) ;
#endif
        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *) p + offsetof(RxBuff_t, buff) ;

        // vedi esame_cust_pbuf.py
        DDB_PRN( DBG_PBUF_RX, ("%08X = %s", p, __func__) )

        p->custom_free_function = pbuf_free_custom ;
        /* Initialize the struct pbuf.
        * This must be performed whenever a buffer's allocated because it may be
        * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
        pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUFFER_SIZE) ;
    }
    else {
        *buff = NULL ;
        DDB_ERR ;
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
    DDB_ERR ;
    DDB_DEBUG("\t ErrorCode %08X", h->ErrorCode) ;
    if ( HAL_ETH_ERROR_DMA & h->ErrorCode ) {
        DDB_DEBUG("\t\t DMACSR %08X", h->DMAErrorCode) ;
    }
    if ( HAL_ETH_ERROR_MAC & h->ErrorCode ) {
        DDB_DEBUG("\t\t MACRXTXSR %08X", h->MACErrorCode) ;
    }
    switch ( h->gState ) {
    case HAL_ETH_STATE_RESET:
        DDB_DEBUG("\t HAL_ETH_STATE_RESET  ") ;
        break ;
    case HAL_ETH_STATE_READY:
        DDB_DEBUG("\t HAL_ETH_STATE_READY  ") ;
        break ;
    case HAL_ETH_STATE_STARTED:
        DDB_DEBUG("\t HAL_ETH_STATE_STARTED") ;
        break ;
    case HAL_ETH_STATE_ERROR:
        DDB_DEBUG("\t HAL_ETH_STATE_ERROR  ") ;
        break ;
    default:
        DDB_DEBUG("\t ???") ;
        break ;
    }
//    if ( tid ) {
//        (void) osSignalSet(tid, EVN_IRQ_ERR) ;
//    }
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

bool is_btp_running(void)
{
    return tid != NULL ;
}

void net_mdma_rx_esito(bool bene)
{
    if ( bene ) {
        DDB_CONTROLLA( osOK == osSignalSet(tid, EVN_MDMA_RX_B) ) ;
    }
    else {
        DDB_CONTROLLA( osOK == osSignalSet(tid, EVN_MDMA_RX_M) ) ;
    }
}

void net_mdma_tx_esito(bool bene)
{
    if ( bene ) {
        DDB_CONTROLLA( osOK == osSignalSet(tid, EVN_MDMA_TX_B) ) ;
    }
    else {
        DDB_CONTROLLA( osOK == osSignalSet(tid, EVN_MDMA_TX_M) ) ;
    }
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
        DDB_DEBUG("[%u] -> %04X", reg, val) ;
#endif
    }
    else {
        DDB_ERR ;
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
        DDB_DEBUG("[%u] <- %04X", reg, val) ;
#endif
    }
    else {
        DDB_ERR ;
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
    HAL_ETH_GetMACConfig(&heth, &MACConf) ;
    MACConf.DuplexMode =
        fullduplex ? ETH_FULLDUPLEX_MODE : ETH_HALFDUPLEX_MODE ;
    MACConf.Speed = centomega ? ETH_SPEED_100M : ETH_SPEED_10M ;
    HAL_ETH_SetMACConfig(&heth, &MACConf) ;
    DDB_CONTROLLA( HAL_OK == HAL_ETH_Start_IT(&heth) ) ;

    // lwip
#if LWIP_DHCP + LWIP_AUTOIP
    ip_bound = false ;
#endif
    netif_set_link_up(&netif) ;
    net_link(true) ;
}

void MAC_fine(void)
{
    // lwip
    netif_set_link_down(&netif) ;

    // mac
    DDB_CONTROLLA( HAL_OK == HAL_ETH_Stop_IT(&heth) ) ;

    // thd
#if LWIP_STATS
#if LWIP_STATS_DISPLAY
    stats_display() ;
#endif
#endif
#if LWIP_DHCP + LWIP_AUTOIP
    net_bound(NULL) ;
#endif
    net_link(false) ;
}
