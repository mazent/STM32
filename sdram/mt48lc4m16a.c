#include <stdbool.h>
#include "tram.h"

/*
 * MT48LC4M16A == IS42S16400J-6
 */

#define MR_BURSTLENGTH_1                ((uint32_t) (0 << 0))
#define MR_BURSTLENGTH_2                ((uint32_t) (1 << 0))
#define MR_BURSTLENGTH_4                ((uint32_t) (2 << 0))
#define MR_BURSTLENGTH_8                ((uint32_t) (3 << 0))
#define MR_BURSTLENGTH_FULLPAGE         ((uint32_t) (7 << 0))

#define MR_BURSTTYPE_SEQUENTIAL         ((uint32_t) (0 << 2))
#define MR_BURSTTYPE_INTERLEAVED        ((uint32_t) (1 << 2))

#define MR_CASLATENCY_1                 ((uint32_t) (1 << 4))
#define MR_CASLATENCY_2                 ((uint32_t) (2 << 4))
#define MR_CASLATENCY_3                 ((uint32_t) (3 << 4))

#define MR_OPERATINGMODE_STANDARD       ((uint32_t) (0 << 7))

#define MR_WRITEBURSTMODE_PROGRAMMED    ((uint32_t) (0 << 9))
#define MR_WRITEBURSTMODE_SINGLE        ((uint32_t) (1 << 9))

// 180 MHz / 2 -> 11.1 ns -> cl=2
#define CAS_LATENCY     2

static SDRAM_HandleTypeDef hsdram1 = {
    .State = HAL_SDRAM_STATE_RESET
} ;

static const uint32_t ramBase = 0xC0000000 ;
                         //  64        Mb     / in byte
static const size_t ramDim = 64 * 1024 * 1024 / 8  ;

static void SDRAM_Initialization_Sequence(void)
{
    FMC_SDRAM_CommandTypeDef command = {
        .CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1
    } ;
	
	do {
	

    /* Step 3:  Configure a clock configuration enable command */
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE ;
    command.AutoRefreshNumber = 1 ;

    /* Send the command */
    if (HAL_OK != HAL_SDRAM_SendCommand(&hsdram1, &command, 0x1000)) {
        break ;
    }

    /* Step 4: Insert 100 ms delay */
    // 100 us sul ds
    HAL_Delay(100) ;

    /* Step 5: Configure a PALL (precharge all) command */
    command.CommandMode = FMC_SDRAM_CMD_PALL ;
    command.AutoRefreshNumber = 1 ;

    /* Send the command */
    if (HAL_OK != HAL_SDRAM_SendCommand(&hsdram1, &command, 0x1000)) {
        break ;
    }

    /* Step 6 : Configure a Auto-Refresh command */
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE ;
    command.AutoRefreshNumber = 2 ;

    /* Send the command */
    if (HAL_OK != HAL_SDRAM_SendCommand(&hsdram1, &command, 0x1000)) {
        break ;
    }

    /* Step 7: Program the external memory mode register */
    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE ;
    command.AutoRefreshNumber = 1 ;
    command.ModeRegisterDefinition = MR_BURSTLENGTH_1 |
                                     MR_BURSTTYPE_SEQUENTIAL |
#if CAS_LATENCY == 2                                     
                                     MR_CASLATENCY_2 |
#elif CAS_LATENCY == 3                                     
                                     MR_CASLATENCY_3 |
#endif
                                     MR_OPERATINGMODE_STANDARD |
                                     MR_WRITEBURSTMODE_SINGLE ;

    /* Send the command */
    if (HAL_OK != HAL_SDRAM_SendCommand(&hsdram1, &command, 0x1000)) {
        break ;
    }

    /* Step 8: Set the refresh rate counter */
    // max=64 ms / 4096 row = 15.625us /11.1ns -> 1408
    // The refresh rate must be increased by 20 SDRAM clock cycles to obtain a safe margin
    // Ne tolgo 100
    HAL_SDRAM_ProgramRefreshRate(&hsdram1, 1308) ;
	
	} while (false) ;
}


void RAM_Iniz(void)
{
    FMC_SDRAM_TimingTypeDef SdramTiming ;

    hsdram1.Instance = FMC_SDRAM_DEVICE ;

    hsdram1.Init.SDBank = FMC_SDRAM_BANK1 ;
    hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8 ;
    hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12 ;
    hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16 ;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4 ;
    hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE ;
        // Fck = 180 MHz / 2
    hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2 ;
        // tCK = 11.1 ns -> CL=2
#if CAS_LATENCY == 2        
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2 ;
#elif CAS_LATENCY == 3        
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3 ;
#endif
    hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE ;
    hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0 ;

    // Clock 180/2=90 MHz => 11.1 ns
        // TMRD: 2 Clock cycles
    SdramTiming.LoadToActiveDelay = 2 ;
        // TXSR: min=67ns
    SdramTiming.ExitSelfRefreshDelay = 6 ;
        // TRAS: min=42ns max=120k
    SdramTiming.SelfRefreshTime = 4 ;
        // TRC: min=60ns
    SdramTiming.RowCycleDelay = 6 ;
        // TWR: 2 Clock cycles
    SdramTiming.WriteRecoveryTime = 2 ;
        // TRP: min=18ns
    SdramTiming.RPDelay = 2 ;
        // TRCD: min=18ns
    SdramTiming.RCDDelay = 2 ;

    HAL_SDRAM_Init(&hsdram1, &SdramTiming) ;

    SDRAM_Initialization_Sequence() ;
}

uint32_t RAM_AddrWalk1(void)
{
    return TRAM_AddrWalk1(ramBase, ramDim) ;
}

uint32_t RAM_AddrWalk0(void)
{
    return TRAM_AddrWalk0(ramBase, ramDim) ;
}

uint32_t RAM_DataWalk1e0(void)
{
    return TRAM_DataWalk1e0_16bit(ramBase) ;
}

void RAM_Finiz(void)
{
    HAL_SDRAM_DeInit(&hsdram1) ;
}

