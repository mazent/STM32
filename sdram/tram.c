#define STAMPA_DBG
#include "includimi.h"

/******************************************

    Test della ram

******************************************/

#define SPIA		((uint8_t) 0x55)

uint32_t TRAM_AddrWalk1(uint32_t Base, uint32_t numByte)
{
	volatile uint8_t * pRam = (uint8_t *) Base ;
	uint32_t uno = 1 ;

	// Segnalino
	pRam[0] = SPIA ;

	// Ciclo sugli indirizzi
	while (uno != numByte) {
		pRam[uno] = ~SPIA ;

		if (SPIA != pRam[0])
			break ;

		uno <<= 1 ;
	}

	if (numByte == uno)
		uno = 0 ;

	return uno ;
}

uint32_t TRAM_AddrWalk0(uint32_t Base, uint32_t numByte)
{
	volatile uint8_t * pRam = (uint8_t *) Base ;
	uint32_t uno = 1 ;

	// Segnalino
	pRam[numByte-1] = SPIA ;

	// Ciclo sugli indirizzi
	while (uno != numByte) {
		uint32_t zero = ~uno ;
		zero &= numByte - 1 ;

		pRam[zero] = ~SPIA ;

		if (SPIA != pRam[numByte-1])
			break ;

		uno <<= 1 ;
	}

	if (numByte == uno)
		uno = 0 ;

	return uno ;
}

uint32_t TRAM_DataWalk1e0_16bit(uint32_t Base)
{
	volatile uint16_t * pRam = (uint16_t *) Base ;
	uint16_t uno = 1 ;
	uint16_t zero = ~uno ;
	int i ;

	// Ciclo sui dati
	for (i=0 ; i<16 ; i++) {
		pRam[0] = uno ;
		pRam[1] = zero ;

		if (uno != pRam[0])
			break ;

		if (zero != pRam[1]) {
			uno = zero ;
			break ;
		} ;

		uno <<= 1 ;
		zero = ~uno ;
	}

	if (16 == i)
		uno = 0 ;

	return uno ;
}
