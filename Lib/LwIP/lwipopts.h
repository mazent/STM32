#ifndef LWIPOPTS_H_
#define LWIPOPTS_H_

#include "lwip_opts.h"

#if LWIP_TCP == 0
#undef LWIP_P_NUM_SOCK_TCP
#define LWIP_P_NUM_SOCK_TCP     0
#endif
#if LWIP_UDP == 0
#undef LWIP_P_NUM_SOCK_UDP
#define LWIP_P_NUM_SOCK_UDP     0
#endif

#define LWIP_P_NUM_SOCK     (LWIP_P_NUM_SOCK_TCP + LWIP_P_NUM_SOCK_UDP)

// Other significant improvements can be made by supplying
// assembly or inline replacements for htons() and htonl()
// if you're using a little-endian architecture.
// \#define lwip_htons(x) your_htons()
// \#define lwip_htonl(x) your_htonl()
// If you \#define them to htons() and htonl(), you should
// \#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS to prevent lwIP from
// defining htonx / ntohx compatibility macros.
#include "stm32h7xx_hal.h"
#define lwip_htons(x)       __REV16(x)
#define lwip_htonl(x)       __REV(x)

// https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html

// uso drv.[c|h] per saltare dentro al task
// e proteggere le variabili interne (mie e sue)
#define NO_SYS                  1

// Fatti da hw
#define CHECKSUM_GEN_IP     0
#define CHECKSUM_GEN_UDP    0
#define CHECKSUM_GEN_TCP    0
#define CHECKSUM_CHECK_IP   0
#define CHECKSUM_CHECK_UDP  0
#define CHECKSUM_CHECK_TCP  0

// abilita la NET_fine()
#ifndef USA_NET_FINE
#   define USA_NET_FINE        0
#endif

/*

Ethernet II
         6           6       2      46..1500
    +----------+----------+------+-----------+
    |          |          |      |           |
    | MAC dst  |  MAC srg | prot | Payload   |
    |          |          |      |           |
    +----------+----------+------+-----------+

IPv4
       12        4         4     0..40     0..65515-opz
    +--------+--------+--------+---------+--------------+
    |        |        |        |         |              |
    | header | IP srg | IP dst | Opzioni | Payload      |
    |        |        |        |         |              |
    +--------+--------+--------+---------+--------------+

UDP
       2      2     2      2    0..65527
    +------+-----+------+-----+-----------+
    |      |     |      |     |           |
    | srg  | dst | dim  | cs  |  Payload  |
    |      |     |      |     |           |
    +------+-----+------+-----+-----------+

TCP
       2      2     4       4     2      2    2    2     0,4,8,..40   0..
    +------+-----+-------+-----+------+-----+----+-----+------------+---------+
    |      |     |       |     |      |     |    |     |            |         |
    | srg  | dst |  seq  | ack | flag | win | cs | urg |    opz     | Payload |
    |      |     |       |     |      |     |    |     |            |         |
    +------+-----+-------+-----+------+-----+----+-----+------------+---------+

*/

#define H_ETH_PAYL      1500
#define H_IP            (12 + 4 + 4)
#define H_UDP           (2 + 2 + 2 + 2)
#define UDP_MAX_PAYL    (H_ETH_PAYL - H_UDP - H_IP)
#define H_TCP           (2 + 2 + 4 + 4 + 2 + 2 + 2 + 2)
#define TCP_MAX_PAYL    (H_ETH_PAYL - H_TCP - H_IP)

// IPv4
// --------------------------------------------------------------

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1

#define IP_REASS_MAX_PBUFS          ETH_RX_DESC_CNT
#define MEMP_NUM_REASSDATA          IP_REASS_MAX_PBUFS
#define IP_FRAG_USES_STATIC_BUF     0
#define IP_DEFAULT_TTL              128
#define IP_SOF_BROADCAST            1
#define IP_SOF_BROADCAST_RECV       1
#define LWIP_BROADCAST_PING         0
#define LWIP_MULTICAST_PING         0
#define LWIP_RAW                    0

#ifndef TCP_MSS
// https://it.wikipedia.org/wiki/Maximum_Segment_Size
#   define TCP_MSS                     600
#endif

#ifndef TCP_WND
#   define TCP_WND                  TCP_MSS
#endif

// https://lists.gnu.org/archive/html/lwip-users/2006-11/msg00007.html
#define TCP_SND_BUF                 (2 * TCP_WND)
#define TCP_LISTEN_BACKLOG          1
#define TCP_SYNMAXRTX               3
#define LWIP_NETIF_HWADDRHINT       1
#if NO_SYS == 1
#define LWIP_NETCONN                0
#define LWIP_SOCKET                 0
#endif
#define ETHARP_TRUST_IP_MAC         0
#define ETH_PAD_SIZE                0
#define LWIP_CHKSUM_ALGORITHM       2

// Keepalive values, compliant with RFC 1122. Don't change this unless you know what you're doing
#define TCP_KEEPIDLE_DEFAULT        10000UL // Default KEEPALIVE timer in milliseconds
#define TCP_KEEPINTVL_DEFAULT       2000UL  // Default Time between KEEPALIVE probes in milliseconds
#define TCP_KEEPCNT_DEFAULT         9U      // Default Counter for KEEPALIVE probes

// Callback
// mi piacerebbe sapere quando ho un indirizzo valido
// inutile: invoco io netif_set_[up|down] #define LWIP_NETIF_STATUS_CALLBACK  0
// inutile: gestisco io il phy #define LWIP_NETIF_LINK_CALLBACK    0
// inutile: stacco + riattacco -> stesso indirizzo -> non invoca! #define LWIP_NETIF_EXT_STATUS_CALLBACK    (LWIP_DHCP + LWIP_AUTOIP)

// Statistiche
#ifdef NDEBUG
#define LWIP_STATS              0
#define LWIP_STATS_DISPLAY      0
#else
#define LWIP_STATS              1
// Occorre invocare stats_display()
#define LWIP_STATS_DISPLAY      1
#endif

// Infrastructure
// --------------------------------------------------------------

//#define LWIP_TIMERS               1
#define LWIP_TIMERS_CUSTOM      0

//#define LWIP_MPU_COMPATIBLE               0
#define LWIP_TCPIP_CORE_LOCKING         0
#define LWIP_TCPIP_CORE_LOCKING_INPUT   0
#define SYS_LIGHTWEIGHT_PROT            0
//#define LWIP_ASSERT_CORE_LOCKED()
//#define LWIP_MARK_TCPIP_THREAD()

// https://lists.gnu.org/archive/html/lwip-users/2003-01/msg01975.html
//#define MEM_LIBC_MALLOC                         0
//#define MEMP_MEM_MALLOC                         0
//#define MEMP_MEM_INIT                           0
#define MEM_ALIGNMENT                           4

//#define MEM_OVERFLOW_CHECK                    0
//#define MEM_SANITY_CHECK                      0
//#define MEMP_OVERFLOW_CHECK                       0
//#define MEMP_SANITY_CHECK                     0
//#define MEM_USE_POOLS                         0
//#define MEM_USE_POOLS_TRY_BIGGER_POOL         0
//#define MEMP_USE_CUSTOM_POOLS                   0
//#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT  0

#define MEMP_NUM_UDP_PCB                LWIP_P_NUM_SOCK_UDP
#define MEMP_NUM_TCP_PCB_LISTEN         LWIP_P_NUM_SOCK_TCP
#define MEMP_NUM_TCP_PCB                LWIP_P_NUM_SOCK_TCP

// vedi https://lwip.fandom.com/wiki/Porting_For_Bare_Metal
#define MEMP_NUM_PBUF               5
#define MEMP_NUM_TCP_SEG            5
#define MEMP_NUM_ARP_QUEUE          4
#define PBUF_POOL_SIZE              (IP_REASS_MAX_PBUFS + 1)

// se usi questa ...
#define LWIP_NETIF_TX_SINGLE_PBUF   0
// ... imposta questa
#define MEMP_NUM_FRAG_PBUF          ETH_TX_DESC_CNT

#define LWIP_DISABLE_TCP_SANITY_CHECKS  1

#define LWIP_SKIP_CONST_CHECK       1
#define LWIP_SKIP_PACKING_CHECK     1

#define LWIP_PROVIDE_ERRNO          1

#define LWIP_SINGLE_NETIF           1

#endif
