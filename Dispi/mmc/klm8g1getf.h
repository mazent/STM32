#ifndef DISPI_MMC_KLM8G1GETF_H_
#define DISPI_MMC_KLM8G1GETF_H_

#define NATIVE_SECTOR_SIZE	512

#define RPMB_SIZE_MULT		4
#define RPMB_SIZE			(RPMB_SIZE_MULT * 128 * 1024)

#define BOOT_SIZE_MULT 		20
#define BOOT_SIZE			(BOOT_SIZE_MULT * 128 * 1024)

#define SEC_COUNT 			0x00E90000


#else
#	warning klm8g1getf.h incluso
#endif
