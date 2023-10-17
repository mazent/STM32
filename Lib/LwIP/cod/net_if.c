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

// senza questa si procede a polling e va male col tempo
// https://community.st.com/t5/stm32-mcus-embedded-software/strange-long-eth-transmission/td-p/597629
#define TX_ASINC    1

//#define STAMPA_ROBA	1
#define DBG_PBUF_RX     0
#define DBG_PBUF_TX     0
#define DBG_EVN         1

#define EVN_ESCI        (1 << 0)
#define EVN_IRQ_PHY     (1 << 1)
#define EVN_IRQ_TX      (1 << 2)
#define EVN_IRQ_RX      (1 << 3)
#define EVN_RIC         (1 << 4)
//#define EVN_IRQ_ERR     (1 << 5)

#ifdef TX_ASINC
static bool trasm = false ;

// se occupato, salvo e trasmetto dopo
#define MAX_LISTA       10

#ifdef LISTA_H_
static
__attribute__( ( section(".dtcm") ) )
union {
    UNA_LISTA s ;
    uint32_t b[MAX_LISTA + 2] ;
} lstTx ;
#else
osMessageQDef(mq_tx, MAX_LISTA, void *) ;
static osMessageQId mq_tx = NULL ;
#endif

#endif

#if LWIP_DHCP + LWIP_AUTOIP
static bool ip_bound = false ;
#endif

struct netif netif ;

static bool init_pool = false ;

static osThreadId tid = NULL ;

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

#ifdef TX_ASINC
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

#endif

static void stampa_pbuf(const struct pbuf * p)
{
    INUTILE(p) ;
#if 1
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
    DBG_PRN( DBG_PBUF_RX, ("%s %08X", __func__, p) )
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
    DBG_PRN( DBG_PBUF_RX, ("%s %08X", __func__, p) )
}

#endif

static
__attribute__( ( section(".no_cache") ) )
ETH_BufferTypeDef txB[ETH_TX_DESC_CNT] ;

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

__WEAK void net_bound(const char * _)
{
    INUTILE(_) ;
}

__WEAK void net_start(void) {}

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
    DBG_PUTS("TX") ;
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

#ifdef TX_ASINC
    trasm = HAL_OK == HAL_ETH_Transmit_IT(&heth, &TxConfig) ;
    if ( !trasm ) {
        DBG_ERR ;
        HAL_ETH_ReleaseTxPacket(&heth) ;
    }
    else {
        LINK_STATS_INC(link.xmit) ;
    }
#else
    LINK_STATS_INC(link.xmit) ;
    CONTROLLA( HAL_OK == HAL_ETH_Transmit(&heth, &TxConfig, 1000) ) ;
    HAL_ETH_ReleaseTxPacket(&heth) ;
#endif
}

#ifdef TX_ASINC
static err_t
port_netif_output(
    struct netif * n_if,
    struct pbuf * p)
{
    INUTILE(n_if) ;
    assert(osThreadGetId() == tid) ;

    DBG_FUN ;
    stampa_pbuf(p) ;

    if ( !PHY_link() ) {}
    else if ( trasm ) {
        // accodo
        DBG_PRN( DBG_PBUF_TX, ("TX accodo %08X[%d]", p, p->tot_len) ) ;
#ifdef LISTA_H_
        if ( LST_ins( &lstTx.s, UINTEGER(p) ) ) {
#else
        if ( osOK == osMessagePut(mq_tx, UINTEGER(p), 0) ) {
#endif
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

#else
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

#endif

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

#ifdef TX_ASINC
#ifdef LISTA_H_
    LST_iniz(&lstTx.s, MAX_LISTA) ;
#else
    if ( NULL == mq_tx ) {
        mq_tx = osMessageCreate(osMessageQ(mq_tx), NULL) ;
        CONTROLLA(NULL != mq_tx) ;
    }
#endif
#endif

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

    DBG_FUN ;

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
#if USA_NET_FINE
        if ( EVN_ESCI & evn.value.signals ) {
            break ;
        }
#endif
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

#if LWIP_TCP + LWIP_UDP
        if ( EVN_RIC & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_RIC") ;

            netop_ric() ;
        }
#endif
#ifdef TX_ASINC
        if ( EVN_IRQ_TX & evn.value.signals ) {
            DBG_PUT(DBG_EVN, "EVN_IRQ_TX") ;
            trasm = false ;
            stampa_tx_desc() ;
            HAL_ETH_ReleaseTxPacket(&heth) ;
#ifdef LISTA_H_
            uint32_t elem ;
            if ( LST_est(&lstTx.s, &elem) ) {
                struct pbuf * p = POINTER(elem) ;

                trasmetti(p) ;
            }
#else
            osEvent msg = osMessageGet(mq_tx, 0) ;
            if ( osEventMessage == msg.status ) {
                struct pbuf * p = msg.value.p ;

                trasmetti(p) ;
            }
#endif
        }
#endif
    }
#if USA_NET_FINE
    DBG_QUA ;

    CONTROLLA( HAL_OK == HAL_ETH_DeInit(&heth) ) ;
    phy_stop() ;
#if LWIP_DHCP
    dhcp_stop(&netif) ;
#endif
#if LWIP_AUTOIP && (LWIP_DHCP_AUTOIP_COOP == 0)
    autoip_stop(&netif) ;
#endif
    netif_set_down(&netif) ;
    netif_remove(&netif) ;
#if LWIP_TCP + LWIP_UDP
    netop_fine() ;
#endif
    eth_rx_free() ;

#if 0
    // Questa manca
    lwip_deinit() ;
#endif
    tid = NULL ;
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

    // It is recommended to put a minimum ring descriptor length of 4
    static_assert(ETH_TX_DESC_CNT >= 4, "OKKIO") ;
    // deve essere potenza di due
    static_assert(POTENZA_DI_2(ETH_TX_DESC_CNT), "OKKIO") ;
    static_assert(ETH_RX_DESC_CNT >= 4, "OKKIO") ;
    static_assert(ETH_TX_DESC_CNT <= MEMP_NUM_FRAG_PBUF, "OKKIO") ;
    static_assert(PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS, "OKKIO") ;

    do {
        if ( NULL != tid ) {
            DBG_ERR ;
            break ;
        }

        if ( NULL == m ) {
            DBG_ERR ;
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
            DBG_ERR ;
            break ;
        }
        memcpy( &ip, _ip, sizeof(ip) ) ;

        if ( NULL == _msk ) {
            DBG_ERR ;
            break ;
        }
        memcpy( &msk, _msk, sizeof(msk) ) ;

        if ( NULL == _gw ) {
            DBG_ERR ;
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

        osThreadDef(netTHD, osPriorityNormal, 0, 1500) ;

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
#if USA_NET_FINE
        RxBuff_t * pB = (RxBuff_t *) p ;
        INIT_LIST_HEAD(&pB->allocated) ;
        list_add(&pB->allocated, &allocated) ;
#endif
        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *) p + offsetof(RxBuff_t, buff) ;

        // vedi esame_cust_pbuf.py
        DBG_PRN( DBG_PBUF_RX, ("%08X = %s", p, __func__) )

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

#ifdef TX_ASINC

void HAL_ETH_TxFreeCallback(uint32_t * buff)
{
    DBG_PRN( DBG_PBUF_TX, ("TX free %08X", buff) ) ;
    pbuf_free( (struct pbuf *) buff ) ;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef * h)
{
    INUTILE(h) ;
    (void) osSignalSet(tid, EVN_IRQ_TX) ;
}

#endif

void ETH_IRQHandler(void)
{
    DDB_PUTS("ETH_IRQ") ;

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
    HAL_ETH_GetMACConfig(&heth, &MACConf) ;
    MACConf.DuplexMode =
        fullduplex ? ETH_FULLDUPLEX_MODE : ETH_HALFDUPLEX_MODE ;
    MACConf.Speed = centomega ? ETH_SPEED_100M : ETH_SPEED_10M ;
    HAL_ETH_SetMACConfig(&heth, &MACConf) ;
    CONTROLLA( HAL_OK == HAL_ETH_Start_IT(&heth) ) ;

#if LWIP_DHCP + LWIP_AUTOIP
    ip_bound = false ;
#endif
#ifdef TX_ASINC
    trasm = false ;
#endif
    // lwip
    netif_set_link_up(&netif) ;
    net_link(true) ;
}

#ifdef NDEBUG
static void stampa_diario(void){}
#else
static void stampa_diario(void)
{
#ifdef DDB_DIM_MSG
    static
    __attribute__( ( section(".dtcm") ) )
    char msg[DDB_DIM_MSG + 100] ;

    while ( true ) {
        int dim = DDB_leggi(msg) ;
        if ( dim > 0 ) {
            printf(msg) ;
        }
        else {
            break ;
        }
    }
#endif
}

#endif

void MAC_fine(void)
{
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

#ifdef TX_ASINC
#ifdef LISTA_H_
    uint32_t elem ;
    while ( LST_est(&lstTx.s, &elem) ) {
        struct pbuf * p = POINTER(elem) ;
        pbuf_free(p) ;
    }
#else
    while ( true ) {
        osEvent msg = osMessageGet(mq_tx, 0) ;
        if ( osEventMessage == msg.status ) {
            pbuf_free( (struct pbuf *) msg.value.p ) ;
        }
        else {
            break ;
        }
    }
#endif
#endif

    stampa_diario() ;

#if LWIP_DHCP + LWIP_AUTOIP
    net_bound(NULL) ;
#endif
    net_link(false) ;
}
