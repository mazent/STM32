#ifndef LWIP_OPTS_H_
#define LWIP_OPTS_H_

/*
    COPIATE QUESTO FILE DA QUALCHE PARTE E RINOMINATELO IN
        lwip_opts.h
    PRENDETEVI LE VOSTRE RESPONSIBILITA' E CAMBIATE I VALORI

    INOLTRE, IN
        stm32h7xx_hal_conf.h
    SCEGLIETE I VALORI PER ETH_TX_DESC_CNT E ETH_RX_DESC_CNT
    (
        HAL_ETH_ReleaseTxPacket vuole una potenza di due!
    )

    Per info:
    https://community.st.com/t5/stm32-mcus/how-to-create-project-for-stm32h7-with-ethernet-and-lwip-stack/ta-p/49308
    https://community.st.com/t5/stm32-mcus-embedded-software/how-to-make-ethernet-and-lwip-working-on-stm32/m-p/261456
*/

// abilita la NET_fine()
// OKKIO che poi non capita piu' l'irq di fine tx
//#define USA_NET_FINE        1

// IPv4
// --------------------------------------------------------------

#define LWIP_TCP                        0
#define LWIP_UDP                        0
#define LWIP_DHCP                       0
#define LWIP_AUTOIP                     0
#if LWIP_DHCP && LWIP_AUTOIP
#define LWIP_DHCP_AUTOIP_COOP           1
#define LWIP_DHCP_AUTOIP_COOP_TRIES     3
#endif

// Controllo di flusso: il remoto si ferma a TCP_WND byte
// NOTA
//     tcp 1460 -> ip 1500 -> eth 1514
//     udp 1480 -> ip 1500 -> eth 1514
#if LWIP_TCP + LWIP_UDP
#   define TCP_MSS                     1460
#   define TCP_WND                  (4 * TCP_MSS)
#else
#   define TCP_MSS                     500
#   define TCP_WND                  TCP_MSS
#endif

// Statistiche
// --------------------------------------------------------------
#ifdef NDEBUG
#   define LWIP_STATS              0
#   define LWIP_STATS_DISPLAY      0
#else
#   define LWIP_STATS              1
// Occorre invocare stats_display()
#   define LWIP_STATS_DISPLAY      1
#endif

// Infrastructure
// --------------------------------------------------------------

// Memoria disponibile
#if LWIP_TCP + LWIP_UDP
#   define MEM_SIZE       (16 * 1024)
#else
#   define MEM_SIZE       (2 * 1024)
#endif

// Numero totale di socket ...
#define LWIP_P_NUM_SOCK_TCP         1
#define LWIP_P_NUM_SOCK_UDP         1

// Debug (piu' stampe -> piu' errori eth)
// --------------------------------------------------------------

#ifndef NDEBUG

#define LWIP_DEBUG

//#define LWIP_DBG_MIN_LEVEL          LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_MIN_LEVEL          LWIP_DBG_LEVEL_WARNING

#define LWIP_DBG_TYPES_ON           (LWIP_DBG_ON | LWIP_DBG_STATE)

#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define AUTOIP_DEBUG                LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF
#define ETHARP_DEBUG                LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF

#endif  // NDEBUG

#endif
