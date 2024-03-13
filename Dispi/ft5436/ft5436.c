#define STAMPA_DBG
#include "utili.h"
#include "ft5436.h"
#include "bsp.h"

#define REG_DEVICE_MODE     0x00
#define REG_GEST_ID         0x01
#define REG_TD_STATUS       0x02
#define REG_CHIP_ID         0xA3
#define REG_FW_ID           0xA6
#define REG_MANUF_ID        0xA8
#define REG_TOUCH1_XH       0x03
#define REG_TOUCH2_XH       0x09
#define REG_TOUCH3_XH       0x0F
#define REG_TOUCH4_XH       0x15
#define REG_TOUCH5_XH       0x1B
#define REG_ERR    0xFF

#define NORMAL_OPERATING_MODE   0

#define NUM_PUNTI       5

static FT5436_TOUCH punti[NUM_PUNTI] ;
static size_t quanti = 0 ;

static bool reg_leggi(
    uint8_t reg,
    uint8_t * val)
{
    if ( !bsp_touch_scrivi(&reg, 1) ) {
        DBG_ERR ;
        return false ;
    }

    if ( !bsp_touch_leggi(val, 1) ) {
        DBG_ERR ;
        return false ;
    }

    return true ;
}

static bool reg_scrivi(
    uint8_t reg,
    uint8_t val)
{
    uint8_t x[2] = {
        reg,
        val
    } ;
    return bsp_touch_scrivi( x, sizeof(x) ) ;
}

bool FT5436_iniz(void)
{
    bool esito = false ;

    static_assert(sizeof(uint32_t) == sizeof(FT5436_TOUCH), "OKKIO") ;

    // Trst min 1 ms
    bsp_touch_reset(false) ;
    bsp_millipausa(1 + 1) ;
    bsp_touch_reset(true) ;
    bsp_millipausa(100 + 1) ;

    do {
        if ( !reg_scrivi(REG_DEVICE_MODE, NORMAL_OPERATING_MODE) ) {
            DBG_ERR ;
            break ;
        }

        uint8_t val ;
        if ( !reg_leggi(REG_MANUF_ID, &val) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("manuf id %02X", val) ;

        if ( !reg_leggi(REG_CHIP_ID, &val) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("chip id %02X", val) ;

        if ( !reg_leggi(REG_FW_ID, &val) ) {
            DBG_ERR ;
            break ;
        }
        DBG_PRINTF("fw id %02X", val) ;

        esito = true ;
    } while ( false ) ;

    return esito ;
}

void FT5436_fine(void)
{
    bsp_touch_reset(false) ;
}

static bool leggi_touch_x(
    int x,
    FT5436_TOUCH * p)
{
    bool esito = false ;
    uint8_t base = REG_ERR ;

    static_assert(4 == sizeof(FT5436_TOUCH), "OKKIO") ;

    switch ( x ) {
    case 1:
        base = REG_TOUCH1_XH ;
        break ;
    case 2:
        base = REG_TOUCH2_XH ;
        break ;
    case 3:
        base = REG_TOUCH3_XH ;
        break ;
    case 4:
        base = REG_TOUCH4_XH ;
        break ;
    case 5:
        base = REG_TOUCH5_XH ;
        break ;
    default:
        DBG_ERR ;
        break ;
    }

    if ( REG_ERR != base ) {
        uint16_t tmp[2] ;
        if ( !bsp_touch_scrivi(&base, 1) ) {
            DBG_ERR ;
        }
        else if ( !bsp_touch_leggi( tmp, sizeof(tmp) ) ) {
            DBG_ERR ;
        }
        else {
            tmp[0] = __REV16(tmp[0]) ;
            tmp[1] = __REV16(tmp[1]) ;
            memcpy( p, tmp, sizeof(tmp) ) ;
            esito = true ;
        }
    }

    return esito ;
}

#define GID_MOVE_UP        0x10
#define GID_MOVE_LEFT      0x14
#define GID_MOVE_DOWN      0x18
#define GID_MOVE_RIGHT     0x1C
#define GID_ZOOM_IN        0x48
#define GID_ZOOM_OUT       0x49
#define GID_NO_GESTURE     0x00

void FT5436_isr(void)
{
#if 0
    // Non supportato
    uint8_t gid ;
    if ( reg_leggi(REG_GEST_ID, &gid) ) {
        switch ( gid ) {
        case GID_MOVE_UP:
            DBG_PUTS("MOVE UP") ;
            break ;
        case GID_MOVE_LEFT:
            DBG_PUTS("MOVE LEFT") ;
            break ;
        case GID_MOVE_DOWN:
            DBG_PUTS("MOVE DOWN") ;
            break ;
        case GID_MOVE_RIGHT:
            DBG_PUTS("MOVE RIGHT") ;
            break ;
        case GID_ZOOM_IN:
            DBG_PUTS("ZOOM IN") ;
            break ;
        case GID_ZOOM_OUT:
            DBG_PUTS("ZOOM OUT") ;
            break ;
        case GID_NO_GESTURE:
            break ;
        default:
            DBG_PRINTF("? gid %02X ?", gid) ;
            break ;
        }
    }
    else {
        DBG_ERR ;
    }
#endif
    uint8_t num_punti ;
    if ( reg_leggi(REG_TD_STATUS, &num_punti) ) {
        DBG_PRINTF("%s: %d punti", __func__, num_punti) ;

        if ( num_punti > NUM_PUNTI ) {
            DBG_QUA ;
            num_punti = NUM_PUNTI ;
        }

        FT5436_TOUCH * t = punti ;
        quanti = 0 ;
        for ( int i = 0 ; i < num_punti ; i++, t++ ) {
            if ( leggi_touch_x(i + 1, t) ) {
                quanti++ ;

                DBG_PRINTF("Punto %d", i + 1) ;
                DBG_PRINTF("    x = %d", t->x) ;
                switch ( t->evn ) {
                case EVN_PUT_DOWN:
                    DBG_PUTS("    Put Down") ;
                    break ;
                case EVN_PUT_UP:
                    DBG_PUTS("    Put Up") ;
                    break ;
                case EVN_CONTACT:
                    DBG_PUTS("    Contact") ;
                    break ;
                case EVN_RESERVED:
                    DBG_PUTS("    Reserved") ;
                    break ;
                }
                DBG_PRINTF("    y = %d", t->y) ;
                DBG_PRINTF("    id = %X", t->id) ;
            }
            else {
                DBG_ERR ;
            }
        }
    }
    else {
        DBG_ERR ;
    }
}

size_t FT5436_punti(void * t)
{
    size_t np = 0 ;

    if ( quanti ) {
        memcpy( t, punti, quanti * sizeof(FT5436_TOUCH) ) ;
        np = quanti ;
        quanti = 0 ;
    }

    return np ;
}
