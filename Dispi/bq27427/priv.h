#ifndef DISPI_BQ27427_PRIV_H_
#define DISPI_BQ27427_PRIV_H_

// Varie del bq27427

#define REG_CNTL        0x00    /* Control */
#define REG_TEMP        0x02    /* Temperature */
#define REG_VOLT        0x04    /* Voltage */
#define REG_FLAGS       0x06    /* Flags */
#define REG_NAC         0x08    /* Nominal Available Capacity */
#define REG_FAC         0x0A    /* Full Available Capacity */
#define REG_RC          0x0C    /* Remaining Capacity */
#define REG_FCC         0x0E    /* Full Charge Capacity */
#define REG_AC          0x10    /* Average Current */
#define REG_AP          0x18    /* Average Power */
#define REG_SOC         0x1C    /* State-of-Charge */
#define REG_IT          0x1E    /* Internal Temperature */
#define REG_RCU         0x28    /* Remaining Capacity Unfiltered */
#define REG_RCF         0x2A    /* Remaining Capacity Filtered */
#define REG_FCCU        0x2C    /* Full Charge Capacity Unfiltered */
#define REG_FCCF        0x2E    /* Full Charge Capacity Filtered */
#define REG_SOCU        0x30    /* State-of-Charge Unfiltered */

#define EREG_DATACLASS          0x3E // sets the data class to be accessed
#define EREG_DATABLOCK          0x3F // sets the data block to be accessed
                                     // se ci sono piu' di BLOCKDATA_DIM byte
#define EREG_BLOCKDATA          0x40 // through 0x5F
#define EREG_BLOCKDATACHECKSUM  0x60 // checksum on the 32 bytes of block data read or written
#define EREG_BLOCKDATACONTROL   0x61 // Writing 0x00 to this command enables BlockData() to access RAM

#define CNTL_CONTROL_STATUS     0x0000
#define CNTL_DEVICE_TYPE        0x0001
#define CNTL_FW_VERSION         0x0002
#define CNTL_DM_CODE            0x0004
#define CNTL_PREV_MACWRITE      0x0007
#define CNTL_CHEM_ID            0x0008
#define CNTL_BAT_INSERT         0x000C
#define CNTL_BAT_REMOVE         0x000D
#define CNTL_SET_CFGUPDATE      0x0013      // Forces the CONTROL_STATUS [CFGUPMODE] bit to 1 and the gauge enters CONFIG UPDATE mode
#define CNTL_SMOOTH_SYNC        0x0019
#define CNTL_SHUTDOWN_ENABLE    0x001B
#define CNTL_SHUTDOWN           0x001C
#define CNTL_SEALED             0x0020      // Places the device in SEALED access mode
#define CNTL_PULSE_SOC_INT      0x0023
#define CNTL_CHEM_A             0x0030      // Dynamically changes existing Chem ID to Chem ID - 3230
#define CNTL_CHEM_B             0x0031      // Dynamically changes existing Chem ID to Chem ID - 1202
#define CNTL_CHEM_C             0x0032      // Dynamically changes existing Chem ID to Chem ID - 3142
#define CNTL_RESET              0x0041      // Performs a full device reset
#define CNTL_SOFT_RESET         0x0042      // Gauge exits CONFIG UPDATE mode
#define CNTL_UNS_KEY_1          0x8000
#define CNTL_UNS_KEY_2          0x8000

#define CONTROL_STATUS_SHUTDOWNEN   (1 << 15)
#define CONTROL_STATUS_WDRESET      (1 << 14)
#define CONTROL_STATUS_SS           (1 << 13)   // sealed mode
#define CONTROL_STATUS_CALMODE      (1 << 12)   // calibration mode
#define CONTROL_STATUS_CCA          (1 << 11)   // the Coulomb Counter Auto-Calibration routine is active
#define CONTROL_STATUS_BCA          (1 << 10)   // the fuel gauge board calibration routine is active
#define CONTROL_STATUS_QMAX_UP      (1 << 9)    // Qmax has updated (Qmax: learned maximum battery capacity)
#define CONTROL_STATUS_RES_UP       (1 << 8)    // resistance has been updated
#define CONTROL_STATUS_INITCOMP     (1 << 7)    // initialization is complete
#define CONTROL_STATUS_SLEEP        (1 << 4)
#define CONTROL_STATUS_LDMD         (1 << 3)    // the algorithm is using constant-power model
#define CONTROL_STATUS_RUP_DIS      (1 << 2)    // the Ra table updates are disabled
#define CONTROL_STATUS_VOK          (1 << 1)    // voltages are ok for Qmax updates
#define CONTROL_STATUS_CHEMCHANGE   (1 << 0)    // the Device Chemistry table has been dynamically changed

#define DEVICE_TYPE_VAL         0x0427

#define FLAG_OT           (1 << 15)     // Over-Temperature condition is detected
#define FLAG_UT           (1 << 14)     // Under-Temperature condition is detected
#define FLAG_FC           (1 << 9)      // Full charge is detected
#define FLAG_CHG          (1 << 8)      // Fast charging allowed
#define FLAG_OCVTAKEN     (1 << 7)      // set to 1 when OCV measurement is performed in RELAXATION mode
#define FLAG_DODCORRECT   (1 << 6)      // DOD correction is being applied
#define FLAG_ITPOR        (1 << 5)      // Indicates a POR or RESET
#define FLAG_CFGUPMODE    (1 << 4)      // CONFIG UPDATE mode
#define FLAG_BAT_DET      (1 << 3)      // Battery insertion detected
#define FLAG_SOC1         (1 << 2)      // StateOfCharge() <= SOC1 Set Threshold
#define FLAG_SOCF         (1 << 1)      // StateOfCharge() <= SOCF Set Threshold
#define FLAG_DSG          (1 << 0)      // Discharging detected

// RAM
#pragma pack(1)
typedef  struct {
    int16_t QmaxCell0 ;         // 0
    uint8_t UpdateStatus ;      // 2
    int16_t ReserveCapmAh ;     // 3
    uint8_t LoadSelectMode ;    // 5
    int16_t DesignCapacity ;    // 6
    int16_t DesignEnergy ;      // 8
    int16_t TerminateVoltage ;  // 10
    uint32_t vuoto12 ;
    int16_t TRise ;             // 16
    int16_t TTimeConstant ;     // 18
    uint8_t SOCIDelta ;         // 20
    int16_t TaperRate ;         // 21
    int16_t SleepCurrent ;      // 23
    int16_t AvgILastRun ;       // 25
    int16_t AvgPLastRun ;       // 27
    int16_t DeltaVoltage ;      // 29
    uint8_t vuoto31 ;
} GASGAUGING_STATE ;
#pragma pack()

#define GASGAUGING_STATE_SUBCLASS       82
#define GASGAUGING_STATE_BLOCK          0    // sizeof(GASGAUGING_STATE) / BLOCKDATA_DIM
#define GASGAUGING_STATE_QMAX_POS       0

#define RA_TABLES_SUBCLASS       89
#define RA_TABLES_BLOCK          0

#else
#   warning priv.h incluso
#endif
