set(CODICE
	eeprom/at24c512c.c
	fpga/MachXO3.c
	mmc/mmc.c
	mmc/rpmb.c               
	sdram/is42s16400j.c
)

add_library(dispi OBJECT ${CODICE})
