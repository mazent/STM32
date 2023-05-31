#include "utili.h"
#include "net.h"
#include "bsp.h"
#include "cmsis_rtos/cmsis_os.h"
//#define USA_DIARIO
#include "diario/diario.h"


#define PHY_ID_LAN8742A        0x0007c130
#define PHY_ID_LAN8720         0x0007c0f0

#define LAN87xx_STT             31
#define LAN87xx_IRQ_MSK         30
#define LAN87xx_IRQ_SRC         29

#define LAN87xx_STT_FULL        (1 << 4)
#define LAN87xx_STT_100         (1 << 3)

#define LAN87xx_IRQ_ENERGYON                            (1 << 7)
#define LAN87xx_IRQ_AUTO_NEGOTIATION_COMPLETE           (1 << 6)
#define LAN87xx_IRQ_REMOTE_FAULT_DETECTED               (1 << 5)
#define LAN87xx_IRQ_LINK_DOWN                           (1 << 4)
#define LAN87xx_IRQ_AUTO_NEGOTIATION_LP_ACKNOWLEDGE     (1 << 3)
#define LAN87xx_IRQ_PARALLEL_DETECTION_FAULT            (1 << 2)
#define LAN87xx_IRQ_AUTO_NEGOTIATION_PAGE_RECEIVED      (1 << 1)

// Registri 802.3 (22.2.4 Management functions)
#define REG__0_CTR           0  // Control B
#define REG__1_STT           1  // Status B
#define REG__2_ID_OUI        2  // PHY Identifier E
#define REG__3_ID_MNF        3  // PHY Identifier E
#define REG__4_AN_A          4  // Auto-Negotiation Advertisement E
#define REG__5_AN            5  // Auto-Negotiation Link Partner Base Page Ability E
#define REG__6_AN            6  // Auto-Negotiation Expansion E
#define REG__7_AN            7  // Auto-Negotiation Next Page Transmit E
#define REG__8_AN            8  // Auto-Negotiation Link Partner Received Next Page E
#define REG__9_MS            9  // MASTER-SLAVE Control Register E
#define REG_10_MS           10  // MASTER-SLAVE Status Register E
#define REG_11_PSE          11  // PSE Control register E
#define REG_12_PSE          12  // PSE Status register E
#define REG_13_MMD_CTRL     13  // MMD Access Control Register E
#define REG_14_MMD_DATA     14  // MMD Access Address Data Register E

#define REG13_ADDR       (0 << 14)
#define REG13_DATA       (1 << 14)
#define REG13_PCS        3

#define LAN8742A_IND_WUCSR   32784
#define LAN8742A_WUCSR_LED2_MSK    (3 << 11)
#define LAN8742A_WUCSR_LED2_nINT   (1 << 11)

#define REG0_RST        (1 << 15) // Reset
#define REG0_LBACK      (1 << 14) // Loopback
#define REG0_SL1        (1 << 13) // Speed Selection (LSB)
#define REG0_AN_E       (1 << 12) // Auto-Negotiation Enable
#define REG0_PDOWN      (1 << 11) // Power Down
#define REG0_ISO        (1 << 10) // Isolate
#define REG0_R_AN       (1 << 9)  // Restart Auto-Negotiation
#define REG0_DUPLEX     (1 << 8)  // Duplex Mode

#define REG1_100_T4     (1 << 15)  // 100BASE-T4
#define REG1_100_FD     (1 << 14)  // 100BASE-X Full Duplex
#define REG1_100        (1 << 13)  // 100BASE-X Half Duplex
#define REG1_10_FD      (1 << 12)  // 10 Mb/s Full Duplex
#define REG1_10         (1 << 11)  // 10 Mb/s Half Duplex
#define REG1_100_T2_FD  (1 << 10)  // 100BASE-T2 Full Duplex
#define REG1_100_T2     (1 << 9)   // 100BASE-T2 Half Duplex
#define REG1_ES         (1 << 8)   // Extended Status
#define REG1_ANC        (1 << 5)   // Auto-Negotiation Complete
#define REG1_RF         (1 << 4)   // Remote Fault
#define REG1_ANA        (1 << 3)   // Auto-Negotiation Ability
#define REG1_LS         (1 << 2)   // Link Status
#define REG1_JD         (1 << 1)   // Jabber Detect
#define REG1_EC         (1 << 0)   // Extended Capability

// 11:5 Technology Ability Field See 28.2.1.2 R/W
#define REG4_TA_10       (1 << 5)    // A0 10BASE-T
#define REG4_TA_10_FD    (1 << 6)    // A1 10BASE-T full duplex
#define REG4_TA_100      (1 << 7)    // A2 100BASE-TX
#define REG4_TA_100_FD   (1 << 8)    // A3 100BASE-TX full duplex
#define REG4_TA_100_T4   (1 << 9)    // A4 100BASE-T4
#define REG4_SF_802_3    1           // 4:0 Selector Field See 28.2.1.2 R/W

const uint32_t lanmsk = LAN87xx_IRQ_LINK_DOWN
                        | LAN87xx_IRQ_AUTO_NEGOTIATION_COMPLETE ;

// Bit 2.15 shall be the MSB of the PHY Identifier,
// and bit 3.0 shall be the LSB of the PHY Identifier
static uint32_t ulPhyID = 0 ;
static uint32_t id_phy = 0 ;
static uint32_t ulConfig ;
static bool link = false ;
static bool full = false ;
static bool m100 = false ;

bool PHY_link(void)
{
    return link ;
}

bool PHY_fullduplex(void)
{
    return full ;
}

bool PHY_100M(void)
{
    return m100 ;
}

uint32_t PHY_id(void)
{
    return id_phy ;
}

void phy_reset(void)
{
    // Resetto
    bsp_phy_reset(true) ;
    HAL_Delay(2) ;

    // Tolgo reset
    bsp_phy_reset(false) ;
    HAL_Delay(100) ;
}

void phy_stop(void)
{
    // Resetto
    bsp_phy_reset(true) ;

    link = full = m100 = false ;
    ulPhyID = id_phy = 0 ;
}

static bool trova(void)
{
    bool trovato = false ;
    DDB_FUN ;

    // hard reset (vedi port_netif_init())
    //phy_reset() ;

    // soft reset
    uint32_t r0 = reg_leggi(REG__0_CTR) ;
    while ( ERRORE_REG_L == r0 ) {
        r0 = reg_leggi(REG__0_CTR) ;
    }
    DDB_DEBUG("r0 = %08X", r0) ;
    r0 |= REG0_RST ;
    while ( !reg_scrivi(REG__0_CTR, r0) ) {
        osDelay(1) ;
    }
    {
        int milli = 0 ;
riprova:
        r0 = reg_leggi(REG__0_CTR) ;
        while ( ERRORE_REG_L == r0 ) {
            r0 = reg_leggi(REG__0_CTR) ;
        }
        if ( r0 & REG0_RST ) {
            osDelay(1) ;
            milli++ ;
            goto riprova ;
        }
        DDB_DEBUG("r0 = %08X (%d ms)", r0, milli) ;
    }

    do {
        uint32_t id_l = reg_leggi(REG__3_ID_MNF) ;
        if ( ERRORE_REG_L == id_l ) {
            DDB_ERR ;
            break ;
        }
        else if ( 0 == id_l ) {
            DDB_ERR ;
            break ;
        }

        uint32_t id_h = reg_leggi(REG__2_ID_OUI) ;
        if ( ERRORE_REG_L == id_h ) {
            DDB_ERR ;
            break ;
        }
        else if ( 0 == id_h ) {
            DDB_ERR ;
            break ;
        }

        // tengo anche la revisione per il collaudo
        id_phy = ( ( ( uint32_t ) id_h ) << 16 ) | id_l ;
        DDB_DEBUG("PHY %X", id_phy) ;

        // tolgo revisione per gestire la famiglia
        ulPhyID = id_phy & 0xFFFFFFF0 ;
        DDB_DEBUG("phy %X", ulPhyID) ;

        trovato = true ;
    } while ( false ) ;

    return trovato ;
}

static uint16_t read_mmd(
    uint32_t devadd,
    uint32_t index)
{
    // Write the MMD Access Control Register with 00b (address)
    // for the MMD Function field and the desired MMD
    // device (3 for PCS) for the MMD Device Address (DEVAD) field.
    reg_scrivi(REG_13_MMD_CTRL, REG13_ADDR | devadd) ;

    // Write the MMD Access Address/Data Register with the 16-bit address
    // of the desired MMD register to read/write within the previously
    // selected MMD device (PCS or Auto-Negotiation).
    reg_scrivi(REG_14_MMD_DATA, index) ;

    // Write the MMD Access Control Register with 01b (data) for the MMD
    // Function field and choose the previously selected MMD device
    // (3 for PCS) for the MMD Device Address (DEVAD) field.
    reg_scrivi(REG_13_MMD_CTRL, REG13_DATA | devadd) ;

    // If reading, read the MMD Access Address/Data Register, which
    // contains the selected MMD register contents.
    // If writing, write the MMD Access Address/Data Register with the
    // register contents intended for the previously selected MMD register.
    return reg_leggi(REG_14_MMD_DATA) ;
}

static void write_mmd(
    uint32_t devadd,
    uint32_t index,
    uint16_t val)
{
    // vedi read_mmd
    reg_scrivi(REG_13_MMD_CTRL, REG13_ADDR | devadd) ;

    reg_scrivi(REG_14_MMD_DATA, index) ;

    reg_scrivi(REG_13_MMD_CTRL, REG13_DATA | devadd) ;

    reg_scrivi(REG_14_MMD_DATA, val) ;
}

static uint32_t configura(
    PHY_SPEED speed,
    PHY_DUPLEX duplex)
{
    uint32_t ulAdvertise = 0 ;

    DDB_FUN ;

    /* Set advertise register. */
    if ( (speed == PHY_SPEED_AUTO) &&
         (duplex == PHY_DUPLEX_AUTO) ) {
        ulAdvertise = REG4_SF_802_3 | REG4_TA_10 | REG4_TA_10_FD
                      | REG4_TA_100 | REG4_TA_100_FD ;
        /* Reset auto-negotiation capability. */
    }
    else {
        /* Always select protocol 802.3u. */
        ulAdvertise = REG4_SF_802_3 ;

        if ( speed == PHY_SPEED_AUTO ) {
            if ( duplex == PHY_DUPLEX_FULL ) {
                ulAdvertise |= REG4_TA_10_FD | REG4_TA_100_FD ;
            }
            else {
                ulAdvertise |= REG4_TA_10 | REG4_TA_100 ;
            }
        }
        else if ( duplex == PHY_DUPLEX_AUTO ) {
            if ( speed == PHY_SPEED_10 ) {
                ulAdvertise |= REG4_TA_10_FD | REG4_TA_10 ;
            }
            else {
                ulAdvertise |= REG4_TA_100_FD | REG4_TA_100 ;
            }
        }
        else if ( speed == PHY_SPEED_100 ) {
            if ( duplex == PHY_DUPLEX_FULL ) {
                ulAdvertise |= REG4_TA_100_FD ;
            }
            else {
                ulAdvertise |= REG4_TA_100 ;
            }
        }
        else {
            if ( duplex == PHY_DUPLEX_FULL ) {
                ulAdvertise |= REG4_TA_10_FD ;
            }
            else {
                ulAdvertise |= REG4_TA_10 ;
            }
        }
    }

    /* Write advertise register. */
    reg_scrivi(REG__4_AN_A, ulAdvertise) ;

    /* Read Control register. */
    ulConfig = reg_leggi(REG__0_CTR) ;

    ulConfig &= NEGA(REG0_SL1 | REG0_DUPLEX) ;

    ulConfig |= REG0_AN_E ;

    if ( (speed == PHY_SPEED_100) ||
         (speed == PHY_SPEED_AUTO) ) {
        ulConfig |= REG0_SL1 ;
    }

    if ( (duplex == PHY_DUPLEX_FULL) ||
         (duplex == PHY_DUPLEX_AUTO) ) {
        ulConfig |= REG0_DUPLEX ;
    }

    DDB_DEBUG("PHY reg4 %04X config %04X", ulAdvertise, ulConfig) ;

    // Abilito IRQ
    if ( PHY_ID_LAN8742A == ulPhyID ) {
        // LED2 -> nINT
        uint16_t tmp = read_mmd(
            REG13_PCS,
            LAN8742A_IND_WUCSR) ;

        tmp &= NEGA(LAN8742A_WUCSR_LED2_MSK) ;
        tmp |= LAN8742A_WUCSR_LED2_nINT ;
        write_mmd(REG13_PCS,
                  LAN8742A_IND_WUCSR,
                  tmp) ;

        tmp = read_mmd(REG13_PCS,
                       LAN8742A_IND_WUCSR) ;
    }
    reg_scrivi(LAN87xx_IRQ_MSK, lanmsk) ;

    return ulAdvertise ;
}

static void negozia(void)
{
    DDB_FUN ;
    reg_scrivi(REG__0_CTR, ulConfig | REG0_R_AN) ;
}

void PHY_iniz(
    PHY_SPEED spid,
    PHY_DUPLEX dup)
{
    DDB_CONTROLLA( trova() ) ;
    (void) configura(spid, dup) ;
    negozia() ;
}

void PHY_isr(void)
{
    uint32_t irq = reg_leggi(LAN87xx_IRQ_SRC) ;
    irq &= lanmsk ;

    // Stampe
    DDB_DEBUG("PHY_EVENT %04X", irq) ;
    if ( irq & LAN87xx_IRQ_ENERGYON ) {
        DDB_PUTS("\t ENERGYON      ") ;
    }
    if ( irq & LAN87xx_IRQ_AUTO_NEGOTIATION_COMPLETE ) {
        DDB_PUTS("\t AUTO_NEGOTIATION_COMPLETE      ") ;
    }
    if ( irq & LAN87xx_IRQ_REMOTE_FAULT_DETECTED ) {
        DDB_PUTS("\t REMOTE_FAULT_DETECTED          ") ;
    }
    if ( irq & LAN87xx_IRQ_LINK_DOWN ) {
        DDB_PUTS("\t LINK_DOWN                      ") ;
    }
    if ( irq & LAN87xx_IRQ_AUTO_NEGOTIATION_LP_ACKNOWLEDGE ) {
        DDB_PUTS("\t AUTO_NEGOTIATION_LP_ACKNOWLEDGE") ;
    }
    if ( irq & LAN87xx_IRQ_PARALLEL_DETECTION_FAULT ) {
        DDB_PUTS("\t PARALLEL_DETECTION_FAULT       ") ;
    }
    if ( irq & LAN87xx_IRQ_AUTO_NEGOTIATION_PAGE_RECEIVED ) {
        DDB_PUTS("\t AUTO_NEGOTIATION_PAGE_RECEIVED ") ;
    }

    if ( irq
         & (LAN87xx_IRQ_LINK_DOWN | LAN87xx_IRQ_AUTO_NEGOTIATION_COMPLETE) ) {
        // Doppia lettura!
        // cfr https://www.oryx-embedded.com/doc/lan8720__driver_8c_source.html
        uint32_t stt = reg_leggi(REG__1_STT) ;
        stt = reg_leggi(REG__1_STT) ;

        DDB_DEBUG("\t stt %04X", stt) ;

        if ( stt & REG1_LS ) {
            // Bene
            if ( !link ) {
                DDB_PUTS("\t\t LINK") ;
                link = true ;

                stt = reg_leggi(LAN87xx_STT) ;

                full = stt & LAN87xx_STT_FULL ;
                DDB_PUTS(full ? "full duplex" : "half duplex") ;
                m100 = stt & LAN87xx_STT_100 ;
                DDB_PUTS(m100 ? "100 Mb" : "10 Mb") ;

                // mac (e altro)
                MAC_iniz(full, m100) ;
            }
        }
        else {
            // Male
            if ( link ) {
                DDB_PUTS("\t\t link") ;

                link = full = m100 = false ;

                MAC_fine() ;

                // phy
                negozia() ;
            }
        }
    }
}
