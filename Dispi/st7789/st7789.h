#ifndef DISPI_ST7789_ST7789_H_
#define DISPI_ST7789_ST7789_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef LCD_WIDTH
// max 240
#define LCD_WIDTH    240
#endif
#ifndef LCD_HEIGHT
// max 320
#define LCD_HEIGHT   240
#endif

// ST7789V - Version 1.3 - 8.8.42 Write data for 16-bit/pixel - pag 105
typedef struct {
    uint16_t
        blu : 5,
        vrd : 6,
        rss : 5 ;
} S_RGB ;

#define RGB_MAX_BLU         0x1F
#define RGB_MAX_VERDE       0x3F
#define RGB_MAX_ROSSO       0x1F

#define RGB(r, g, b)                     \
    (uint16_t) (                         \
        (b & RGB_MAX_BLU)                \
        | ( (g & RGB_MAX_VERDE) << 5 )   \
        | ( (r & RGB_MAX_ROSSO) << 11 )  \
        )

bool ST7789_iniz(void) ;
void ST7789_fine(void) ;

bool ST7789_colora(const S_RGB *) ;
bool ST7789_disegna(size_t, size_t, const void *) ;

// Interfaccia
// ===================================
// SPI (max 6.6 Mbit, 8 bit)
// TSCYCR Serial clock cycle (Read) 150 ns
extern bool st7789_spi_iniz_cmd(void) ;
// SPI (max 15.1 Mbit, 16 bit)
// TSCYCW Serial clock cycle (Write) 66 ns
extern bool st7789_spi_iniz_pix(void) ;
// Comandi (bloccanti)
extern bool st7789_spi_tx(
    const void * v,
    uint16_t dim) ;
extern bool st7789_spi_txrx(
    const void * t,
    void * r,
    uint16_t dim) ;
// Trasmette con dma ...
extern bool st7789_spi_tx_pix(
    const void * v,
    size_t dimw) ;
// ... alla fine invoca
extern void st7789_spi_tx_pix_cb(void) ;

// Pin D/CX#
extern void bsp_st7789_d_cneg(bool) ;
// Pin RESX
extern void bsp_st7789_reset(bool) ;

// Altro
extern void bsp_millipausa(uint32_t) ;

#else
#   warning st7789.h incluso
#endif
