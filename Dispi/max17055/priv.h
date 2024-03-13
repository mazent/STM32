#ifndef DISPI_MAX17055_PRIV_H_
#define DISPI_MAX17055_PRIV_H_

// https://chromium.googlesource.com/chromiumos/platform/ec/+/master/driver/battery

/*
 * Convert the register values to the units that match
 * smart battery protocol.
 */

#define BATTERY_MAX17055_RSENSE		.01

/* Voltage reg value to mV */
#define VOLTAGE_CONV(REG)       ((REG * 5) >> 6)
/* Current reg value to mA */
#define CURRENT_CONV(REG)       ((((REG * 25) >> 4) / BATTERY_MAX17055_RSENSE) / 1000)
/* Capacity reg value to mAh */
#define CAPACITY_CONV(REG)      (REG * 5 / BATTERY_MAX17055_RSENSE)
/* Time reg value to minute */
#define TIME_CONV(REG)          ((REG * 3) >> 5)
/* Temperature reg value to 0.1K */
#define TEMPERATURE_CONV(REG)   (((REG * 10) >> 8) + 2731)
/* Percentage reg value to 1% */
#define PERCENTAGE_CONV(REG)    (REG >> 8)
/* Cycle count reg value (LSB = 1%) to absolute count (100%) */
#define CYCLE_COUNT_CONV(REG)	((REG * 5) >> 9)


#define REG_STATUS          0x00
#define REG_REPSOC			0x06
#define REG_AVGCURRENT		0x0B
#define REG_AVGTA			0x16
#define REG_CYCLES 			0x17
#define REG_DESIGNCAP       0x18
#define REG_AVGVCELL		0x19
#define REG_CONFIG          0x1D
#define REG_ICHGTERM        0x1E
#define REG_DEVNAME         0x21
#define REG_DIETEMP			0x34
#define REG_VEMPTY          0x3A
#define REG_FSTAT           0x3D
#define REG_TIMER 			0x3E
#define REG_DQACC           0x45
#define REG_HIBCFG          0xBA
#define REG_CONFIG2         0xBB
#define REG_TIMERH 			0xBE
#define REG_MODELCFG        0xDB

#define REG_SOFT_WAKEUP     0x60

#define VAL_DEVNAME         0x4010

#define BIT_STATUS_BR       (1 << 15)
#define BIT_STATUS_SMX      (1 << 14)
#define BIT_STATUS_TMX      (1 << 13)
#define BIT_STATUS_VMX      (1 << 12)
#define BIT_STATUS_BI       (1 << 11)
#define BIT_STATUS_SMN      (1 << 10)
#define BIT_STATUS_TMN      (1 << 9)
#define BIT_STATUS_VMN      (1 << 8)
#define BIT_STATUS_DSOCI    (1 << 7)
#define BIT_STATUS_IMX      (1 << 6)
// This bit is set to 0 when a battery is present (AIN is detected)
#define BIT_STATUS_BST      (1 << 3)
#define BIT_STATUS_IMN      (1 << 2)
#define BIT_STATUS_POR      (1 << 1)

#define BIT_CONFIG_TSEL     (1 << 15)
#define BIT_CONFIG_SS       (1 << 14)
#define BIT_CONFIG_TS       (1 << 13)
#define BIT_CONFIG_VS       (1 << 12)
#define BIT_CONFIG_IS       (1 << 11)
#define BIT_CONFIG_AINSH    (1 << 10)
#define BIT_CONFIG_TEN      (1 << 9)
#define BIT_CONFIG_TEX      (1 << 8)
#define BIT_CONFIG_SHDN     (1 << 7)
#define BIT_CONFIG_COMMSH   (1 << 6)
#define BIT_CONFIG_ETHRM    (1 << 4)
#define BIT_CONFIG_FTHRM    (1 << 3)
#define BIT_CONFIG_AEN      (1 << 2)
#define BIT_CONFIG_BEI      (1 << 1)
#define BIT_CONFIG_BER      (1 << 0)

#define BIT_CONFIG2_ATRATEEN    (1 << 13)
#define BIT_CONFIG2_DPEN        (1 << 12)
#define BIT_CONFIG2_POWR(a)     ( (a & 0x0F) << 8 )
#define BIT_CONFIG2_DSOCEN      (1 << 7)
#define BIT_CONFIG2_TALRTEN     (1 << 6)
#define BIT_CONFIG2_LDMDL       (1 << 5)
#define BIT_CONFIG2_4           (1 << 4)
#define BIT_CONFIG2_3_2         (2 << 3)
#define BIT_CONFIG2_CPMODE      (1 << 1)

#define BIT_FSTAT_RELDT     (1 << 9)
#define BIT_FSTAT_EDET      (1 << 8)
#define BIT_FSTAT_FQ        (1 << 7)
#define BIT_FSTAT_RELDT2    (1 << 6)
#define BIT_FSTAT_DNR       (1 << 0)

#define SW_CMD_CLEAR            0x0000
#define SW_CMD_SOFT_WAKEUP      0x0090

#define BIT_MODELCFG_REFRESH        (1 << 15)

#else
#   warning priv.h incluso
#endif
