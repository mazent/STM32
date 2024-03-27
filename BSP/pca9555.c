#define STAMPA_DBG
#include "utili.h"
#include "pca9555.h"
#include "i2c.h"


#define REG_INP_0       0
#define REG_INP_1       1
#define REG_OUT_0       2
#define REG_OUT_1       3
#define REG_INV_0       4
#define REG_INV_1       5
#define REG_CFG_0       6
#define REG_CFG_1       7


static bool chx_iniz(uint8_t ind)
{
    bool esito = false ;

    do {
#if 0
        // Polarity Inversion Registers: inverto tutto (attivo basso)
        {
            uint8_t x[3] = {
                REG_INV_0, 0xFF, 0xFF
            } ;
            if ( !I2C_scrivi( ind, x, sizeof(x) ) ) {
                break ;
            }
        }

        // Output port registers: disabilito tutto
        {
            uint8_t x[3] = {
                REG_OUT_0, 0, 0
            } ;
            if ( !I2C_scrivi( ind, x, sizeof(x) ) ) {
                break ;
            }
        }
#else
        // Output port registers: disabilito tutto
        {
            uint8_t x[3] = {
                REG_OUT_0, 0xFF, 0xFF
            } ;
            if ( !I2C_scrivi( ind, x, sizeof(x) ) ) {
                break ;
            }
        }
#endif

        // Configuration Registers: uscite
        {
            uint8_t x[3] = {
                REG_CFG_0, 0, 0
            } ;
            if ( !I2C_scrivi( ind, x, sizeof(x) ) ) {
                break ;
            }
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool ch1_iniz(void)
{
	DBG_FUN;

    return chx_iniz(IND_PCA9555_U21) ;
}

static bool ch2_iniz(void)
{
	DBG_FUN;

    return chx_iniz(IND_PCA9555_U22) ;
}

bool IOE_iniz(void)
{
    return ch1_iniz() && ch2_iniz() ;
}

static bool scrivi(
    uint8_t ind,
    uint16_t pin)
{
    uint8_t x[3] = {
        REG_OUT_0,
        (uint8_t) pin,
        (uint8_t) (pin >> 8)
    } ;

    return I2C_scrivi( ind, x, sizeof(x) ) ;
}

bool IOE_u21(uint16_t val)
{
    return scrivi(IND_PCA9555_U21, val) ;
}

bool IOE_u22(uint16_t val)
{
    return scrivi(IND_PCA9555_U22, val) ;
}

bool IOE_u21_l(uint16_t * p)
{
    uint8_t x = REG_INP_0 ;
    if ( I2C_scrivi(IND_PCA9555_U21, &x, 1) ) {
        return I2C_leggi( IND_PCA9555_U21, p, sizeof(uint16_t) ) ;
    }

    return false ;
}

bool IOE_u22_l(uint16_t * p)
{
    uint8_t x = REG_INP_0 ;
    if ( I2C_scrivi(IND_PCA9555_U22, &x, 1) ) {
        return I2C_leggi( IND_PCA9555_U22, p, sizeof(uint16_t) ) ;
    }

    return false ;
}

void IOE_fine(void)
{
    // Configuration Registers: ingressi
    uint8_t x[3] = {
        REG_CFG_0, 0xFF, 0xFF
    } ;
    (void) I2C_scrivi( IND_PCA9555_U21, x, sizeof(x) ) ;
    (void) I2C_scrivi( IND_PCA9555_U22, x, sizeof(x) ) ;
}
