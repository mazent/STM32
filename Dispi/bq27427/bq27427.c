#define STAMPA_DBG
#include "utili.h"
#include "priv.h"
#include "bq27427_cfg.h"

#define TMP_CAP     100

#define INITCOMP_MS        100
#define INITCOMP_CNT       10

#define SET_CFGUPDATE_MS        100
#define SET_CFGUPDATE_CNT       12

static bool leggi(
    uint8_t indir,
    void * v,
    uint8_t dim)
{
    bool esito = false ;

    do {
        if ( !bsp_gg_scrivi(&indir, 1) ) {
            DBG_ERR ;
            break ;
        }

        if ( !bsp_gg_leggi(v, dim) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool scrivi(
    uint8_t indir,
    const void * v,
    uint8_t dim)
{
    bool esito = false ;
    uint8_t tmp[TMP_CAP] = {
        indir
    } ;

    memcpy(tmp + 1, v, dim) ;

    do {
        if ( !bsp_gg_scrivi(tmp, 1 + dim) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cntl_cmd(
    uint16_t cntl,      // CNTL_xxx
    void * ret)
{
    bool esito = false ;

    do {
        if ( !scrivi( REG_CNTL, &cntl, sizeof(cntl) ) ) {
            DBG_ERR ;
            break ;
        }

        if ( ret ) {
            if ( !leggi( REG_CNTL, ret, sizeof(uint16_t) ) ) {
                DBG_ERR ;
                break ;
            }
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool flags(uint16_t * p)
{
    return leggi( REG_FLAGS, p, sizeof(uint16_t) ) ;
}

static bool unseal(void)
{
    bool esito = false ;

    do {
        uint16_t k = CNTL_UNS_KEY_1 ;
        if ( !scrivi( REG_CNTL, &k, sizeof(k) ) ) {
            DBG_ERR ;
            break ;
        }

        k = CNTL_UNS_KEY_2 ;
        if ( !scrivi( REG_CNTL, &k, sizeof(k) ) ) {
            DBG_ERR ;
            break ;
        }

        uint16_t s ;
        if ( !cntl_cmd(CNTL_CONTROL_STATUS, &s) ) {
            DBG_ERR ;
            break ;
        }

        esito = 0 == (CONTROL_STATUS_SS & s) ;
    } while ( false ) ;

    return esito ;
}

static bool set_cfgupdate(void)
{
    bool esito = false ;

    do {
        if ( !cntl_cmd(CNTL_SET_CFGUPDATE, NULL) ) {
            DBG_ERR ;
            break ;
        }

        for ( int _ = 0 ; _ < SET_CFGUPDATE_CNT ; _++ ) {
            uint16_t f ;
            if ( flags(&f) ) {
                if ( FLAG_CFGUPMODE & f ) {
                    esito = true ;
                    break ;
                }
            }
            bsp_millipausa(SET_CFGUPDATE_MS) ;
        }
    } while ( false ) ;

    return esito ;
}

static uint8_t block_data_checksum(const void * v)
{
    const uint8_t * bd = v ;
    uint8_t cs = 0 ;
    for ( int i = 0 ; i < BLOCKDATA_DIM ; ++i ) {
        cs += bd[i] ;
    }
    return 255 - cs ;
}

static bool cambia_iniz(void)
{
    bool esito = false ;

    do {
        if ( !unseal() ) {
            DBG_ERR ;
            break ;
        }

        if ( !set_cfgupdate() ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cambia_fine(void)
{
    bool esito = false ;

    do {
        if ( !cntl_cmd(CNTL_SOFT_RESET, NULL) ) {
            DBG_ERR ;
            break ;
        }

        for ( int _ = 0 ; _ < SET_CFGUPDATE_CNT ; _++ ) {
            uint16_t f ;
            if ( flags(&f) ) {
                if ( 0 == (FLAG_CFGUPMODE & f) ) {
                    esito = true ;
                    break ;
                }
            }
            bsp_millipausa(SET_CFGUPDATE_MS) ;
        }

        if ( !cntl_cmd(CNTL_SEALED, NULL) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cambia_chimica(void)
{
    bool esito = false ;

    do {
        uint16_t cid ;
        if ( !cntl_cmd(CNTL_CHEM_ID, &cid) ) {
            DBG_ERR ;
            break ;
        }

        if ( BQ27427_CHEM_ID == cid ) {
            esito = true ;
            break ;
        }

        if ( !cambia_iniz() ) {
            DBG_ERR ;
            break ;
        }

#if BQ27427_CHEM_ID == BQ27427_CHEM_ID_4_35_V
        uint16_t chi = CNTL_CHEM_A ;
#elif BQ27427_CHEM_ID == BQ27427_CHEM_ID_4_2_V
        uint16_t chi = CNTL_CHEM_B ;
#elif BQ27427_CHEM_ID == BQ27427_CHEM_ID_4_4_V
        uint16_t chi = CNTL_CHEM_C ;
#else
#error BQ27427_CHEM_ID
#endif
        if ( !cntl_cmd(chi, NULL) ) {
            DBG_ERR ;
            break ;
        }

        if ( !cambia_fine() ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cambia_prm(const BQ27427_RA * p)
{
    bool esito = false ;

    do {
        uint8_t bdc = 0 ;
        if ( !scrivi( EREG_BLOCKDATACONTROL, &bdc, sizeof(bdc) ) ) {
            DBG_ERR ;
            break ;
        }

        // Gas Gauging: State
        uint8_t subc = GASGAUGING_STATE_SUBCLASS ;
        if ( !scrivi( EREG_DATACLASS, &subc, sizeof(subc) ) ) {
            DBG_ERR ;
            break ;
        }
        uint8_t subb = GASGAUGING_STATE_BLOCK ;
        if ( !scrivi( EREG_DATABLOCK, &subb, sizeof(subb) ) ) {
            DBG_ERR ;
            break ;
        }
        uint8_t cs ;
        if ( !leggi( EREG_BLOCKDATACHECKSUM, &cs, sizeof(cs) ) ) {
            DBG_ERR ;
            break ;
        }
        GASGAUGING_STATE ggs ;
        if ( !leggi( EREG_BLOCKDATA, &ggs, sizeof(ggs) ) ) {
            DBG_ERR ;
            break ;
        }
        if ( cs != block_data_checksum(&ggs) ) {
            DBG_ERR ;
        }
        if ( p ) {
            ggs.QmaxCell0 = p->Qmax ;
        }
        ggs.DesignCapacity = BQ27427_DESIGN_CAPACITY_mAh ;
        ggs.DesignEnergy = BQ27427_DESIGN_ENERGY_mWh ;
        ggs.TerminateVoltage = BQ27427_TERMINATE_VOLTAGE_mV ;
        ggs.TaperRate = BQ27427_TAPER_RATE_hx10 ;
        cs = block_data_checksum(&ggs) ;
        if ( !scrivi( EREG_BLOCKDATA, &ggs, sizeof(ggs) ) ) {
            DBG_ERR ;
            break ;
        }
        if ( !scrivi( EREG_BLOCKDATACHECKSUM, &cs, sizeof(cs) ) ) {
            DBG_ERR ;
            break ;
        }
        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cambia_ra(const BQ27427_RA * p)
{
    bool esito = false ;

    do {
        uint8_t bdc = 0 ;
        if ( !scrivi( EREG_BLOCKDATACONTROL, &bdc, sizeof(bdc) ) ) {
            DBG_ERR ;
            break ;
        }

        uint8_t subc = RA_TABLES_SUBCLASS ;
        if ( !scrivi( EREG_DATACLASS, &subc, sizeof(subc) ) ) {
            DBG_ERR ;
            break ;
        }
        uint8_t subb = RA_TABLES_BLOCK ;
        if ( !scrivi( EREG_DATABLOCK, &subb, sizeof(subb) ) ) {
            DBG_ERR ;
            break ;
        }

        if ( !scrivi(EREG_BLOCKDATA, p->Ra, BLOCKDATA_DIM) ) {
            DBG_ERR ;
            break ;
        }
        uint8_t cs = block_data_checksum(p->Ra) ;
        if ( !scrivi( EREG_BLOCKDATACHECKSUM, &cs, sizeof(cs) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool cambia_batt(const BQ27427_RA * p)
{
    bool esito = false ;

    do {
        if ( !cambia_iniz() ) {
            DBG_ERR ;
            break ;
        }

        if ( !cambia_prm(p) ) {
            DBG_ERR ;
            break ;
        }

        if ( p ) {
            if ( !cambia_ra(p) ) {
                DBG_ERR ;
                break ;
            }
        }

        if ( !cambia_fine() ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool initialization_complete(void)
{
    bool esito = false ;

    for ( int _ = 0 ; _ < INITCOMP_CNT && !esito ; _++ ) {
        uint16_t s ;
        if ( !cntl_cmd(CNTL_CONTROL_STATUS, &s) ) {
            DBG_ERR ;
        }
        else {
            esito = CONTROL_STATUS_INITCOMP == (CONTROL_STATUS_INITCOMP & s) ;
        }
        bsp_millipausa(INITCOMP_MS) ;
    }

    return esito ;
}

bool BQ27427_iniz(const BQ27427_RA * p)
{
    bool esito = false ;

    static_assert(BLOCKDATA_DIM == sizeof(GASGAUGING_STATE), "OKKIO") ;

    do {
        if ( !initialization_complete() ) {
            DBG_ERR ;
            break ;
        }
#ifdef DBG_ABIL
        // cinema
        uint16_t dt ;
        if ( !cntl_cmd(CNTL_DEVICE_TYPE, &dt) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("Device type %04X (0427)", dt) ;

        uint16_t vfw ;
        if ( !cntl_cmd(CNTL_FW_VERSION, &vfw) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("Ver FW %04X", vfw) ;

        uint16_t cid ;
        if ( !cntl_cmd(CNTL_CHEM_ID, &cid) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("Chem ID %04X", cid) ;
#endif

        uint16_t f ;
        if ( !flags(&f) ) {
            DBG_ERR ;
            break ;
        }

        if ( FLAG_ITPOR & f ) {
            // Bisogna riconfigurare
            if ( !cambia_chimica() ) {
                DBG_ERR ;
                break ;
            }
            if ( !cambia_batt(p) ) {
                DBG_ERR ;
                break ;
            }
        }
        esito = true ;
    } while ( false ) ;

    if ( !esito ) {
        DBG_QUA ;

        if ( !unseal() ) {
            DBG_ERR ;
        }
        else if ( !cntl_cmd(CNTL_RESET, NULL) ) {
            DBG_ERR ;
        }
    }

    return esito ;
}

bool BQ27427_ra(BQ27427_RA * p)
{
    bool esito = false ;

    do {
        // Aggiornati?
        uint16_t s ;
        if ( !cntl_cmd(CNTL_CONTROL_STATUS, &s) ) {
            DBG_ERR ;
            break ;
        }
        if ( 0 == (CONTROL_STATUS_QMAX_UP | CONTROL_STATUS_RES_UP) & s ) {
            DBG_ERR ;
            break ;
        }

        // Preparazione
        if ( !unseal() ) {
            DBG_ERR ;
            break ;
        }

        uint8_t bdc = 0 ;
        if ( !scrivi( EREG_BLOCKDATACONTROL, &bdc, sizeof(bdc) ) ) {
            DBG_ERR ;
            break ;
        }

        // Qmax
        {
            uint8_t subc = GASGAUGING_STATE_SUBCLASS ;
            if ( !scrivi( EREG_DATACLASS, &subc, sizeof(subc) ) ) {
                DBG_ERR ;
                break ;
            }
            uint8_t subb = GASGAUGING_STATE_BLOCK ;
            if ( !scrivi( EREG_DATABLOCK, &subb, sizeof(subb) ) ) {
                DBG_ERR ;
                break ;
            }
#ifdef DBG_ABIL
            uint8_t cs ;
            if ( !leggi( EREG_BLOCKDATACHECKSUM, &cs, sizeof(cs) ) ) {
                DBG_ERR ;
                break ;
            }
            DBG_PRINTF("Qmax csum %02X", cs) ;
#endif
            int16_t Qmax ;
            if ( !leggi( EREG_BLOCKDATA + GASGAUGING_STATE_QMAX_POS, &Qmax,
                         sizeof(Qmax) ) ) {
                DBG_ERR ;
                break ;
            }
            p->Qmax = Qmax ;
        }

        // Ra
        {
            uint8_t subc = RA_TABLES_SUBCLASS ;
            if ( !scrivi( EREG_DATACLASS, &subc, sizeof(subc) ) ) {
                DBG_ERR ;
                break ;
            }
            uint8_t subb = RA_TABLES_BLOCK ;
            if ( !scrivi( EREG_DATABLOCK, &subb, sizeof(subb) ) ) {
                DBG_ERR ;
                break ;
            }
#ifdef DBG_ABIL
            uint8_t cs ;
            if ( !leggi( EREG_BLOCKDATACHECKSUM, &cs, sizeof(cs) ) ) {
                DBG_ERR ;
                break ;
            }
            DBG_PRINTF("Ra csum %02X", cs) ;
#endif
            if ( !leggi(EREG_BLOCKDATA, p->Ra, BLOCKDATA_DIM) ) {
                DBG_ERR ;
                break ;
            }
        }

        if ( !cntl_cmd(CNTL_SEALED, NULL) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    if ( !esito ) {
        DBG_QUA ;

        if ( !unseal() ) {
            DBG_ERR ;
        }
        else if ( !cntl_cmd(CNTL_RESET, NULL) ) {
            DBG_ERR ;
        }
    }

    return esito ;
}

bool BQ27427_temperature(void * p_uint16_t)
{
    return leggi( REG_TEMP, p_uint16_t, sizeof(uint16_t) ) ;
}

bool BQ27427_voltage(void * p_uint16_t)
{
    return leggi( REG_VOLT, p_uint16_t, sizeof(uint16_t) ) ;
}

bool BQ27427_averageCurrent(void * p_int16_t)
{
    return leggi( REG_AC, p_int16_t, sizeof(int16_t) ) ;
}

bool BQ27427_stateOfCharge(void * p_uint16_t)
{
    return leggi( REG_SOC, p_uint16_t, sizeof(uint16_t) ) ;
}

bool BQ27427_device_type(void * p_uint16_t)
{
    return cntl_cmd(CNTL_DEVICE_TYPE, p_uint16_t) ;
}

bool BQ27427_chem_id(void * p_uint16_t)
{
    return cntl_cmd(CNTL_CHEM_ID, p_uint16_t) ;
}

bool BQ27427_reset(void)
{
    bool esito = false ;

    if ( !unseal() ) {
        DBG_ERR ;
    }
    else if ( !cntl_cmd(CNTL_RESET, NULL) ) {
        DBG_ERR ;
    }
    else {
        esito = true ;
    }

    return esito ;
}

