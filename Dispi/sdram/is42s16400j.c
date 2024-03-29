#include "bsp.h"
#include "sdram.h"

#ifdef HAL_SDRAM_MODULE_ENABLED

#if SDRAM_BANK == FMC_SDRAM_BANK1
#   define FMC_SDRAM_CMD_TARGET_BANK    FMC_SDRAM_CMD_TARGET_BANK1
#else
#   define FMC_SDRAM_CMD_TARGET_BANK    FMC_SDRAM_CMD_TARGET_BANK2
#endif

#define REFRESH_COUNT       ( (uint32_t) 0x056A )   /* SDRAM refresh counter (90MHz SDRAM clock) */

#define SDRAM_MODEREG_BURST_LENGTH_1             ( (uint16_t) 0x0000 )
#define SDRAM_MODEREG_BURST_LENGTH_2             ( (uint16_t) 0x0001 )
#define SDRAM_MODEREG_BURST_LENGTH_4             ( (uint16_t) 0x0002 )
#define SDRAM_MODEREG_BURST_LENGTH_8             ( (uint16_t) 0x0004 )
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ( (uint16_t) 0x0000 )
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ( (uint16_t) 0x0008 )
#define SDRAM_MODEREG_CAS_LATENCY_2              ( (uint16_t) 0x0020 )
#define SDRAM_MODEREG_CAS_LATENCY_3              ( (uint16_t) 0x0030 )
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ( (uint16_t) 0x0000 )
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ( (uint16_t) 0x0000 )
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ( (uint16_t) 0x0200 )

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef * hsdram)
{
    FMC_SDRAM_CommandTypeDef Command ;
    __IO uint32_t tmpmrd = 0 ;

    /* Step 3:  Configure a clock configuration enable command */
    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE ;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK ;
    Command.AutoRefreshNumber = 1 ;
    Command.ModeRegisterDefinition = 0 ;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000) ;

    /* Step 4: Insert 100 ms delay */
    HAL_Delay(100) ;

    /* Step 5: Configure a PALL (precharge all) command */
    Command.CommandMode = FMC_SDRAM_CMD_PALL ;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK ;
    Command.AutoRefreshNumber = 1 ;
    Command.ModeRegisterDefinition = 0 ;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000) ;

    /* Step 6 : Configure a Auto-Refresh command */
    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE ;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK ;
    Command.AutoRefreshNumber = 4 ;
    Command.ModeRegisterDefinition = 0 ;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000) ;

    /* Step 7: Program the external memory mode register */
    tmpmrd = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_2
             | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
             | SDRAM_MODEREG_CAS_LATENCY_3
             | SDRAM_MODEREG_OPERATING_MODE_STANDARD
             | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ;

    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE ;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK ;
    Command.AutoRefreshNumber = 1 ;
    Command.ModeRegisterDefinition = tmpmrd ;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000) ;

    /* Step 8: Set the refresh rate counter */
    /* (15.62 us x Freq) - 20 */
    /* Set the device refresh counter */
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT) ;
}

#endif
