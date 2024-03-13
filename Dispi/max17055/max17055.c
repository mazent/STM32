#define STAMPA_DBG
#include "utili.h"
#include "priv.h"
#include "max17055_cfg.h"

//#define STAMPA_ROBA		1

#define CONTA_POR       12
#define MILLI_POR       100

#define CONTA_CFG       10
#define MILLI_CFG       100

#define CONTA_IC       10
#define MILLI_IC       1

static bool leggi(
    uint8_t indir,
    void * v)
{
    bool esito = false ;

    do {
        if ( !bsp_gg_scrivi(&indir, 1) ) {
            DBG_ERR ;
            break ;
        }

        if ( !bsp_gg_leggi( v, sizeof(uint16_t) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool scrivi(
    uint8_t indir,
    const void * v)
{
    bool esito = false ;
    uint8_t tmp[1 + sizeof(uint16_t)] = {
        indir
    } ;

    memcpy( tmp + 1, v, sizeof(uint16_t) ) ;

    do {
        if ( !bsp_gg_scrivi( tmp, sizeof(tmp) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool MAX17055_iniz(void)
{
    bool esito = false ;

    do {
        // https://ez.analog.com/power/battery-management-system/w/documents/30780/when-max17055-is-powered-up-from-a-dead-battery-by-usb-charging-the-devname-is-read-but-return-value-is-different-from-value-in-datasheet-why
        uint16_t lui ;
        if ( !leggi(REG_DEVNAME, &lui) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("devname %04X", lui) ;
        if ( VAL_DEVNAME != lui ) {
            DBG_ERR ;
            break ;
        }

        // 1. Check for POR
        uint16_t reg_status ;
        if ( !leggi(REG_STATUS, &reg_status) ) {
            DBG_ERR ;
            break ;
        }

        if ( reg_status & BIT_STATUS_POR ) {
            DBG_PUTS("POR!") ;
            // 2. Delay until FSTAT.DNR bit == 0
            // After power-up, wait for the MAX17055 to complete its startup operations
            // This takes 710ms from power-up.
            int conta = 0 ;
            for ( ; conta < CONTA_POR ; conta++ ) {
                uint16_t fstat ;
                if ( leggi(REG_FSTAT, &fstat) ) {
                    if ( 0 == (fstat & BIT_FSTAT_DNR) ) {
                        break ;
                    }
                }
                else {
                    DBG_QUA ;
                }

                bsp_millipausa(MILLI_POR) ;
            }
            if ( CONTA_POR == conta ) {
                DBG_ERR ;
                break ;
            }

            // 3. Initialize configuration
            // Store original HibCFG value
            uint16_t hibcfg ;
            if ( !leggi(REG_HIBCFG, &hibcfg) ) {
                DBG_ERR ;
                break ;
            }

            // Exit Hibernate Mode step 1
            uint16_t cmd = SW_CMD_SOFT_WAKEUP ;
            if ( !scrivi(REG_SOFT_WAKEUP, &cmd) ) {
                DBG_ERR ;
                break ;
            }
            // Exit Hibernate Mode step 2
            cmd = 0 ;
            if ( !scrivi(REG_HIBCFG, &cmd) ) {
                DBG_ERR ;
                break ;
            }
            // Exit Hibernate Mode step 3
            if ( !scrivi(REG_SOFT_WAKEUP, &cmd) ) {
                DBG_ERR ;
                break ;
            }

            // 3.1 OPTION 1 EZ Config (no INI file is needed):
            // Write DesignCap
            uint16_t cfg = MAX17055_DESIGN_CAP ;
            if ( !scrivi(REG_DESIGNCAP, &cfg) ) {
                DBG_ERR ;
                break ;
            }
#if 0
            // Write dQAcc
            // This register tracks change in battery charge between relaxation points. It is available to the user for debug purposes
            cfg <<= 6 ;
            if ( !scrivi(REG_DQACC, &cfg) ) {
                DBG_ERR ;
                break ;
            }
#endif
            // Write IchgTerm
            cfg = MAX17055_ICHGTERM ;
            if ( !scrivi(REG_ICHGTERM, &cfg) ) {
                DBG_ERR ;
                break ;
            }
#ifdef MAX17055_EMPTY_VOLTAGE
            // Write VEmpty
            cfg = MAX17055_EMPTY_VOLTAGE ;
            if ( !scrivi(REG_VEMPTY, &cfg) ) {
                DBG_ERR ;
                break ;
            }
#endif
            // Write ModelCFG
            cfg = MAX17055_MODELCFG ;
            if ( !scrivi(REG_MODELCFG, &cfg) ) {
                DBG_ERR ;
                break ;
            }

            // Poll ModelCFG.Refresh(highest bit), proceed to Step 4 when ModelCFG.Refresh = 0.
            for ( conta = 0 ; conta < CONTA_CFG ; conta++ ) {
                if ( leggi(REG_MODELCFG, &cfg) ) {
                    if ( 0 == (cfg & BIT_MODELCFG_REFRESH) ) {
                        break ;
                    }
                }
                else {
                    DBG_QUA ;
                }

                bsp_millipausa(MILLI_CFG) ;
            }
            if ( CONTA_CFG == conta ) {
                DBG_ERR ;
                break ;
            }

            // Restore Original HibCFG value
            if ( !scrivi(REG_HIBCFG, &hibcfg) ) {
                DBG_ERR ;
                break ;
            }

            // 4. Initialization complete
            if ( !leggi(REG_STATUS, &reg_status) ) {
                DBG_ERR ;
                break ;
            }
            // Write and Verify Status with POR bit cleared
            reg_status &= NEGA(BIT_STATUS_POR) ;
            for ( conta = 0 ; conta < CONTA_IC ; conta++ ) {
                if ( !scrivi(REG_STATUS, &reg_status) ) {
                    DBG_ERR ;
                }

                bsp_millipausa(MILLI_IC) ;

                if ( leggi(REG_STATUS, &reg_status) ) {
                    if ( 0 == (reg_status & BIT_STATUS_POR) ) {
                        break ;
                    }
                }
                else {
                    DBG_ERR ;
                }
            }
            if ( CONTA_IC == conta ) {
                DBG_ERR ;
                break ;
            }
        }
        else {
            DBG_PUTS("por") ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool MAX17055_temperature(void * p_uint16_t)
{
    int16_t reg ;
    bool esito = leggi(REG_AVGTA, &reg) ;
    if ( esito ) {
#ifdef STAMPA_ROBA
        DBG_PRINTF("temp %04X", reg) ;
#endif
        uint16_t conv = TEMPERATURE_CONV(reg) ;
        memcpy( p_uint16_t, &conv, sizeof(conv) ) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool MAX17055_voltage(void * p_uint16_t)
{
    uint16_t reg ;
    bool esito = leggi(REG_AVGVCELL, &reg) ;
    if ( esito ) {
#ifdef STAMPA_ROBA
        DBG_PRINTF("tens %04X", reg) ;
#endif
        uint16_t conv = VOLTAGE_CONV(reg) ;
        memcpy( p_uint16_t, &conv, sizeof(conv) ) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool MAX17055_current(void * p_int16_t)
{
    int16_t reg ;
    bool esito = leggi(REG_AVGCURRENT, &reg) ;
    if ( esito ) {
#ifdef STAMPA_ROBA
        DBG_PRINTF("corr %04X", reg) ;
#endif
        int16_t conv = (int) CURRENT_CONV(reg) ;
        memcpy( p_int16_t, &conv, sizeof(conv) ) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool MAX17055_stateOfCharge(void * p_uint16_t)
{
    uint16_t reg ;
    bool esito = leggi(REG_REPSOC, &reg) ;
    if ( esito ) {
#ifdef STAMPA_ROBA
        DBG_PRINTF("soc %04X", reg) ;
#endif
        uint16_t conv = PERCENTAGE_CONV(reg) ;
        memcpy( p_uint16_t, &conv, sizeof(conv) ) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool MAX17055_timer(void * p_uint32_t)
{
    bool esito = false ;

    do {
        union {
            uint32_t t32 ;
            uint16_t t16[2] ;
        } u ;
rileggi:
        if ( !leggi(REG_TIMER, &u.t16[0]) ) {
            DBG_ERR ;
            break ;
        }
        if ( 0xFFFF == u.t16[0] ) {
            // aspetto overflow
            bsp_millipausa(200) ;
            goto rileggi ;
        }

        if ( !leggi(REG_TIMERH, &u.t16[1]) ) {
            DBG_ERR ;
            break ;
        }

#ifdef STAMPA_ROBA
        DBG_PRINTF("tim %08X", u.t32) ;
#endif

        double x = u.t32 * .1758 ;
        uint32_t y = (int) (x + .5) ;
        memcpy( p_uint32_t, &y, sizeof(y) ) ;
        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool MAX17055_cicli(void * p_uint16_t)
{
    uint16_t reg ;
    bool esito = leggi(REG_CYCLES, &reg) ;
    if ( esito ) {
        memcpy( p_uint16_t, &reg, sizeof(reg) ) ;
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}
