//#define STAMPA_DBG
#include "utili.h"
#include "mmc.h"

#ifdef DBG_ABIL
#define STAMPA_CID_CSD      1
#endif

#ifdef STAMPA_CID_CSD

static int meseanno(
    uint8_t md,
    char * * cmese)
{
    uint8_t mese = md >> 4 ;
    *cmese = "?" ;
    switch ( mese ) {
    case 1:
        *cmese = "gen" ;
        break ;
    case 2:
        *cmese = "feb" ;
        break ;
    case 3:
        *cmese = "mar" ;
        break ;
    case 4:
        *cmese = "apr" ;
        break ;
    case 5:
        *cmese = "mag" ;
        break ;
    case 6:
        *cmese = "giu" ;
        break ;
    case 7:
        *cmese = "lug" ;
        break ;
    case 8:
        *cmese = "ago" ;
        break ;
    case 9:
        *cmese = "set" ;
        break ;
    case 10:
        *cmese = "ott" ;
        break ;
    case 11:
        *cmese = "nov" ;
        break ;
    case 12:
        *cmese = "dic" ;
        break ;
    }
    return 2013 + (md & 0x0F) ;
}

void stampa_cid(void)
{
    S_MMC_CID cid ;

    MMC_cid(&cid) ;

    DBG_PRINT_HEX("CID", &cid, 16) ;
    DBG_PRINTF("\t Manufacturer ID:       %02X", cid.MID) ;
    DBG_PRINTF("\t Card/BGA:              %X", cid.CBX) ;
    DBG_PRINTF("\t OEM/Application ID:    %02X", cid.OID) ;
    char pn[6 + 1] ;
    memcpy(pn, cid.PNM, 6) ;
    pn[6] = 0 ;
    DBG_PRINTF("\t Product Name part:     %s", pn) ;
    DBG_PRINTF("\t Product Revision:      %02X", cid.PRV) ;
    DBG_PRINTF("\t Product Serial Number: %08X", cid.PSN) ;
    char * cmese ;
    int anno = meseanno(cid.MDT, &cmese) ;
    DBG_PRINTF("\t Manufacturing Date:    %02X -> %s %d",
               cid.MDT,
               cmese,
               anno) ;
}

void stampa_csd(void)
{
    S_MMC_CSD csd ;

    MMC_csd(&csd) ;

    DBG_PRINT_HEX("CSD", &csd, 16) ;

    DBG_PRINTF("\t crc %X", csd.crc) ;
    DBG_PRINTF("\t ecc %X", csd.ecc) ;
    DBG_PRINTF("\t FILE_FORMAT %X", csd.FILE_FORMAT) ;
    DBG_PRINTF("\t TMP_WRITE_PROTECT %X", csd.TMP_WRITE_PROTECT) ;
    DBG_PRINTF("\t PERM_WRITE_PROTECT %X", csd.PERM_WRITE_PROTECT) ;
    DBG_PRINTF("\t COPY %X", csd.COPY) ;
    DBG_PRINTF("\t FILE_FORMAT_GRP %X", csd.FILE_FORMAT_GRP) ;
    DBG_PRINTF("\t CONTENT_PROT_APP %X", csd.CONTENT_PROT_APP) ;
    DBG_PRINTF("\t WRITE_BL_PARTIAL %X", csd.WRITE_BL_PARTIAL) ;
    DBG_PRINTF("\t WRITE_BL_LEN %X", csd.WRITE_BL_LEN) ;
    DBG_PRINTF("\t R2W_FACTOR %X", csd.R2W_FACTOR) ;
    DBG_PRINTF("\t DEFAULT_ECC %X", csd.DEFAULT_ECC) ;
    DBG_PRINTF("\t WP_GRP_ENABLE %X", csd.WP_GRP_ENABLE) ;
    DBG_PRINTF("\t WP_GRP_SIZE %X", csd.WP_GRP_SIZE) ;
    DBG_PRINTF("\t ERASE_GRP_MULT %X", csd.ERASE_GRP_MULT) ;
    DBG_PRINTF("\t ERASE_GRP_SIZE %X", csd.ERASE_GRP_SIZE) ;
    DBG_PRINTF("\t C_SIZE_MULT %X", csd.C_SIZE_MULT) ;
    DBG_PRINTF("\t VDD_W_CURR_MAX %X", csd.VDD_W_CURR_MAX) ;
    DBG_PRINTF("\t VDD_W_CURR_MIN %X", csd.VDD_W_CURR_MIN) ;
    DBG_PRINTF("\t VDD_R_CURR_MAX %X", csd.VDD_R_CURR_MAX) ;
    DBG_PRINTF("\t VDD_R_CURR_MIN %X", csd.VDD_R_CURR_MIN) ;
    uint16_t c_size = csd.C_SIZE_h << 2 | csd.C_SIZE_l ;
    DBG_PRINTF("\t C_SIZE %X", c_size) ;
    DBG_PRINTF("\t DSR_IMP %X", csd.DSR_IMP) ;
    DBG_PRINTF("\t READ_BLK_MISALIGN %X", csd.READ_BLK_MISALIGN) ;
    DBG_PRINTF("\t WRITE_BLK_MISALIGN %X", csd.WRITE_BLK_MISALIGN) ;
    DBG_PRINTF("\t READ_BL_PARTIAL %X", csd.READ_BL_PARTIAL) ;
    DBG_PRINTF("\t READ_BL_LEN %X", csd.READ_BL_LEN) ;
    DBG_PRINTF("\t CCC %X", csd.CCC) ;
    DBG_PRINTF("\t TRAN_SPEED %X", csd.TRAN_SPEED) ;
    DBG_PRINTF("\t NSAC %X", csd.NSAC) ;
    DBG_PRINTF("\t TAAC %X", csd.TAAC) ;
    DBG_PRINTF("\t SPEC_VERS %X", csd.SPEC_VERS) ;
    DBG_PRINTF("\t CSD_STRUCTURE %X", csd.CSD_STRUCTURE) ;
}

void stampa_ecsd(void)
{
    S_MMC_EXT_CSD csd ;

    MMC_ext_csd(&csd) ;

    DBG_PRINT_HEX( "ECSD", &csd, sizeof(S_MMC_EXT_CSD) ) ;

    DBG_PUTS("\t Modes Segment") ;
    DBG_PRINTF("\t\t CMDQ_MODE_EN %X", csd.CMDQ_MODE_EN) ;
    DBG_PRINTF("\t\t SECURE_REMOVAL_TYPE %X", csd.SECURE_REMOVAL_TYPE) ;
    DBG_PRINTF("\t\t PRODUCT_STATE_AWARENESS_ENABLEMENT %X",
               csd.PRODUCT_STATE_AWARENESS_ENABLEMENT) ;
    DBG_PRINT_HEX("\t\t MAX_PRE_LOADING_DATA_SIZE",
                  csd.MAX_PRE_LOADING_DATA_SIZE,
                  4) ;
    DBG_PRINT_HEX("\t\t PRE_LOADING_DATA_SIZE",
                  csd.PRE_LOADING_DATA_SIZE,
                  4) ;
    DBG_PRINTF("\t\t FFU_STATUS %X", csd.FFU_STATUS) ;
    DBG_PRINTF("\t\t MODE_OPERATION_CODES %X", csd.MODE_OPERATION_CODES) ;
    DBG_PRINTF("\t\t MODE_CONFIG %X", csd.MODE_CONFIG) ;
    DBG_PRINTF("\t\t BARRIER_CTRL %X", csd.BARRIER_CTRL) ;
    DBG_PRINTF("\t\t FLUSH_CACHE %X", csd.FLUSH_CACHE) ;
    DBG_PRINTF("\t\t CACHE_CTRL %X", csd.CACHE_CTRL) ;
    DBG_PRINTF("\t\t POWER_OFF_NOTIFICATION %X", csd.POWER_OFF_NOTIFICATION) ;
    DBG_PRINTF("\t\t PACKED_FAILURE_INDEX %X", csd.PACKED_FAILURE_INDEX) ;
    DBG_PRINTF("\t\t PACKED_COMMAND_STATUS %X", csd.PACKED_COMMAND_STATUS) ;
    DBG_PRINT_HEX("\t\t CONTEXT_CONF", csd.CONTEXT_CONF, 15) ;
    DBG_PRINT_HEX("\t\t EXT_PARTITIONS_ATTRIBUTE",
                  csd.EXT_PARTITIONS_ATTRIBUTE,
                  2) ;
    DBG_PRINT_HEX("\t\t EXCEPTION_EVENTS_STATUS",
                  csd.EXCEPTION_EVENTS_STATUS,
                  2) ;
    DBG_PRINT_HEX("\t\t EXCEPTION_EVENTS_CTRL",
                  csd.EXCEPTION_EVENTS_CTRL,
                  2) ;
    DBG_PRINTF("\t\t DYNCAP_NEEDED %X", csd.DYNCAP_NEEDED) ;
    DBG_PRINTF("\t\t CLASS_6_CTRL %X", csd.CLASS_6_CTRL) ;
    DBG_PRINTF("\t\t INI_TIMEOUT_EMU %X", csd.INI_TIMEOUT_EMU) ;
    DBG_PRINTF("\t\t DATA_SECTOR_SIZE %X", csd.DATA_SECTOR_SIZE) ;
    DBG_PRINTF("\t\t USE_NATIVE_SECTOR %X", csd.USE_NATIVE_SECTOR) ;
    DBG_PRINTF("\t\t NATIVE_SECTOR_SIZE %X", csd.NATIVE_SECTOR_SIZE) ;
    DBG_PRINT_HEX("\t\t VENDOR_SPECIFIC_FIELD",
                  csd.VENDOR_SPECIFIC_FIELD,
                  64) ;
    DBG_PRINTF("\t\t PROGRAM_CID_CSD_DDR_SUPPORT %X",
               csd.PROGRAM_CID_CSD_DDR_SUPPORT) ;
    DBG_PRINTF("\t\t PERIODIC_WAKEUP %X", csd.PERIODIC_WAKEUP) ;
    DBG_PRINTF("\t\t TCASE_SUPPORT %X", csd.TCASE_SUPPORT) ;
    DBG_PRINTF("\t\t PRODUCTION_STATE_AWARENESS %X",
               csd.PRODUCTION_STATE_AWARENESS) ;
    DBG_PRINTF("\t\t SEC_BAD_BLK_MGMNT %X", csd.SEC_BAD_BLK_MGMNT) ;
    DBG_PRINT_HEX("\t\t ENH_START_ADDR", csd.ENH_START_ADDR, 4) ;
    DBG_PRINT_HEX("\t\t ENH_SIZE_MULT", csd.ENH_SIZE_MULT, 3) ;
    DBG_PRINT_HEX("\t\t GP_SIZE_MULT", csd.GP_SIZE_MULT, 12) ;
    DBG_PRINTF("\t\t PARTITION_SETTING_COMPLETED %X",
               csd.PARTITION_SETTING_COMPLETED) ;
    DBG_PRINTF("\t\t PARTITIONS_ATTRIBUTE %X", csd.PARTITIONS_ATTRIBUTE) ;
    DBG_PRINT_HEX("\t\t MAX_ENH_SIZE_MULT", csd.MAX_ENH_SIZE_MULT, 3) ;
    DBG_PRINTF("\t\t PARTITIONING_SUPPORT %X", csd.PARTITIONING_SUPPORT) ;
    DBG_PRINTF("\t\t HPI_MGMT %X", csd.HPI_MGMT) ;
    DBG_PRINTF("\t\t RST_n_FUNCTION %X", csd.RST_n_FUNCTION) ;
    DBG_PRINTF("\t\t BKOPS_EN %X", csd.BKOPS_EN) ;
    DBG_PRINTF("\t\t BKOPS_START %X", csd.BKOPS_START) ;
    DBG_PRINTF("\t\t SANITIZE_START %X", csd.SANITIZE_START) ;
    DBG_PRINTF("\t\t WR_REL_PARAM %X", csd.WR_REL_PARAM) ;
    DBG_PRINTF("\t\t WR_REL_SET %X", csd.WR_REL_SET) ;
    DBG_PRINTF("\t\t RPMB_SIZE_MULT %X", csd.RPMB_SIZE_MULT) ;
    DBG_PRINTF("\t\t FW_CONFIG %X", csd.FW_CONFIG) ;
    DBG_PRINTF("\t\t USER_WP %X", csd.USER_WP) ;
    DBG_PRINTF("\t\t BOOT_WP %X", csd.BOOT_WP) ;
    DBG_PRINTF("\t\t BOOT_WP_STATUS %X", csd.BOOT_WP_STATUS) ;
    DBG_PRINTF("\t\t ERASE_GROUP_DEF %X", csd.ERASE_GROUP_DEF) ;
    DBG_PRINTF("\t\t BOOT_BUS_CONDITIONS %X", csd.BOOT_BUS_CONDITIONS) ;
    DBG_PRINTF("\t\t BOOT_CONFIG_PROT %X", csd.BOOT_CONFIG_PROT) ;
    DBG_PRINTF("\t\t PARTITION_CONFIG %X", csd.PARTITION_CONFIG) ;
    DBG_PRINTF("\t\t ERASED_MEM_CONT %X", csd.ERASED_MEM_CONT) ;
    DBG_PRINTF("\t\t BUS_WIDTH %X", csd.BUS_WIDTH) ;
    DBG_PRINTF("\t\t HS_TIMING %X", csd.HS_TIMING) ;
    DBG_PRINTF("\t\t POWER_CLASS %X", csd.POWER_CLASS) ;
    DBG_PRINTF("\t\t CMD_SET_REV %X", csd.CMD_SET_REV) ;
    DBG_PRINTF("\t\t CMD_SET %X", csd.CMD_SET) ;

    DBG_PUTS("\t Properties Segment") ;
    DBG_PRINTF("\t\t EXT_CSD_REV %X", csd.EXT_CSD_REV) ;
    DBG_PRINTF("\t\t CSD_STRUCTURE_VER %X", csd.CSD_STRUCTURE_VER) ;
    DBG_PRINTF("\t\t DEVICE_TYPE %X", csd.DEVICE_TYPE) ;
    DBG_PRINTF("\t\t DRIVER_STRENGTH %X", csd.DRIVER_STRENGTH) ;
    DBG_PRINTF("\t\t OUT_OF_INTERRUPT_TIME %X", csd.OUT_OF_INTERRUPT_TIME) ;
    DBG_PRINTF("\t\t PARTITION_SWITCH_TIME %X", csd.PARTITION_SWITCH_TIME) ;
    DBG_PRINTF("\t\t PWR_CL_52_195 %X", csd.PWR_CL_52_195) ;
    DBG_PRINTF("\t\t PWR_CL_26_195 %X", csd.PWR_CL_26_195) ;
    DBG_PRINTF("\t\t PWR_CL_52_360 %X", csd.PWR_CL_52_360) ;
    DBG_PRINTF("\t\t PWR_CL_26_360 %X", csd.PWR_CL_26_360) ;
    DBG_PRINTF("\t\t MIN_PERF_R_4_26 %X", csd.MIN_PERF_R_4_26) ;
    DBG_PRINTF("\t\t MIN_PERF_W_4_26 %X", csd.MIN_PERF_W_4_26) ;
    DBG_PRINTF("\t\t MIN_PERF_R_8_26_4_52 %X", csd.MIN_PERF_R_8_26_4_52) ;
    DBG_PRINTF("\t\t MIN_PERF_W_8_26_4_52 %X", csd.MIN_PERF_W_8_26_4_52) ;
    DBG_PRINTF("\t\t MIN_PERF_R_8_52 %X", csd.MIN_PERF_R_8_52) ;
    DBG_PRINTF("\t\t MIN_PERF_W_8_52 %X", csd.MIN_PERF_W_8_52) ;
    DBG_PRINT_HEX("\t\t SEC_COUNT", csd.SEC_COUNT, 4) ;
    DBG_PRINTF("\t\t S_A_TIMEOUT %X", csd.S_A_TIMEOUT) ;
    DBG_PRINTF("\t\t S_C_VCCQ %X", csd.S_C_VCCQ) ;
    DBG_PRINTF("\t\t S_C_VCC %X", csd.S_C_VCC) ;
    DBG_PRINTF("\t\t HC_WP_GRP_SIZE %X", csd.HC_WP_GRP_SIZE) ;
    DBG_PRINTF("\t\t REL_WR_SEC_C %X", csd.REL_WR_SEC_C) ;
    DBG_PRINTF("\t\t ERASE_TIMEOUT_MULT %X", csd.ERASE_TIMEOUT_MULT) ;
    DBG_PRINTF("\t\t HC_ERASE_GRP_SIZE %X", csd.HC_ERASE_GRP_SIZE) ;
    DBG_PRINTF("\t\t ACC_SIZE %X", csd.ACC_SIZE) ;
    DBG_PRINTF("\t\t BOOT_SIZE_MULT %X", csd.BOOT_SIZE_MULT) ;
    DBG_PRINTF("\t\t BOOT_INFO %X", csd.BOOT_INFO) ;
    DBG_PRINTF("\t\t SEC_TRIM_MULT %X", csd.SEC_TRIM_MULT) ;
    DBG_PRINTF("\t\t SEC_ERASE_MULT %X", csd.SEC_ERASE_MULT) ;
    DBG_PRINTF("\t\t SEC_FEATURE_SUPPORT %X", csd.SEC_FEATURE_SUPPORT) ;
    DBG_PRINTF("\t\t TRIM_MULT %X", csd.TRIM_MULT) ;
    DBG_PRINTF("\t\t MIN_PERF_DDR_R_8_52 %X", csd.MIN_PERF_DDR_R_8_52) ;
    DBG_PRINTF("\t\t MIN_PERF_DDR_W_8_52 %X", csd.MIN_PERF_DDR_W_8_52) ;
    DBG_PRINTF("\t\t PWR_CL_200_130 %X", csd.PWR_CL_200_130) ;
    DBG_PRINTF("\t\t PWR_CL_200_195 %X", csd.PWR_CL_200_195) ;
    DBG_PRINTF("\t\t PWR_CL_DDR_52_195 %X", csd.PWR_CL_DDR_52_195) ;
    DBG_PRINTF("\t\t PWR_CL_DDR_52_360 %X", csd.PWR_CL_DDR_52_360) ;
    DBG_PRINTF("\t\t INI_TIMEOUT_AP %X", csd.INI_TIMEOUT_AP) ;
    DBG_PRINT_HEX("\t\t CORRECTLY_PRG_SECTORS_NUM",
                  csd.CORRECTLY_PRG_SECTORS_NUM,
                  4) ;
    DBG_PRINTF("\t\t BKOPS_STATUS %X", csd.BKOPS_STATUS) ;
    DBG_PRINTF("\t\t POWER_OFF_LONG_TIME %X", csd.POWER_OFF_LONG_TIME) ;
    DBG_PRINTF("\t\t GENERIC_CMD6_TIME %X", csd.GENERIC_CMD6_TIME) ;
    DBG_PRINT_HEX("\t\t CACHE_SIZE", csd.CACHE_SIZE, 4) ;
    DBG_PRINTF("\t\t PWR_CL_DDR_200_360 %X", csd.PWR_CL_DDR_200_360) ;
    DBG_PRINT_HEX("\t\t FIRMWARE_VERSION", csd.FIRMWARE_VERSION, 8) ;
    DBG_PRINT_HEX("\t\t DEVICE_VERSION", csd.DEVICE_VERSION, 2) ;
    DBG_PRINTF("\t\t OPTIMAL_TRIM_UNIT_SIZE %X", csd.OPTIMAL_TRIM_UNIT_SIZE) ;
    DBG_PRINTF("\t\t OPTIMAL_WRITE_SIZE %X", csd.OPTIMAL_WRITE_SIZE) ;
    DBG_PRINTF("\t\t OPTIMAL_READ_SIZE %X", csd.OPTIMAL_READ_SIZE) ;
    DBG_PRINTF("\t\t PRE_EOL_INFO %X", csd.PRE_EOL_INFO) ;
    DBG_PRINTF("\t\t DEVICE_LIFE_TIME_EST_TYP_A %X",
               csd.DEVICE_LIFE_TIME_EST_TYP_A) ;
    DBG_PRINTF("\t\t DEVICE_LIFE_TIME_EST_TYP_B %X",
               csd.DEVICE_LIFE_TIME_EST_TYP_B) ;
    DBG_PRINT_HEX("\t\t VENDOR_PROPRIETARY_HEALTH_REPORT",
                  csd.VENDOR_PROPRIETARY_HEALTH_REPORT,
                  32) ;
    DBG_PRINT_HEX("\t\t NUMBER_OF_FW_SECTORS_CORRECTLY_PROGRAMMED",
                  csd.NUMBER_OF_FW_SECTORS_CORRECTLY_PROGRAMMED,
                  4) ;
    DBG_PRINTF("\t\t CMDQ_DEPTH %X", csd.CMDQ_DEPTH) ;
    DBG_PRINTF("\t\t CMDQ_SUPPORT %X", csd.CMDQ_SUPPORT) ;
    DBG_PRINTF("\t\t BARRIER_SUPPORT %X", csd.BARRIER_SUPPORT) ;
    DBG_PRINT_HEX("\t\t FFU_ARG", csd.FFU_ARG, 4) ;
    DBG_PRINTF("\t\t OPERATION_CODE_TIMEOUT %X", csd.OPERATION_CODE_TIMEOUT) ;
    DBG_PRINTF("\t\t FFU_FEATURES %X", csd.FFU_FEATURES) ;
    DBG_PRINTF("\t\t SUPPORTED_MODES %X", csd.SUPPORTED_MODES) ;
    DBG_PRINTF("\t\t EXT_SUPPORT %X", csd.EXT_SUPPORT) ;
    DBG_PRINTF("\t\t LARGE_UNIT_SIZE_M1 %X", csd.LARGE_UNIT_SIZE_M1) ;
    DBG_PRINTF("\t\t CONTEXT_CAPABILITIES %X", csd.CONTEXT_CAPABILITIES) ;
    DBG_PRINTF("\t\t TAG_RES_SIZE %X", csd.TAG_RES_SIZE) ;
    DBG_PRINTF("\t\t TAG_UNIT_SIZE %X", csd.TAG_UNIT_SIZE) ;
    DBG_PRINTF("\t\t DATA_TAG_SUPPORT %X", csd.DATA_TAG_SUPPORT) ;
    DBG_PRINTF("\t\t MAX_PACKED_WRITES %X", csd.MAX_PACKED_WRITES) ;
    DBG_PRINTF("\t\t MAX_PACKED_READS %X", csd.MAX_PACKED_READS) ;
    DBG_PRINTF("\t\t BKOPS_SUPPORT %X", csd.BKOPS_SUPPORT) ;
    DBG_PRINTF("\t\t HPI_FEATURES %X", csd.HPI_FEATURES) ;
    DBG_PRINTF("\t\t S_CMD_SET %X", csd.S_CMD_SET) ;
    DBG_PRINTF("\t\t EXT_SECURITY_ERR %X", csd.EXT_SECURITY_ERR) ;
}

#else

void stampa_cid(void) {}
void stampa_csd(void) {}
void stampa_ecsd(void){}

#endif
