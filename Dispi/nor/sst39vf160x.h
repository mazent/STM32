#ifndef DISPI_SST39VF160X_SST39VF160X_H_
#define DISPI_SST39VF160X_SST39VF160X_H_

/*
    Esempio di inizializzazione che usa RY/BY# con FMC a 96 MHz

    hnor1.Instance = FMC_NORSRAM_DEVICE ;
    hnor1.Extended = FMC_NORSRAM_EXTENDED_DEVICE ;
    hnor1.Init.NSBank = NORF_BANK ;
    hnor1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE ;
    hnor1.Init.MemoryType = FMC_MEMORY_TYPE_NOR ;
    hnor1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16 ;
    hnor1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE ;
    hnor1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE ;
    hnor1.Init.WaitSignal = FMC_WAIT_SIGNAL_ENABLE ;
    hnor1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW ;
    hnor1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_ENABLE ;
    hnor1.Init.WaitSignalActive = FMC_WAIT_TIMING_DURING_WS ;
    hnor1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE ;
    hnor1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE ;
    hnor1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ASYNC ;
    hnor1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE ;
    hnor1.Init.PageSize = FMC_PAGE_SIZE_NONE ;

	// Trc >= 70 ns
	FMC_NORSRAM_TimingTypeDef Timing = {
		.AddressSetupTime = 4,    // Toe >= 35 ns
		.DataSetupTime = 3,
		.AccessMode = FMC_ACCESS_MODE_B,
		// inutili (ma evito assert)
		.AddressHoldTime = 1,
		.CLKDivision = 2,
		.DataLatency = 2,
	} ;
*/

// Organized as 1M x16
#define NOR_SIZE            (1024 * 1024)

// Uniform 2 KWord sectors
#define NOR_SECTOR_SIZE     2048

#define NOR_SECTORS         (NOR_SIZE / NOR_SECTOR_SIZE)

// Durata massima
// Tbp  Word-Program    10 µs
// Tse  Sector-Erase    25 ms
// Tbe  Block-Erase     25 ms
// Tsce Chip-Erase      50 ms

#define NOR_SECTOR_ERASE_MS    25
#define NOR_BLOCK_ERASE_MS     25
#define NOR_CHIP_ERASE_MS      50

bool nor_erase_chip(void) ;

bool nor_erase_sector(uint32_t sector) ;

bool nor_program(
    uint32_t ofsW,
    uint16_t data) ;

uint16_t * nor_addr(uint32_t ofsW) ;

// restituisce manuf[2] seguito da dev[2]
bool nor_id(uint8_t * mem) ;

#else
#   warning sst39vf160x.h incluso
#endif
