#ifndef MMC_H_
#define MMC_H_

#include <stdbool.h>
#include <stdint.h>

bool MMC_iniz(void) ;

bool MMC_leggi(uint32_t /*blocco*/, void * /*dati*/) ;
bool MMC_scrivi(uint32_t /*blocco*/, const void * /*dati*/) ;

bool MMC_trim(
    uint32_t da,
    uint32_t a) ;

// Dopo MMC_iniz si usa MMC_PART_USER
typedef enum {
    MMC_PART_NONVALE,
    MMC_PART_BOOT_1,
    MMC_PART_BOOT_2,
    MMC_PART_RPMB,
    MMC_PART_USER
} MMC_PART ;

bool MMC_access(MMC_PART /*part*/) ;

// Caratteristiche
// -------------------------------

#pragma pack(1)

typedef struct {
    uint8_t
        nu0 : 1,
        crc : 7 ;
    uint8_t MDT ;
    uint32_t PSN ;
    uint8_t PRV ;
    char PNM[6] ;
    uint8_t OID ;
    uint8_t
        CBX : 2,
        ris114 : 6 ;
    uint8_t MID ;
} S_MMC_CID ;

#pragma pack()

typedef struct {
    uint32_t
        nu0 : 1,
        crc : 7,
        ecc : 2,
        FILE_FORMAT : 2,
        TMP_WRITE_PROTECT : 1,
        PERM_WRITE_PROTECT : 1,
        COPY : 1,
        FILE_FORMAT_GRP : 1,
        CONTENT_PROT_APP : 1,
        ris17 : 4,
        WRITE_BL_PARTIAL : 1,
        WRITE_BL_LEN : 4,
        R2W_FACTOR : 3,
        DEFAULT_ECC : 2,
        WP_GRP_ENABLE : 1 ;
    uint32_t
        WP_GRP_SIZE : 5,
        ERASE_GRP_MULT : 5,
        ERASE_GRP_SIZE : 5,
        C_SIZE_MULT : 3,
        VDD_W_CURR_MAX : 3,
        VDD_W_CURR_MIN : 3,
        VDD_R_CURR_MAX : 3,
        VDD_R_CURR_MIN : 3,
        C_SIZE_l : 2 ;
    uint32_t
        C_SIZE_h : 10,
        res74 : 2,
        DSR_IMP : 1,
        READ_BLK_MISALIGN : 1,
        WRITE_BLK_MISALIGN : 1,
        READ_BL_PARTIAL : 1,
        READ_BL_LEN : 4,
        CCC : 12 ;
    uint32_t
        TRAN_SPEED : 8,
        NSAC : 8,
        TAAC : 8,
        ris120 : 2,
        SPEC_VERS : 4,
        CSD_STRUCTURE : 2 ;
} S_MMC_CSD ;

typedef struct {
    // Modes Segment
#ifdef MMC_SPEC_4_51
    // JEDEC Standard No. 84-B451
    // Electrical Standard 4.51
    uint8_t ris0[32] ;
#else
    // Samsung eMMC Product family
    // eMMC 5.1 Specification compatibility
    // Rev. 1.21 Jul. 2017
    uint8_t ris0[15] ;
    uint8_t CMDQ_MODE_EN ;
    uint8_t SECURE_REMOVAL_TYPE ;
    uint8_t PRODUCT_STATE_AWARENESS_ENABLEMENT ;
    uint8_t MAX_PRE_LOADING_DATA_SIZE[4] ;
    uint8_t PRE_LOADING_DATA_SIZE[4] ;
    uint8_t FFU_STATUS ;
    uint8_t ris27[2] ;
    uint8_t MODE_OPERATION_CODES ;
    uint8_t MODE_CONFIG ;
    uint8_t BARRIER_CTRL ;
#endif
    uint8_t FLUSH_CACHE ;
    uint8_t CACHE_CTRL ;
    uint8_t POWER_OFF_NOTIFICATION ;
    uint8_t PACKED_FAILURE_INDEX ;
    uint8_t PACKED_COMMAND_STATUS ;
    uint8_t CONTEXT_CONF[15] ;
    uint8_t EXT_PARTITIONS_ATTRIBUTE[2] ;
    uint8_t EXCEPTION_EVENTS_STATUS[2] ;
    uint8_t EXCEPTION_EVENTS_CTRL[2] ;
    uint8_t DYNCAP_NEEDED ;
    uint8_t CLASS_6_CTRL ;
    uint8_t INI_TIMEOUT_EMU ;
    uint8_t DATA_SECTOR_SIZE ;
    uint8_t USE_NATIVE_SECTOR ;
    uint8_t NATIVE_SECTOR_SIZE ;
    uint8_t VENDOR_SPECIFIC_FIELD[64] ;
    uint8_t ris128[2] ;
    uint8_t PROGRAM_CID_CSD_DDR_SUPPORT ;
    uint8_t PERIODIC_WAKEUP ;
    uint8_t TCASE_SUPPORT ;
#ifdef MMC_SPEC_4_51
    uint8_t ris133 ;
#else
    uint8_t PRODUCTION_STATE_AWARENESS ;
#endif
    uint8_t SEC_BAD_BLK_MGMNT ;
    uint8_t ris135 ;
    uint8_t ENH_START_ADDR[4] ;
    uint8_t ENH_SIZE_MULT[3] ;
    uint8_t GP_SIZE_MULT[12] ;
    uint8_t PARTITION_SETTING_COMPLETED ;
    uint8_t PARTITIONS_ATTRIBUTE ;
    uint8_t MAX_ENH_SIZE_MULT[3] ;
    uint8_t PARTITIONING_SUPPORT ;
    uint8_t HPI_MGMT ;
    uint8_t RST_n_FUNCTION ;
    uint8_t BKOPS_EN ;
    uint8_t BKOPS_START ;
    uint8_t SANITIZE_START ;
    uint8_t WR_REL_PARAM ;
    uint8_t WR_REL_SET ;
    uint8_t RPMB_SIZE_MULT ;  // rpmb part. in 128 KiB
    uint8_t FW_CONFIG ;
    uint8_t ris170 ;
    uint8_t USER_WP ;
    uint8_t ris172 ;
    uint8_t BOOT_WP ;
    uint8_t BOOT_WP_STATUS ;
    uint8_t ERASE_GROUP_DEF ;
    uint8_t ris176 ;
    uint8_t BOOT_BUS_CONDITIONS ;
    uint8_t BOOT_CONFIG_PROT ;
    uint8_t PARTITION_CONFIG ;
    uint8_t ris180 ;
    uint8_t ERASED_MEM_CONT ;
    uint8_t ris182 ;
    uint8_t BUS_WIDTH ;
    uint8_t ris184 ;
    uint8_t HS_TIMING ;
    uint8_t ris186 ;
    uint8_t POWER_CLASS ;
    uint8_t ris188 ;
    uint8_t CMD_SET_REV ;
    uint8_t ris190 ;
    uint8_t CMD_SET ;

    // Properties Segment
    uint8_t EXT_CSD_REV ;
    uint8_t ris193 ;
    uint8_t CSD_STRUCTURE_VER ;
    uint8_t ris195 ;
    uint8_t DEVICE_TYPE ;
    uint8_t DRIVER_STRENGTH ;
    uint8_t OUT_OF_INTERRUPT_TIME ;
    uint8_t PARTITION_SWITCH_TIME ;
    uint8_t PWR_CL_52_195 ;
    uint8_t PWR_CL_26_195 ;
    uint8_t PWR_CL_52_360 ;
    uint8_t PWR_CL_26_360 ;
    uint8_t ris204 ;
    uint8_t MIN_PERF_R_4_26 ;
    uint8_t MIN_PERF_W_4_26 ;
    uint8_t MIN_PERF_R_8_26_4_52 ;
    uint8_t MIN_PERF_W_8_26_4_52 ;
    uint8_t MIN_PERF_R_8_52 ;
    uint8_t MIN_PERF_W_8_52 ;
    uint8_t ris211 ;
    uint8_t SEC_COUNT[4] ;  // user data area
    uint8_t ris216 ;
    uint8_t S_A_TIMEOUT ;
    uint8_t ris218 ;
    uint8_t S_C_VCCQ ;
    uint8_t S_C_VCC ;
    uint8_t HC_WP_GRP_SIZE ;
    uint8_t REL_WR_SEC_C ;
    uint8_t ERASE_TIMEOUT_MULT ;
    uint8_t HC_ERASE_GRP_SIZE ;
    uint8_t ACC_SIZE ;
    uint8_t BOOT_SIZE_MULT ; // boot part. in 128 KiB
    uint8_t ris227 ;
    uint8_t BOOT_INFO ;
    uint8_t SEC_TRIM_MULT ;
    uint8_t SEC_ERASE_MULT ;
    uint8_t SEC_FEATURE_SUPPORT ;
    uint8_t TRIM_MULT ;
    uint8_t ris233 ;
    uint8_t MIN_PERF_DDR_R_8_52 ;
    uint8_t MIN_PERF_DDR_W_8_52 ;
    uint8_t PWR_CL_200_130 ;
    uint8_t PWR_CL_200_195 ;
    uint8_t PWR_CL_DDR_52_195 ;
    uint8_t PWR_CL_DDR_52_360 ;
    uint8_t ris240 ;
    uint8_t INI_TIMEOUT_AP ;
    uint8_t CORRECTLY_PRG_SECTORS_NUM[4] ;
    uint8_t BKOPS_STATUS ;
    uint8_t POWER_OFF_LONG_TIME ;
    uint8_t GENERIC_CMD6_TIME ;
    uint8_t CACHE_SIZE[4] ;
#ifdef MMC_SPEC_4_51
    uint8_t ris253[241] ;
#else
    uint8_t PWR_CL_DDR_200_360 ;
    uint8_t FIRMWARE_VERSION[8] ;
    uint8_t DEVICE_VERSION[2] ;
    uint8_t OPTIMAL_TRIM_UNIT_SIZE ;
    uint8_t OPTIMAL_WRITE_SIZE ;
    uint8_t OPTIMAL_READ_SIZE ;
    uint8_t PRE_EOL_INFO ;
    uint8_t DEVICE_LIFE_TIME_EST_TYP_A ;
    uint8_t DEVICE_LIFE_TIME_EST_TYP_B ;
    uint8_t VENDOR_PROPRIETARY_HEALTH_REPORT[32] ;
    uint8_t NUMBER_OF_FW_SECTORS_CORRECTLY_PROGRAMMED[4] ;
    uint8_t ris306 ;
    uint8_t CMDQ_DEPTH ;
    uint8_t CMDQ_SUPPORT ;
    uint8_t ris309[177] ;
    uint8_t BARRIER_SUPPORT ;
    uint8_t FFU_ARG[4] ;
    uint8_t OPERATION_CODE_TIMEOUT ;
    uint8_t FFU_FEATURES ;
    uint8_t SUPPORTED_MODES ;
#endif
    uint8_t EXT_SUPPORT ;
    uint8_t LARGE_UNIT_SIZE_M1 ;
    uint8_t CONTEXT_CAPABILITIES ;
    uint8_t TAG_RES_SIZE ;
    uint8_t TAG_UNIT_SIZE ;
    uint8_t DATA_TAG_SUPPORT ;
    uint8_t MAX_PACKED_WRITES ;
    uint8_t MAX_PACKED_READS ;
    uint8_t BKOPS_SUPPORT ;
    uint8_t HPI_FEATURES ;
    uint8_t S_CMD_SET ;
    uint8_t EXT_SECURITY_ERR ;
    uint8_t ris506[6] ;
} S_MMC_EXT_CSD ;

void MMC_cid(S_MMC_CID * cid) ;
void MMC_csd(S_MMC_CSD * csd) ;
void MMC_ext_csd(S_MMC_EXT_CSD * csd) ;

#else
#   warning mmc.h incluso
#endif
