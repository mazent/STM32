#ifndef APP_PCA9555_H_
#define APP_PCA9555_H_

#include <stdbool.h>
#include <stdint.h>

// 0100.011r
#define IND_PCA9555_U21     0x46
// 0100.010r
#define IND_PCA9555_U22     0x44


#define U21_CH1_2_K0L6       (1 << 0)
#define U21_CH1_2_L3K11      (1 << 1)
#define U21_CH1_2_K1L7       (1 << 2)
#define U21_CH1_2_K5L11      (1 << 3)
#define U21_CH1_2_K2L8       (1 << 4)
#define U21_CH1_2_K3L9       (1 << 5)
#define U21_CH1_2_K4L10      (1 << 6)
#define U21_CH1_2_L4K12      (1 << 7)
#define U21_CH1_2_L1K9       (1 << 8)
#define U21_CH1_2_K6L12      (1 << 9)
#define U21_CH1_2_L2K10      (1 << 10)
#define U21_CH1_2_L5K13      (1 << 11)
#define U21_CH1_2_L0K8       (1 << 12)
#define U21_CH1_2_KTO        (1 << 13)
#define U21_CH1_NU           (1 << 14)
#define U21_KDIR             (1 << 15)

#define U22_CH2_2_K0L6      (1 << 0)
#define U22_CH2_2_L3K11     (1 << 1)
#define U22_CH2_2_K1L7      (1 << 2)
#define U22_CH2_2_K5L11     (1 << 3)
#define U22_CH2_2_K2L8      (1 << 4)
#define U22_CH2_2_K3L9      (1 << 5)
#define U22_CH2_2_K4L10     (1 << 6)
#define U22_CH2_2_L4K12     (1 << 7)
#define U22_CH2_2_L1K9      (1 << 8)
#define U22_CH2_2_K6L12     (1 << 9)
#define U22_CH2_2_L2K10     (1 << 10)
#define U22_CH2_2_L5K13     (1 << 11)
#define U22_CH2_2_L0K8      (1 << 12)
#define U22_CH2_2_L_TO      (1 << 13)
#define U22_CH1_NU1         (1 << 14)
#define U22_CH1_NU2         (1 << 15)

// Le uscite sono tutte attive basse e sono girate! o anche no
bool IOE_iniz(void) ;

// Tutti ingressi
void IOE_fine(void) ;

bool IOE_u21(uint16_t /*val*/) ;
bool IOE_u22(uint16_t /*val*/) ;

bool IOE_u21_l(uint16_t * /*p*/) ;
bool IOE_u22_l(uint16_t * /*p*/) ;

#else
#   warning pca9555.h incluso
#endif
