#define STAMPA_DBG
#include "utili.h"
#include "st7789.h"

#define DIM_IMG     (57600)

/*
 * cfr:
 * https://blog.embeddedexpert.io/?p=1215
 * https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/st7789.c#L31
 */

#define X_SHIFT 0
#define Y_SHIFT 0

//#define RDDID       0x04    // Read Display ID (Non funziona)
#define SLPOUT      0x11    // Sleep Out
#define NORON       0x13    // Normal Display Mode On
#define INVON       0x21    // Display Inversion On
#define DISPON      0x29    // Display On
#define CASET       0x2A    // Column Address Set
#define RASET       0x2B    // Row Address Set
#define RAMWR       0x2C    // Memory Write
#define MADCTL      0x36    // Memory Data Access Control
#define COLMOD      0x3A    // Interface Pixel Format
#define RAMCTRL     0xB0    // RAM Control
#define RGBCTRL     0xB1    // RGB Interface Control
#define PORCTRL     0xB2    // Porch Setting
#define GCTRL       0xB7    // Gate Control
#define VCOMS       0xBB    // VCOMS Setting
#define LCMCTRL     0xC0    // LCM Control
#define VDVVRHEN    0xC2    // VDV and VRH Command Enable
#define VRHS        0xC3    // VRH Set
#define VDVS        0xC4    // VDV Set
#define FRCTRL2     0xC6    // Frame Rate Control in Normal Mode
#define PWCTRL1     0xD0    // Power Control 1
#define RDID1       0xDA    // Read ID1
#define RDID2       0xDB    // Read ID2
#define RDID3       0xDC    // Read ID3
#define PVGAMCTRL   0xE0    // Positive Voltage Gamma Control
#define NVGAMCTRL   0xE1    // Negative Voltage Gamma Control

#define VAL_ID1       0x85
#define VAL_ID2       0x85
#define VAL_ID3       0x52

//  RGB565 (16bit)
// The Command 3Ah should be set at 55h when writing 16-bit/pixel data into
// frame memory, but 3Ah should be re-set to 66h when reading pixel data from
// frame memory
#define VAL_COLMOD      0x55

#define VAL_VCOMS       0x32
#define VAL_VRHS        0x19

typedef struct {
    uint8_t reg ;
    const void * prm ;
    size_t dim ;
} UN_COMANDO ;

static const uint8_t colmod = VAL_COLMOD ;
static const uint8_t vcoms = VAL_VCOMS ;
static const uint8_t vrhs = VAL_VRHS ;

static const uint8_t pwctrl1[] = {
    0xA4, 0xA1
} ;
static const uint8_t pvgamctrl[] = {
    0xD0,
    0x08,
    0x0E,
    0x09,
    0x09,
    0x05,
    0x31,
    0x33,
    0x48,
    0x17,
    0x14,
    0x15,
    0x31,
    0x34,
} ;
static const uint8_t nvgamctrl[] = {
    0xD0,
    0x08,
    0x0E,
    0x09,
    0x09,
    0x15,
    0x31,
    0x33,
    0x48,
    0x17,
    0x14,
    0x15,
    0x31,
    0x34,
} ;

static const UN_COMANDO vIniz[] = {
    // cfr void initi(void) nel codice del cinese
    // KD015QVFMA001-4SPI-16BIT RGB CODE.txt
    // -------------------------------------------
    // tolti comandi inutili + quelli con prm sbagliati
    //write_command(0x11);
    //delay(120);      //Delay 120ms
    // SLPOUT lo scrivo dopo

    //write_command(0x36);
    //write_data(0x00);
    // MZ { MADCTL, &zero, sizeof(zero) },
    //write_command(0x3A);
    //write_data(0x55);//65K
    { COLMOD, &colmod, sizeof(colmod) },
    //write_command(0xB2);
    //write_data(0x0C);
    //write_data(0x0C);
    //write_data(0x00);
    //write_data(0x33);
    //write_data(0x33);
    // MZ { PORCTRL, porctrl, sizeof(porctrl) },
    //write_command(0xB7);//VGH VGL
    //write_data(0x35);
    // MZ { GCTRL, &gctrl, 1 },
    //write_command(0xBB);
    //write_data(0x32); //Vcom=1.35V
    { VCOMS, &vcoms, 1 },
    //write_command(0xC2);
    //write_data(0x01);
    // manca un parametro fisso!
    // MZ { VDVVRHEN, &vdvvrhen, sizeof(vdvvrhen) },
    //write_command(0xC3);
    //write_data(0x19); //GVDD=4.8V
    { VRHS, &vrhs, 1 },
    //write_command(0xC4);
    //write_data(0x20); //VDV, 0x20:0v
    // MZ { VDVS, &vdvs, 1 },
    //write_command(0xC6);
    //write_data(0x0F); //0x0F:60Hz
    // MZ { FRCTRL2, &frctrl2, sizeof(frctrl2) },
    //write_command(0xD0);
    //write_data(0xA4);
    //write_data(0xA1);
    { PWCTRL1, pwctrl1, sizeof(pwctrl1) },
    //write_command(0xE0);  //GAMMA
    //write_data(0xD0);
    //write_data(0x08);
    //write_data(0x0E);
    //write_data(0x09);
    //write_data(0x09);
    //write_data(0x05);
    //write_data(0x31);
    //write_data(0x33);
    //write_data(0x48);
    //write_data(0x17);
    //write_data(0x14);
    //write_data(0x15);
    //write_data(0x31);
    //write_data(0x34);
    { PVGAMCTRL, pvgamctrl, sizeof(pvgamctrl) },
    //write_command(0xE1); //GAMMA
    //write_data(0xD0);
    //write_data(0x08);
    //write_data(0x0E);
    //write_data(0x09);
    //write_data(0x09);
    //write_data(0x15);
    //write_data(0x31);
    //write_data(0x33);
    //write_data(0x48);
    //write_data(0x17);
    //write_data(0x14);
    //write_data(0x15);
    //write_data(0x31);
    //write_data(0x34);
    { NVGAMCTRL, nvgamctrl, sizeof(nvgamctrl) },
    //write_command(0x21);
    { INVON, NULL, 0 },
    //write_command(0xB0);
    //write_data(0x11); //set RGB interface
    //write_data(0x00);
    // ????????? { RAMCTRL, ramctrl, sizeof(ramctrl) },
    //write_command(0xB1);
    //write_data(0x40); // set DE mode ; SET Hs,Vs,DE,DOTCLK signal polarity
    //write_data(0x00);
    //write_data(0x00);
    // ??? { RGBCTRL, rgbctrl, sizeof(rgbctrl) }
    //write_command(0x29); //display on
    //write_command(0x2c);
    // DISPON lo mando dopo
} ;

static
__attribute__( ( section(".no_cache") ) )
uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT] = {
    0
} ;

static bool reg_scrivi(
    uint8_t reg,
    const void * v,
    size_t dim)
{
    bool esito = false ;

    do {
        bsp_st7789_d_cneg(false) ;

        if ( !st7789_spi_tx( &reg, sizeof(reg) ) ) {
            DBG_ERR ;
            break ;
        }

        bsp_st7789_d_cneg(true) ;

        if ( v ) {
            if ( !st7789_spi_tx(v, dim) ) {
                DBG_ERR ;
                break ;
            }
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool set_address_window(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1)
{
    bool esito = false ;
    uint16_t x_start = x0 + X_SHIFT, x_end = x1 + X_SHIFT ;
    uint16_t y_start = y0 + Y_SHIFT, y_end = y1 + Y_SHIFT ;

    do {
        /* Column Address set */
        uint16_t cas[4] = {
            __REV16(x_start),
            __REV16(x_end),
        } ;
        if ( !reg_scrivi( CASET, cas, sizeof(cas) ) ) {
            DBG_ERR ;
            break ;
        }

        /* Row Address set */
        uint16_t ras[4] = {
            __REV16(y_start),
            __REV16(y_end),
        } ;
        if ( !reg_scrivi( RASET, ras, sizeof(ras) ) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

static bool invia_frame(
    size_t dx,
    size_t dy)
{
    bool esito = false ;

    do {
        if ( !st7789_spi_iniz_cmd() ) {
            DBG_ERR ;
            break ;
        }

        if ( !set_address_window(0, 0,
                                 dx - 1, dy - 1) ) {
            DBG_ERR ;
            break ;
        }

        if ( !reg_scrivi(RAMWR, NULL, 0) ) {
            DBG_ERR ;
            break ;
        }

        if ( !st7789_spi_iniz_pix() ) {
            DBG_ERR ;
            break ;
        }

        if ( !st7789_spi_tx_pix(frame_buffer,
                                dx * dy) ) {
            DBG_ERR ;
            break ;
        }

        esito = true ;
    } while ( false ) ;

    return esito ;
}

void ST7789_fine(void)
{
    bsp_st7789_reset(false) ;
}

bool ST7789_iniz(void)
{
    bool esito = true ;

#if 1
    const uint16_t rosso = RGB(RGB_MAX_ROSSO, 0, 0) ;
    for ( size_t i = 0 ; i < DIM_VETT(frame_buffer) ; i++ ) {
        frame_buffer[i] = rosso ;
    }
#else
    for ( size_t i = 0 ; i < DIM_IMG ; i++ ) {
        frame_buffer[i] = __REV16(immagine[i]) ;
    }
#endif

    // cfr void initi(void) nel codice del cinese
    // Nn si sa mai
    // res=1;
    // delay(1);
    bsp_st7789_reset(true) ;
    bsp_millipausa(1 + 1) ;

    // Trw: Reset pulse duration 10 us min
    // res=0;
    // delay(10);
    bsp_st7789_reset(false) ;
    bsp_millipausa(10) ;

    // Trt: Da 5 a 120 ms
    // res=1;
    // delay(120);
    bsp_st7789_reset(true) ;
    bsp_millipausa(120 + 1) ;

    CONTROLLA( st7789_spi_iniz_cmd() ) ;

    {
        // Verifico ID
        uint8_t cmd[] = {
            RDID1, 0
        } ;
        uint8_t rsp[sizeof(cmd)] ;

        if ( st7789_spi_txrx( cmd, rsp, sizeof(cmd) ) ) {
            if ( rsp[1] != VAL_ID1 ) {
                DBG_ERR ;
                DBG_PRINTF("    ID1 %02X != %02X", rsp[1], VAL_ID1) ;
                return false ;
            }
        }

        cmd[0] = RDID2 ;
        if ( st7789_spi_txrx( cmd, rsp, sizeof(cmd) ) ) {
            if ( rsp[1] != VAL_ID2 ) {
                DBG_ERR ;
                DBG_PRINTF("    ID2 %02X != %02X", rsp[1], VAL_ID2) ;
                return false ;
            }
        }

        cmd[0] = RDID3 ;
        if ( st7789_spi_txrx( cmd, rsp, sizeof(cmd) ) ) {
            if ( rsp[1] != VAL_ID3 ) {
                DBG_ERR ;
                DBG_PRINTF("    ID3 %02X != %02X", rsp[1], VAL_ID3) ;
                return false ;
            }
        }
    }

    const UN_COMANDO * uc = vIniz ;
    for ( size_t _ = 0 ; _ < DIM_VETT(vIniz) ; _++, uc++ ) {
        if ( !reg_scrivi(uc->reg, uc->prm, uc->dim) ) {
            DBG_ERR ;
            esito = false ;
            break ;
        }
    }

    do {
        // Out of sleep mode
        if ( !reg_scrivi(SLPOUT, NULL, 0) ) {
            DBG_ERR ;
            esito = false ;
            break ;
        }
        // It will be necessary to wait 5msec before sending any new commands to a display module following this command to
        // allow time for the supply voltages and clock circuits to stabilize.
        //
        // It will be necessary to wait 120msec after sending sleep out command (when in sleep in mode) before sending an sleep
        // in command.
        bsp_millipausa(150) ;
#if 0
        // Gia' cosi' dopo hw reset
        // Normal Display on
        if ( !reg_scrivi(NORON, NULL, 0) ) {
            DBG_ERR ;
            esito = false ;
            break ;
        }
        // cfr https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/st7789.c
        // che scrive (anche) registri inesistenti
        bsp_millipausa(150) ;
#endif
        // Main screen turned on
        if ( !reg_scrivi(DISPON, NULL, 0) ) {
            DBG_ERR ;
            esito = false ;
            break ;
        }
        bsp_millipausa(50) ;
    } while ( false ) ;

    if ( esito ) {
        esito = invia_frame(LCD_WIDTH, LCD_HEIGHT) ;
    }

    return esito ;
}

bool ST7789_colora(const S_RGB * x)
{
    uint16_t tmp ;

    memcpy( &tmp, x, sizeof(tmp) ) ;

    if ( frame_buffer[0] != tmp ) {
        for ( size_t i = 0 ; i < DIM_VETT(frame_buffer) ; i++ ) {
            frame_buffer[i] = tmp ;
        }
    }

    return invia_frame(LCD_WIDTH, LCD_HEIGHT) ;
}

bool ST7789_disegna(
    size_t dx,
    size_t dy,
    const void * v)
{
    memcpy( frame_buffer, v, dx * dy * sizeof(uint16_t) ) ;
    return invia_frame(dx, dy) ;
}

__WEAK void st7789_spi_tx_pix_cb(void)
{
    DBG_PRINTF("IMPLEMENTA %s (se ti serve)", __func__) ;
}
